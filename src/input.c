#include<allegro5/allegro5.h>
#include<stdio.h>
#include"input.h"
#include"logger.h"
#include"types.h"

#define BINDINGS_TO_READ 15

struct keyboard_bindings {
	int a_button;
	int b_button;
	int start;
	int select;
	int up;
	int down;
	int left;
	int right;
};

struct gamepad_bindings {
	int a_button;
	int b_button;
	int start;
	int select;
	int d_pad;
	int axis_h;
	int axis_v;
};

static ALLEGRO_EVENT_QUEUE *g_event_queue = NULL;
static ALLEGRO_EVENT g_event;

static struct keyboard_bindings g_keyboard_bindings;
static struct gamepad_bindings g_gamepad_bindings;
static bool g_pressed_keys[ALLEGRO_KEY_MAX];

static void _input_error(const char *feature)
{
	logger_log(LOG_FATAL,
		"INPUT MODULE",
		"%s failure.\n",
		feature);
}

void _input_load_default_bindings() {
	logger_print(LOG_INFO, "INPUT MODULE: loading default bindings.\n");

	struct keyboard_bindings bindings = {
			ALLEGRO_KEY_Z,
			ALLEGRO_KEY_X,
			ALLEGRO_KEY_S,
			ALLEGRO_KEY_A,
			ALLEGRO_KEY_UP,
			ALLEGRO_KEY_DOWN,
			ALLEGRO_KEY_LEFT,
			ALLEGRO_KEY_RIGHT
	};

	g_keyboard_bindings = bindings;

	struct gamepad_bindings pad_bindings = { 0, 1, 7, 6, 3, 0, 1};

	g_gamepad_bindings = pad_bindings;
}

int _input_load_custom_bindings(const char *input_config_path) {
	logger_print(LOG_INFO, "INPUT MODULE: loading custom bindings from %s.\n", input_config_path);


	FILE *bindings_fileptr = fopen(input_config_path, "rb");

	if(bindings_fileptr == NULL) {
		_input_error("Couldn't open bindings file");
		return 0;
	}

	char line[256];

	int binding_values[BINDINGS_TO_READ];
	int current_index = 0;
	while(fgets(line, sizeof(line), bindings_fileptr)) {
		if(line[0] == '#')
			continue;
		int value = atoi(line);
		binding_values[current_index++] = value;
		if(current_index == BINDINGS_TO_READ)
			break;
	}

	fclose(bindings_fileptr);
	if(current_index != BINDINGS_TO_READ) {
		_input_error("Config file doesn't contain all sections");
		return 0;
	}

	// Fill from sections
	struct keyboard_bindings bindings = {
			binding_values[0],
			binding_values[1],
			binding_values[2],
			binding_values[3],
			binding_values[4],
			binding_values[5],
			binding_values[6],
			binding_values[7]
	};

	g_keyboard_bindings = bindings;

	struct gamepad_bindings pad_bindings = {
			binding_values[8],
			binding_values[9],
			binding_values[10],
			binding_values[11],
			binding_values[12],
			binding_values[13],
			binding_values[14]
	};

	g_gamepad_bindings = pad_bindings;
	return 1;
}

/**
 * Initialize input module:
 *	- Install both keyboard and joypad support
 *	- Load configuration
 *
 *	@param  input_config_path    path to file containing input bindings data to load.
 *								 If NULL, default bindings are used.
 *
 *	@return	1 if everything succeeded, 0 if an error occurred during module
 *          initialization
 */
int input_prepare(const char *input_config_path)
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

	if(input_config_path == NULL) {
		_input_load_default_bindings();
		return 1;
	} else {
		return _input_load_custom_bindings(input_config_path);
	}
}

void _input_parse_keyboard(bool is_down)
{
	g_pressed_keys[g_event.keyboard.keycode] = is_down;
}

void _input_parse_gamepad_buttons(ALLEGRO_EVENT event, struct all_inputs* inputs, bool is_down) {
	int button_num = event.joystick.button;
	if(button_num == g_gamepad_bindings.a_button)
		inputs->A = is_down;
	else if(button_num == g_gamepad_bindings.b_button)
		inputs->B = is_down;
	else if(button_num == g_gamepad_bindings.start)
		inputs->START = is_down;
	else if(button_num == g_gamepad_bindings.select)
		inputs->SELECT = is_down;
}

void _input_parse_gamepad_joysticks(ALLEGRO_EVENT event, struct all_inputs* inputs) {
	if(event.joystick.stick == g_gamepad_bindings.d_pad) {
		if(event.joystick.axis == g_gamepad_bindings.axis_h) {
			inputs->LEFT = false;
			inputs->RIGHT = false;

			if(abs(event.joystick.pos) < 0.1)
				return;
			if(event.joystick.pos < 0)
				inputs->LEFT = true;
			else
				inputs->RIGHT = true;
		} else {
			inputs->UP = false;
			inputs->DOWN = false;

			if(abs(event.joystick.pos) < 0.1)
				return;
			if(event.joystick.pos < 0)
				inputs->UP = true;
			else
				inputs->DOWN = true;
		}
	}
}

void _input_fill_inputs_from_keyboard(struct all_inputs* inputs) {
	inputs->A = g_pressed_keys[g_keyboard_bindings.a_button];
	inputs->B = g_pressed_keys[g_keyboard_bindings.b_button];
	inputs->START = g_pressed_keys[g_keyboard_bindings.start];
	inputs->SELECT = g_pressed_keys[g_keyboard_bindings.select];
	inputs->UP = g_pressed_keys[g_keyboard_bindings.up];
	inputs->DOWN = g_pressed_keys[g_keyboard_bindings.down];
	inputs->LEFT = g_pressed_keys[g_keyboard_bindings.left];
	inputs->RIGHT = g_pressed_keys[g_keyboard_bindings.right];
}

void input_check_queue(struct all_inputs* inputs)
{
	bool event_exists = al_get_next_event(g_event_queue, &g_event);
	bool is_keyboard = false;

	if(event_exists) {
		switch (g_event.type) {
		case ALLEGRO_EVENT_KEY_DOWN:
			_input_parse_keyboard(true);
			is_keyboard = true;
			break;
		case ALLEGRO_EVENT_KEY_UP:
			_input_parse_keyboard(false);
			is_keyboard = true;
			break;
		case ALLEGRO_EVENT_JOYSTICK_AXIS:
			_input_parse_gamepad_joysticks(g_event, inputs);
			break;
		case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
			_input_parse_gamepad_buttons(g_event, inputs, true);
			break;
		case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP:
			_input_parse_gamepad_buttons(g_event, inputs, false);
			break;
		//TODO parse emulation specific input, load states etc.
		default:
			break;
		}
	}

	if(is_keyboard)
		_input_fill_inputs_from_keyboard(inputs);
}
