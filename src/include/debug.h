#ifndef SRC_INCLUDE_DEBUG_H_
#define SRC_INCLUDE_DEBUG_H_

#include"types.h"

void debug_print_instruction(u16 pc);
void debug_assert(bool expr, const char *msg);

#endif /* SRC_INCLUDE_DEBUG_H_ */
