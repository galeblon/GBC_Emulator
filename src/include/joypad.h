#ifndef SRC_INCLUDE_JOYPAD_H_
#define SRC_INCLUDE_JOYPAD_H_

#include"types.h"

struct joypad_register_bits {
	u8 RIGHT_A : 1;
	u8 LEFT_B : 1;
	u8 UP_SELECT : 1;
	u8 DOWN_START : 1;
	u8 SELECT_DIRECTIONS: 1;
	u8 SELECT_BUTTONS: 1;
	u8 zero : 2;
}  __attribute__((packed));

struct joypad_register {
	union {
		struct joypad_register_bits BITS;
		u8 REG;
	};
};

void joypad_prepare(void);
void joypad_step(void);

#endif /* SRC_INCLUDE_JOYPAD_H_ */
