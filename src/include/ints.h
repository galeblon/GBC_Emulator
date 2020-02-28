#ifndef INTS_H_
#define INTS_H_

#include "types.h"

bool IME;

void set_ime(void);
void reset_ime(void);

void ints_prepare(void);
void check_ints(void);

#endif /* INTS_H_ */
