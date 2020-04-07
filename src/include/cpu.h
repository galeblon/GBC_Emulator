#ifndef CPU_H_
#define CPU_H_

#include<stdio.h>
#include"types.h"

#define CPU_CLOCK_SPEED 4194304

// Return number of cycles of executed instruction
// returns -1 if encountered fatal error
int cpu_single_step(void);

// Set Program Counter to given address
void cpu_jump(a16 addr);

// Push current Program Counter on stack
// Then set Program Counter to given address
void cpu_jump_push(a16 addr);

// Load all instructions
void cpu_prepare(void);

// Write given data to memory pointed by SP
// Then decrement SP by proper amount
void cpu_push8(u8 data);
void cpu_push16(u16 data);

void cpu_register_print(FILE *out);

#endif /* CPU_H_ */
