#include"cpu.h"
#include"logger.h"
#include"mem.h"
#include"regs.h"

#define INSTRUCTIONS_NUMBER 256

#define _CPU_IS_HALF_CARRY(a, b) ((((a & 0x0F) + (b & 0x0F)) & 0x10) == 0x10)
#define _CPU_IS_HALF_BORROW(a, b) (((a & 0x0F) - (b & 0x0F)) < 0)

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

// STUB functions, they will be replaced when memory module is done
static d8 mem_read8(a16 address)
{
	return READ_8ROM(address);
}

static d16 mem_read16(a16 address)
{
	return READ_16ROM(address);
}

static void mem_write8(a16 address, d8 data)
{
	data = address + data;
	return;
}

static void mem_write16(a16 address, d16 data)
{
	data = address + data;
	return;
}

// CPU basic instructions

static int _cpu_nop(void)
{
	g_registers.PC += 1;
	return 4;
}

static int _cpu_ld_bc_d16(void)
{
	g_registers.PC += 1;
	d16 operand = mem_read16(g_registers.PC);
	g_registers.PC += 2;
	g_registers.BC = operand;
	return 12;
}

static int _cpu_ld_imm_bc_a(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.BC;
	d8 data = g_registers.A;
	mem_write8(address, data);
	return 8;
}

static int _cpu_inc_bc(void)
{
	g_registers.PC += 1;
	g_registers.BC += 1;
	return 8;
}

static int _cpu_inc_b(void)
{
	g_registers.PC += 1;
	d8 left_operand = g_registers.B;
	d8 right_operand = 0x01;
	g_registers.B = left_operand + right_operand;
	g_registers.FLAGS.Z = g_registers.B == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left_operand, right_operand);
	return 4;
}

// TODO find reliable source about when half-borrow should be set
static int _cpu_dec_b(void)
{
	g_registers.PC += 1;
	d8 left_operand = g_registers.B;
	d8 right_operand = -1;
	g_registers.B = left_operand - right_operand;
	g_registers.FLAGS.Z = g_registers.B == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = !_CPU_IS_HALF_BORROW(left_operand, right_operand);
	return 4;
}

static int _cpu_ld_b_d8(void)
{
	g_registers.PC += 1;
	d8 operand = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	g_registers.B = operand;
	return 8;
}

static int _cpu_rlca(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.A & 0x80) != 0;
	g_registers.A <<= 1;
	g_registers.A |= g_registers.FLAGS.C;
	return 4;
}

static int _cpu_ld_imm_a16_sp(void)
{
	g_registers.PC += 1;
	a16 address = mem_read16(g_registers.PC);
	d16 data = g_registers.SP;
	g_registers.PC += 2;
	mem_write16(address, data);
	return 20;
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
	g_instruction_table[0x00] = _cpu_nop;
	g_instruction_table[0x01] = _cpu_ld_bc_d16;
	g_instruction_table[0x02] = _cpu_ld_imm_bc_a;
	g_instruction_table[0x03] = _cpu_inc_bc;
	g_instruction_table[0x04] = _cpu_inc_b;
	g_instruction_table[0x05] = _cpu_dec_b;
	g_instruction_table[0x06] = _cpu_ld_b_d8;
	g_instruction_table[0x07] = _cpu_rlca;
	g_instruction_table[0x08] = _cpu_ld_imm_a16_sp;

	g_instruction_table[0xC2] = _cpu_jp_nz_a16;
	g_instruction_table[0xC3] = _cpu_jp_a16;

	registers_prepare(&g_registers);
}
