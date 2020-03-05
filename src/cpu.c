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


// CPU specific instructions

static int _cpu_nop(void)
{
	g_registers.PC += 1;
	return 4;
}

// Load instructions

// Special loads from A reg
//========================================
static int _cpu_ld_imm_bc_a(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.BC;
	d8 data = g_registers.A;
	mem_write8(address, data);
	return 8;
}

static int _cpu_ld_imm_de_a(void){
	g_registers.PC += 1;
	a16 address = g_registers.DE;
	d8 data = g_registers.A;
	mem_write8(address, data);
	return 8;
}

static int _cpu_ld_imm_hl_inc_a(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL++;
	d8 data = g_registers.A;
	mem_write8(address, data);
	return 8;
}

static int _cpu_ld_imm_hl_dec_a(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL--;
	d8 data = g_registers.A;
	mem_write8(address, data);
	return 8;
}

static int _cpu_ldh_imm_a8_a(void)
{
	g_registers.PC += 1;
	a8 address = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	a16 extended_address = 0xFF00 + (a16)address;
	d8 data = g_registers.A;
	mem_write8(extended_address, data);
	return 12;
}

static int _cpu_ld_imm_a16_a(void)
{
	g_registers.PC += 1;
	a16 address = mem_read16(g_registers.PC);
	g_registers.PC += 2;
	d8 data = g_registers.A;
	mem_write8(address, data);
	return 16;
}

static int _cpu_ld_imm_c_a(void)
{
	g_registers.PC += 1;
	a16 address = 0xFF00 + (a16)g_registers.C;
	g_registers.PC += 1;
	d8 data = g_registers.A;
	mem_write8(address, data);
	return 8;
}

// Special loads to A reg
//========================================
static int _cpu_ld_a_imm_bc(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.BC;
	d8 data = mem_read8(address);
	g_registers.A = data;
	return 8;
}

static int _cpu_ld_a_imm_de(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.DE;
	d8 data = mem_read8(address);
	g_registers.A = data;
	return 8;
}

static int _cpu_ld_a_imm_hl_inc(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL++;
	d8 data = mem_read8(address);
	g_registers.A = data;
	return 8;
}

static int _cpu_ld_a_imm_hl_dec(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL--;
	d8 data = mem_read8(address);
	g_registers.A = data;
	return 8;
}

static int _cpu_ldh_a_imm_a8(void)
{
	g_registers.PC += 1;
	a8 address = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	a16 extended_address = 0xFF00 + (a16)address;
	g_registers.A = mem_read8(extended_address);
	return 12;
}

static int _cpu_ld_a_imm_a16(void)
{
	g_registers.PC += 1;
	a16 address = mem_read16(g_registers.PC);
	g_registers.PC += 2;
	g_registers.A = mem_read8(address);
	return 16;
}

static int _cpu_ld_a_imm_c(void)
{
	g_registers.PC += 1;
	a16 address = 0xFF00 + (a16)g_registers.C;
	g_registers.PC += 1;
	g_registers.A = mem_read8(address);
	return 8;
}

// load to r8 a d8 value
//========================================
static int _cpu_ld_a_d8(void)
{
	g_registers.PC += 1;
	d8 data = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	g_registers.A = data;
	return 8;
}

static int _cpu_ld_b_d8(void)
{
	g_registers.PC += 1;
	d8 data = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	g_registers.B = data;
	return 8;
}

static int _cpu_ld_c_d8(void)
{
	g_registers.PC += 1;
	d8 data = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	g_registers.C = data;
	return 8;
}

static int _cpu_ld_d_d8(void)
{
	g_registers.PC += 1;
	d8 data = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	g_registers.D = data;
	return 8;
}

static int _cpu_ld_e_d8(void)
{
	g_registers.PC += 1;
	d8 data = mem_read8(g_registers.PC);
	g_registers.E = data;
	return 8;
}

static int _cpu_ld_h_d8(void)
{
	g_registers.PC += 1;
	d8 data = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	g_registers.H = data;
	return 8;
}

