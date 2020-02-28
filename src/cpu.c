#include<stdio.h>
#include"cpu.h"
#include"rom.h"
#include"regs.h"

#define INSTRUCTIONS_NUMBER 256

typedef int (*cpu_instruction_t)();

static cpu_instruction_t g_instruction_table[INSTRUCTIONS_NUMBER];
static cpu_instruction_t g_cb_prefix_instruction_table[INSTRUCTIONS_NUMBER];

static struct cpu_registers g_registers;


// Instructions
static int _cpu_not_implemented()
{
	// TODO Print some usefull debug information
	// This  way of accessing memory is temporary
	d8 instruction_code = read8ROM(g_registers.PC);
	fprintf(stderr, "INSTRUCTION CODE 0x%X NOT IMPLEMENTED", instruction_code);
	return -1;
}

static int _cpu_nop()
{
	g_registers.PC += 1;
	return 4;
}

static int _cpu_jp_nz_a16()
{
	a16 operand = read16ROM(g_registers.PC + 1);
	g_registers.PC += 3;
	// TODO macro for easy access to flags
	if(g_registers.FLAGS.Z != 0)
		return 12;

	g_registers.PC = operand;
	return 16;
}

static int _cpu_jp_a16()
{
	a16 operand = read16ROM(g_registers.PC + 1);
	g_registers.PC = operand;
	return 16;
}


int cpu_single_step()
{
	// Fetch
	d8 instruction_code = read8ROM(g_registers.PC);
	// Decode & Execute
	return g_instruction_table[instruction_code]();
}


void cpu_prepare()
{
	for(int i=0; i<INSTRUCTIONS_NUMBER; i++) {
		g_instruction_table[i] = _cpu_not_implemented;
		g_cb_prefix_instruction_table[i] = _cpu_not_implemented;
	}

	// TODO fill all instructions
	g_instruction_table[0x0] = _cpu_nop;
	g_instruction_table[0xC2] = _cpu_jp_nz_a16;
	g_instruction_table[0xC3] = _cpu_jp_a16;

	registers_prepare(&g_registers);
}
