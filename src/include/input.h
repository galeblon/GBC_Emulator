#ifndef SRC_INCLUDE_INPUT_H_
#define SRC_INCLUDE_INPUT_H_

#include<SDL2/SDL.h>
#include"types.h"

#define INPUT_BINDINGS_TO_READ 7

struct all_inputs {
	bool DOWN;
	bool UP;
	bool LEFT;
	bool RIGHT;
	bool START;
	bool SELECT;
	bool A;
	bool B;
	bool QUIT;
};

struct keyboard_bindings {
	int a_button;
	int b_button;
	int start;
	int select;
	int up;
	int down;
	int left;
	int right;
	int quit;
};

struct gamepad_bindings {
	int a_button;
	int b_button;
	int start;
	int select;
	int d_pad_up;
	int d_pad_down;
	int d_pad_left;
	int d_pad_right;
	int axis_h;
	int axis_v;
	int quit;
};


struct input_bindings {
	struct keyboard_bindings keyboard;
	struct gamepad_bindings gamepad;
	bool filled;
};

int input_prepare(struct input_bindings *input_bindings);
void input_handle_event(SDL_Event event, struct all_inputs* inputs);
void input_destroy(void);


#endif /* SRC_INCLUDE_INPUT_H_ */
