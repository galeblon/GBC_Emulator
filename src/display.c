#include<allegro5/allegro5.h>
#include<pthread.h>
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

static pthread_t           g_thread;
static pthread_mutex_t     g_display_lock;
static pthread_cond_t      g_draw_cond;
static bool                g_display_kill = false;

static float g_scale = SCALING_FACTOR;
static colour g_display_buff[SCREEN_HEIGHT][SCREEN_WIDTH];

static void _display_error(enum logger_log_type type, char *title, char *message)
{
	logger_log(
		type,
		title,
		"[DISPLAY MODULE] %s\n",
		message
	);
}

void display_submit(colour screen[SCREEN_HEIGHT][SCREEN_WIDTH])
{
	logger_print(LOG_WARN, "display_submit: trying to lock\n");
	pthread_mutex_lock(&g_display_lock);
	memcpy(g_display_buff, screen, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(colour));
	logger_print(LOG_WARN, "display_submit: signalling\n");
	pthread_cond_broadcast(&g_draw_cond);
	logger_print(LOG_WARN, "display_submit: unlocking\n");
	pthread_mutex_unlock(&g_display_lock);
}

static void *_display_draw(void *arg __attribute__((unused)))
{
	while (!g_display_kill) {

		// Use pthread_cond_wait as inter-thread synchronisation
		pthread_mutex_lock(&g_display_lock);
		logger_print(LOG_WARN, "display_draw: waiting for signal\n");
		pthread_cond_wait(&g_draw_cond, &g_display_lock);
		logger_print(LOG_WARN, "display_draw: signal rcvd\n");
		pthread_mutex_unlock(&g_display_lock);

		ALLEGRO_EVENT event;

		bool event_present = al_get_next_event(g_event_queue, &event);

		// Drop frames which would not be seen due to refresh rate
		if(!event_present || event.type != ALLEGRO_EVENT_TIMER) {
			logger_print(LOG_WARN, "[Display] Frame dropped\n");
			continue;
		}

		al_set_target_bitmap(g_bitmap);
		ALLEGRO_LOCKED_REGION *lr = al_lock_bitmap(
			g_bitmap,
			ALLEGRO_PIXEL_FORMAT_ANY,
			ALLEGRO_LOCK_WRITEONLY
		);

		u8 *ptr = (u8*) lr->data;

		logger_print(LOG_WARN, "display_draw: trying to lock to draw\n");
		pthread_mutex_lock(&g_display_lock);
		logger_print(LOG_WARN, "display_draw: locked to draw\n");
		for (int y = 0; y < SCREEN_HEIGHT; y++) {
			colour *line = g_display_buff[y];

			for(int x = 0; x < SCREEN_WIDTH; x++) {
				*ptr++ = line[x].r;
				*ptr++ = line[x].g;
				*ptr++ = line[x].b;
				*ptr++ = (line[x].a) ? 0 : 255;
			}

			// Bitmap is flipped, so after copying colors to a row n ptr points to
			// first element of row n-1. We need to move to row n+1, so the row
			// size is subtracted twice (lr->pitch is negative in upside-down
			// bitmap).
			ptr += lr->pitch * 2;
		}
		logger_print(LOG_WARN, "display_draw: unlocking after drawing\n");
		pthread_mutex_unlock(&g_display_lock);

		al_unlock_bitmap(g_bitmap);
		al_set_target_backbuffer(g_display);
		al_draw_scaled_bitmap(
				g_bitmap,
				0,
				0,
				SCREEN_WIDTH,
				SCREEN_HEIGHT,
				0,
				0,
				SCREEN_WIDTH * g_scale,
				SCREEN_HEIGHT * g_scale,
				0);
		al_flip_display();
	}
	return NULL;
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

bool display_prepare(float frequency, char * rom_title, bool fullscreen)
{
	if(!al_init()) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO INIT",
			"FAILED TO INITIALISE ALLEGRO."
		);
		return false;
	}

	g_timer = al_create_timer(frequency);
	if (!g_timer) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO TIMER",
			"FAILED TO CREATE A TIMER."
		);
		return false;
	}

	if (fullscreen) {
		al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
		ALLEGRO_MONITOR_INFO monitor;

		if(al_get_monitor_info(0, &monitor)) {
			int w = monitor.x2 - monitor.x1;
			int h = monitor.y2 - monitor.y1;
			g_scale = MIN((float)w / SCREEN_WIDTH, (float)h / SCREEN_HEIGHT);
		}
	}

	g_display = al_create_display(SCREEN_WIDTH * g_scale, SCREEN_HEIGHT * g_scale);
	if(!g_display) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO DISPLAY",
			"FAILED TO INITIALISE DISPLAY."
		);
		return false;
	}

	al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ABGR_8888);
	g_bitmap = al_create_bitmap(SCREEN_WIDTH, SCREEN_HEIGHT);
	if(!g_bitmap) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO DISPLAY",
			"FAILED TO INITIALISE BITMAP."
		);
		return false;
	}

	g_event_queue = al_create_event_queue();
	if (!g_event_queue) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO QUEUE",
			"FAILED TO CREATE EVENT QUEUE."
		);
		return false;
	}

	g_close_event_queue = al_create_event_queue();
	if (!g_close_event_queue) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO QUEUE",
			"FAILED TO CREATE EVENT QUEUE."
		);
		return false;
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

	pthread_mutex_init(&g_display_lock, NULL);
	pthread_cond_init(&g_draw_cond, NULL);
	int sc = pthread_create(&g_thread, NULL, _display_draw, NULL);
	if (sc != 0) {
		_display_error(
			LOG_FATAL,
			"DISPLAY THREAD",
			"FAILED TO CREATE DISPLAY THREAD."  // TODO: log status code
		);
		return false;
	}

	return true;
}

void display_destroy(void)
{
	void *unused;
	pthread_mutex_lock(&g_display_lock);
	g_display_kill = true;
	pthread_cond_broadcast(&g_draw_cond);
	pthread_mutex_unlock(&g_display_lock);

	pthread_join(g_thread, &unused);
	pthread_cond_destroy(&g_draw_cond);
	pthread_mutex_destroy(&g_display_lock);

	al_destroy_display(g_display);
	al_destroy_event_queue(g_event_queue);
	al_destroy_event_queue(g_close_event_queue);
}
