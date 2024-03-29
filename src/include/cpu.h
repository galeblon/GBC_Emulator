#ifndef CPU_H_
#define CPU_H_

#include"types.h"
#include"logger.h"

#define CPU_CLOCK_SPEED 4194304

// Return number of cycles of executed instruction
// returns -1 if encountered fatal error
int cpu_single_step(void);

// Set Program Counter to given address
void cpu_jump(a16 addr);

// Push current Program Counter on stack
// Then set Program Counter to given address
void cpu_call(a16 addr);

// Load all instructions
void cpu_prepare(void);

// Write given data to memory pointed by SP
// Then decrement SP by proper amount
void cpu_push8(u8 data);
void cpu_push16(u16 data);

// Interface for interacting with the halted state of processor
bool cpu_get_halted();
void cpu_set_halted(bool val);

// Interface for interacting with the stopped state of processor
bool cpu_get_stopped();
void cpu_set_stopped(bool val);

// Interface for reading whether emulator is in double speed mode.
bool cpu_is_double_speed();

// For debugging purposes
struct cpu_registers cpu_register_get();

#endif /* CPU_H_ */
