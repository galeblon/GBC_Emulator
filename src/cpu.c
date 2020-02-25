#include<stdio.h>
#include"cpu.h"
#include"rom.h"
#include"regs.h"

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

int not_implemented(){
	// TODO Print some usefull debug information
	// This  way of accessing memory is temporary
	d8 instruction_code = read8ROM(REGISTERS.PC);
	fprintf(stderr, "INSTRUCTION CODE 0x%X NOT IMPLEMENTED", instruction_code);
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