static int _cpu_ld_l_d8(void)
{
	g_registers.PC += 1;
	d8 data = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	g_registers.L = data;
	return 8;
}

static int _cpu_ld_imm_hl_d8(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL;
	d8 data= mem_read8(g_registers.PC);
	g_registers.PC += 1;
	mem_write8(address, data);
	return 12;
}

// regular loads to A reg
//========================================
static int _cpu_ld_a_a(void)
{
	g_registers.PC += 1;
	// Assigment to itself
	return 4;
}

static int _cpu_ld_a_b(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.B;
	return 4;
}

static int _cpu_ld_a_c(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.C;
	return 4;
}

static int _cpu_ld_a_d(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.D;
	return 4;
}

static int _cpu_ld_a_e(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.E;
	return 4;
}

static int _cpu_ld_a_h(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.H;
	return 4;
}

static int _cpu_ld_a_l(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.L;
	return 4;
}

static int _cpu_ld_a_imm_hl(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL;
	d8 data = mem_read8(address);
	g_registers.A = data;
	return 8;
}

// regular loads to B reg
//========================================
static int _cpu_ld_b_a(void)
{
	g_registers.PC += 1;
	g_registers.B = g_registers.A;
	return 4;
}

static int _cpu_ld_b_b(void)
{
	g_registers.PC += 1;
	// Assigment to itself
	return 4;
}

static int _cpu_ld_b_c(void)
{
	g_registers.PC += 1;
	g_registers.B = g_registers.C;
	return 4;
}

static int _cpu_ld_b_d(void)
{
	g_registers.PC += 1;
	g_registers.B = g_registers.D;
	return 4;
}

static int _cpu_ld_b_e(void)
{
	g_registers.PC += 1;
	g_registers.B = g_registers.E;
	return 4;
}

static int _cpu_ld_b_h(void)
{
	g_registers.PC += 1;
	g_registers.B = g_registers.H;
	return 4;
}

static int _cpu_ld_b_l(void)
{
	g_registers.PC += 1;
	g_registers.B = g_registers.L;
	return 4;
}

static int _cpu_ld_b_imm_hl(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL;
	d8 data = mem_read8(address);
	g_registers.B = data;
	return 8;
}

// regular loads to C reg
//========================================
static int _cpu_ld_c_a(void)
{
	g_registers.PC += 1;
	g_registers.C = g_registers.A;
	return 4;
}

static int _cpu_ld_c_b(void)
{
	g_registers.PC += 1;
	g_registers.C = g_registers.B;
	return 4;
}

static int _cpu_ld_c_c(void)
{
	g_registers.PC += 1;
	// Assigment to itself
	return 4;
}

static int _cpu_ld_c_d(void)
{
	g_registers.PC += 1;
	g_registers.C = g_registers.D;
	return 4;
}

static int _cpu_ld_c_e(void)
{
	g_registers.PC += 1;
	g_registers.C = g_registers.E;
	return 4;
}

static int _cpu_ld_c_h(void)
{
	g_registers.PC += 1;
	g_registers.C = g_registers.H;
	return 4;
}

static int _cpu_ld_c_l(void)
{
	g_registers.PC += 1;
	g_registers.C = g_registers.L;
	return 4;
}

static int _cpu_ld_c_imm_hl(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL;
	d8 data = mem_read8(address);
	g_registers.C = data;
	return 8;
}

// regular loads to D reg
//========================================
static int _cpu_ld_d_a(void)
{
	g_registers.PC += 1;
	g_registers.D = g_registers.A;
	return 4;
}

static int _cpu_ld_d_b(void)
{
	g_registers.PC += 1;
	g_registers.D = g_registers.B;
	return 4;
}

static int _cpu_ld_d_c(void)
{
	g_registers.PC += 1;
	g_registers.D = g_registers.C;
	return 4;
}

static int _cpu_ld_d_d(void)
{
	g_registers.PC += 1;
	// Assigment to itself
	return 4;
}

static int _cpu_ld_d_e(void)
{
	g_registers.PC += 1;
	g_registers.D = g_registers.E;
	return 4;
}

