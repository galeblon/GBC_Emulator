#ifndef SRC_INCLUDE_INPUT_H_
#define SRC_INCLUDE_INPUT_H_

#include"types.h"

#define INPUT_BINDINGS_TO_READ 15


struct all_inputs {
	bool DOWN;
	bool UP;
	bool LEFT;
	bool RIGHT;
	bool START;
	bool SELECT;
	bool A;
	bool B;
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


struct input_bindings {
	struct keyboard_bindings keyboard;
	struct gamepad_bindings gamepad;
	bool filled;
};

int input_prepare(struct input_bindings *input_bindings);
void input_check_queue(struct all_inputs *inputs);


#endif /* SRC_INCLUDE_INPUT_H_ */
