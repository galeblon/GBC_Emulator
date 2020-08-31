#ifndef SRC_INCLUDE_PLAYER_INPUT_H_
#define SRC_INCLUDE_PLAYER_INPUT_H_

#include"types.h"

struct all_inputs {
	u8 DOWN;
	u8 UP;
	u8 LEFT;
	u8 RIGHT;
	u8 START;
	u8 SELECT;
	u8 A;
	u8 B;
};

void player_input_prepare();
void player_input_check_queue(struct all_inputs* inputs);


#endif /* SRC_INCLUDE_PLAYER_INPUT_H_ */
