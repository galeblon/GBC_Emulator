#include<allegro5/allegro5.h>
#include"cpu.h"
#include"debug.h"
#include"display.h"
#include"logger.h"

static ALLEGRO_BITMAP      *g_bitmap            = NULL;
static ALLEGRO_DISPLAY     *g_display           = NULL;
static ALLEGRO_TIMER       *g_timer             = NULL;
static ALLEGRO_EVENT_QUEUE *g_close_event_queue = NULL;
static ALLEGRO_EVENT_QUEUE *g_event_queue       = NULL;
static ALLEGRO_EVENT       g_event;


static void _display_error(enum logger_log_type type, char *title, char *message)
{
	char *full_message = logger_get_msg_buffer();
	snprintf(
		full_message,
		LOG_MESSAGE_MAX_SIZE,
		"[DISPLAY MODULE] %s\n",
		message
	);
	logger_log(
		type,
		title,
		full_message
	);
}

void display_prepare(float frequency, char * rom_title)
{
	if(!al_init()) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO INIT",
			"FAILED TO INITIALISE ALLEGRO."
		);
		return;
	}

	g_timer = al_create_timer(frequency);
	if (!g_timer) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO TIMER",
			"FAILED TO CREATE A TIMER."
		);
		return;
	}

	g_display = al_create_display( 160 * SCALING_FACTOR, 144 * SCALING_FACTOR );
	if(!g_display) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO DISPLAY",
			"FAILED TO INITIALISE DISPLAY."
		);
    	return;
	}

	al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ABGR_8888);
	g_bitmap = al_create_bitmap( 160 * SCALING_FACTOR, 144 * SCALING_FACTOR );
	if(!g_bitmap) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO DISPLAY",
			"FAILED TO INITIALISE BITMAP."
		);
    	return;
	}

	g_event_queue = al_create_event_queue();
	if (!g_event_queue) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO QUEUE",
			"FAILED TO CREATE EVENT QUEUE."
		);
		return;
	}

	g_close_event_queue = al_create_event_queue();
	if (!g_close_event_queue) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO QUEUE",
			"FAILED TO CREATE EVENT QUEUE."
		);
		return;
	}

	al_register_event_source(
		g_close_event_queue,
		al_get_display_event_source(g_display)
	);
	al_register_event_source(
		g_event_queue,
		al_get_timer_event_source(g_timer)
	);

	al_start_timer(g_timer);

	// Display a black screen, set the title
	al_set_window_title(g_display, rom_title);
	al_clear_to_color( al_map_rgb(0, 0, 0) );
	al_flip_display();
}


void display_draw_line(colour line[160], int index)
{
	debug_assert(index < 144, "display_draw_line: index out of bounds");

	al_set_target_bitmap(g_bitmap);
	ALLEGRO_LOCKED_REGION *lr = al_lock_bitmap_region(
		g_bitmap,
		0,
		index*SCALING_FACTOR,
		160*SCALING_FACTOR,
		SCALING_FACTOR,
		ALLEGRO_PIXEL_FORMAT_ANY,
		ALLEGRO_LOCK_WRITEONLY
	);
	u8 * ptr;

	for(int y=0; y<SCALING_FACTOR; y++)
	{
		ptr = (u8*) lr->data + y * lr->pitch;
		for(int i=0; i<160; i++)
		{
			for(int j=0; j<SCALING_FACTOR; j++)
			{
				*ptr++ = line[i].r;
				*ptr++ = line[i].g;
				*ptr++ = line[i].b;
				*ptr++ = (line[i].a) ? 0 : 255;
			}
		}
	}
	al_unlock_bitmap(g_bitmap);

	if(index == 143) {
		al_set_target_backbuffer(g_display);
		al_draw_bitmap(g_bitmap, 0, 0, 0);
		al_flip_display();
	}
}


bool display_get_closed_status(void)
{
	// Fetch the event (if one exists)
	bool event_exists = al_get_next_event(g_close_event_queue, &g_event);

	if(event_exists)
		// Handle the event
		switch (g_event.type) {
			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				return true;
			default:
				return false;
		}
	else
		return false;
}


void display_destroy(void)
{
	al_destroy_display(g_display);
	al_destroy_event_queue(g_event_queue);
	al_destroy_event_queue(g_close_event_queue);
}
