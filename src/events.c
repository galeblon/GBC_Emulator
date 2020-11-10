#include<SDL2/SDL.h>
#include"events.h"
#include"input.h"
#include"logger.h"


static SDL_Thread * g_thread;
static SDL_mutex  * g_mutex;

static bool              g_closed      = false;
static bool              g_frame_ready = false;
static struct all_inputs g_inputs;

bool events_is_frame_ready(void)
{
	bool frame_ready = false;
	if (SDL_LockMutex(g_mutex) == 0) {
		frame_ready = g_frame_ready;
		g_frame_ready = false;
		SDL_UnlockMutex(g_mutex);
	}
	return frame_ready;
}

bool events_is_display_closed(void)
{
	bool closed = false;
	if (SDL_LockMutex(g_mutex) == 0) {
		closed = g_closed;
		SDL_UnlockMutex(g_mutex);
	}

	return closed;
}

struct all_inputs events_get_inputs(void)
{
	struct all_inputs inputs = {0};

	if (SDL_LockMutex(g_mutex) == 0) {
		inputs = g_inputs;
		SDL_UnlockMutex(g_mutex);
	}

	return inputs;
}

static void _events_error(enum logger_log_type type, char *title, const char *message)
{
	logger_log(
		type,
		title,
		"[EVENTS MODULE] %s\n",
		message
	);
}


static int _events_filter(
		__attribute__((unused)) void* userdata,
		SDL_Event* event
)
{
	switch(event->type) {
		case SDL_QUIT:
		case SDL_USEREVENT:
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		case SDL_JOYAXISMOTION:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			return 1;
			break;
		default:
			return 0;
			break;
		}
}

int events_thread(__attribute__((unused)) void *data)
{
	SDL_Event event;

	while(SDL_WaitEvent(&event)) {
		if (SDL_LockMutex(g_mutex) == 0) {
			switch(event.type) {
			case SDL_QUIT:
				g_closed = true;
				SDL_UnlockMutex(g_mutex);
				return 0;
			case SDL_USEREVENT:
				if (event.user.code == FRAME_TIMER_EVENT)
					g_frame_ready = true;
				break;
			//Nice idea - similarly to MEM, register input handlers
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			case SDL_JOYAXISMOTION:
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				input_handle_event(event, &g_inputs);
				break;
			default:
				break;
			}
			SDL_UnlockMutex(g_mutex);
		}
	}
	return 0;
}

bool events_prepare(struct input_bindings *input_bindings)
{
	SDL_SetEventFilter(_events_filter, NULL);

	g_mutex = SDL_CreateMutex();
	if (g_mutex == NULL) {
		_events_error(LOG_FATAL, "SDL Mutex", SDL_GetError());
		return false;
	}

	g_thread = SDL_CreateThread(events_thread, "events_thread", NULL);
	if (g_thread == NULL) {
		_events_error(LOG_FATAL, "SDL Thread", SDL_GetError());
		return false;
	}

	if (!input_prepare(input_bindings))
		return false;

	return true;
}

void events_destroy(void)
{
	SDL_DestroyMutex(g_mutex);
	SDL_WaitThread(g_thread, NULL);
	input_destroy();
}
