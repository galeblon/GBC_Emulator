#include<SDL2/SDL.h>
#include"input.h"
#include"logger.h"
#include"types.h"


#define SDL_MAX_AXIS_VALUE 32768
#define BUTTON_NUM         8


static SDL_GameController * g_sdl_controller = NULL;

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

static void _input_load_default_bindings() {
	struct keyboard_bindings bindings = {
			SDLK_z,
			SDLK_x,
			SDLK_RETURN,
			SDLK_SPACE,
			SDLK_UP,
			SDLK_DOWN,
			SDLK_LEFT,
			SDLK_RIGHT
	};

	g_keyboard_bindings = bindings;

	struct gamepad_bindings pad_bindings = {
			SDL_CONTROLLER_BUTTON_A,
			SDL_CONTROLLER_BUTTON_B,
			SDL_CONTROLLER_BUTTON_START,
			SDL_CONTROLLER_BUTTON_BACK,
			SDL_CONTROLLER_BUTTON_DPAD_UP,
			SDL_CONTROLLER_BUTTON_DPAD_DOWN,
			SDL_CONTROLLER_BUTTON_DPAD_LEFT,
			SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
			SDL_CONTROLLER_AXIS_LEFTX,
			SDL_CONTROLLER_AXIS_LEFTY};

	g_gamepad_bindings = pad_bindings;
}


static void _input_sdl_parse_keyboard(
	SDL_Event event,
	struct all_inputs* inputs
)
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
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
	if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0) {
		_input_error( SDL_GetError() );
		return 0;
	}

	g_sdl_controller = SDL_GameControllerOpen(0);

	return 1;
}


static void _input_sdl_parse_joystick_axis(SDL_Event event, struct all_inputs* inputs) {
	float pos_abs = (event.caxis.value < 0) ? -event.caxis.value : event.caxis.value;
	if(event.caxis.axis == g_gamepad_bindings.axis_h) {
		inputs->LEFT = false;
		inputs->RIGHT = false;

		if(pos_abs < g_epsilon * SDL_MAX_AXIS_VALUE)
			return;
		if(event.caxis.value < 0)
			inputs->LEFT = true;
		else
			inputs->RIGHT = true;
	} else if(event.caxis.axis == g_gamepad_bindings.axis_v) {
		inputs->UP = false;
		inputs->DOWN = false;

		if(pos_abs < g_epsilon * SDL_MAX_AXIS_VALUE)
			return;
		if(event.caxis.value < 0)
			inputs->UP = true;
		else
			inputs->DOWN = true;
	}
}

static void _input_sdl_parse_joystick_buttons(SDL_Event event, struct all_inputs* inputs) {
	int button_num = event.cbutton.button;

	if(button_num == g_gamepad_bindings.a_button)
		inputs->A = event.cbutton.state == SDL_PRESSED;
	else if(button_num == g_gamepad_bindings.b_button)
		inputs->B = event.cbutton.state == SDL_PRESSED;
	else if(button_num == g_gamepad_bindings.start)
		inputs->START = event.cbutton.state == SDL_PRESSED;
	else if(button_num == g_gamepad_bindings.select)
		inputs->SELECT = event.cbutton.state == SDL_PRESSED;
	else if(button_num == g_gamepad_bindings.d_pad_up)
		inputs->UP = event.cbutton.state == SDL_PRESSED;
	else if(button_num == g_gamepad_bindings.d_pad_down)
		inputs->DOWN = event.cbutton.state == SDL_PRESSED;
	else if(button_num == g_gamepad_bindings.d_pad_left)
		inputs->LEFT = event.cbutton.state == SDL_PRESSED;
	else if(button_num == g_gamepad_bindings.d_pad_right)
		inputs->RIGHT = event.cbutton.state == SDL_PRESSED;
}


static void _input_sdl_handle_event(SDL_Event event, struct all_inputs* inputs)
{
	switch(event.type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			_input_sdl_parse_keyboard(event, inputs);
			break;
		case SDL_CONTROLLERAXISMOTION:
			_input_sdl_parse_joystick_axis(event, inputs);
			break;
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			_input_sdl_parse_joystick_buttons(event, inputs);
			break;
		default:
			break;
		}
}


static void _input_sdl_destroy(void)
{
	if(g_sdl_controller != NULL)
		SDL_GameControllerClose(g_sdl_controller);
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

	_input_load_default_bindings();
	if(input_bindings == NULL) {
		logger_print(LOG_INFO, "INPUT MODULE: using default bindings.\n");
	} else {
		logger_print(LOG_INFO, "INPUT MODULE: using custom bindings.\n");
		g_keyboard_bindings = input_bindings->keyboard;
	}

	return sdl_result;
}

void input_handle_event(SDL_Event event, struct all_inputs* inputs)
{
	_input_sdl_handle_event(event, inputs);
}

void input_destroy(void)
{
	_input_sdl_destroy();
}
