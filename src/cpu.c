#include"cpu.h"
#include"rom.h"
#include"regs.h"
#include"logger.h"

int cpu_single_step(){
	// Fetch
	d8 instruction_code = read8ROM(REGISTERS.PC);
	// Decode & Execute
	return CPU_INSTRUCTION_TABLE[instruction_code]();
}


void cpu_prepare(){
	for(int i=0; i<INSTRUCTIONS_NUMBER; i++){
		CPU_INSTRUCTION_TABLE[i] = not_implemented;
		CB_PREFIX_CPU_INSTRUCTION_TABLE[i] = not_implemented;
	}

	// TODO fill all instructions
	CPU_INSTRUCTION_TABLE[0x0] = _cpu_nop;
	CPU_INSTRUCTION_TABLE[0xC2] = _jp_nz_a16;
	CPU_INSTRUCTION_TABLE[0xC3] = _jp_a16;
}

void cpu_register_print(_IO_FILE *out) {
	fprintf(out,
		"\tA: 0x%02X F: 0x%02X\n"
		"\tB: 0x%02X C: 0x%02X\n"
		"\tD: 0x%02X E: 0x%02X\n"
		"\tH: 0x%02X L: 0x%02X\n"
		"\tSP: 0x%04X\n"
		"\tPC: 0x%04X\n"
		"\tZNHC\n"
		"\t%d%d%d%d\n",
		REGISTERS.A,
		REGISTERS.F,
		REGISTERS.B,
		REGISTERS.C,
		REGISTERS.D,
		REGISTERS.E,
		REGISTERS.H,
		REGISTERS.L,
		REGISTERS.SP,
		REGISTERS.PC,
		REGISTERS.FLAGS.Z,
		REGISTERS.FLAGS.N,
		REGISTERS.FLAGS.H,
		REGISTERS.FLAGS.C);
}

int not_implemented(){
	// This  way of accessing memory is temporary
	d8 instruction_code = read8ROM(REGISTERS.PC);
	char message[50];
	sprintf(message, "INSTRUCTION CODE 0x%02X NOT IMPLEMENTED\n", instruction_code);
	emulator_log(LOG_FATAL,
			"UNKOWN INSTRUCTION",
			message);
	return -1;
}


// Instructions
int _cpu_nop(){
	REGISTERS.PC += 1;
	return 4;
}

int _jp_nz_a16(){
	a16 operand = read16ROM(REGISTERS.PC + 1);
	REGISTERS.PC += 3;
	// TODO macro for easy access to flags
	if((REGISTERS.F & 0b10000000) != 0){
		return 12;
	}
	REGISTERS.PC = operand;
	return 16;
}

int _jp_a16(){
	a16 operand = read16ROM(REGISTERS.PC + 1);
	REGISTERS.PC = operand;
	return 16;
}
