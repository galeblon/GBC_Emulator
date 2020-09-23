#include<allegro5/allegro5.h>
#include<allegro5/allegro_primitives.h>
#include"cpu.h"
#include"debug.h"
#include"display.h"
#include"logger.h"

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

	if(!al_init_primitives_addon()) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO INIT PRIMITIVES ADDON",
			"FAILED TO INITIALISE ALLEGRO PRIMITIVES ADDON."
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
	debug_assert(index <= 144, "display_draw_line: index out of bounds");

	ALLEGRO_COLOR current_colour;
	for(int i=0; i<160; i++)
	{
		current_colour = al_map_rgba(
			line[i].r,
			line[i].g,
			line[i].b,
			line[i].a ? 0 : 255
		);


		al_draw_filled_rectangle(
			i               * SCALING_FACTOR,
			index           * SCALING_FACTOR,
			(i + 1)         * SCALING_FACTOR,
			(index + 1)     * SCALING_FACTOR,
			current_colour
		);

	}

	if(index == 144)
		al_flip_display();
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
