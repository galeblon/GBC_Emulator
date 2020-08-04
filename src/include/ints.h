#ifndef INTS_H_
#define INTS_H_

enum ints_interrupt_type {
	V_BLANK,
	LCDC_STAT,
	TIMER_OVERFLOW,
	SERIAL_IO_TRANSFER_COMPLETE,
	HIGH_TO_LOW_P10_P13
};

void ints_set_ime(void);
void ints_reset_ime(void);

void ints_prepare(void);
void ints_check(void);

void ints_request(int interrupt_number);
void ints_request_type(enum ints_interrupt_type interrupt);

#endif /* INTS_H_ */
