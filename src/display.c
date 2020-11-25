#include<SDL2/SDL.h>
#include<time.h>
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


#if defined(__x86_64__) && defined(JITTER)
#define SEC (1000000000)
#define NSEC_PER_CLOCK  (SEC / CPU_CLOCK_SPEED)

static bool				t_e_started;
static float			t_e_framerate;
static struct timespec 	t_e_start;
static struct timespec 	t_e_end;
static long 			t_e_jitter_total = 0;
static long 			t_e_frame_total  = 0;
static long 			t_e_clocks_total  = 0;
static double			t_e_max_jitter;
static long				t_e_frame_expected = 0;
static long				t_e_clocks_omitted = 0;

static inline long timespec_diff(struct timespec *t_end, struct timespec *t_start)
{
	return (t_end->tv_sec * SEC + t_end->tv_nsec) - (t_start->tv_sec * SEC + t_start->tv_nsec);
}

void display_frame_increase()
{
	t_e_frame_expected++;
}

void display_clocks_decrease(u16 clocks)
{
	t_e_clocks_omitted += (long)clocks;
}
#endif


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
	user_event.code  = FRAME_TIMER_EVENT;
	user_event.data1 = NULL;
	user_event.data2 = NULL;

	event.type = SDL_USEREVENT;
	event.user = user_event;

	SDL_PushEvent(&event);
	return interval;
}


static void _display_sdl_prepare(float period, char * rom_title, bool fullscreen)
{
#if defined(__x86_64__) && defined(JITTER)
	t_e_framerate = period;
#endif
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
		fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_SHOWN
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
		SDL_RENDERER_ACCELERATED
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

	SDL_RenderSetLogicalSize(g_renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

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

	if (fullscreen)
		SDL_ShowCursor(SDL_DISABLE);
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
	if(!events_is_frame_ready()) {
#ifdef DEBUG
		logger_print(LOG_INFO, "[DISPLAY] SDL Premature frame calculation\n");
#endif
		return;
	}

#if defined(__x86_64__) && defined(JITTER)
	if(!t_e_started) {
		t_e_started = true;
	} else {
		clock_gettime(CLOCK_MONOTONIC, &t_e_end);

		double this_jitter =
				timespec_diff(&t_e_end, &t_e_start)
				- t_e_framerate * SEC
				- t_e_clocks_omitted * NSEC_PER_CLOCK;
		this_jitter = this_jitter < 0 ? -this_jitter : this_jitter;
		t_e_jitter_total += this_jitter;
		t_e_frame_total  += 1;
		t_e_clocks_total += t_e_clocks_omitted;
		t_e_max_jitter = t_e_max_jitter < this_jitter ? this_jitter : t_e_max_jitter;
	}
	clock_gettime(CLOCK_MONOTONIC, &t_e_start);
	t_e_frame_expected = 0;
	t_e_clocks_omitted = 0;
#endif

	_display_sdl_draw(screen);
}


bool display_get_closed_status(void)
{
	bool closed = events_is_display_closed();

	return closed;
}


void display_destroy(void)
{
#if defined(__x86_64__) && defined(JITTER)
	printf("JITTER_AVG: %lf\n", (double)t_e_jitter_total / (double)t_e_frame_total );
	printf("JITTER_MAX: %lf\n", t_e_max_jitter);
#endif
	_display_sdl_destroy();
}
