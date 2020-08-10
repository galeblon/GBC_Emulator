#ifndef INTS_H_
#define INTS_H_

enum ints_interrupt_type {
	INT_V_BLANK,
	INT_LCDC,
	INT_TIMER_OVERFLOW,
	INT_SERIAL_IO_TRANSFER_COMPLETE,
	INT_HIGH_TO_LOW_P10_P13
};

void ints_set_ime(void);
void ints_reset_ime(void);

void ints_prepare(void);
void ints_check(void);

void ints_request(int interrupt_number);
void ints_request_type(enum ints_interrupt_type interrupt);

#endif /* INTS_H_ */
