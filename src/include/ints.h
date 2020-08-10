#ifndef INTS_H_
#define INTS_H_

enum ints_interrupt_type {
	INT_V_BLANK                     = 0,
	INT_LCDC                        = 1,
	INT_TIMER_OVERFLOW              = 2,
	INT_SERIAL_IO_TRANSFER_COMPLETE = 3,
	INT_HIGH_TO_LOW_P10_P13         = 4
};

void ints_set_ime(void);
void ints_reset_ime(void);

void ints_prepare(void);
void ints_check(void);

void ints_request(enum ints_interrupt_type interrupt);

#endif /* INTS_H_ */
