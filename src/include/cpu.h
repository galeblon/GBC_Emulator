#ifndef CPU_H_
#define CPU_H_

#include<stdio.h>

#define CPU_CLOCK_SPEED 4194304

// Return number of cycles of executed instruction
// returns -1 if encountered fatal error
int cpu_single_step(void);

// Load all instructions
void cpu_prepare(void);

void cpu_register_print(FILE *out);

#endif /* CPU_H_ */
