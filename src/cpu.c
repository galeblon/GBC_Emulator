#include"cpu.h"
#include"logger.h"
#include"mem.h"
#include"regs.h"

#define INSTRUCTIONS_NUMBER 256

typedef int (*cpu_instruction_t)(void);

static cpu_instruction_t g_instruction_table[INSTRUCTIONS_NUMBER];
static cpu_instruction_t g_cb_prefix_instruction_table[INSTRUCTIONS_NUMBER];

static struct cpu_registers g_registers;

void cpu_register_print(FILE *out) {
	fprintf(out,
		"\tA: 0x%02X F: 0x%02X\n"
		"\tB: 0x%02X C: 0x%02X\n"
		"\tD: 0x%02X E: 0x%02X\n"
		"\tH: 0x%02X L: 0x%02X\n"
		"\tSP: 0x%04X\n"
		"\tPC: 0x%04X\n"
		"\tZNHC\n"
		"\t%d%d%d%d\n",
		g_registers.A,
		g_registers.F,
		g_registers.B,
		g_registers.C,
		g_registers.D,
		g_registers.E,
		g_registers.H,
		g_registers.L,
		g_registers.SP,
		g_registers.PC,
		g_registers.FLAGS.Z,
		g_registers.FLAGS.N,
		g_registers.FLAGS.H,
		g_registers.FLAGS.C);
}

static int _cpu_not_implemented(void)
{
	// This  way of accessing memory is temporary
	d8 instruction_code = mem_read8(g_registers.PC);
	char *message = logger_get_msg_buffer();
	snprintf(message,
		LOG_MESSAGE_MAX_SIZE,
		"INSTRUCTION CODE 0x%02X NOT IMPLEMENTED\n",
		instruction_code);
	logger_log(LOG_FATAL,
		"UNKOWN INSTRUCTION",
		message);
	return -1;
}

static int _cpu_nop(void)
{
	g_registers.PC += 1;
	return 4;
}

static int _cpu_jp_nz_a16(void)
{
	a16 operand = mem_read16(g_registers.PC + 1);
	g_registers.PC += 3;
	if(g_registers.FLAGS.Z != 0)
		return 12;

	g_registers.PC = operand;
	return 16;
}

static int _cpu_jp_a16(void)
{
	a16 operand = mem_read16(g_registers.PC + 1);
	g_registers.PC = operand;
	return 16;
}


int cpu_single_step(void)
{
	// Fetch
	d8 instruction_code = mem_read8(g_registers.PC);
	// Decode & Execute
	return g_instruction_table[instruction_code]();
}


void cpu_prepare(void)
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