static int _cpu_ld_d_h(void)
{
	g_registers.PC += 1;
	g_registers.D = g_registers.H;
	return 4;
}

static int _cpu_ld_d_l(void)
{
	g_registers.PC += 1;
	g_registers.D = g_registers.L;
	return 4;
}

static int _cpu_ld_d_imm_hl(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL;
	d8 data = mem_read8(address);
	g_registers.D = data;
	return 8;
}

// regular loads to E reg
//========================================
static int _cpu_ld_e_a(void)
{
	g_registers.PC += 1;
	g_registers.E = g_registers.A;
	return 4;
}

static int _cpu_ld_e_b(void)
{
	g_registers.PC += 1;
	g_registers.E = g_registers.B;
	return 4;
}

static int _cpu_ld_e_c(void)
{
	g_registers.PC += 1;
	g_registers.E = g_registers.C;
	return 4;
}

static int _cpu_ld_e_d(void)
{
	g_registers.PC += 1;
	g_registers.E = g_registers.D;
	return 4;
}

static int _cpu_ld_e_e(void)
{
	g_registers.PC += 1;
	// Assigment to itself
	return 4;
}

static int _cpu_ld_e_h(void)
{
	g_registers.PC += 1;
	g_registers.E = g_registers.H;
	return 4;
}

static int _cpu_ld_e_l(void)
{
	g_registers.PC += 1;
	g_registers.E = g_registers.L;
	return 4;
}

static int _cpu_ld_e_imm_hl(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL;
	d8 data = mem_read8(address);
	g_registers.E = data;
	return 8;
}

// regular loads to H reg
//========================================
static int _cpu_ld_h_a(void)
{
	g_registers.PC += 1;
	g_registers.H = g_registers.A;
	return 4;
}

static int _cpu_ld_h_b(void)
{
	g_registers.PC += 1;
	g_registers.H = g_registers.B;
	return 4;
}

static int _cpu_ld_h_c(void)
{
	g_registers.PC += 1;
	g_registers.H = g_registers.C;
	return 4;
}

static int _cpu_ld_h_d(void)
{
	g_registers.PC += 1;
	g_registers.H = g_registers.D;
	return 4;
}

static int _cpu_ld_h_e(void)
{
	g_registers.PC += 1;
	g_registers.H = g_registers.E;
	return 4;
}

static int _cpu_ld_h_h(void)
{
	g_registers.PC += 1;
	// Assigment to itself
	return 4;
}

static int _cpu_ld_h_l(void)
{
	g_registers.PC += 1;
	g_registers.H = g_registers.L;
	return 4;
}

static int _cpu_ld_h_imm_hl(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL;
	d8 data = mem_read8(address);
	g_registers.H = data;
	return 8;
}

// regular loads to L reg
//========================================
static int _cpu_ld_l_a(void)
{
	g_registers.PC += 1;
	g_registers.L = g_registers.A;
	return 4;
}

static int _cpu_ld_l_b(void)
{
	g_registers.PC += 1;
	g_registers.L = g_registers.B;
	return 4;
}

static int _cpu_ld_l_c(void)
{
	g_registers.PC += 1;
	g_registers.L = g_registers.C;
	return 4;
}

static int _cpu_ld_l_d(void)
{
	g_registers.PC += 1;
	g_registers.L = g_registers.D;
	return 4;
}

static int _cpu_ld_l_e(void)
{
	g_registers.PC += 1;
	g_registers.L = g_registers.E;
	return 4;
}

static int _cpu_ld_l_h(void)
{
	g_registers.PC += 1;
	g_registers.L = g_registers.H;
	return 4;
}

static int _cpu_ld_l_l(void)
{
	g_registers.PC += 1;
	// Assigment to itself
	return 4;
}

static int _cpu_ld_l_imm_hl(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL;
	d8 data = mem_read8(address);
	g_registers.L = data;
	return 8;
}

