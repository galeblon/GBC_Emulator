#include<SDL2/SDL.h>
#include<allegro5/allegro5.h>
#include"cpu.h"
#include"debug.h"
#include"display.h"
#include"events.h"
#include"logger.h"


static SDL_Window   * g_window    = NULL;
static SDL_Renderer * g_renderer  = NULL;
static SDL_Texture  * g_texture   = NULL;
static uint32_t       g_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
static SDL_TimerID    g_sdl_timer;


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


// -------------- SDL SECTION --------------


static Uint32 _display_timer_callback(
		Uint32 interval,
		__attribute__((unused)) void * param )
{
	SDL_Event event;
	SDL_UserEvent user_event;

	user_event.type  = SDL_USEREVENT;
	user_event.code  = 0;
	user_event.data1 = NULL;
	user_event.data2 = NULL;

	event.type = SDL_USEREVENT;
	event.user = user_event;

	SDL_PushEvent(&event);
	return interval;
}


static void _display_sdl_prepare(float period, char * rom_title, bool fullscreen)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
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
        //SDL_WINDOW_OPENGL? What is best here?
		SDL_WINDOW_SHOWN
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

    g_sdl_timer = SDL_AddTimer(period * 1000, _display_timer_callback, NULL);
    if(g_sdl_timer == 0) {
    	_display_error(
			LOG_FATAL,
			"SDL TIMER",
			SDL_GetError()
		);
        return;
    }
}


static void _display_sdl_draw(colour screen[SCREEN_HEIGHT][SCREEN_WIDTH])
{
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
}


static void _display_sdl_destroy()
{
	if(g_sdl_timer != 0)
		SDL_RemoveTimer(g_sdl_timer);
	if(g_texture != NULL)
		SDL_DestroyTexture(g_texture);
	if(g_window != NULL)
		SDL_DestroyWindow(g_window);
	SDL_Quit();
}


// -------------- MAIN SECTION --------------


void display_prepare(float period, char * rom_title, bool fullscreen)
{
	_display_sdl_prepare(period, rom_title, fullscreen);
}

void display_draw(colour screen[SCREEN_HEIGHT][SCREEN_WIDTH])
{
	if(!events_get_frame_status()) {
		logger_print(LOG_INFO, "[DISPLAY] SDL Premature frame calculation\n");
		return;
	}
	events_reset_frame_status();

	_display_sdl_draw(screen);
}


bool display_get_closed_status(void)
{
	bool closed = events_get_closed_status();

	return closed;
}


void display_destroy(void)
{
	_display_sdl_destroy();
}
