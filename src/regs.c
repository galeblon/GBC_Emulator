#include "regs.h"
#include "rom.h"

void registers_prepare(struct cpu_registers *regs)
{
	regs->AF = 0x01B0;
	regs->BC = 0x0013;
	regs->DE = 0x00D8;
	regs->HL = 0x014D;
	regs->SP = 0xFFFE;
	regs->PC = ROM_ENTRY_POINT;
}
