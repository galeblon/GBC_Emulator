#include "regs.h"
#include "rom.h"

void registers_prepare(struct cpu_registers *regs)
{
	if (rom_is_cgb()) {
		regs->DE = 0xFF56;
		regs->HL = 0x000D;
	} else {
		regs->DE = 0x0008;
		regs->HL = 0x007C;
	}
	regs->AF = 0x1180;
	regs->BC = 0x0000;
	regs->SP = 0xFFFE;
	regs->PC = ROM_ENTRY_POINT;
}
