#include "regs.h"
#include "rom.h"

void registers_prepare(){
	REGISTERS.AF = 0x01B0;
	REGISTERS.BC = 0x0013;
	REGISTERS.DE = 0x00D8;
	REGISTERS.HL = 0x014D;
	REGISTERS.SP = 0xFFFE;
	REGISTERS.PC = ROM_H_ENTRY_POINT;
}
