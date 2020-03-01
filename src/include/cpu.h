#ifndef CPU_H_
#define CPU_H_

#include"stdio.h"

#define CPU_CLOCK_SPEED 4194304
#define INSTRUCTIONS_NUMBER 256

// Return number of cycles of executed instruction
// returns -1 if encountered fatal error
int cpu_single_step();

// Load all instructions
void cpu_prepare();

typedef int(*CPU_INSTRUCTION)();

CPU_INSTRUCTION CPU_INSTRUCTION_TABLE[INSTRUCTIONS_NUMBER];
CPU_INSTRUCTION CB_PREFIX_CPU_INSTRUCTION_TABLE[INSTRUCTIONS_NUMBER];


void cpu_register_print(_IO_FILE *out);

int not_implemented();



// Instructions
int _cpu_nop();
int _jp_nz_a16();
int _jp_a16();

#endif /* CPU_H_ */