// regular loads to immediate memory pointed by HL reg
//========================================
static int _cpu_ld_imm_hl_a(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL;
	d8 data = g_registers.A;
	mem_write8(address, data);
	return 8;
}

static int _cpu_ld_imm_hl_b(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL;
	d8 data = g_registers.B;
	mem_write8(address, data);
	return 8;
}

static int _cpu_ld_imm_hl_c(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL;
	d8 data = g_registers.C;
	mem_write8(address, data);
	return 8;}

static int _cpu_ld_imm_hl_d(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL;
	d8 data = g_registers.D;
	mem_write8(address, data);
	return 8;}

static int _cpu_ld_imm_hl_e(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL;
	d8 data = g_registers.E;
	mem_write8(address, data);
	return 8;
}

static int _cpu_ld_imm_hl_h(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL;
	d8 data = g_registers.H;
	mem_write8(address, data);
	return 8;
}

static int _cpu_ld_imm_hl_l(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL;
	d8 data = g_registers.L;
	mem_write8(address, data);
	return 8;
}

// 16 bit loads
//========================================
static int _cpu_ld_bc_d16(void)
{
	g_registers.PC += 1;
	d16 data = mem_read16(g_registers.PC);
	g_registers.PC += 2;
	g_registers.BC = data;
	return 12;
}

static int _cpu_ld_de_d16(void)
{
	g_registers.PC += 1;
	d16 data = mem_read16(g_registers.PC);
	g_registers.PC += 2;
	g_registers.DE = data;
	return 12;
}

static int _cpu_ld_hl_d16(void)
{
	g_registers.PC += 1;
	d16 data = mem_read16(g_registers.PC);
	g_registers.PC += 2;
	g_registers.HL = data;
	return 12;
}

