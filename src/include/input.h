#ifndef SRC_INCLUDE_INPUT_H_
#define SRC_INCLUDE_INPUT_H_

#include"types.h"

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

int input_prepare(const char *input_config_path);
void input_check_queue(struct all_inputs* inputs);


#endif /* SRC_INCLUDE_INPUT_H_ */
