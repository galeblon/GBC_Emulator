#include<allegro5/allegro5.h>
#include<stdio.h>
#include"input.h"
#include"logger.h"
#include"types.h"

static ALLEGRO_EVENT_QUEUE *g_event_queue = NULL;
static ALLEGRO_EVENT g_event;

static void _input_error(const char *feature)
{
	char *message = logger_get_msg_buffer();
	snprintf(message,
		LOG_MESSAGE_MAX_SIZE,
		"%s failure.\n",
		feature);
	logger_log(LOG_FATAL,
		"INPUT MODULE",
		message);
}

void input_prepare(void)
{
	if (!al_install_keyboard()) {
		_input_error("Keyboard initialization failure");
	}
	if (!al_install_joystick()) {
		_input_error("Joystick initialization failure");
	}

	g_event_queue = al_create_event_queue();
	if (!g_event_queue) {
		_input_error("Event queue initialization failure");
	}

	al_register_event_source(g_event_queue, al_get_keyboard_event_source());
	al_register_event_source(g_event_queue, al_get_joystick_event_source());
}

void _input_parse_keyboard(struct all_inputs* inputs, bool is_down)
{
	switch (g_event.keyboard.keycode) {
		case ALLEGRO_KEY_DOWN:
			inputs->DOWN = is_down;
			break;
		case ALLEGRO_KEY_UP:
			inputs->UP = is_down;
			break;
		case ALLEGRO_KEY_LEFT:
			inputs->LEFT = is_down;
			break;
		case ALLEGRO_KEY_RIGHT:
			inputs->RIGHT = is_down;
			break;
		case ALLEGRO_KEY_A:
			inputs->SELECT = is_down;
			break;
		case ALLEGRO_KEY_S:
			inputs->START = is_down;
			break;
		case ALLEGRO_KEY_Z:
			inputs->A = is_down;
			break;
		case ALLEGRO_KEY_X:
			inputs->B = is_down;
			break;
		default:
			break;
	}
}

void input_check_queue(struct all_inputs* inputs)
{
	bool event_exists = al_get_next_event(g_event_queue, &g_event);

	if(event_exists) {
		switch (g_event.type) {
		case ALLEGRO_EVENT_KEY_DOWN:
			_input_parse_keyboard(inputs, true);
			break;
		case ALLEGRO_EVENT_KEY_UP:
			_input_parse_keyboard(inputs, false);
			break;
		//TODO parse the joystick
		//TODO parse emulation specific input, load states etc.
		default:
			break;
		}
	}
}
