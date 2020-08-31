#ifndef SRC_INCLUDE_JOYPAD_H_
#define SRC_INCLUDE_JOYPAD_H_

#include"types.h"

struct __attribute__((packed)) joypad_register {
	u8 zero : 2;
	u8 SELECT_BUTTONS: 1;
	u8 SELECT_DIRECTIONS: 1;
	u8 DOWN_START : 1;
	u8 UP_SELECT : 1;
	u8 LEFT_B : 1;
	u8 RIGHT_A : 1;
};

void joypad_prepare(void);
void joypad_step(void);

d8 joypad_get_register_state(void);
void joypad_set_register_selection(bool select_buttons, bool select_directions);

#endif /* SRC_INCLUDE_JOYPAD_H_ */
