#include<SDL2/SDL.h>
#include"events.h"
#include"logger.h"


#define EVENT_NUM_PER_STEP 25


SDL_Event g_event;
SDL_Event g_got_events[EVENT_NUM_PER_STEP];

bool g_closed      = false;
bool g_frame_ready = false;


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


void events_prepare()
{
	SDL_SetEventFilter(_events_filter, NULL);
}


bool events_get_closed_status(void)
{
	return g_closed;
}


bool events_get_frame_status(void)
{
	return g_frame_ready;
}


void events_reset_frame_status(void)
{
	g_frame_ready = false;
}


static void _events_peep_method(
	void (*input_function)(SDL_Event, struct all_inputs*),
	struct all_inputs* inputs
)
{
	SDL_PumpEvents();
	int event_count = SDL_PeepEvents(
		g_got_events,
		EVENT_NUM_PER_STEP,
		SDL_GETEVENT,
		SDL_FIRSTEVENT,
		SDL_LASTEVENT
	);
	if(event_count == -1) {
		_events_error(
			LOG_FATAL,
			"SDL PEEP",
			SDL_GetError()
		);
		return;
	}

	for(int i=0; i<event_count; i++) {
		switch(g_got_events[i].type) {
		case SDL_QUIT:
			g_closed = true;
			break;
		case SDL_USEREVENT:
			g_frame_ready = true;
			break;
		//Nice idea - similarly to MEM, register input handlers
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		case SDL_JOYAXISMOTION:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			input_function(g_got_events[i], inputs);
			break;
		default:
			break;
		}
	}
}


static void _events_poll_method(
	void (*input_function)(SDL_Event, struct all_inputs*),
	struct all_inputs* inputs
)
{
	while(SDL_PollEvent(&g_event))
		switch(g_event.type) {
		case SDL_QUIT:
			g_closed = true;
			break;
		case SDL_USEREVENT:
			g_frame_ready = true;
			break;
		//Nice idea - similarly to MEM, register input handlers
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		case SDL_JOYAXISMOTION:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			input_function(g_event, inputs);
			break;
		default:
			break;
		}
}


void events_step(
	void (*input_function)(SDL_Event, struct all_inputs*),
	struct all_inputs* inputs
)
{
	//Handle all of the events
	_events_poll_method(input_function, inputs);
}
