#ifndef SRC_INCLUDE_DEBUG_H_
#define SRC_INCLUDE_DEBUG_H_

#include"types.h"

// returns the length of given instruction
int debug_op_length(d8 opcode);

// return the mnemonic of given instruction
char* debug_op_mnemonic_format(d8 opcode);

char* debug_op_extended_mnemonic_format(d8 opcode);

#endif /* SRC_INCLUDE_DEBUG_H_ */
