#include<SDL2/SDL.h>
#include"events.h"
#include"input.h"
#include"logger.h"
#include"types.h"


#define SDL_MAX_AXIS_VALUE 32768
#define BUTTON_NUM         8


static SDL_Joystick * g_sdl_joystick = NULL;

static struct keyboard_bindings g_keyboard_bindings;
static struct gamepad_bindings  g_gamepad_bindings;

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
			SDLK_z,
			SDLK_x,
			SDLK_s,
			SDLK_a,
			SDLK_UP,
			SDLK_DOWN,
			SDLK_LEFT,
			SDLK_RIGHT
	};

	g_keyboard_bindings = bindings;

	struct gamepad_bindings pad_bindings = { 0, 1, 7, 6, 3, 0, 1};

	g_gamepad_bindings = pad_bindings;
}


static void _input_sdl_parse_keyboard(SDL_Event event, struct all_inputs* inputs)
{
	int key_code = (int)SDL_GetKeyFromScancode(event.key.keysym.scancode);
	bool is_down = event.key.state == SDL_PRESSED;

	if(key_code == g_keyboard_bindings.a_button)
		inputs->A = is_down;
	else if(key_code == g_keyboard_bindings.b_button)
		inputs->B = is_down;
	else if(key_code == g_keyboard_bindings.start)
		inputs->START = is_down;
	else if(key_code == g_keyboard_bindings.select)
		inputs->SELECT = is_down;
	else if(key_code == g_keyboard_bindings.up)
		inputs->UP = is_down;
	else if(key_code == g_keyboard_bindings.down)
		inputs->DOWN = is_down;
	else if(key_code == g_keyboard_bindings.left)
		inputs->LEFT = is_down;
	else if(key_code == g_keyboard_bindings.right)
		inputs->RIGHT = is_down;
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
	switch(event.type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			_input_sdl_parse_keyboard(event, inputs);
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
	events_prepare();

	int sdl_result     = _input_sdl_prepare();

	if(input_bindings == NULL) {
		logger_print(LOG_INFO, "INPUT MODULE: using default bindings.\n");
		_input_load_default_bindings();
	} else {
		logger_print(LOG_INFO, "INPUT MODULE: using custom bindings.\n");
		g_keyboard_bindings = input_bindings->keyboard;
		g_gamepad_bindings  = input_bindings->gamepad;
	}

	return sdl_result;
}


void input_check_queue(struct all_inputs* inputs)
{
	events_step(_input_sdl_handle_event, inputs);
}

void input_destroy(void)
{
	_input_sdl_destroy();
}
