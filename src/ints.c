#include "rom.h"
#include "regs.h"
#include "include\ints.h"

#define IFAddress 0xFF0F
#define IEAddress 0xFFFF

#define readIF read8ROM(IFAddress)
#define readIE read8ROM(IEAddress)
//Placeholder until real function arrives - write16(what, where)
#define writeIF(val) write16(val, IFAddress)
#define writeIE(val) write16(val, IEAddress)

#define isF0(reg) reg & 0x01 == 1
#define isF1(reg) reg & 0x02 == 1
#define isF2(reg) reg & 0x04 == 1
#define isF3(reg) reg & 0x08 == 1
#define isF4(reg) reg & 0x10 == 1
#define isF5(reg) reg & 0x20 == 1
#define isF6(reg) reg & 0x40 == 1
#define isF7(reg) reg & 0x80 == 1

void set_ime(void) {
	IME = true;
}

void reset_ime(void) {
	IME = false;
}

void ints_prepare(void) {
	setIME();
	writeIF(0x00);
	writeIE(0x1F);
}

void check_ints(void) {
	if (!IME)
		return;

	d8 IF = readIF;
	d8 IE = readIE;
	IF &= IE;
	if (!IF)
		return;
	reset_ime();
	REGISTERS.SP -= 2;
	//Placeholder until real function arrives - write16(what, where)
	write16(REGISTERS.PC, REGISTERS.SP);

	if (isF0(IF)) {
		REGISTERS.PC = 0x0040;
	} else if (isF1(IF)) {
		REGISTERS.PC = 0x0048;
	} else if (isF2(IF)) {
		REGISTERS.PC = 0x0050;
	} else if (isF3(IF)) {
		REGISTERS.PC = 0x0058;
	} else if (isF4(IF)) {
		REGISTERS.PC = 0x0060;
	} else {
		fprintf(stderr, "[INTERRUPT MODULE] UNDEFINED BEHAVIOUR ERROR - INTERRUPT TRIGGERED DOES NOT EXIST.\n");
		fprintf(stderr, "    IF: 0x%X    IE: 0x%X\n", IF, IE);
		fprintf(stderr, "REGISTER STATUS DURING ERROR:\n");
		fprintf(stderr, "    AF: 0x%X    BC: 0x%X\n", REGISTERS.AF, REGISTERS.BC);
		fprintf(stderr, "    DE: 0x%X    HL: 0x%X\n", REGISTERS.DE, REGISTERS.HL);
		fprintf(stderr, "    SP: 0x%X    PC: 0x%X\n", REGISTERS.SP, REGISTERS.PC);
		fprintf(stderr, "RECOVERY: POPPING STACK; SETTING IE TO 0x1F; CLEARING IF.\n\n");
		REGISTERS.SP += 2;
		writeIE(0x1F);
	}
	writeIF(0x00);
}
