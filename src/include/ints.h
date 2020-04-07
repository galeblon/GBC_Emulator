#ifndef INTS_H_
#define INTS_H_

#include"types.h"

bool g_ime;

void ints_set_ime(void);
void ints_reset_ime(void);

void ints_prepare(void);
void ints_check(void);

#endif /* INTS_H_ */
