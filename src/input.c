#include<SDL2/SDL.h>
#include<allegro5/allegro5.h>
#include"events.h"
#include"input.h"
#include"logger.h"
#include"types.h"


#define SDL_MAX_AXIS_VALUE 32768


static SDL_Joystick * g_sdl_joystick = NULL;

static ALLEGRO_EVENT_QUEUE *g_event_queue = NULL;
static ALLEGRO_EVENT g_event;

static struct keyboard_bindings g_keyboard_bindings;
static struct gamepad_bindings  g_gamepad_bindings;
static bool                     g_pressed_keys[ALLEGRO_KEY_MAX];

static float                    g_epsilon = 0.1;


static void _input_error(const char *feature)
{
	logger_log(
		LOG_FATAL,
		"INPUT MODULE",
		"%s failure.\n",
		feature);
}

void _input_load_default_bindings() {
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

void _input_fill_inputs_from_keyboard(struct all_inputs* inputs) {
	inputs->A      = g_pressed_keys[g_keyboard_bindings.a_button];
	inputs->B      = g_pressed_keys[g_keyboard_bindings.b_button];
	inputs->START  = g_pressed_keys[g_keyboard_bindings.start];
	inputs->SELECT = g_pressed_keys[g_keyboard_bindings.select];
	inputs->UP     = g_pressed_keys[g_keyboard_bindings.up];
	inputs->DOWN   = g_pressed_keys[g_keyboard_bindings.down];
	inputs->LEFT   = g_pressed_keys[g_keyboard_bindings.left];
	inputs->RIGHT  = g_pressed_keys[g_keyboard_bindings.right];
}


static void _input_parse_keyboard(int key_code, bool is_down)
{
	g_pressed_keys[key_code] = is_down;
}


// -------------- ALLEGRO SECTION --------------


static int _input_allegro_prepare(void)
{
	if (!al_install_keyboard()) {
		_input_error("Keyboard initialization failure");
		return 0;
	}
	if (!al_install_joystick()) {
		_input_error("Joystick initialization failure");
		return 0;
	}

	g_event_queue = al_create_event_queue();
	if (!g_event_queue) {
		_input_error("Event queue initialization failure");
		return 0;
	}

	al_register_event_source(g_event_queue, al_get_keyboard_event_source());
	al_register_event_source(g_event_queue, al_get_joystick_event_source());

	return 1;
}


static void _input_parse_gamepad_buttons(ALLEGRO_EVENT event, struct all_inputs* inputs, bool is_down) {
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


static void _input_parse_gamepad_joysticks(ALLEGRO_EVENT event, struct all_inputs* inputs) {
	if(event.joystick.stick == g_gamepad_bindings.d_pad) {
		float pos_abs = event.joystick.pos < 0 ? -event.joystick.pos : event.joystick.pos;
		if(event.joystick.axis == g_gamepad_bindings.axis_h) {
			inputs->LEFT = false;
			inputs->RIGHT = false;

			if(pos_abs < g_epsilon)
				return;
			if(event.joystick.pos < 0)
				inputs->LEFT = true;
			else
				inputs->RIGHT = true;
		} else {
			inputs->UP = false;
			inputs->DOWN = false;

			if(pos_abs < g_epsilon)
				return;
			if(event.joystick.pos < 0)
				inputs->UP = true;
			else
				inputs->DOWN = true;
		}
	}
}


static void _input_allegro_handle_event(struct all_inputs* inputs)
{
	bool event_exists = al_get_next_event(g_event_queue, &g_event);
	bool is_keyboard = false;

	if(event_exists) {
		switch (g_event.type) {
		case ALLEGRO_EVENT_KEY_DOWN:
			_input_parse_keyboard(g_event.keyboard.keycode, true);
			is_keyboard = true;
			break;
		case ALLEGRO_EVENT_KEY_UP:
			_input_parse_keyboard(g_event.keyboard.keycode, false);
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


// -------------- SDL SECTION --------------


static int _input_sdl_prepare(void)
{

	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0) {
        _input_error( SDL_GetError() );
        return 0;
    }

	if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0) {
        _input_error( SDL_GetError() );
        return 0;
    }

	// SDL_NumJoysticks() allows to check how many are connected
	// This will return NULL if there are no joysticks
	// Could be safe to assume that no one would plug in
	// a controller in the middle of emulation, right?
	// Right?
	g_sdl_joystick = SDL_JoystickOpen(0);

	return 1;
}


static void _input_sdl_parse_joystick_axis(SDL_Event event, struct all_inputs* inputs) {
	// .which tells u the ID of the controller
	if(event.jaxis.which == g_gamepad_bindings.d_pad) {
		float pos_abs = (event.jaxis.value < 0) ? -event.jaxis.value : event.jaxis.value;
		// 0 is horizontal by default
		if(event.jaxis.axis == g_gamepad_bindings.axis_h) {
			inputs->LEFT = false;
			inputs->RIGHT = false;

			if(pos_abs < g_epsilon * SDL_MAX_AXIS_VALUE)
				return;
			if(event.jaxis.value < 0)
				inputs->LEFT = true;
			else
				inputs->RIGHT = true;
		} else {
			inputs->UP = false;
			inputs->DOWN = false;

			if(pos_abs < g_epsilon * SDL_MAX_AXIS_VALUE)
				return;
			if(event.jaxis.value < 0)
				inputs->UP = true;
			else
				inputs->DOWN = true;
		}
	}
}


static void _input_sdl_parse_joystick_buttons(SDL_Event event, struct all_inputs* inputs) {
	// .which tells u the ID of the controller
	if(event.jbutton.which == g_gamepad_bindings.d_pad) {
		int button_num = event.jbutton.button;

		if(button_num == g_gamepad_bindings.a_button)
			inputs->A = event.jbutton.state == SDL_PRESSED;
		else if(button_num == g_gamepad_bindings.b_button)
			inputs->B = event.jbutton.state == SDL_PRESSED;
		else if(button_num == g_gamepad_bindings.start)
			inputs->START = event.jbutton.state == SDL_PRESSED;
		else if(button_num == g_gamepad_bindings.select)
			inputs->SELECT = event.jbutton.state == SDL_PRESSED;
	}
}


static void _input_sdl_handle_event(SDL_Event event, struct all_inputs* inputs)
{
	bool is_keyboard = false;


	switch(event.type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			_input_parse_keyboard(
				SDL_GetKeyFromScancode(event.key.keysym.scancode),
				event.key.state == SDL_PRESSED
			);
			is_keyboard = true;
			break;
		case SDL_JOYAXISMOTION:
			_input_sdl_parse_joystick_axis(event, inputs);
			break;
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			_input_sdl_parse_joystick_buttons(event, inputs);
			break;
		default:
			break;
		}


	if(is_keyboard)
		_input_fill_inputs_from_keyboard(inputs);
}


static void _input_sdl_destroy(void)
{
	if(g_sdl_joystick != NULL)
		SDL_JoystickClose(g_sdl_joystick);
	if(SDL_WasInit(SDL_INIT_JOYSTICK))
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
	if(SDL_WasInit(SDL_INIT_GAMECONTROLLER))
		SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}


// -------------- MAIN SECTION --------------


/**
 * Initialize input module:
 *	- Install both keyboard and joypad support
 *	- Load configuration
 *
 *	@param  input_bindngs    pointer to input binding structure..
 *								 If NULL, default bindings are used.
 *
 *	@return	1 if everything succeeded, 0 if an error occurred during module
 *          initialization
 */
int input_prepare(struct input_bindings *input_bindings)
{
	int sdl_result     = _input_sdl_prepare();

	int allegro_result = _input_allegro_prepare();

	if(input_bindings == NULL) {
		logger_print(LOG_INFO, "INPUT MODULE: using default bindings.\n");
		_input_load_default_bindings();
	} else {
		logger_print(LOG_INFO, "INPUT MODULE: using custom bindings.\n");
		g_keyboard_bindings = input_bindings->keyboard;
		g_gamepad_bindings  = input_bindings->gamepad;
	}

	return sdl_result & allegro_result;
}


void input_check_queue(struct all_inputs* inputs)
{
	_input_allegro_handle_event(inputs);

	events_step(_input_sdl_handle_event, inputs);
}

void input_destroy(void)
{
	_input_sdl_destroy();
}