static int _cpu_ld_sp_d16(void)
{
	g_registers.PC += 1;
	d16 data = mem_read16(g_registers.PC);
	g_registers.PC += 2;
	g_registers.SP = data;
	return 12;
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

static int _cpu_ld_sp_hl(void)
{
	g_registers.PC += 1;
	g_registers.SP = g_registers.HL;
	return 8;
}

static int _cpu_ld_hl_sp_and_d8(void)
{
	g_registers.PC += 1;
	d8 index = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	d16 result = g_registers.SP + index;
	int result_extended = g_registers.SP + index;
	g_registers.FLAGS.Z = 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(g_registers.SP, index);
	g_registers.FLAGS.C = result != result_extended;
	g_registers.HL = result;
	return 12;
}

static int _cpu_pop_bc(void)
{
	g_registers.PC += 1;
	g_registers.B = mem_read8(g_registers.SP);
	g_registers.C = mem_read8(g_registers.SP + 1);
	g_registers.SP += 2;
	return 12;
}

static int _cpu_pop_de(void)
{
	g_registers.PC += 1;
	g_registers.D = mem_read8(g_registers.SP);
	g_registers.E = mem_read8(g_registers.SP + 1);
	g_registers.SP += 2;
	return 12;
}

static int _cpu_pop_hl(void)
{
	g_registers.PC += 1;
	g_registers.H = mem_read8(g_registers.SP);
	g_registers.L = mem_read8(g_registers.SP + 1);
	g_registers.SP += 2;
	return 12;
}

static int _cpu_pop_af(void)
{
	g_registers.PC += 1;
	g_registers.A = mem_read8(g_registers.SP);
	g_registers.F = mem_read8(g_registers.SP + 1);
	g_registers.SP += 2;
	return 12;
}

static int _cpu_push_bc(void)
{
	g_registers.PC += 1;
	mem_write8(g_registers.SP - 1, g_registers.B);
	mem_write8(g_registers.SP - 2, g_registers.C);
	g_registers.SP -= 2;
	return 16;
}

static int _cpu_push_de(void)
{
	g_registers.PC += 1;
	mem_write8(g_registers.SP - 1, g_registers.D);
	mem_write8(g_registers.SP - 2, g_registers.E);
	g_registers.SP -= 2;
	return 16;
}

static int _cpu_push_hl(void)
{
	g_registers.PC += 1;
	mem_write8(g_registers.SP - 1, g_registers.H);
	mem_write8(g_registers.SP - 2, g_registers.L);
	g_registers.SP -= 2;
	return 16;
}

static int _cpu_push_af(void)
{
	g_registers.PC += 1;
	mem_write8(g_registers.SP - 1, g_registers.A);
	mem_write8(g_registers.SP - 2, g_registers.F);
	g_registers.SP -= 2;
	return 16;
}

//================================================
// Rest is ungrouped for now.

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
	// Missing
	g_instruction_table[0x0A] = _cpu_ld_a_imm_bc;
	// Missing
	g_instruction_table[0x0E] = _cpu_ld_c_d8;
	// Missing
	g_instruction_table[0x11] = _cpu_ld_de_d16;
	g_instruction_table[0x12] = _cpu_ld_imm_de_a;
	// Missing
	g_instruction_table[0x16] = _cpu_ld_d_d8;
	// Missing
	g_instruction_table[0x1A] = _cpu_ld_a_imm_de;
	// Missing
	g_instruction_table[0x1E] = _cpu_ld_e_d8;
	// Missing
	g_instruction_table[0X21] = _cpu_ld_hl_d16;
	g_instruction_table[0x22] = _cpu_ld_imm_hl_inc_a;
	// Missing
	g_instruction_table[0x26] = _cpu_ld_h_d8;
	// Missing
	g_instruction_table[0x2A] = _cpu_ld_a_imm_hl_inc;
	// Missing
	g_instruction_table[0x2E] = _cpu_ld_l_d8;
	// Missing
	g_instruction_table[0X31] = _cpu_ld_sp_d16;
	g_instruction_table[0x32] = _cpu_ld_imm_hl_dec_a;
	// Missing
	g_instruction_table[0x36] = _cpu_ld_imm_hl_d8;
	// Missing
	g_instruction_table[0x3A] = _cpu_ld_a_imm_hl_dec;
	// Missing
	g_instruction_table[0x3E] = _cpu_ld_a_d8;
	// Missing
	g_instruction_table[0x40] = _cpu_ld_b_b;
	g_instruction_table[0x41] = _cpu_ld_b_c;
	g_instruction_table[0x42] = _cpu_ld_b_d;
	g_instruction_table[0x43] = _cpu_ld_b_e;
	g_instruction_table[0x44] = _cpu_ld_b_h;
	g_instruction_table[0x45] = _cpu_ld_b_l;
	g_instruction_table[0x46] = _cpu_ld_b_imm_hl;
	g_instruction_table[0x47] = _cpu_ld_b_a;
	g_instruction_table[0x48] = _cpu_ld_c_b;
	g_instruction_table[0x49] = _cpu_ld_c_c;
	g_instruction_table[0x4A] = _cpu_ld_c_d;
	g_instruction_table[0x4B] = _cpu_ld_c_e;
	g_instruction_table[0x4C] = _cpu_ld_c_h;
	g_instruction_table[0x4D] = _cpu_ld_c_l;
	g_instruction_table[0x4E] = _cpu_ld_c_imm_hl;
	g_instruction_table[0x4F] = _cpu_ld_c_a;
	g_instruction_table[0x50] = _cpu_ld_d_b;
	g_instruction_table[0x51] = _cpu_ld_d_c;
	g_instruction_table[0x52] = _cpu_ld_d_d;
	g_instruction_table[0x53] = _cpu_ld_d_e;
	g_instruction_table[0x54] = _cpu_ld_d_h;
	g_instruction_table[0x55] = _cpu_ld_d_l;
	g_instruction_table[0x56] = _cpu_ld_d_imm_hl;
	g_instruction_table[0x57] = _cpu_ld_d_a;
	g_instruction_table[0x58] = _cpu_ld_e_b;
	g_instruction_table[0x59] = _cpu_ld_e_c;
	g_instruction_table[0x5A] = _cpu_ld_e_d;
	g_instruction_table[0x5B] = _cpu_ld_e_e;
	g_instruction_table[0x5C] = _cpu_ld_e_h;
	g_instruction_table[0x5D] = _cpu_ld_e_l;
	g_instruction_table[0x5E] = _cpu_ld_e_imm_hl;
	g_instruction_table[0x5F] = _cpu_ld_e_a;
	g_instruction_table[0x60] = _cpu_ld_h_b;
	g_instruction_table[0x61] = _cpu_ld_h_c;
	g_instruction_table[0x62] = _cpu_ld_h_d;
	g_instruction_table[0x63] = _cpu_ld_h_e;
	g_instruction_table[0x64] = _cpu_ld_h_h;
	g_instruction_table[0x65] = _cpu_ld_h_l;
	g_instruction_table[0x66] = _cpu_ld_h_imm_hl;
	g_instruction_table[0x67] = _cpu_ld_h_a;
	g_instruction_table[0x68] = _cpu_ld_l_b;
	g_instruction_table[0x69] = _cpu_ld_l_c;
	g_instruction_table[0x6A] = _cpu_ld_l_d;
	g_instruction_table[0x6B] = _cpu_ld_l_e;
	g_instruction_table[0x6C] = _cpu_ld_l_h;
	g_instruction_table[0x6D] = _cpu_ld_l_l;
	g_instruction_table[0x6E] = _cpu_ld_l_imm_hl;
	g_instruction_table[0x6F] = _cpu_ld_l_a;
	g_instruction_table[0x70] = _cpu_ld_imm_hl_b;
	g_instruction_table[0x71] = _cpu_ld_imm_hl_c;
	g_instruction_table[0x72] = _cpu_ld_imm_hl_d;
	g_instruction_table[0x73] = _cpu_ld_imm_hl_e;
	g_instruction_table[0x74] = _cpu_ld_imm_hl_h;
	g_instruction_table[0x75] = _cpu_ld_imm_hl_l;
	// Missing
	g_instruction_table[0x77] = _cpu_ld_imm_hl_a;
	g_instruction_table[0x78] = _cpu_ld_a_b;
	g_instruction_table[0x79] = _cpu_ld_a_c;
	g_instruction_table[0x7A] = _cpu_ld_a_d;
	g_instruction_table[0x7B] = _cpu_ld_a_e;
	g_instruction_table[0x7C] = _cpu_ld_a_h;
	g_instruction_table[0x7D] = _cpu_ld_a_l;
	g_instruction_table[0x7E] = _cpu_ld_a_imm_hl;
	g_instruction_table[0x7F] = _cpu_ld_a_a;
	// Missing
	g_instruction_table[0xC1] = _cpu_pop_bc;
	g_instruction_table[0xC2] = _cpu_jp_nz_a16;
	g_instruction_table[0xC3] = _cpu_jp_a16;
	// Missing
	g_instruction_table[0xC5] = _cpu_push_bc;
	// Missing
	g_instruction_table[0xD1] = _cpu_pop_de;
	// Missing
	g_instruction_table[0xD5] = _cpu_push_de;
	// Missing
	g_instruction_table[0XE0] = _cpu_ldh_imm_a8_a;
	g_instruction_table[0xE1] = _cpu_pop_hl;
	g_instruction_table[0xE2] = _cpu_ld_imm_c_a;
	// Missing
	g_instruction_table[0xE5] = _cpu_push_hl;
	// Missing
	g_instruction_table[0xEA] = _cpu_ld_imm_a16_a;
	// Missing
	g_instruction_table[0XF0] = _cpu_ldh_a_imm_a8;
	g_instruction_table[0xF1] = _cpu_pop_af;
	g_instruction_table[0xF2] = _cpu_ld_a_imm_c;
	// Missing
	g_instruction_table[0xF5] = _cpu_push_af;
	// Missing
	g_instruction_table[0xF8] = _cpu_ld_hl_sp_and_d8;
	g_instruction_table[0xF9] = _cpu_ld_sp_hl;
	g_instruction_table[0xFA] = _cpu_ld_a_imm_a16;
	// Missing

	registers_prepare(&g_registers);
}
