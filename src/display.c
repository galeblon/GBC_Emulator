#include<SDL2/SDL.h>
#include<allegro5/allegro5.h>
#include"cpu.h"
#include"debug.h"
#include"display.h"
#include"logger.h"


static SDL_Window   * g_window   = NULL;
static SDL_Renderer * g_renderer = NULL;
static SDL_Texture  * g_texture  = NULL;
static uint32_t       g_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

// --------------------------------------------

static ALLEGRO_BITMAP      *g_bitmap            = NULL;
static ALLEGRO_DISPLAY     *g_display           = NULL;
static ALLEGRO_TIMER       *g_timer             = NULL;
static ALLEGRO_EVENT_QUEUE *g_close_event_queue = NULL;
static ALLEGRO_EVENT_QUEUE *g_event_queue       = NULL;
static ALLEGRO_EVENT       g_event;

static float g_scale = SCALING_FACTOR;

static void _display_error(enum logger_log_type type, char *title, const char *message)
{
	logger_log(
		type,
		title,
		"[DISPLAY MODULE] %s\n",
		message
	);
}

void display_prepare(float frequency, char * rom_title, bool fullscreen)
{

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        _display_error(
			LOG_FATAL,
			"SDL INIT",
			SDL_GetError()
		);
        return;
    }

    g_window = SDL_CreateWindow(
        rom_title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH  * g_scale,
        SCREEN_HEIGHT * g_scale,
        SDL_WINDOW_OPENGL
    );
    if (g_window == NULL) {
    	_display_error(
			LOG_FATAL,
			"SDL WINDOW",
			SDL_GetError()
		);
        return;
    }

    g_renderer = SDL_CreateRenderer(
    	g_window,
		-1,
		SDL_RENDERER_SOFTWARE
	);
    if (g_renderer == NULL) {
    	_display_error(
			LOG_FATAL,
			"SDL RENDERER",
			SDL_GetError()
		);
        return;
    } else {
    	if (SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE) != 0) {
			_display_error(
				LOG_FATAL,
				"SDL SET RENDER COLOR",
				SDL_GetError()
			);
			return;
		}
    }

    g_texture = SDL_CreateTexture(
    	g_renderer,
		SDL_PIXELFORMAT_ABGR8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH,
		SCREEN_HEIGHT
    );
    if (g_texture == NULL) {
    	_display_error(
			LOG_FATAL,
			"SDL TEXTURE",
			SDL_GetError()
		);
        return;
    }


// --------------------------------------------



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
    	return;
	}

	al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ABGR_8888);
	g_bitmap = al_create_bitmap(SCREEN_WIDTH, SCREEN_HEIGHT);
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

void display_draw(colour screen[SCREEN_HEIGHT][SCREEN_WIDTH])
{
	ALLEGRO_EVENT event;

	bool event_present = al_get_next_event(g_event_queue, &event);

	// Drop frames which would not be seen due to refresh rate
	if(!event_present || event.type != ALLEGRO_EVENT_TIMER) {
		logger_print(LOG_INFO, "[Display] Frame dropped\n");
		return;
	}


// --------------------------------------------

	if (SDL_RenderClear(g_renderer) != 0) {
    	_display_error(
			LOG_FATAL,
			"SDL CLEAR",
			SDL_GetError()
		);
        return;
    }



	u8 *ptr = (u8*) g_buffer;
	for (int y = 0; y < SCREEN_HEIGHT; y++) {

		for(int x = 0; x < SCREEN_WIDTH; x++) {
			*ptr++ = screen[y][x].r;
			*ptr++ = screen[y][x].g;
			*ptr++ = screen[y][x].b;
			*ptr++ = (screen[y][x].a) ? SDL_ALPHA_OPAQUE : SDL_ALPHA_TRANSPARENT;
		}

	}

	SDL_UpdateTexture(
		g_texture,
		NULL,
		&g_buffer,
		SCREEN_WIDTH * 4
	);

	SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
	SDL_RenderPresent(g_renderer);




// --------------------------------------------




	al_set_target_bitmap(g_bitmap);
	ALLEGRO_LOCKED_REGION *lr = al_lock_bitmap(
		g_bitmap,
		ALLEGRO_PIXEL_FORMAT_ANY,
		ALLEGRO_LOCK_WRITEONLY
	);

	ptr = (u8*) lr->data;
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		colour *line = screen[y];

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
	if(g_texture != NULL)
		SDL_DestroyTexture(g_texture);
	if(g_window != NULL)
		SDL_DestroyWindow(g_window);
	SDL_Quit();

// --------------------------------------------

	al_destroy_display(g_display);
	al_destroy_event_queue(g_event_queue);
	al_destroy_event_queue(g_close_event_queue);
}
