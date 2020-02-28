#ifndef CPU_H_
#define CPU_H_

#define CPU_CLOCK_SPEED 4194304

// Return number of cycles of executed instruction
// returns -1 if encountered fatal error
int cpu_single_step();

// Load all instructions
void cpu_prepare();

#endif /* CPU_H_ */
