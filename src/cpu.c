#include"cpu.h"
#include"debug.h"
#include"ints.h"
#include"logger.h"
#include"mem.h"
#include"regs.h"

#define INSTRUCTIONS_NUMBER 256

#define _CPU_IS_HALF_CARRY(a, b) ((((a & 0x0F) + (b & 0x0F)) & 0x10) == 0x10)
#define _CPU_IS_CARRY(a, b) ((((a & 0xFF) + (b & 0xFF)) & 0x100) == 0x100)

#define _CPU_IS_HALF_CARRY16(a, b) ((((a & 0x0FFF) + (b & 0x0FFF)) & 0x1000) == 0x1000)
#define _CPU_IS_CARRY16(a, b) (((((int)a & 0xFFFF) + ((int)b & 0xFFFF)) & 0x10000) == 0x10000)

#define _CPU_IS_HALF_CARRY_C(a, b, carry) ((((a & 0x0F) + (b & 0x0F) + carry) & 0x10) == 0x10)
#define _CPU_IS_CARRY_C(a, b, carry) ((((a & 0xFF) + (b & 0xFF) + carry) & 0x100) == 0x100)

#define _CPU_IS_HALF_BORROW(a, b) (((a & 0x0F) - (b & 0x0F)) < 0)
#define _CPU_IS_BORROW(a, b) (a < b)

#define _CPU_IS_HALF_BORROW_C(a, b, carry) (((a & 0x0F) - (b & 0x0F) - carry) < 0)
#define _CPU_IS_BORROW_C(a, b, carry) (a < (b + carry))

#define IME_OP_DI 0
#define IME_OP_EI 1

typedef int (*cpu_instruction_t)(void);

static cpu_instruction_t g_instruction_table[INSTRUCTIONS_NUMBER];
static cpu_instruction_t g_cb_prefix_instruction_table[INSTRUCTIONS_NUMBER];

static struct cpu_registers g_registers;

// cpu state
static bool g_cpu_halted = 0;
static bool g_cpu_stopped = 0;

// cpu ime delay
static int g_ime_delay = 0;
static int g_ime_op = IME_OP_DI;

void cpu_register_print(FILE *out)
{
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


// CPU specific instructions

// Misc instructions
//========================================

static int _cpu_nop(void)
{
	g_registers.PC += 1;
	return 4;
}

static int _cpu_stop(void)
{
	g_cpu_stopped = 1;
	g_registers.PC += 2;
	return 4;
}

static int _cpu_halt(void)
{
	g_cpu_halted = 1;
	g_registers.PC += 1;
	return 4;
}

static int _cpu_prefix_cb(void)
{
	g_registers.PC += 1;
	d8 opcode = mem_read8(g_registers.PC);
	return g_cb_prefix_instruction_table[opcode]();
}

static int _cpu_di(void)
{
	g_ime_delay = 2;
	g_ime_op = IME_OP_DI;
	g_registers.PC += 1;
	return 4;
}

static int _cpu_ei(void)
{
	g_ime_delay = 2;
	g_ime_op = IME_OP_EI;
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
	mem_write8(address, g_registers.A);
	return 8;
}

static int _cpu_ld_imm_de_a(void){
	g_registers.PC += 1;
	a16 address = g_registers.DE;
	mem_write8(address, g_registers.A);
	return 8;
}

static int _cpu_ld_imm_hl_inc_a(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL++;
	mem_write8(address, g_registers.A);
	return 8;
}

static int _cpu_ld_imm_hl_dec_a(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL--;
	mem_write8(address, g_registers.A);
	return 8;
}

static int _cpu_ldh_imm_a8_a(void)
{
	g_registers.PC += 1;
	a16 address = (a16)mem_read8(g_registers.PC) + 0xFF00;
	g_registers.PC += 1;
	mem_write8(address, g_registers.A);
	return 12;
}

static int _cpu_ld_imm_a16_a(void)
{
	g_registers.PC += 1;
	a16 address = mem_read16(g_registers.PC);
	g_registers.PC += 2;
	mem_write8(address, g_registers.A);
	return 16;
}

static int _cpu_ld_imm_c_a(void)
{
	g_registers.PC += 1;
	a16 address = (a16)g_registers.C + 0xFF00;
	g_registers.PC += 1;
	mem_write8(address, g_registers.A);
	return 8;
}

// Special loads to A reg
//========================================
static int _cpu_ld_a_imm_bc(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.BC;
	g_registers.A = mem_read8(address);
	return 8;
}

static int _cpu_ld_a_imm_de(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.DE;
	g_registers.A = mem_read8(address);
	return 8;
}

static int _cpu_ld_a_imm_hl_inc(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL++;
	g_registers.A = mem_read8(address);
	return 8;
}

static int _cpu_ld_a_imm_hl_dec(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL--;
	g_registers.A = mem_read8(address);
	return 8;
}

static int _cpu_ldh_a_imm_a8(void)
{
	g_registers.PC += 1;
	a16 address = (a16)mem_read8(g_registers.PC) + 0xFF00;
	g_registers.PC += 1;
	g_registers.A = mem_read8(address);
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
	a16 address = (a16)g_registers.C + 0xFF00;
	g_registers.PC += 1;
	g_registers.A = mem_read8(address);
	return 8;
}

// load to r8 a d8 value
//========================================
static int _cpu_ld_a_d8(void)
{
	g_registers.PC += 1;
	g_registers.A = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	return 8;
}

static int _cpu_ld_b_d8(void)
{
	g_registers.PC += 1;
	g_registers.B = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	return 8;
}

static int _cpu_ld_c_d8(void)
{
	g_registers.PC += 1;
	g_registers.C = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	return 8;
}

static int _cpu_ld_d_d8(void)
{
	g_registers.PC += 1;
	g_registers.D = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	return 8;
}

static int _cpu_ld_e_d8(void)
{
	g_registers.PC += 1;
	g_registers.E = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	return 8;
}

static int _cpu_ld_h_d8(void)
{
	g_registers.PC += 1;
	g_registers.H = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	return 8;
}

static int _cpu_ld_l_d8(void)
{
	g_registers.PC += 1;
	g_registers.L = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	return 8;
}

static int _cpu_ld_imm_hl_d8(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL;
	d8 data = mem_read8(g_registers.PC);
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
	g_registers.A = mem_read8(address);
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
	return 8;
}

static int _cpu_ld_imm_hl_d(void)
{
	g_registers.PC += 1;
	a16 address = g_registers.HL;
	d8 data = g_registers.D;
	mem_write8(address, data);
	return 8;
}

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
	g_registers.PC += 2;
	d16 data = g_registers.SP;
	mem_write16(address, data);
	return 20;
}

static int _cpu_ld_sp_hl(void)
{
	g_registers.PC += 1;
	g_registers.SP = g_registers.HL;
	return 8;
}

static int _cpu_ld_hl_sp_add_d8(void)
{
	g_registers.PC += 1;
	s8 index = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	d16 result = g_registers.SP + index;
	g_registers.FLAGS.Z = 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(g_registers.SP, (d8)index);
	g_registers.FLAGS.C = _CPU_IS_CARRY(g_registers.SP, (d8)index);
	g_registers.HL = result;
	return 12;
}

static int _cpu_pop_bc(void)
{
	g_registers.PC += 1;
	g_registers.C = mem_read8(g_registers.SP);
	g_registers.B = mem_read8(g_registers.SP + 1);
	g_registers.SP += 2;
	return 12;
}

static int _cpu_pop_de(void)
{
	g_registers.PC += 1;
	g_registers.E = mem_read8(g_registers.SP);
	g_registers.D = mem_read8(g_registers.SP + 1);
	g_registers.SP += 2;
	return 12;
}

static int _cpu_pop_hl(void)
{
	g_registers.PC += 1;
	g_registers.L = mem_read8(g_registers.SP);
	g_registers.H = mem_read8(g_registers.SP + 1);
	g_registers.SP += 2;
	return 12;
}

static int _cpu_pop_af(void)
{
	g_registers.PC += 1;
	// Flag register 4 lower bits are always 0
	g_registers.F = mem_read8(g_registers.SP) & 0xF0;
	g_registers.A = mem_read8(g_registers.SP + 1);
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

// Jump instructions
//========================================
static int _cpu_jr_nz_r8(void)
{
	g_registers.PC += 1;
	r8 offset = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	if(g_registers.FLAGS.Z == 0) {
		g_registers.PC += offset;
		return 12;
	}
	return 8;
}

static int _cpu_jp_nz_a16(void)
{
	g_registers.PC += 1;
	a16 absolute = mem_read16(g_registers.PC);
	g_registers.PC += 2;
	if(g_registers.FLAGS.Z == 0) {
		g_registers.PC = absolute;
		return 16;
	}
	return 12;
}

static int _cpu_jr_nc_r8(void)
{
	g_registers.PC += 1;
	r8 offset = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	if(g_registers.FLAGS.C == 0) {
		g_registers.PC += offset;
		return 12;
	}
	return 8;
}

static int _cpu_jp_nc_a16(void)
{
	g_registers.PC += 1;
	a16 absolute = mem_read16(g_registers.PC);
	g_registers.PC += 2;
	if(g_registers.FLAGS.C == 0) {
		g_registers.PC = absolute;
		return 16;
	}
	return 12;
}

static int _cpu_jr_z_r8(void)
{
	g_registers.PC += 1;
	r8 offset = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	if(g_registers.FLAGS.Z == 1) {
		g_registers.PC += offset;
		return 12;
	}
	return 8;
}

static int _cpu_jp_z_a16(void)
{
	g_registers.PC += 1;
	a16 absolute = mem_read16(g_registers.PC);
	g_registers.PC += 2;
	if(g_registers.FLAGS.Z == 1) {
		g_registers.PC = absolute;
		return 16;
	}
	return 12;
}

static int _cpu_jr_c_r8(void)
{
	g_registers.PC += 1;
	r8 offset = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	if(g_registers.FLAGS.C == 1) {
		g_registers.PC += offset;
		return 12;
	}
	return 8;
}

static int _cpu_jp_c_a16(void)
{
	g_registers.PC += 1;
	a16 absolute = mem_read16(g_registers.PC);
	g_registers.PC += 2;
	if(g_registers.FLAGS.C == 1) {
		g_registers.PC = absolute;
		return 16;
	}
	return 12;
}

static int _cpu_jr_r8(void)
{
	g_registers.PC += 1;
	r8 offset = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	g_registers.PC += offset;
	return 12;
}

static int _cpu_jp_a16(void)
{
	g_registers.PC += 1;
	a16 absolute = mem_read16(g_registers.PC);
	g_registers.PC += 2;
	g_registers.PC = absolute;
	return 16;
}

static int _cpu_jp_imm_hl(void)
{
	g_registers.PC += 1;
	g_registers.PC = g_registers.HL;
	return 4;
}

// Call instructions
//========================================
static int _cpu_call_nz_a16(void)
{
	g_registers.PC += 1;
	a16 absolute = mem_read16(g_registers.PC);
	g_registers.PC += 2;
	if(g_registers.FLAGS.Z == 0) {
		mem_write8(g_registers.SP-1, (g_registers.PC >> 8) & 0xFF);
		mem_write8(g_registers.SP-2, g_registers.PC & 0xFF);
		g_registers.PC = absolute;
		g_registers.SP -= 2;
		return 24;
	}
	return 12;
}

static int _cpu_call_nc_a16(void)
{
	g_registers.PC += 1;
	a16 absolute = mem_read16(g_registers.PC);
	g_registers.PC += 2;
	if(g_registers.FLAGS.C == 0) {
		mem_write8(g_registers.SP-1, (g_registers.PC >> 8) & 0xFF);
		mem_write8(g_registers.SP-2, g_registers.PC & 0xFF);
		g_registers.PC = absolute;
		g_registers.SP -= 2;
		return 24;
	}
	return 12;
}

static int _cpu_call_z_a16(void)
{
	g_registers.PC += 1;
	a16 absolute = mem_read16(g_registers.PC);
	g_registers.PC += 2;
	if(g_registers.FLAGS.Z == 1) {
		mem_write8(g_registers.SP-1, (g_registers.PC >> 8) & 0xFF);
		mem_write8(g_registers.SP-2, g_registers.PC & 0xFF);
		g_registers.PC = absolute;
		g_registers.SP -= 2;
		return 24;
	}
	return 12;
}

static int _cpu_call_c_a16(void)
{
	g_registers.PC += 1;
	a16 absolute = mem_read16(g_registers.PC);
	g_registers.PC += 2;
	if(g_registers.FLAGS.C == 1) {
		mem_write8(g_registers.SP-1, (g_registers.PC >> 8) & 0xFF);
		mem_write8(g_registers.SP-2, g_registers.PC & 0xFF);
		g_registers.PC = absolute;
		g_registers.SP -= 2;
		return 24;
	}
	return 12;
}

static int _cpu_call_a16(void)
{
	g_registers.PC += 1;
	a16 absolute = mem_read16(g_registers.PC);
	g_registers.PC += 2;
	mem_write8(g_registers.SP-1, (g_registers.PC >> 8) & 0xFF);
	mem_write8(g_registers.SP-2, g_registers.PC & 0xFF);
	g_registers.PC = absolute;
	g_registers.SP -= 2;
	return 24;
}

// Return instructions
//========================================
static int _cpu_ret_nz(void)
{
	g_registers.PC += 1;
	if(g_registers.FLAGS.Z == 0) {
		a16 addr = 0x0000;
		addr = mem_read8(g_registers.SP);
		addr += (mem_read8(g_registers.SP+1) << 8);
		g_registers.PC = addr;
		g_registers.SP += 2;
		return 20;
	}
	return 8;
}

static int _cpu_ret_nc(void)
{
	g_registers.PC += 1;
	if(g_registers.FLAGS.C == 0) {
		a16 addr = 0x0000;
		addr = mem_read8(g_registers.SP);
		addr += (mem_read8(g_registers.SP+1) << 8);
		g_registers.PC = addr;
		g_registers.SP += 2;
		return 20;
	}
	return 8;
}

static int _cpu_ret_z(void)
{
	g_registers.PC += 1;
	if(g_registers.FLAGS.Z == 1) {
		a16 addr = 0x0000;
		addr = mem_read8(g_registers.SP);
		addr += (mem_read8(g_registers.SP+1) << 8);
		g_registers.PC = addr;
		g_registers.SP += 2;
		return 20;
	}
	return 8;
}

static int _cpu_ret_c(void)
{
	g_registers.PC += 1;
	if(g_registers.FLAGS.C == 1) {
		a16 addr = 0x0000;
		addr = mem_read8(g_registers.SP);
		addr += (mem_read8(g_registers.SP+1) << 8);
		g_registers.PC = addr;
		g_registers.SP += 2;
		return 20;
	}
	return 8;
}

static int _cpu_ret(void)
{
	g_registers.PC += 1;
	a16 addr = 0x0000;
	addr = mem_read8(g_registers.SP);
	addr += (mem_read8(g_registers.SP+1) << 8);
	g_registers.PC = addr;
	g_registers.SP += 2;
	return 16;
}

static int _cpu_reti(void)
{
	g_registers.PC += 1;
	a16 addr = 0x0000;
	addr = mem_read8(g_registers.SP);
	addr += (mem_read8(g_registers.SP+1) << 8);
	g_registers.PC = addr;
	g_registers.SP += 2;
	ints_set_ime();
	return 16;
}

// Return instructions
//========================================
static int _cpu_rst_00H(void)
{
	g_registers.PC += 1;
	mem_write8(g_registers.SP-1, (g_registers.PC >> 8) & 0xFF);
	mem_write8(g_registers.SP-2, g_registers.PC & 0xFF);
	g_registers.PC = 0x0000;
	g_registers.SP -= 2;
	return 16;
}

static int _cpu_rst_08H(void)
{
	g_registers.PC += 1;
	mem_write8(g_registers.SP-1, (g_registers.PC >> 8) & 0xFF);
	mem_write8(g_registers.SP-2, g_registers.PC & 0xFF);
	g_registers.PC = 0x0008;
	g_registers.SP -= 2;
	return 16;
}

static int _cpu_rst_10H(void)
{
	g_registers.PC += 1;
	mem_write8(g_registers.SP-1, (g_registers.PC >> 8) & 0xFF);
	mem_write8(g_registers.SP-2, g_registers.PC & 0xFF);
	g_registers.PC = 0x0010;
	g_registers.SP -= 2;
	return 16;
}

static int _cpu_rst_18H(void)
{
	g_registers.PC += 1;
	mem_write8(g_registers.SP-1, (g_registers.PC >> 8) & 0xFF);
	mem_write8(g_registers.SP-2, g_registers.PC & 0xFF);
	g_registers.PC = 0x0018;
	g_registers.SP -= 2;
	return 16;
}

static int _cpu_rst_20H(void)
{
	g_registers.PC += 1;
	mem_write8(g_registers.SP-1, (g_registers.PC >> 8) & 0xFF);
	mem_write8(g_registers.SP-2, g_registers.PC & 0xFF);
	g_registers.PC = 0x0020;
	g_registers.SP -= 2;
	return 16;
}

static int _cpu_rst_28H(void)
{
	g_registers.PC += 1;
	mem_write8(g_registers.SP-1, (g_registers.PC >> 8) & 0xFF);
	mem_write8(g_registers.SP-2, g_registers.PC & 0xFF);
	g_registers.PC = 0x0028;
	g_registers.SP -= 2;
	return 16;
}

static int _cpu_rst_30H(void)
{
	g_registers.PC += 1;
	mem_write8(g_registers.SP-1, (g_registers.PC >> 8) & 0xFF);
	mem_write8(g_registers.SP-2, g_registers.PC & 0xFF);
	g_registers.PC = 0x0030;
	g_registers.SP -= 2;
	return 16;
}

static int _cpu_rst_38H(void)
{
	g_registers.PC += 1;
	mem_write8(g_registers.SP-1, (g_registers.PC >> 8) & 0xFF);
	mem_write8(g_registers.SP-2, g_registers.PC & 0xFF);
	g_registers.PC = 0x0038;
	g_registers.SP -= 2;
	return 16;
}

// 8 bit ADD instructions
//================================================

static int _cpu_add_a_b(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.B;
	g_registers.A = left + right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left, right);
	g_registers.FLAGS.C = _CPU_IS_CARRY(left, right);
	return 4;
}

static int _cpu_add_a_c(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.C;
	g_registers.A = left + right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left, right);
	g_registers.FLAGS.C = _CPU_IS_CARRY(left, right);
	return 4;
}

static int _cpu_add_a_d(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.D;
	g_registers.A = left + right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left, right);
	g_registers.FLAGS.C = _CPU_IS_CARRY(left, right);
	return 4;
}

static int _cpu_add_a_e(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.E;
	g_registers.A = left + right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left, right);
	g_registers.FLAGS.C = _CPU_IS_CARRY(left, right);
	return 4;
}

static int _cpu_add_a_h(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.H;
	g_registers.A = left + right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left, right);
	g_registers.FLAGS.C = _CPU_IS_CARRY(left, right);
	return 4;
}

static int _cpu_add_a_l(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.L;
	g_registers.A = left + right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left, right);
	g_registers.FLAGS.C = _CPU_IS_CARRY(left, right);
	return 4;
}

static int _cpu_add_a_imm_hl(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = mem_read8(g_registers.HL);
	g_registers.A = left + right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left, right);
	g_registers.FLAGS.C = _CPU_IS_CARRY(left, right);
	return 8;
}

static int _cpu_add_a_a(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.A;
	g_registers.A = left + right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left, right);
	g_registers.FLAGS.C = _CPU_IS_CARRY(left, right);
	return 4;
}

static int _cpu_add_a_d8(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	g_registers.A = left + right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left, right);
	g_registers.FLAGS.C = _CPU_IS_CARRY(left, right);
	return 8;
}

// 8 bit ADC instructions
//================================================

static int _cpu_adc_a_b(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.B;
	g_registers.A = left + right + g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY_C(left, right, g_registers.FLAGS.C);
	g_registers.FLAGS.C = _CPU_IS_CARRY_C(left, right, g_registers.FLAGS.C);
	return 4;
}

static int _cpu_adc_a_c(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.C;
	g_registers.A = left + right + g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY_C(left, right, g_registers.FLAGS.C);
	g_registers.FLAGS.C = _CPU_IS_CARRY_C(left, right, g_registers.FLAGS.C);
	return 4;
}

static int _cpu_adc_a_d(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.D;
	g_registers.A = left + right + g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY_C(left, right, g_registers.FLAGS.C);
	g_registers.FLAGS.C = _CPU_IS_CARRY_C(left, right, g_registers.FLAGS.C);
	return 4;
}

static int _cpu_adc_a_e(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.E;
	g_registers.A = left + right + g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY_C(left, right, g_registers.FLAGS.C);
	g_registers.FLAGS.C = _CPU_IS_CARRY_C(left, right, g_registers.FLAGS.C);
	return 4;
}

static int _cpu_adc_a_h(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.H;
	g_registers.A = left + right + g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY_C(left, right, g_registers.FLAGS.C);
	g_registers.FLAGS.C = _CPU_IS_CARRY_C(left, right, g_registers.FLAGS.C);
	return 4;
}

static int _cpu_adc_a_l(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.L;
	g_registers.A = left + right + g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY_C(left, right, g_registers.FLAGS.C);
	g_registers.FLAGS.C = _CPU_IS_CARRY_C(left, right, g_registers.FLAGS.C);
	return 4;
}

static int _cpu_adc_a_imm_hl(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = mem_read8(g_registers.HL);
	g_registers.A = left + right + g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY_C(left, right, g_registers.FLAGS.C);
	g_registers.FLAGS.C = _CPU_IS_CARRY_C(left, right, g_registers.FLAGS.C);
	return 8;
}

static int _cpu_adc_a_a(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.A;
	g_registers.A = left + right + g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY_C(left, right, g_registers.FLAGS.C);
	g_registers.FLAGS.C = _CPU_IS_CARRY_C(left, right, g_registers.FLAGS.C);
	return 4;
}

static int _cpu_adc_a_d8(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	g_registers.A = left + right + g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY_C(left, right, g_registers.FLAGS.C);
	g_registers.FLAGS.C = _CPU_IS_CARRY_C(left, right, g_registers.FLAGS.C);
	return 8;
}


// 8 bit SUB instructions
//================================================

static int _cpu_sub_b(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.B;
	g_registers.A = left - right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	g_registers.FLAGS.C = _CPU_IS_BORROW(left, right);
	return 4;
}

static int _cpu_sub_c(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.C;
	g_registers.A = left - right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	g_registers.FLAGS.C = _CPU_IS_BORROW(left, right);
	return 4;
}

static int _cpu_sub_d(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.D;
	g_registers.A = left - right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	g_registers.FLAGS.C = _CPU_IS_BORROW(left, right);
	return 4;
}

static int _cpu_sub_e(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.E;
	g_registers.A = left - right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	g_registers.FLAGS.C = _CPU_IS_BORROW(left, right);
	return 4;
}

static int _cpu_sub_h(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.H;
	g_registers.A = left - right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	g_registers.FLAGS.C = _CPU_IS_BORROW(left, right);
	return 4;
}

static int _cpu_sub_l(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.L;
	g_registers.A = left - right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	g_registers.FLAGS.C = _CPU_IS_BORROW(left, right);
	return 4;
}

static int _cpu_sub_imm_hl(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = mem_read8(g_registers.HL);
	g_registers.A = left - right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	g_registers.FLAGS.C = _CPU_IS_BORROW(left, right);
	return 8;
}

static int _cpu_sub_a(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.A;
	g_registers.A = left - right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	g_registers.FLAGS.C = _CPU_IS_BORROW(left, right);
	return 4;
}

static int _cpu_sub_d8(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	g_registers.A = left - right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	g_registers.FLAGS.C = _CPU_IS_BORROW(left, right);
	return 8;
}

// 8 bit SBC instructions
//================================================

static int _cpu_sbc_a_b(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.B;
	g_registers.A = left - right - g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW_C(left, right, g_registers.FLAGS.C);
	g_registers.FLAGS.C = _CPU_IS_BORROW_C(left, right, g_registers.FLAGS.C);
	return 4;
}

static int _cpu_sbc_a_c(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.C;
	g_registers.A = left - right - g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW_C(left, right, g_registers.FLAGS.C);
	g_registers.FLAGS.C = _CPU_IS_BORROW_C(left, right, g_registers.FLAGS.C);
	return 4;
}

static int _cpu_sbc_a_d(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.D;
	g_registers.A = left - right - g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW_C(left, right, g_registers.FLAGS.C);
	g_registers.FLAGS.C = _CPU_IS_BORROW_C(left, right, g_registers.FLAGS.C);
	return 4;
}

static int _cpu_sbc_a_e(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.E;
	g_registers.A = left - right - g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW_C(left, right, g_registers.FLAGS.C);
	g_registers.FLAGS.C = _CPU_IS_BORROW_C(left, right, g_registers.FLAGS.C);
	return 4;
}

static int _cpu_sbc_a_h(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.H;
	g_registers.A = left - right - g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW_C(left, right, g_registers.FLAGS.C);
	g_registers.FLAGS.C = _CPU_IS_BORROW_C(left, right, g_registers.FLAGS.C);
	return 4;
}

static int _cpu_sbc_a_l(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.L;
	g_registers.A = left - right - g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW_C(left, right, g_registers.FLAGS.C);
	g_registers.FLAGS.C = _CPU_IS_BORROW_C(left, right, g_registers.FLAGS.C);
	return 4;
}

static int _cpu_sbc_a_imm_hl(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = mem_read8(g_registers.HL);
	g_registers.A = left - right - g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW_C(left, right, g_registers.FLAGS.C);
	g_registers.FLAGS.C = _CPU_IS_BORROW_C(left, right, g_registers.FLAGS.C);
	return 8;
}

static int _cpu_sbc_a_a(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.A;
	g_registers.A = left - right - g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW_C(left, right, g_registers.FLAGS.C);
	g_registers.FLAGS.C = _CPU_IS_BORROW_C(left, right, g_registers.FLAGS.C);
	return 4;
}

static int _cpu_sbc_a_d8(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	g_registers.A = left - right - g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW_C(left, right, g_registers.FLAGS.C);
	g_registers.FLAGS.C = _CPU_IS_BORROW_C(left, right, g_registers.FLAGS.C);
	return 8;
}

// AND instructions
//================================================

static int _cpu_and_b(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A & g_registers.B;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_and_c(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A & g_registers.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_and_d(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A & g_registers.D;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_and_e(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A & g_registers.E;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_and_h(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A & g_registers.H;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_and_l(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A & g_registers.L;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_and_imm_hl(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A & mem_read8(g_registers.HL);
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	g_registers.FLAGS.C = 0;
	return 8;
}

static int _cpu_and_a(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A & g_registers.A;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_and_d8(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A & mem_read8(g_registers.PC);
	g_registers.PC += 1;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	g_registers.FLAGS.C = 0;
	return 8;
}


// XOR instructions
//================================================

static int _cpu_xor_b(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A ^ g_registers.B;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_xor_c(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A ^ g_registers.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_xor_d(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A ^ g_registers.D;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_xor_e(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A ^ g_registers.E;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_xor_h(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A ^ g_registers.H;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_xor_l(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A ^ g_registers.L;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_xor_imm_hl(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A ^ mem_read8(g_registers.HL);
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 8;
}

static int _cpu_xor_a(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A ^ g_registers.A;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_xor_d8(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A ^ mem_read8(g_registers.PC);
	g_registers.PC += 1;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 8;
}


// OR instructions
//================================================

static int _cpu_or_b(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A | g_registers.B;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_or_c(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A | g_registers.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_or_d(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A | g_registers.D;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_or_e(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A | g_registers.E;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_or_h(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A | g_registers.H;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_or_l(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A | g_registers.L;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_or_imm_hl(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A | mem_read8(g_registers.HL);
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 8;
}

static int _cpu_or_a(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A | g_registers.A;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 4;
}

static int _cpu_or_d8(void)
{
	g_registers.PC += 1;
	g_registers.A = g_registers.A | mem_read8(g_registers.PC);
	g_registers.PC += 1;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 8;
}


// CP instructions
//================================================

static int _cpu_cp_b(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.B;
	g_registers.FLAGS.Z = (left - right) == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	g_registers.FLAGS.C = _CPU_IS_BORROW(left, right);
	return 4;
}

static int _cpu_cp_c(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.C;
	g_registers.FLAGS.Z = (left - right) == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	g_registers.FLAGS.C = _CPU_IS_BORROW(left, right);
	return 4;
}

static int _cpu_cp_d(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.D;
	g_registers.FLAGS.Z = (left - right) == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	g_registers.FLAGS.C = _CPU_IS_BORROW(left, right);
	return 4;
}

static int _cpu_cp_e(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.E;
	g_registers.FLAGS.Z = (left - right) == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	g_registers.FLAGS.C = _CPU_IS_BORROW(left, right);
	return 4;
}

static int _cpu_cp_h(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.H;
	g_registers.FLAGS.Z = (left - right) == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	g_registers.FLAGS.C = _CPU_IS_BORROW(left, right);
	return 4;
}

static int _cpu_cp_l(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.L;
	g_registers.FLAGS.Z = (left - right) == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	g_registers.FLAGS.C = _CPU_IS_BORROW(left, right);
	return 4;
}

static int _cpu_cp_imm_hl(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = mem_read8(g_registers.HL);
	g_registers.FLAGS.Z = (left - right) == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	g_registers.FLAGS.C = _CPU_IS_BORROW(left, right);
	return 8;
}

static int _cpu_cp_a(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = g_registers.A;
	g_registers.FLAGS.Z = (left - right) == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	g_registers.FLAGS.C = _CPU_IS_BORROW(left, right);
	return 4;
}

static int _cpu_cp_d8(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (left - right) == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	g_registers.FLAGS.C = _CPU_IS_BORROW(left, right);
	return 8;
}

// 8-bit INC instructions
//================================================

static int _cpu_inc_b(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.B;
	d8 right = 0x01;
	g_registers.B = left + right;
	g_registers.FLAGS.Z = g_registers.B == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left, right);
	return 4;
}

static int _cpu_inc_c(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.C;
	d8 right = 0x01;
	g_registers.C = left + right;
	g_registers.FLAGS.Z = g_registers.C == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left, right);
	return 4;
}

static int _cpu_inc_d(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.D;
	d8 right = 0x01;
	g_registers.D = left + right;
	g_registers.FLAGS.Z = g_registers.D == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left, right);
	return 4;
}

static int _cpu_inc_e(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.E;
	d8 right = 0x01;
	g_registers.E = left + right;
	g_registers.FLAGS.Z = g_registers.E == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left, right);
	return 4;
}

static int _cpu_inc_h(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.H;
	d8 right = 0x01;
	g_registers.H = left + right;
	g_registers.FLAGS.Z = g_registers.H == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left, right);
	return 4;
}

static int _cpu_inc_l(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.L;
	d8 right = 0x01;
	g_registers.L = left + right;
	g_registers.FLAGS.Z = g_registers.L == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left, right);
	return 4;
}

static int _cpu_inc_imm_hl(void)
{
	g_registers.PC += 1;
	d8 left = mem_read8(g_registers.HL);
	d8 right = 0x01;
	d8 temp = left + right;
	mem_write8(g_registers.HL, temp);
	g_registers.FLAGS.Z = temp == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left, right);
	return 12;
}

static int _cpu_inc_a(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = 0x01;
	g_registers.A = left + right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left, right);
	return 4;
}

// 8-bit DEC instructions
//================================================

static int _cpu_dec_b(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.B;
	d8 right = 0x01;
	g_registers.B = left - right;
	g_registers.FLAGS.Z = g_registers.B == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	return 4;
}

static int _cpu_dec_c(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.C;
	d8 right = 0x01;
	g_registers.C = left - right;
	g_registers.FLAGS.Z = g_registers.C == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	return 4;
}

static int _cpu_dec_d(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.D;
	d8 right = 0x01;
	g_registers.D = left - right;
	g_registers.FLAGS.Z = g_registers.D == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	return 4;
}

static int _cpu_dec_e(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.E;
	d8 right = 0x01;
	g_registers.E = left - right;
	g_registers.FLAGS.Z = g_registers.E == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	return 4;
}

static int _cpu_dec_h(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.H;
	d8 right = 0x01;
	g_registers.H = left - right;
	g_registers.FLAGS.Z = g_registers.H == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	return 4;
}

static int _cpu_dec_l(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.L;
	d8 right = 0x01;
	g_registers.L = left - right;
	g_registers.FLAGS.Z = g_registers.L == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	return 4;
}

static int _cpu_dec_imm_hl(void)
{
	g_registers.PC += 1;
	d8 left = mem_read8(g_registers.HL);
	d8 right = 0x01;
	d8 temp = left - right;
	mem_write8(g_registers.HL, temp);
	g_registers.FLAGS.Z = temp == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	return 12;
}

static int _cpu_dec_a(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.A;
	d8 right = 0x01;
	g_registers.A = left - right;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = _CPU_IS_HALF_BORROW(left, right);
	return 4;
}


//16-bit INC instructions
//================================================

static int _cpu_inc_bc(void)
{
	g_registers.PC += 1;
	g_registers.BC += 1;
	return 8;
}

static int _cpu_inc_de(void)
{
	g_registers.PC += 1;
	g_registers.DE += 1;
	return 8;
}

static int _cpu_inc_hl(void)
{
	g_registers.PC += 1;
	g_registers.HL += 1;
	return 8;
}

static int _cpu_inc_sp(void)
{
	g_registers.PC += 1;
	g_registers.SP += 1;
	return 8;
}

//16-bit DEC instructions
//================================================

static int _cpu_dec_bc(void)
{
	g_registers.PC += 1;
	g_registers.BC -= 1;
	return 8;
}

static int _cpu_dec_de(void)
{
	g_registers.PC += 1;
	g_registers.DE -= 1;
	return 8;
}

static int _cpu_dec_hl(void)
{
	g_registers.PC += 1;
	g_registers.HL -= 1;
	return 8;
}

static int _cpu_dec_sp(void)
{
	g_registers.PC += 1;
	g_registers.SP -= 1;
	return 8;
}

// 8-bit primary arithmethic and logical
//================================================

static int _cpu_daa(void)
{
	g_registers.PC += 1;
	if(!g_registers.FLAGS.N) {
		// After BCD addition
		if(g_registers.FLAGS.C || g_registers.A > 0x99) {
			g_registers.A += 0x60;
			g_registers.FLAGS.C = 1;
		}
		if(g_registers.FLAGS.H || (g_registers.A & 0x0F) > 0x09) {
			g_registers.A += 0x06;
		}
	} else {
		// After BCD subtraction
		if(g_registers.FLAGS.C) {
			g_registers.A -= 0x60;
		}
		if(g_registers.FLAGS.H) {
			g_registers.A -= 0x06;
		}
	}
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.H = 0;
	return 4;
}

static int _cpu_scf(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 1;
	return 4;
}

static int _cpu_ccf(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = ~g_registers.FLAGS.C;
	return 4;
}

static int _cpu_cpl(void)
{
	g_registers.PC += 1;
	g_registers.A = ~g_registers.A;
	return 4;
}

// 16-bit ADD instructions
//================================================

static int _cpu_add_hl_bc(void)
{
	g_registers.PC += 1;
	d16 left = g_registers.HL;
	d16 right = g_registers.BC;
	g_registers.HL = left + right;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY16(left, right);
	g_registers.FLAGS.C = _CPU_IS_CARRY16(left, right);
	return 8;
}

static int _cpu_add_hl_de(void)
{
	g_registers.PC += 1;
	d16 left = g_registers.HL;
	d16 right = g_registers.DE;
	g_registers.HL = left + right;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY16(left, right);
	g_registers.FLAGS.C = _CPU_IS_CARRY16(left, right);
	return 8;
}

static int _cpu_add_hl_hl(void)
{
	g_registers.PC += 1;
	d16 left = g_registers.HL;
	d16 right = g_registers.HL;
	g_registers.HL = left + right;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY16(left, right);
	g_registers.FLAGS.C = _CPU_IS_CARRY16(left, right);
	return 8;
}

static int _cpu_add_hl_sp(void)
{
	g_registers.PC += 1;
	d16 left = g_registers.HL;
	d16 right = g_registers.SP;
	g_registers.HL = left + right;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY16(left, right);
	g_registers.FLAGS.C = _CPU_IS_CARRY16(left, right);
	return 8;
}

static int _cpu_add_sp_r8(void)
{
	g_registers.PC += 1;
	d16 left = g_registers.SP;
	r8 right = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	g_registers.SP = left + right;
	g_registers.FLAGS.Z = 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left, (d8)right);
	g_registers.FLAGS.C = _CPU_IS_CARRY(left, (d8)right);
	return 16;
}

// 8-bit rotation/shifts and bit instructions
//================================================


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

static int _cpu_rla(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp = g_registers.FLAGS.C;
	g_registers.FLAGS.C = (g_registers.A & 0x80) != 0;
	g_registers.A <<= 1;
	g_registers.A |= temp;
	return 4;
}

static int _cpu_rrca(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.A & 0x01) != 0;
	g_registers.A >>= 1;
	g_registers.A |= g_registers.FLAGS.C << 7;
	return 4;
}

static int _cpu_rra(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp = g_registers.FLAGS.C;
	g_registers.FLAGS.C = (g_registers.A & 0x01) != 0;
	g_registers.A >>= 1;
	g_registers.A |= temp << 7;
	return 4;
}

static int _cpu_rlc_b(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.B & 0x80) != 0;
	g_registers.B <<= 1;
	g_registers.B |= g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.B == 0;
	return 8;
}

static int _cpu_rlc_c(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.C & 0x80) != 0;
	g_registers.C <<= 1;
	g_registers.C |= g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.C == 0;
	return 8;
}

static int _cpu_rlc_d(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.D & 0x80) != 0;
	g_registers.D <<= 1;
	g_registers.D |= g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.D == 0;
	return 8;
}

static int _cpu_rlc_e(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.E & 0x80) != 0;
	g_registers.E <<= 1;
	g_registers.E |= g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.E == 0;
	return 8;
}

static int _cpu_rlc_h(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.H & 0x80) != 0;
	g_registers.H <<= 1;
	g_registers.H |= g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.H == 0;
	return 8;
}

static int _cpu_rlc_l(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.L & 0x80) != 0;
	g_registers.L <<= 1;
	g_registers.L |= g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.L == 0;
	return 8;
}

static int _cpu_rlc_imm_hl(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp = mem_read8(g_registers.HL);
	g_registers.FLAGS.C = (temp & 0x80) != 0;
	temp <<= 1;
	temp |= g_registers.FLAGS.C;
	g_registers.FLAGS.Z = temp == 0;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_rlc_a(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.A & 0x80) != 0;
	g_registers.A <<= 1;
	g_registers.A |= g_registers.FLAGS.C;
	g_registers.FLAGS.Z = g_registers.A == 0;
	return 8;
}

static int _cpu_rrc_b(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.B & 0x01) != 0;
	g_registers.B >>= 1;
	g_registers.B |= g_registers.FLAGS.C << 7;
	g_registers.FLAGS.Z = g_registers.B == 0;
	return 8;
}

static int _cpu_rrc_c(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.C & 0x01) != 0;
	g_registers.C >>= 1;
	g_registers.C |= g_registers.FLAGS.C << 7;
	g_registers.FLAGS.Z = g_registers.C == 0;
	return 8;
}

static int _cpu_rrc_d(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.D & 0x01) != 0;
	g_registers.D >>= 1;
	g_registers.D |= g_registers.FLAGS.C << 7;
	g_registers.FLAGS.Z = g_registers.D == 0;
	return 8;
}

static int _cpu_rrc_e(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.E & 0x01) != 0;
	g_registers.E >>= 1;
	g_registers.E |= g_registers.FLAGS.C << 7;
	g_registers.FLAGS.Z = g_registers.E == 0;
	return 8;
}

static int _cpu_rrc_h(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.H & 0x01) != 0;
	g_registers.H >>= 1;
	g_registers.H |= g_registers.FLAGS.C << 7;
	g_registers.FLAGS.Z = g_registers.H == 0;
	return 8;
}

static int _cpu_rrc_l(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.L & 0x01) != 0;
	g_registers.L >>= 1;
	g_registers.L |= g_registers.FLAGS.C << 7;
	g_registers.FLAGS.Z = g_registers.L == 0;
	return 8;
}

static int _cpu_rrc_imm_hl(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp = mem_read8(g_registers.HL);
	g_registers.FLAGS.C = (temp & 0x01) != 0;
	temp >>= 1;
	temp |= g_registers.FLAGS.C << 7;
	g_registers.FLAGS.Z = temp == 0;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_rrc_a(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.A & 0x01) != 0;
	g_registers.A >>= 1;
	g_registers.A |= g_registers.FLAGS.C << 7;
	g_registers.FLAGS.Z = g_registers.A == 0;
	return 8;
}

static int _cpu_rl_b(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp_c = g_registers.FLAGS.C;
	g_registers.FLAGS.C = (g_registers.B & 0x80) != 0;
	g_registers.B <<= 1;
	g_registers.B |= temp_c;
	g_registers.FLAGS.Z = g_registers.B == 0;
	return 8;
}

static int _cpu_rl_c(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp_c = g_registers.FLAGS.C;
	g_registers.FLAGS.C = (g_registers.C & 0x80) != 0;
	g_registers.C <<= 1;
	g_registers.C |= temp_c;
	g_registers.FLAGS.Z = g_registers.C == 0;
	return 8;
}

static int _cpu_rl_d(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp_c = g_registers.FLAGS.C;
	g_registers.FLAGS.C = (g_registers.D & 0x80) != 0;
	g_registers.D <<= 1;
	g_registers.D |= temp_c;
	g_registers.FLAGS.Z = g_registers.D == 0;
	return 8;
}

static int _cpu_rl_e(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp_c = g_registers.FLAGS.C;
	g_registers.FLAGS.C = (g_registers.E & 0x80) != 0;
	g_registers.E <<= 1;
	g_registers.E |= temp_c;
	g_registers.FLAGS.Z = g_registers.E == 0;
	return 8;
}

static int _cpu_rl_h(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp_c = g_registers.FLAGS.C;
	g_registers.FLAGS.C = (g_registers.H & 0x80) != 0;
	g_registers.H <<= 1;
	g_registers.H |= temp_c;
	g_registers.FLAGS.Z = g_registers.H == 0;
	return 8;
}

static int _cpu_rl_l(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp_c = g_registers.FLAGS.C;
	g_registers.FLAGS.C = (g_registers.L & 0x80) != 0;
	g_registers.L <<= 1;
	g_registers.L |= temp_c;
	g_registers.FLAGS.Z = g_registers.L == 0;
	return 8;
}

static int _cpu_rl_imm_hl(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp = mem_read8(g_registers.HL);
	d8 temp_c = g_registers.FLAGS.C;
	g_registers.FLAGS.C = (temp & 0x80) != 0;
	temp <<= 1;
	temp |= temp_c;
	g_registers.FLAGS.Z = temp == 0;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_rl_a(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp_c = g_registers.FLAGS.C;
	g_registers.FLAGS.C = (g_registers.A & 0x80) != 0;
	g_registers.A <<= 1;
	g_registers.A |= temp_c;
	g_registers.FLAGS.Z = g_registers.A == 0;
	return 8;
}

static int _cpu_rr_b(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp_c = g_registers.FLAGS.C;
	g_registers.FLAGS.C = (g_registers.B & 0x01) != 0;
	g_registers.B >>= 1;
	g_registers.B |= temp_c << 7;
	g_registers.FLAGS.Z = g_registers.B == 0;
	return 8;
}

static int _cpu_rr_c(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp_c = g_registers.FLAGS.C;
	g_registers.FLAGS.C = (g_registers.C & 0x01) != 0;
	g_registers.C >>= 1;
	g_registers.C |= temp_c << 7;
	g_registers.FLAGS.Z = g_registers.C == 0;
	return 8;
}

static int _cpu_rr_d(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp_c = g_registers.FLAGS.C;
	g_registers.FLAGS.C = (g_registers.D & 0x01) != 0;
	g_registers.D >>= 1;
	g_registers.D |= temp_c << 7;
	g_registers.FLAGS.Z = g_registers.D == 0;
	return 8;
}

static int _cpu_rr_e(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp_c = g_registers.FLAGS.C;
	g_registers.FLAGS.C = (g_registers.E & 0x01) != 0;
	g_registers.E >>= 1;
	g_registers.E |= temp_c << 7;
	g_registers.FLAGS.Z = g_registers.E == 0;
	return 8;
}

static int _cpu_rr_h(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp_c = g_registers.FLAGS.C;
	g_registers.FLAGS.C = (g_registers.H & 0x01) != 0;
	g_registers.H >>= 1;
	g_registers.H |= temp_c << 7;
	g_registers.FLAGS.Z = g_registers.H == 0;
	return 8;
}

static int _cpu_rr_l(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp_c = g_registers.FLAGS.C;
	g_registers.FLAGS.C = (g_registers.L & 0x01) != 0;
	g_registers.L >>= 1;
	g_registers.L |= temp_c << 7;
	g_registers.FLAGS.Z = g_registers.L == 0;
	return 8;
}

static int _cpu_rr_imm_hl(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp = mem_read8(g_registers.HL);
	d8 temp_c = g_registers.FLAGS.C;
	g_registers.FLAGS.C = (temp & 0x01) != 0;
	temp >>= 1;
	temp |= temp_c << 7;
	g_registers.FLAGS.Z = temp == 0;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_rr_a(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp_c = g_registers.FLAGS.C;
	g_registers.FLAGS.C = (g_registers.A & 0x01) != 0;
	g_registers.A >>= 1;
	g_registers.A |= temp_c << 7;
	g_registers.FLAGS.Z = g_registers.A == 0;
	return 8;
}

static int _cpu_sla_b(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.B & 0x80) != 0;
	g_registers.B <<= 1;
	g_registers.FLAGS.Z = g_registers.B == 0;
	return 8;
}

static int _cpu_sla_c(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.C & 0x80) != 0;
	g_registers.C <<= 1;
	g_registers.FLAGS.Z = g_registers.C == 0;
	return 8;
}

static int _cpu_sla_d(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.D & 0x80) != 0;
	g_registers.D <<= 1;
	g_registers.FLAGS.Z = g_registers.D == 0;
	return 8;
}

static int _cpu_sla_e(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.E & 0x80) != 0;
	g_registers.E <<= 1;
	g_registers.FLAGS.Z = g_registers.E == 0;
	return 8;
}

static int _cpu_sla_h(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.H & 0x80) != 0;
	g_registers.H <<= 1;
	g_registers.FLAGS.Z = g_registers.H == 0;
	return 8;
}

static int _cpu_sla_l(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.L & 0x80) != 0;
	g_registers.L <<= 1;
	g_registers.FLAGS.Z = g_registers.L == 0;
	return 8;
}

static int _cpu_sla_imm_hl(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp = mem_read8(g_registers.HL);
	g_registers.FLAGS.C = (temp & 0x80) != 0;
	temp <<= 1;
	g_registers.FLAGS.Z = temp == 0;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_sla_a(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.A & 0x80) != 0;
	g_registers.A <<= 1;
	g_registers.FLAGS.Z = g_registers.A == 0;
	return 8;
}

static int _cpu_sra_b(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.B & 0x01) != 0;
	d8 old_bit = g_registers.B & 0x80;
	g_registers.B >>= 1;
	g_registers.B |= old_bit;
	g_registers.FLAGS.Z = g_registers.B == 0;
	return 8;
}

static int _cpu_sra_c(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.C & 0x01) != 0;
	d8 old_bit = g_registers.C & 0x80;
	g_registers.C >>= 1;
	g_registers.C |= old_bit;
	g_registers.FLAGS.Z = g_registers.C == 0;
	return 8;
}

static int _cpu_sra_d(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.D & 0x01) != 0;
	d8 old_bit = g_registers.D & 0x80;
	g_registers.D >>= 1;
	g_registers.D |= old_bit;
	g_registers.FLAGS.Z = g_registers.D == 0;
	return 8;
}

static int _cpu_sra_e(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.E & 0x01) != 0;
	d8 old_bit = g_registers.E & 0x80;
	g_registers.E >>= 1;
	g_registers.E |= old_bit;
	g_registers.FLAGS.Z = g_registers.E == 0;
	return 8;
}

static int _cpu_sra_h(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.H & 0x01) != 0;
	d8 old_bit = g_registers.H & 0x80;
	g_registers.H >>= 1;
	g_registers.H |= old_bit;
	g_registers.FLAGS.Z = g_registers.H == 0;
	return 8;
}

static int _cpu_sra_l(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.L & 0x01) != 0;
	d8 old_bit = g_registers.L & 0x80;
	g_registers.L >>= 1;
	g_registers.L |= old_bit;
	g_registers.FLAGS.Z = g_registers.L == 0;
	return 8;
}

static int _cpu_sra_imm_hl(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	d8 temp = mem_read8(g_registers.HL);
	g_registers.FLAGS.C = (temp & 0x01) != 0;
	d8 old_bit = temp & 0x80;
	temp >>= 1;
	temp |= old_bit;
	g_registers.FLAGS.Z = temp == 0;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_sra_a(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = (g_registers.A & 0x01) != 0;
	d8 old_bit = g_registers.A & 0x80;
	g_registers.A >>= 1;
	g_registers.A |= old_bit;
	g_registers.FLAGS.Z = g_registers.A == 0;
	return 8;
}

static int _cpu_swap_b(void)
{
	g_registers.PC += 1;
	d8 upper_nibble = (g_registers.B & 0xF0) >> 4;
	d8 lower_nibble = g_registers.B & 0x0F;
	g_registers.B = (lower_nibble << 4) | upper_nibble;
	g_registers.FLAGS.Z = g_registers.B == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 8;
}

static int _cpu_swap_c(void)
{
	g_registers.PC += 1;
	d8 upper_nibble = (g_registers.C & 0xF0) >> 4;
	d8 lower_nibble = g_registers.C & 0x0F;
	g_registers.C = (lower_nibble << 4) | upper_nibble;
	g_registers.FLAGS.Z = g_registers.C == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 8;
}

static int _cpu_swap_d(void)
{
	g_registers.PC += 1;
	d8 upper_nibble = (g_registers.D & 0xF0) >> 4;
	d8 lower_nibble = g_registers.D & 0x0F;
	g_registers.D = (lower_nibble << 4) | upper_nibble;
	g_registers.FLAGS.Z = g_registers.D == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 8;
}

static int _cpu_swap_e(void)
{
	g_registers.PC += 1;
	d8 upper_nibble = (g_registers.E & 0xF0) >> 4;
	d8 lower_nibble = g_registers.E & 0x0F;
	g_registers.E = (lower_nibble << 4) | upper_nibble;
	g_registers.FLAGS.Z = g_registers.E == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 8;
}

static int _cpu_swap_h(void)
{
	g_registers.PC += 1;
	d8 upper_nibble = (g_registers.H & 0xF0) >> 4;
	d8 lower_nibble = g_registers.H & 0x0F;
	g_registers.H = (lower_nibble << 4) | upper_nibble;
	g_registers.FLAGS.Z = g_registers.H == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 8;
}

static int _cpu_swap_l(void)
{
	g_registers.PC += 1;
	d8 upper_nibble = (g_registers.L & 0xF0) >> 4;
	d8 lower_nibble = g_registers.L & 0x0F;
	g_registers.L = (lower_nibble << 4) | upper_nibble;
	g_registers.FLAGS.Z = g_registers.L == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 8;
}

static int _cpu_swap_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	d8 upper_nibble = (temp & 0xF0) >> 4;
	d8 lower_nibble = temp & 0x0F;
	temp = (lower_nibble << 4) | upper_nibble;
	g_registers.FLAGS.Z = temp == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_swap_a(void)
{
	g_registers.PC += 1;
	d8 upper_nibble = (g_registers.A & 0xF0) >> 4;
	d8 lower_nibble = g_registers.A & 0x0F;
	g_registers.A = (lower_nibble << 4) | upper_nibble;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	g_registers.FLAGS.C = 0;
	return 8;
}

static int _cpu_srl_b(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.C = (g_registers.B & 0x01) != 0;
	g_registers.B >>= 1;
	g_registers.FLAGS.Z = g_registers.B == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	return 8;
}

static int _cpu_srl_c(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.C = (g_registers.C & 0x01) != 0;
	g_registers.C >>= 1;
	g_registers.FLAGS.Z = g_registers.C == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	return 8;
}

static int _cpu_srl_d(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.C = (g_registers.D & 0x01) != 0;
	g_registers.D >>= 1;
	g_registers.FLAGS.Z = g_registers.D == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	return 8;
}

static int _cpu_srl_e(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.C = (g_registers.E & 0x01) != 0;
	g_registers.E >>= 1;
	g_registers.FLAGS.Z = g_registers.E == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	return 8;
}

static int _cpu_srl_h(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.C = (g_registers.H & 0x01) != 0;
	g_registers.H >>= 1;
	g_registers.FLAGS.Z = g_registers.H == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	return 8;
}

static int _cpu_srl_l(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.C = (g_registers.L & 0x01) != 0;
	g_registers.L >>= 1;
	g_registers.FLAGS.Z = g_registers.L == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	return 8;
}

static int _cpu_srl_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	g_registers.FLAGS.C = (temp & 0x01) != 0;
	temp >>= 1;
	g_registers.FLAGS.Z = temp == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_srl_a(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.C = (g_registers.A & 0x01) != 0;
	g_registers.A >>= 1;
	g_registers.FLAGS.Z = g_registers.A == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 0;
	return 8;
}

static int _cpu_bit_0_b(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.B & 0x01) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_0_c(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.C & 0x01) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_0_d(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.D & 0x01) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_0_e(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.E & 0x01) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_0_h(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.H & 0x01) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_0_l(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.L & 0x01) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_0_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	g_registers.FLAGS.Z = (temp & 0x01) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 16;
}

static int _cpu_bit_0_a(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.A & 0x01) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_1_b(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.B & 0x02) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_1_c(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.C & 0x02) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_1_d(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.D & 0x02) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_1_e(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.E & 0x02) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_1_h(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.H & 0x02) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_1_l(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.L & 0x02) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_1_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	g_registers.FLAGS.Z = (temp & 0x02) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 16;
}

static int _cpu_bit_1_a(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.A & 0x02) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_2_b(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.B & 0x04) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_2_c(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.C & 0x04) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_2_d(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.D & 0x04) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_2_e(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.E & 0x04) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_2_h(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.H & 0x04) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_2_l(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.L & 0x04) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_2_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	g_registers.FLAGS.Z = (temp & 0x04) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 16;
}

static int _cpu_bit_2_a(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.A & 0x04) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_3_b(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.B & 0x08) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_3_c(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.C & 0x08) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_3_d(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.D & 0x08) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_3_e(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.E & 0x08) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_3_h(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.H & 0x08) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_3_l(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.L & 0x08) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_3_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	g_registers.FLAGS.Z = (temp & 0x08) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 16;
}

static int _cpu_bit_3_a(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.A & 0x08) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_4_b(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.B & 0x10) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_4_c(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.C & 0x10) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_4_d(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.D & 0x10) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_4_e(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.E & 0x10) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_4_h(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.H & 0x10) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_4_l(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.L & 0x10) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_4_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	g_registers.FLAGS.Z = (temp & 0x10) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 16;
}

static int _cpu_bit_4_a(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.A & 0x10) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_5_b(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.B & 0x20) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_5_c(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.C & 0x20) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_5_d(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.D & 0x20) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_5_e(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.E & 0x20) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_5_h(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.H & 0x20) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_5_l(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.L & 0x20) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_5_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	g_registers.FLAGS.Z = (temp & 0x20) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 16;
}

static int _cpu_bit_5_a(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.A & 0x20) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_6_b(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.B & 0x40) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_6_c(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.C & 0x40) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_6_d(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.D & 0x40) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_6_e(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.E & 0x40) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_6_h(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.H & 0x40) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_6_l(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.L & 0x40) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_6_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	g_registers.FLAGS.Z = (temp & 0x40) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 16;
}

static int _cpu_bit_6_a(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.A & 0x40) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_7_b(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.B & 0x80) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_7_c(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.C & 0x80) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_7_d(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.D & 0x80) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_7_e(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.E & 0x80) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_7_h(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.H & 0x80) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_7_l(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.L & 0x80) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_bit_7_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	g_registers.FLAGS.Z = (temp & 0x80) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 16;
}

static int _cpu_bit_7_a(void)
{
	g_registers.PC += 1;
	g_registers.FLAGS.Z = (g_registers.A & 0x80) == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = 1;
	return 8;
}

static int _cpu_res_0_b(void)
{
	g_registers.PC += 1;
	g_registers.B &= ~0x01;
	return 8;
}

static int _cpu_res_0_c(void)
{
	g_registers.PC += 1;
	g_registers.C &= ~0x01;
	return 8;
}

static int _cpu_res_0_d(void)
{
	g_registers.PC += 1;
	g_registers.D &= ~0x01;
	return 8;
}

static int _cpu_res_0_e(void)
{
	g_registers.PC += 1;
	g_registers.E &= ~0x01;
	return 8;
}

static int _cpu_res_0_h(void)
{
	g_registers.PC += 1;
	g_registers.H &= ~0x01;
	return 8;
}

static int _cpu_res_0_l(void)
{
	g_registers.PC += 1;
	g_registers.L &= ~0x01;
	return 8;
}

static int _cpu_res_0_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	temp &= ~0x01;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_res_0_a(void)
{
	g_registers.PC += 1;
	g_registers.A &= ~0x01;
	return 8;
}

static int _cpu_res_1_b(void)
{
	g_registers.PC += 1;
	g_registers.B &= ~0x02;
	return 8;
}

static int _cpu_res_1_c(void)
{
	g_registers.PC += 1;
	g_registers.C &= ~0x02;
	return 8;
}

static int _cpu_res_1_d(void)
{
	g_registers.PC += 1;
	g_registers.D &= ~0x02;
	return 8;
}

static int _cpu_res_1_e(void)
{
	g_registers.PC += 1;
	g_registers.E &= ~0x02;
	return 8;
}

static int _cpu_res_1_h(void)
{
	g_registers.PC += 1;
	g_registers.H &= ~0x02;
	return 8;
}

static int _cpu_res_1_l(void)
{
	g_registers.PC += 1;
	g_registers.L &= ~0x02;
	return 8;
}

static int _cpu_res_1_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	temp &= ~0x02;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_res_1_a(void)
{
	g_registers.PC += 1;
	g_registers.A &= ~0x02;
	return 8;
}

static int _cpu_res_2_b(void)
{
	g_registers.PC += 1;
	g_registers.B &= ~0x04;
	return 8;
}

static int _cpu_res_2_c(void)
{
	g_registers.PC += 1;
	g_registers.C &= ~0x04;
	return 8;
}

static int _cpu_res_2_d(void)
{
	g_registers.PC += 1;
	g_registers.D &= ~0x04;
	return 8;
}

static int _cpu_res_2_e(void)
{
	g_registers.PC += 1;
	g_registers.E &= ~0x04;
	return 8;
}

static int _cpu_res_2_h(void)
{
	g_registers.PC += 1;
	g_registers.H &= ~0x04;
	return 8;
}

static int _cpu_res_2_l(void)
{
	g_registers.PC += 1;
	g_registers.L &= ~0x04;
	return 8;
}

static int _cpu_res_2_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	temp &= ~0x04;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_res_2_a(void)
{
	g_registers.PC += 1;
	g_registers.A &= ~0x04;
	return 8;
}

static int _cpu_res_3_b(void)
{
	g_registers.PC += 1;
	g_registers.B &= ~0x08;
	return 8;
}

static int _cpu_res_3_c(void)
{
	g_registers.PC += 1;
	g_registers.C &= ~0x08;
	return 8;
}

static int _cpu_res_3_d(void)
{
	g_registers.PC += 1;
	g_registers.D &= ~0x08;
	return 8;
}

static int _cpu_res_3_e(void)
{
	g_registers.PC += 1;
	g_registers.E &= ~0x08;
	return 8;
}

static int _cpu_res_3_h(void)
{
	g_registers.PC += 1;
	g_registers.H &= ~0x08;
	return 8;
}

static int _cpu_res_3_l(void)
{
	g_registers.PC += 1;
	g_registers.L &= ~0x08;
	return 8;
}

static int _cpu_res_3_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	temp &= ~0x08;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_res_3_a(void)
{
	g_registers.PC += 1;
	g_registers.A &= ~0x08;
	return 8;
}

static int _cpu_res_4_b(void)
{
	g_registers.PC += 1;
	g_registers.B &= ~0x10;
	return 8;
}

static int _cpu_res_4_c(void)
{
	g_registers.PC += 1;
	g_registers.C &= ~0x10;
	return 8;
}

static int _cpu_res_4_d(void)
{
	g_registers.PC += 1;
	g_registers.D &= ~0x10;
	return 8;
}

static int _cpu_res_4_e(void)
{
	g_registers.PC += 1;
	g_registers.E &= ~0x10;
	return 8;
}

static int _cpu_res_4_h(void)
{
	g_registers.PC += 1;
	g_registers.H &= ~0x10;
	return 8;
}

static int _cpu_res_4_l(void)
{
	g_registers.PC += 1;
	g_registers.L &= ~0x10;
	return 8;
}

static int _cpu_res_4_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	temp &= ~0x10;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_res_4_a(void)
{
	g_registers.PC += 1;
	g_registers.A &= ~0x10;
	return 8;
}

static int _cpu_res_5_b(void)
{
	g_registers.PC += 1;
	g_registers.B &= ~0x20;
	return 8;
}

static int _cpu_res_5_c(void)
{
	g_registers.PC += 1;
	g_registers.C &= ~0x20;
	return 8;
}

static int _cpu_res_5_d(void)
{
	g_registers.PC += 1;
	g_registers.D &= ~0x20;
	return 8;
}

static int _cpu_res_5_e(void)
{
	g_registers.PC += 1;
	g_registers.E &= ~0x20;
	return 8;
}

static int _cpu_res_5_h(void)
{
	g_registers.PC += 1;
	g_registers.H &= ~0x20;
	return 8;
}

static int _cpu_res_5_l(void)
{
	g_registers.PC += 1;
	g_registers.L &= ~0x20;
	return 8;
}

static int _cpu_res_5_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	temp &= ~0x20;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_res_5_a(void)
{
	g_registers.PC += 1;
	g_registers.A &= ~0x20;
	return 8;
}

static int _cpu_res_6_b(void)
{
	g_registers.PC += 1;
	g_registers.B &= ~0x40;
	return 8;
}

static int _cpu_res_6_c(void)
{
	g_registers.PC += 1;
	g_registers.C &= ~0x40;
	return 8;
}

static int _cpu_res_6_d(void)
{
	g_registers.PC += 1;
	g_registers.D &= ~0x40;
	return 8;
}

static int _cpu_res_6_e(void)
{
	g_registers.PC += 1;
	g_registers.E &= ~0x40;
	return 8;
}

static int _cpu_res_6_h(void)
{
	g_registers.PC += 1;
	g_registers.H &= ~0x40;
	return 8;
}

static int _cpu_res_6_l(void)
{
	g_registers.PC += 1;
	g_registers.L &= ~0x40;
	return 8;
}

static int _cpu_res_6_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	temp &= ~0x40;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_res_6_a(void)
{
	g_registers.PC += 1;
	g_registers.A &= ~0x40;
	return 8;
}

static int _cpu_res_7_b(void)
{
	g_registers.PC += 1;
	g_registers.B &= ~0x80;
	return 8;
}

static int _cpu_res_7_c(void)
{
	g_registers.PC += 1;
	g_registers.C &= ~0x80;
	return 8;
}

static int _cpu_res_7_d(void)
{
	g_registers.PC += 1;
	g_registers.D &= ~0x80;
	return 8;
}

static int _cpu_res_7_e(void)
{
	g_registers.PC += 1;
	g_registers.E &= ~0x80;
	return 8;
}

static int _cpu_res_7_h(void)
{
	g_registers.PC += 1;
	g_registers.H &= ~0x80;
	return 8;
}

static int _cpu_res_7_l(void)
{
	g_registers.PC += 1;
	g_registers.L &= ~0x80;
	return 8;
}

static int _cpu_res_7_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	temp &= ~0x80;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_res_7_a(void)
{
	g_registers.PC += 1;
	g_registers.A &= ~0x80;
	return 8;
}

static int _cpu_set_0_b(void)
{
	g_registers.PC += 1;
	g_registers.B |= 0x01;
	return 8;
}

static int _cpu_set_0_c(void)
{
	g_registers.PC += 1;
	g_registers.C |= 0x01;
	return 8;
}

static int _cpu_set_0_d(void)
{
	g_registers.PC += 1;
	g_registers.D |= 0x01;
	return 8;
}

static int _cpu_set_0_e(void)
{
	g_registers.PC += 1;
	g_registers.E |= 0x01;
	return 8;
}

static int _cpu_set_0_h(void)
{
	g_registers.PC += 1;
	g_registers.H |= 0x01;
	return 8;
}

static int _cpu_set_0_l(void)
{
	g_registers.PC += 1;
	g_registers.L |= 0x01;
	return 8;
}

static int _cpu_set_0_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	temp |= 0x01;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_set_0_a(void)
{
	g_registers.PC += 1;
	g_registers.A |= 0x01;
	return 8;
}

static int _cpu_set_1_b(void)
{
	g_registers.PC += 1;
	g_registers.B |= 0x02;
	return 8;
}

static int _cpu_set_1_c(void)
{
	g_registers.PC += 1;
	g_registers.C |= 0x02;
	return 8;
}

static int _cpu_set_1_d(void)
{
	g_registers.PC += 1;
	g_registers.D |= 0x02;
	return 8;
}

static int _cpu_set_1_e(void)
{
	g_registers.PC += 1;
	g_registers.E |= 0x02;
	return 8;
}

static int _cpu_set_1_h(void)
{
	g_registers.PC += 1;
	g_registers.H |= 0x02;
	return 8;
}

static int _cpu_set_1_l(void)
{
	g_registers.PC += 1;
	g_registers.L |= 0x02;
	return 8;
}

static int _cpu_set_1_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	temp |= 0x02;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_set_1_a(void)
{
	g_registers.PC += 1;
	g_registers.A |= 0x02;
	return 8;
}

static int _cpu_set_2_b(void)
{
	g_registers.PC += 1;
	g_registers.B |= 0x04;
	return 8;
}

static int _cpu_set_2_c(void)
{
	g_registers.PC += 1;
	g_registers.C |= 0x04;
	return 8;
}

static int _cpu_set_2_d(void)
{
	g_registers.PC += 1;
	g_registers.D |= 0x04;
	return 8;
}

static int _cpu_set_2_e(void)
{
	g_registers.PC += 1;
	g_registers.E |= 0x04;
	return 8;
}

static int _cpu_set_2_h(void)
{
	g_registers.PC += 1;
	g_registers.H |= 0x04;
	return 8;
}

static int _cpu_set_2_l(void)
{
	g_registers.PC += 1;
	g_registers.L |= 0x04;
	return 8;
}

static int _cpu_set_2_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	temp |= 0x04;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_set_2_a(void)
{
	g_registers.PC += 1;
	g_registers.A |= 0x04;
	return 8;
}

static int _cpu_set_3_b(void)
{
	g_registers.PC += 1;
	g_registers.B |= 0x08;
	return 8;
}

static int _cpu_set_3_c(void)
{
	g_registers.PC += 1;
	g_registers.C |= 0x08;
	return 8;
}

static int _cpu_set_3_d(void)
{
	g_registers.PC += 1;
	g_registers.D |= 0x08;
	return 8;
}

static int _cpu_set_3_e(void)
{
	g_registers.PC += 1;
	g_registers.E |= 0x08;
	return 8;
}

static int _cpu_set_3_h(void)
{
	g_registers.PC += 1;
	g_registers.H |= 0x08;
	return 8;
}

static int _cpu_set_3_l(void)
{
	g_registers.PC += 1;
	g_registers.L |= 0x08;
	return 8;
}

static int _cpu_set_3_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	temp |= 0x08;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_set_3_a(void)
{
	g_registers.PC += 1;
	g_registers.A |= 0x08;
	return 8;
}

static int _cpu_set_4_b(void)
{
	g_registers.PC += 1;
	g_registers.B |= 0x10;
	return 8;
}

static int _cpu_set_4_c(void)
{
	g_registers.PC += 1;
	g_registers.C |= 0x10;
	return 8;
}

static int _cpu_set_4_d(void)
{
	g_registers.PC += 1;
	g_registers.D |= 0x10;
	return 8;
}

static int _cpu_set_4_e(void)
{
	g_registers.PC += 1;
	g_registers.E |= 0x10;
	return 8;
}

static int _cpu_set_4_h(void)
{
	g_registers.PC += 1;
	g_registers.H |= 0x10;
	return 8;
}

static int _cpu_set_4_l(void)
{
	g_registers.PC += 1;
	g_registers.L |= 0x10;
	return 8;
}

static int _cpu_set_4_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	temp |= 0x10;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_set_4_a(void)
{
	g_registers.PC += 1;
	g_registers.A |= 0x10;
	return 8;
}

static int _cpu_set_5_b(void)
{
	g_registers.PC += 1;
	g_registers.B |= 0x20;
	return 8;
}

static int _cpu_set_5_c(void)
{
	g_registers.PC += 1;
	g_registers.C |= 0x20;
	return 8;
}

static int _cpu_set_5_d(void)
{
	g_registers.PC += 1;
	g_registers.D |= 0x20;
	return 8;
}

static int _cpu_set_5_e(void)
{
	g_registers.PC += 1;
	g_registers.E |= 0x20;
	return 8;
}

static int _cpu_set_5_h(void)
{
	g_registers.PC += 1;
	g_registers.H |= 0x20;
	return 8;
}

static int _cpu_set_5_l(void)
{
	g_registers.PC += 1;
	g_registers.L |= 0x20;
	return 8;
}

static int _cpu_set_5_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	temp |= 0x20;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_set_5_a(void)
{
	g_registers.PC += 1;
	g_registers.A |= 0x20;
	return 8;
}

static int _cpu_set_6_b(void)
{
	g_registers.PC += 1;
	g_registers.B |= 0x40;
	return 8;
}

static int _cpu_set_6_c(void)
{
	g_registers.PC += 1;
	g_registers.C |= 0x40;
	return 8;
}

static int _cpu_set_6_d(void)
{
	g_registers.PC += 1;
	g_registers.D |= 0x40;
	return 8;
}

static int _cpu_set_6_e(void)
{
	g_registers.PC += 1;
	g_registers.E |= 0x40;
	return 8;
}

static int _cpu_set_6_h(void)
{
	g_registers.PC += 1;
	g_registers.H |= 0x40;
	return 8;
}

static int _cpu_set_6_l(void)
{
	g_registers.PC += 1;
	g_registers.L |= 0x40;
	return 8;
}

static int _cpu_set_6_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	temp |= 0x40;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_set_6_a(void)
{
	g_registers.PC += 1;
	g_registers.A |= 0x40;
	return 8;
}

static int _cpu_set_7_b(void)
{
	g_registers.PC += 1;
	g_registers.B |= 0x80;
	return 8;
}

static int _cpu_set_7_c(void)
{
	g_registers.PC += 1;
	g_registers.C |= 0x80;
	return 8;
}

static int _cpu_set_7_d(void)
{
	g_registers.PC += 1;
	g_registers.D |= 0x80;
	return 8;
}

static int _cpu_set_7_e(void)
{
	g_registers.PC += 1;
	g_registers.E |= 0x80;
	return 8;
}

static int _cpu_set_7_h(void)
{
	g_registers.PC += 1;
	g_registers.H |= 0x80;
	return 8;
}

static int _cpu_set_7_l(void)
{
	g_registers.PC += 1;
	g_registers.L |= 0x80;
	return 8;
}

static int _cpu_set_7_imm_hl(void)
{
	g_registers.PC += 1;
	d8 temp = mem_read8(g_registers.HL);
	temp |= 0x80;
	mem_write8(g_registers.HL, temp);
	return 16;
}

static int _cpu_set_7_a(void)
{
	g_registers.PC += 1;
	g_registers.A |= 0x80;
	return 8;
}

void _cpu_debug_console(u16 pc_old)
{
	d8 opcode = mem_read8(pc_old);
	printf("0x%04X\t", pc_old);
	int len = debug_op_length(opcode);
	switch(len) {
	case 1:
		printf("%s" ,debug_op_mnemonic_format(opcode));
		break;
	case 2:
		printf(debug_op_mnemonic_format(opcode), mem_read8(pc_old+1));
		break;
	case 3:
		printf(debug_op_mnemonic_format(opcode), mem_read16(pc_old+1));
		break;
	case 4:
		// CB prefix special print
		printf("CB %s",debug_op_extended_mnemonic_format(mem_read8(pc_old+1)));
	}
	printf("\n");
}


int cpu_single_step(void)
{
	if(g_cpu_stopped) {
		return 1;
	} else if(g_cpu_halted) {
		return 1;
	} else {
		// Fetch
#ifdef DEBUG
		_cpu_debug_console(g_registers.PC);
#endif
		d8 instruction_code = mem_read8(g_registers.PC);
		// Decode & Execute
		int cycles = g_instruction_table[instruction_code]();

		if(g_ime_delay > 0) {
			g_ime_delay -= 1;
		}
		if(g_ime_delay == 0) {
			g_ime_delay = -1;
			g_ime_op == IME_OP_EI ? ints_set_ime() : ints_reset_ime();
		}
		return cycles;
	}
}


void cpu_jump(a16 addr)
{
	g_registers.PC = addr;
}


void cpu_call(a16 addr)
{
	// Push PC onto stack
	cpu_push16(g_registers.PC);
	// Jump to given address
	cpu_jump(addr);
}


void cpu_prepare(void)
{
	g_instruction_table[0x00] = _cpu_nop;
	g_instruction_table[0x01] = _cpu_ld_bc_d16;
	g_instruction_table[0x02] = _cpu_ld_imm_bc_a;
	g_instruction_table[0x03] = _cpu_inc_bc;
	g_instruction_table[0x04] = _cpu_inc_b;
	g_instruction_table[0x05] = _cpu_dec_b;
	g_instruction_table[0x06] = _cpu_ld_b_d8;
	g_instruction_table[0x07] = _cpu_rlca;
	g_instruction_table[0x08] = _cpu_ld_imm_a16_sp;
	g_instruction_table[0x09] = _cpu_add_hl_bc;
	g_instruction_table[0x0A] = _cpu_ld_a_imm_bc;
	g_instruction_table[0x0B] = _cpu_dec_bc;
	g_instruction_table[0x0C] = _cpu_inc_c;
	g_instruction_table[0x0D] = _cpu_dec_c;
	g_instruction_table[0x0E] = _cpu_ld_c_d8;
	g_instruction_table[0x0F] = _cpu_rrca;
	g_instruction_table[0x10] = _cpu_stop;
	g_instruction_table[0x11] = _cpu_ld_de_d16;
	g_instruction_table[0x12] = _cpu_ld_imm_de_a;
	g_instruction_table[0x13] = _cpu_inc_de;
	g_instruction_table[0x14] = _cpu_inc_d;
	g_instruction_table[0x15] = _cpu_dec_d;
	g_instruction_table[0x16] = _cpu_ld_d_d8;
	g_instruction_table[0x17] = _cpu_rla;
	g_instruction_table[0x18] = _cpu_jr_r8;
	g_instruction_table[0x19] = _cpu_add_hl_de;
	g_instruction_table[0x1A] = _cpu_ld_a_imm_de;
	g_instruction_table[0x1B] = _cpu_dec_de;
	g_instruction_table[0x1C] = _cpu_inc_e;
	g_instruction_table[0x1D] = _cpu_dec_e;
	g_instruction_table[0x1E] = _cpu_ld_e_d8;
	g_instruction_table[0x1F] = _cpu_rra;
	g_instruction_table[0x20] = _cpu_jr_nz_r8;
	g_instruction_table[0x21] = _cpu_ld_hl_d16;
	g_instruction_table[0x22] = _cpu_ld_imm_hl_inc_a;
	g_instruction_table[0x23] = _cpu_inc_hl;
	g_instruction_table[0x24] = _cpu_inc_h;
	g_instruction_table[0x25] = _cpu_dec_h;
	g_instruction_table[0x26] = _cpu_ld_h_d8;
	g_instruction_table[0x27] = _cpu_daa;
	g_instruction_table[0x28] = _cpu_jr_z_r8;
	g_instruction_table[0x29] = _cpu_add_hl_hl;
	g_instruction_table[0x2A] = _cpu_ld_a_imm_hl_inc;
	g_instruction_table[0x2B] = _cpu_dec_hl;
	g_instruction_table[0x2C] = _cpu_inc_l;
	g_instruction_table[0x2D] = _cpu_dec_l;
	g_instruction_table[0x2E] = _cpu_ld_l_d8;
	g_instruction_table[0x2F] = _cpu_cpl;
	g_instruction_table[0x30] = _cpu_jr_nc_r8;
	g_instruction_table[0x31] = _cpu_ld_sp_d16;
	g_instruction_table[0x32] = _cpu_ld_imm_hl_dec_a;
	g_instruction_table[0x33] = _cpu_inc_sp;
	g_instruction_table[0x34] = _cpu_inc_imm_hl;
	g_instruction_table[0x35] = _cpu_dec_imm_hl;
	g_instruction_table[0x36] = _cpu_ld_imm_hl_d8;
	g_instruction_table[0x37] = _cpu_scf;
	g_instruction_table[0x38] = _cpu_jr_c_r8;
	g_instruction_table[0x39] = _cpu_add_hl_sp;
	g_instruction_table[0x3A] = _cpu_ld_a_imm_hl_dec;
	g_instruction_table[0x3B] = _cpu_dec_sp;
	g_instruction_table[0x3C] = _cpu_inc_a;
	g_instruction_table[0x3D] = _cpu_dec_a;
	g_instruction_table[0x3E] = _cpu_ld_a_d8;
	g_instruction_table[0x3F] = _cpu_ccf;
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
	g_instruction_table[0x76] = _cpu_halt;
	g_instruction_table[0x77] = _cpu_ld_imm_hl_a;
	g_instruction_table[0x78] = _cpu_ld_a_b;
	g_instruction_table[0x79] = _cpu_ld_a_c;
	g_instruction_table[0x7A] = _cpu_ld_a_d;
	g_instruction_table[0x7B] = _cpu_ld_a_e;
	g_instruction_table[0x7C] = _cpu_ld_a_h;
	g_instruction_table[0x7D] = _cpu_ld_a_l;
	g_instruction_table[0x7E] = _cpu_ld_a_imm_hl;
	g_instruction_table[0x7F] = _cpu_ld_a_a;
	g_instruction_table[0x80] = _cpu_add_a_b;
	g_instruction_table[0x81] = _cpu_add_a_c;
	g_instruction_table[0x82] = _cpu_add_a_d;
	g_instruction_table[0x83] = _cpu_add_a_e;
	g_instruction_table[0x84] = _cpu_add_a_h;
	g_instruction_table[0x85] = _cpu_add_a_l;
	g_instruction_table[0x86] = _cpu_add_a_imm_hl;
	g_instruction_table[0x87] = _cpu_add_a_a;
	g_instruction_table[0x88] = _cpu_adc_a_b;
	g_instruction_table[0x89] = _cpu_adc_a_c;
	g_instruction_table[0x8A] = _cpu_adc_a_d;
	g_instruction_table[0x8B] = _cpu_adc_a_e;
	g_instruction_table[0x8C] = _cpu_adc_a_h;
	g_instruction_table[0x8D] = _cpu_adc_a_l;
	g_instruction_table[0x8E] = _cpu_adc_a_imm_hl;
	g_instruction_table[0x8F] = _cpu_adc_a_a;
	g_instruction_table[0x90] = _cpu_sub_b;
	g_instruction_table[0x91] = _cpu_sub_c;
	g_instruction_table[0x92] = _cpu_sub_d;
	g_instruction_table[0x93] = _cpu_sub_e;
	g_instruction_table[0x94] = _cpu_sub_h;
	g_instruction_table[0x95] = _cpu_sub_l;
	g_instruction_table[0x96] = _cpu_sub_imm_hl;
	g_instruction_table[0x97] = _cpu_sub_a;
	g_instruction_table[0x98] = _cpu_sbc_a_b;
	g_instruction_table[0x99] = _cpu_sbc_a_c;
	g_instruction_table[0x9A] = _cpu_sbc_a_d;
	g_instruction_table[0x9B] = _cpu_sbc_a_e;
	g_instruction_table[0x9C] = _cpu_sbc_a_h;
	g_instruction_table[0x9D] = _cpu_sbc_a_l;
	g_instruction_table[0x9E] = _cpu_sbc_a_imm_hl;
	g_instruction_table[0x9F] = _cpu_sbc_a_a;
	g_instruction_table[0xA0] = _cpu_and_b;
	g_instruction_table[0xA1] = _cpu_and_c;
	g_instruction_table[0xA2] = _cpu_and_d;
	g_instruction_table[0xA3] = _cpu_and_e;
	g_instruction_table[0xA4] = _cpu_and_h;
	g_instruction_table[0xA5] = _cpu_and_l;
	g_instruction_table[0xA6] = _cpu_and_imm_hl;
	g_instruction_table[0xA7] = _cpu_and_a;
	g_instruction_table[0xA8] = _cpu_xor_b;
	g_instruction_table[0xA9] = _cpu_xor_c;
	g_instruction_table[0xAA] = _cpu_xor_d;
	g_instruction_table[0xAB] = _cpu_xor_e;
	g_instruction_table[0xAC] = _cpu_xor_h;
	g_instruction_table[0xAD] = _cpu_xor_l;
	g_instruction_table[0xAE] = _cpu_xor_imm_hl;
	g_instruction_table[0xAF] = _cpu_xor_a;
	g_instruction_table[0xB0] = _cpu_or_b;
	g_instruction_table[0xB1] = _cpu_or_c;
	g_instruction_table[0xB2] = _cpu_or_d;
	g_instruction_table[0xB3] = _cpu_or_e;
	g_instruction_table[0xB4] = _cpu_or_h;
	g_instruction_table[0xB5] = _cpu_or_l;
	g_instruction_table[0xB6] = _cpu_or_imm_hl;
	g_instruction_table[0xB7] = _cpu_or_a;
	g_instruction_table[0xB8] = _cpu_cp_b;
	g_instruction_table[0xB9] = _cpu_cp_c;
	g_instruction_table[0xBA] = _cpu_cp_d;
	g_instruction_table[0xBB] = _cpu_cp_e;
	g_instruction_table[0xBC] = _cpu_cp_h;
	g_instruction_table[0xBD] = _cpu_cp_l;
	g_instruction_table[0xBE] = _cpu_cp_imm_hl;
	g_instruction_table[0xBF] = _cpu_cp_a;
	g_instruction_table[0xC0] = _cpu_ret_nz;
	g_instruction_table[0xC1] = _cpu_pop_bc;
	g_instruction_table[0xC2] = _cpu_jp_nz_a16;
	g_instruction_table[0xC3] = _cpu_jp_a16;
	g_instruction_table[0xC4] = _cpu_call_nz_a16;
	g_instruction_table[0xC5] = _cpu_push_bc;
	g_instruction_table[0xC6] = _cpu_add_a_d8;
	g_instruction_table[0xC7] = _cpu_rst_00H;
	g_instruction_table[0xC8] = _cpu_ret_z;
	g_instruction_table[0xC9] = _cpu_ret;
	g_instruction_table[0xCA] = _cpu_jp_z_a16;
	g_instruction_table[0xCB] = _cpu_prefix_cb;
	g_instruction_table[0xCC] = _cpu_call_z_a16;
	g_instruction_table[0xCD] = _cpu_call_a16;
	g_instruction_table[0xCE] = _cpu_adc_a_d8;
	g_instruction_table[0xCF] = _cpu_rst_08H;
	g_instruction_table[0xD0] = _cpu_ret_nc;
	g_instruction_table[0xD1] = _cpu_pop_de;
	g_instruction_table[0xD2] = _cpu_jp_nc_a16;
	g_instruction_table[0xD3] = _cpu_not_implemented;
	g_instruction_table[0xD4] = _cpu_call_nc_a16;
	g_instruction_table[0xD5] = _cpu_push_de;
	g_instruction_table[0xD6] = _cpu_sub_d8;
	g_instruction_table[0xD7] = _cpu_rst_10H;
	g_instruction_table[0xD8] = _cpu_ret_c;
	g_instruction_table[0xD9] = _cpu_reti;
	g_instruction_table[0xDA] = _cpu_jp_c_a16;
	g_instruction_table[0xDB] = _cpu_not_implemented;
	g_instruction_table[0xDC] = _cpu_call_c_a16;
	g_instruction_table[0xDD] = _cpu_not_implemented;
	g_instruction_table[0xDE] = _cpu_sbc_a_d8;
	g_instruction_table[0xDF] = _cpu_rst_18H;
	g_instruction_table[0XE0] = _cpu_ldh_imm_a8_a;
	g_instruction_table[0xE1] = _cpu_pop_hl;
	g_instruction_table[0xE2] = _cpu_ld_imm_c_a;
	g_instruction_table[0xE3] = _cpu_not_implemented;
	g_instruction_table[0xE4] = _cpu_not_implemented;
	g_instruction_table[0xE5] = _cpu_push_hl;
	g_instruction_table[0xE6] = _cpu_and_d8;
	g_instruction_table[0xE7] = _cpu_rst_20H;
	g_instruction_table[0xE8] = _cpu_add_sp_r8;
	g_instruction_table[0xE9] = _cpu_jp_imm_hl;
	g_instruction_table[0xEA] = _cpu_ld_imm_a16_a;
	g_instruction_table[0xEB] = _cpu_not_implemented;
	g_instruction_table[0xEC] = _cpu_not_implemented;
	g_instruction_table[0xED] = _cpu_not_implemented;
	g_instruction_table[0xEE] = _cpu_xor_d8;
	g_instruction_table[0xEF] = _cpu_rst_28H;
	g_instruction_table[0XF0] = _cpu_ldh_a_imm_a8;
	g_instruction_table[0xF1] = _cpu_pop_af;
	g_instruction_table[0xF2] = _cpu_ld_a_imm_c;
	g_instruction_table[0xF3] = _cpu_di;
	g_instruction_table[0xF5] = _cpu_push_af;
	g_instruction_table[0xF6] = _cpu_or_d8;
	g_instruction_table[0xF7] = _cpu_rst_30H;
	g_instruction_table[0xF8] = _cpu_ld_hl_sp_add_d8;
	g_instruction_table[0xF9] = _cpu_ld_sp_hl;
	g_instruction_table[0xFA] = _cpu_ld_a_imm_a16;
	g_instruction_table[0xFB] = _cpu_ei;
	g_instruction_table[0xFC] = _cpu_not_implemented;
	g_instruction_table[0xFD] = _cpu_not_implemented;
	g_instruction_table[0xFE] = _cpu_cp_d8;
	g_instruction_table[0xFF] = _cpu_rst_38H;

	g_cb_prefix_instruction_table[0x00] = _cpu_rlc_b;
	g_cb_prefix_instruction_table[0x01] = _cpu_rlc_c;
	g_cb_prefix_instruction_table[0x02] = _cpu_rlc_d;
	g_cb_prefix_instruction_table[0x03] = _cpu_rlc_e;
	g_cb_prefix_instruction_table[0x04] = _cpu_rlc_h;
	g_cb_prefix_instruction_table[0x05] = _cpu_rlc_l;
	g_cb_prefix_instruction_table[0x06] = _cpu_rlc_imm_hl;
	g_cb_prefix_instruction_table[0x07] = _cpu_rlc_a;
	g_cb_prefix_instruction_table[0x08] = _cpu_rrc_b;
	g_cb_prefix_instruction_table[0x09] = _cpu_rrc_c;
	g_cb_prefix_instruction_table[0x0A] = _cpu_rrc_d;
	g_cb_prefix_instruction_table[0x0B] = _cpu_rrc_e;
	g_cb_prefix_instruction_table[0x0C] = _cpu_rrc_h;
	g_cb_prefix_instruction_table[0x0D] = _cpu_rrc_l;
	g_cb_prefix_instruction_table[0x0E] = _cpu_rrc_imm_hl;
	g_cb_prefix_instruction_table[0x0F] = _cpu_rrc_a;
	g_cb_prefix_instruction_table[0x10] = _cpu_rl_b;
	g_cb_prefix_instruction_table[0x11] = _cpu_rl_c;
	g_cb_prefix_instruction_table[0x12] = _cpu_rl_d;
	g_cb_prefix_instruction_table[0x13] = _cpu_rl_e;
	g_cb_prefix_instruction_table[0x14] = _cpu_rl_h;
	g_cb_prefix_instruction_table[0x15] = _cpu_rl_l;
	g_cb_prefix_instruction_table[0x16] = _cpu_rl_imm_hl;
	g_cb_prefix_instruction_table[0x17] = _cpu_rl_a;
	g_cb_prefix_instruction_table[0x18] = _cpu_rr_b;
	g_cb_prefix_instruction_table[0x19] = _cpu_rr_c;
	g_cb_prefix_instruction_table[0x1A] = _cpu_rr_d;
	g_cb_prefix_instruction_table[0x1B] = _cpu_rr_e;
	g_cb_prefix_instruction_table[0x1C] = _cpu_rr_h;
	g_cb_prefix_instruction_table[0x1D] = _cpu_rr_l;
	g_cb_prefix_instruction_table[0x1E] = _cpu_rr_imm_hl;
	g_cb_prefix_instruction_table[0x1F] = _cpu_rr_a;
	g_cb_prefix_instruction_table[0x20] = _cpu_sla_b;
	g_cb_prefix_instruction_table[0x21] = _cpu_sla_c;
	g_cb_prefix_instruction_table[0x22] = _cpu_sla_d;
	g_cb_prefix_instruction_table[0x23] = _cpu_sla_e;
	g_cb_prefix_instruction_table[0x24] = _cpu_sla_h;
	g_cb_prefix_instruction_table[0x25] = _cpu_sla_l;
	g_cb_prefix_instruction_table[0x26] = _cpu_sla_imm_hl;
	g_cb_prefix_instruction_table[0x27] = _cpu_sla_a;
	g_cb_prefix_instruction_table[0x28] = _cpu_sra_b;
	g_cb_prefix_instruction_table[0x29] = _cpu_sra_c;
	g_cb_prefix_instruction_table[0x2A] = _cpu_sra_d;
	g_cb_prefix_instruction_table[0x2B] = _cpu_sra_e;
	g_cb_prefix_instruction_table[0x2C] = _cpu_sra_h;
	g_cb_prefix_instruction_table[0x2D] = _cpu_sra_l;
	g_cb_prefix_instruction_table[0x2E] = _cpu_sra_imm_hl;
	g_cb_prefix_instruction_table[0x2F] = _cpu_sra_a;
	g_cb_prefix_instruction_table[0x30] = _cpu_swap_b;
	g_cb_prefix_instruction_table[0x31] = _cpu_swap_c;
	g_cb_prefix_instruction_table[0x32] = _cpu_swap_d;
	g_cb_prefix_instruction_table[0x33] = _cpu_swap_e;
	g_cb_prefix_instruction_table[0x34] = _cpu_swap_h;
	g_cb_prefix_instruction_table[0x35] = _cpu_swap_l;
	g_cb_prefix_instruction_table[0x36] = _cpu_swap_imm_hl;
	g_cb_prefix_instruction_table[0x37] = _cpu_swap_a;
	g_cb_prefix_instruction_table[0x38] = _cpu_srl_b;
	g_cb_prefix_instruction_table[0x39] = _cpu_srl_c;
	g_cb_prefix_instruction_table[0x3A] = _cpu_srl_d;
	g_cb_prefix_instruction_table[0x3B] = _cpu_srl_e;
	g_cb_prefix_instruction_table[0x3C] = _cpu_srl_h;
	g_cb_prefix_instruction_table[0x3D] = _cpu_srl_l;
	g_cb_prefix_instruction_table[0x3E] = _cpu_srl_imm_hl;
	g_cb_prefix_instruction_table[0x3F] = _cpu_srl_a;
	g_cb_prefix_instruction_table[0x40] = _cpu_bit_0_b;
	g_cb_prefix_instruction_table[0x41] = _cpu_bit_0_c;
	g_cb_prefix_instruction_table[0x42] = _cpu_bit_0_d;
	g_cb_prefix_instruction_table[0x43] = _cpu_bit_0_e;
	g_cb_prefix_instruction_table[0x44] = _cpu_bit_0_h;
	g_cb_prefix_instruction_table[0x45] = _cpu_bit_0_l;
	g_cb_prefix_instruction_table[0x46] = _cpu_bit_0_imm_hl;
	g_cb_prefix_instruction_table[0x47] = _cpu_bit_0_a;
	g_cb_prefix_instruction_table[0x48] = _cpu_bit_1_b;
	g_cb_prefix_instruction_table[0x49] = _cpu_bit_1_c;
	g_cb_prefix_instruction_table[0x4A] = _cpu_bit_1_d;
	g_cb_prefix_instruction_table[0x4B] = _cpu_bit_1_e;
	g_cb_prefix_instruction_table[0x4C] = _cpu_bit_1_h;
	g_cb_prefix_instruction_table[0x4D] = _cpu_bit_1_l;
	g_cb_prefix_instruction_table[0x4E] = _cpu_bit_1_imm_hl;
	g_cb_prefix_instruction_table[0x4F] = _cpu_bit_1_a;
	g_cb_prefix_instruction_table[0x50] = _cpu_bit_2_b;
	g_cb_prefix_instruction_table[0x51] = _cpu_bit_2_c;
	g_cb_prefix_instruction_table[0x52] = _cpu_bit_2_d;
	g_cb_prefix_instruction_table[0x53] = _cpu_bit_2_e;
	g_cb_prefix_instruction_table[0x54] = _cpu_bit_2_h;
	g_cb_prefix_instruction_table[0x55] = _cpu_bit_2_l;
	g_cb_prefix_instruction_table[0x56] = _cpu_bit_2_imm_hl;
	g_cb_prefix_instruction_table[0x57] = _cpu_bit_2_a;
	g_cb_prefix_instruction_table[0x58] = _cpu_bit_3_b;
	g_cb_prefix_instruction_table[0x59] = _cpu_bit_3_c;
	g_cb_prefix_instruction_table[0x5A] = _cpu_bit_3_d;
	g_cb_prefix_instruction_table[0x5B] = _cpu_bit_3_e;
	g_cb_prefix_instruction_table[0x5C] = _cpu_bit_3_h;
	g_cb_prefix_instruction_table[0x5D] = _cpu_bit_3_l;
	g_cb_prefix_instruction_table[0x5E] = _cpu_bit_3_imm_hl;
	g_cb_prefix_instruction_table[0x5F] = _cpu_bit_3_a;
	g_cb_prefix_instruction_table[0x60] = _cpu_bit_4_b;
	g_cb_prefix_instruction_table[0x61] = _cpu_bit_4_c;
	g_cb_prefix_instruction_table[0x62] = _cpu_bit_4_d;
	g_cb_prefix_instruction_table[0x63] = _cpu_bit_4_e;
	g_cb_prefix_instruction_table[0x64] = _cpu_bit_4_h;
	g_cb_prefix_instruction_table[0x65] = _cpu_bit_4_l;
	g_cb_prefix_instruction_table[0x66] = _cpu_bit_4_imm_hl;
	g_cb_prefix_instruction_table[0x67] = _cpu_bit_4_a;
	g_cb_prefix_instruction_table[0x68] = _cpu_bit_5_b;
	g_cb_prefix_instruction_table[0x69] = _cpu_bit_5_c;
	g_cb_prefix_instruction_table[0x6A] = _cpu_bit_5_d;
	g_cb_prefix_instruction_table[0x6B] = _cpu_bit_5_e;
	g_cb_prefix_instruction_table[0x6C] = _cpu_bit_5_h;
	g_cb_prefix_instruction_table[0x6D] = _cpu_bit_5_l;
	g_cb_prefix_instruction_table[0x6E] = _cpu_bit_5_imm_hl;
	g_cb_prefix_instruction_table[0x6F] = _cpu_bit_5_a;
	g_cb_prefix_instruction_table[0x70] = _cpu_bit_6_b;
	g_cb_prefix_instruction_table[0x71] = _cpu_bit_6_c;
	g_cb_prefix_instruction_table[0x72] = _cpu_bit_6_d;
	g_cb_prefix_instruction_table[0x73] = _cpu_bit_6_e;
	g_cb_prefix_instruction_table[0x74] = _cpu_bit_6_h;
	g_cb_prefix_instruction_table[0x75] = _cpu_bit_6_l;
	g_cb_prefix_instruction_table[0x76] = _cpu_bit_6_imm_hl;
	g_cb_prefix_instruction_table[0x77] = _cpu_bit_6_a;
	g_cb_prefix_instruction_table[0x78] = _cpu_bit_7_b;
	g_cb_prefix_instruction_table[0x79] = _cpu_bit_7_c;
	g_cb_prefix_instruction_table[0x7A] = _cpu_bit_7_d;
	g_cb_prefix_instruction_table[0x7B] = _cpu_bit_7_e;
	g_cb_prefix_instruction_table[0x7C] = _cpu_bit_7_h;
	g_cb_prefix_instruction_table[0x7D] = _cpu_bit_7_l;
	g_cb_prefix_instruction_table[0x7E] = _cpu_bit_7_imm_hl;
	g_cb_prefix_instruction_table[0x7F] = _cpu_bit_7_a;
	g_cb_prefix_instruction_table[0x80] = _cpu_res_0_b;
	g_cb_prefix_instruction_table[0x81] = _cpu_res_0_c;
	g_cb_prefix_instruction_table[0x82] = _cpu_res_0_d;
	g_cb_prefix_instruction_table[0x83] = _cpu_res_0_e;
	g_cb_prefix_instruction_table[0x84] = _cpu_res_0_h;
	g_cb_prefix_instruction_table[0x85] = _cpu_res_0_l;
	g_cb_prefix_instruction_table[0x86] = _cpu_res_0_imm_hl;
	g_cb_prefix_instruction_table[0x87] = _cpu_res_0_a;
	g_cb_prefix_instruction_table[0x88] = _cpu_res_1_b;
	g_cb_prefix_instruction_table[0x89] = _cpu_res_1_c;
	g_cb_prefix_instruction_table[0x8A] = _cpu_res_1_d;
	g_cb_prefix_instruction_table[0x8B] = _cpu_res_1_e;
	g_cb_prefix_instruction_table[0x8C] = _cpu_res_1_h;
	g_cb_prefix_instruction_table[0x8D] = _cpu_res_1_l;
	g_cb_prefix_instruction_table[0x8E] = _cpu_res_1_imm_hl;
	g_cb_prefix_instruction_table[0x8F] = _cpu_res_1_a;
	g_cb_prefix_instruction_table[0x90] = _cpu_res_2_b;
	g_cb_prefix_instruction_table[0x91] = _cpu_res_2_c;
	g_cb_prefix_instruction_table[0x92] = _cpu_res_2_d;
	g_cb_prefix_instruction_table[0x93] = _cpu_res_2_e;
	g_cb_prefix_instruction_table[0x94] = _cpu_res_2_h;
	g_cb_prefix_instruction_table[0x95] = _cpu_res_2_l;
	g_cb_prefix_instruction_table[0x96] = _cpu_res_2_imm_hl;
	g_cb_prefix_instruction_table[0x97] = _cpu_res_2_a;
	g_cb_prefix_instruction_table[0x98] = _cpu_res_3_b;
	g_cb_prefix_instruction_table[0x99] = _cpu_res_3_c;
	g_cb_prefix_instruction_table[0x9A] = _cpu_res_3_d;
	g_cb_prefix_instruction_table[0x9B] = _cpu_res_3_e;
	g_cb_prefix_instruction_table[0x9C] = _cpu_res_3_h;
	g_cb_prefix_instruction_table[0x9D] = _cpu_res_3_l;
	g_cb_prefix_instruction_table[0x9E] = _cpu_res_3_imm_hl;
	g_cb_prefix_instruction_table[0x9F] = _cpu_res_3_a;
	g_cb_prefix_instruction_table[0xA0] = _cpu_res_4_b;
	g_cb_prefix_instruction_table[0xA1] = _cpu_res_4_c;
	g_cb_prefix_instruction_table[0xA2] = _cpu_res_4_d;
	g_cb_prefix_instruction_table[0xA3] = _cpu_res_4_e;
	g_cb_prefix_instruction_table[0xA4] = _cpu_res_4_h;
	g_cb_prefix_instruction_table[0xA5] = _cpu_res_4_l;
	g_cb_prefix_instruction_table[0xA6] = _cpu_res_4_imm_hl;
	g_cb_prefix_instruction_table[0xA7] = _cpu_res_4_a;
	g_cb_prefix_instruction_table[0xA8] = _cpu_res_5_b;
	g_cb_prefix_instruction_table[0xA9] = _cpu_res_5_c;
	g_cb_prefix_instruction_table[0xAA] = _cpu_res_5_d;
	g_cb_prefix_instruction_table[0xAB] = _cpu_res_5_e;
	g_cb_prefix_instruction_table[0xAC] = _cpu_res_5_h;
	g_cb_prefix_instruction_table[0xAD] = _cpu_res_5_l;
	g_cb_prefix_instruction_table[0xAE] = _cpu_res_5_imm_hl;
	g_cb_prefix_instruction_table[0xAF] = _cpu_res_5_a;
	g_cb_prefix_instruction_table[0xB0] = _cpu_res_6_b;
	g_cb_prefix_instruction_table[0xB1] = _cpu_res_6_c;
	g_cb_prefix_instruction_table[0xB2] = _cpu_res_6_d;
	g_cb_prefix_instruction_table[0xB3] = _cpu_res_6_e;
	g_cb_prefix_instruction_table[0xB4] = _cpu_res_6_h;
	g_cb_prefix_instruction_table[0xB5] = _cpu_res_6_l;
	g_cb_prefix_instruction_table[0xB6] = _cpu_res_6_imm_hl;
	g_cb_prefix_instruction_table[0xB7] = _cpu_res_6_a;
	g_cb_prefix_instruction_table[0xB8] = _cpu_res_7_b;
	g_cb_prefix_instruction_table[0xB9] = _cpu_res_7_c;
	g_cb_prefix_instruction_table[0xBA] = _cpu_res_7_d;
	g_cb_prefix_instruction_table[0xBB] = _cpu_res_7_e;
	g_cb_prefix_instruction_table[0xBC] = _cpu_res_7_h;
	g_cb_prefix_instruction_table[0xBD] = _cpu_res_7_l;
	g_cb_prefix_instruction_table[0xBE] = _cpu_res_7_imm_hl;
	g_cb_prefix_instruction_table[0xBF] = _cpu_res_7_a;
	g_cb_prefix_instruction_table[0xC0] = _cpu_set_0_b;
	g_cb_prefix_instruction_table[0xC1] = _cpu_set_0_c;
	g_cb_prefix_instruction_table[0xC2] = _cpu_set_0_d;
	g_cb_prefix_instruction_table[0xC3] = _cpu_set_0_e;
	g_cb_prefix_instruction_table[0xC4] = _cpu_set_0_h;
	g_cb_prefix_instruction_table[0xC5] = _cpu_set_0_l;
	g_cb_prefix_instruction_table[0xC6] = _cpu_set_0_imm_hl;
	g_cb_prefix_instruction_table[0xC7] = _cpu_set_0_a;
	g_cb_prefix_instruction_table[0xC8] = _cpu_set_1_b;
	g_cb_prefix_instruction_table[0xC9] = _cpu_set_1_c;
	g_cb_prefix_instruction_table[0xCA] = _cpu_set_1_d;
	g_cb_prefix_instruction_table[0xCB] = _cpu_set_1_e;
	g_cb_prefix_instruction_table[0xCC] = _cpu_set_1_h;
	g_cb_prefix_instruction_table[0xCD] = _cpu_set_1_l;
	g_cb_prefix_instruction_table[0xCE] = _cpu_set_1_imm_hl;
	g_cb_prefix_instruction_table[0xCF] = _cpu_set_1_a;
	g_cb_prefix_instruction_table[0xD0] = _cpu_set_2_b;
	g_cb_prefix_instruction_table[0xD1] = _cpu_set_2_c;
	g_cb_prefix_instruction_table[0xD2] = _cpu_set_2_d;
	g_cb_prefix_instruction_table[0xD3] = _cpu_set_2_e;
	g_cb_prefix_instruction_table[0xD4] = _cpu_set_2_h;
	g_cb_prefix_instruction_table[0xD5] = _cpu_set_2_l;
	g_cb_prefix_instruction_table[0xD6] = _cpu_set_2_imm_hl;
	g_cb_prefix_instruction_table[0xD7] = _cpu_set_2_a;
	g_cb_prefix_instruction_table[0xD8] = _cpu_set_3_b;
	g_cb_prefix_instruction_table[0xD9] = _cpu_set_3_c;
	g_cb_prefix_instruction_table[0xDA] = _cpu_set_3_d;
	g_cb_prefix_instruction_table[0xDB] = _cpu_set_3_e;
	g_cb_prefix_instruction_table[0xDC] = _cpu_set_3_h;
	g_cb_prefix_instruction_table[0xDD] = _cpu_set_3_l;
	g_cb_prefix_instruction_table[0xDE] = _cpu_set_3_imm_hl;
	g_cb_prefix_instruction_table[0xDF] = _cpu_set_3_a;
	g_cb_prefix_instruction_table[0xE0] = _cpu_set_4_b;
	g_cb_prefix_instruction_table[0xE1] = _cpu_set_4_c;
	g_cb_prefix_instruction_table[0xE2] = _cpu_set_4_d;
	g_cb_prefix_instruction_table[0xE3] = _cpu_set_4_e;
	g_cb_prefix_instruction_table[0xE4] = _cpu_set_4_h;
	g_cb_prefix_instruction_table[0xE5] = _cpu_set_4_l;
	g_cb_prefix_instruction_table[0xE6] = _cpu_set_4_imm_hl;
	g_cb_prefix_instruction_table[0xE7] = _cpu_set_4_a;
	g_cb_prefix_instruction_table[0xE8] = _cpu_set_5_b;
	g_cb_prefix_instruction_table[0xE9] = _cpu_set_5_c;
	g_cb_prefix_instruction_table[0xEA] = _cpu_set_5_d;
	g_cb_prefix_instruction_table[0xEB] = _cpu_set_5_e;
	g_cb_prefix_instruction_table[0xEC] = _cpu_set_5_h;
	g_cb_prefix_instruction_table[0xED] = _cpu_set_5_l;
	g_cb_prefix_instruction_table[0xEE] = _cpu_set_5_imm_hl;
	g_cb_prefix_instruction_table[0xEF] = _cpu_set_5_a;
	g_cb_prefix_instruction_table[0xF0] = _cpu_set_6_b;
	g_cb_prefix_instruction_table[0xF1] = _cpu_set_6_c;
	g_cb_prefix_instruction_table[0xF2] = _cpu_set_6_d;
	g_cb_prefix_instruction_table[0xF3] = _cpu_set_6_e;
	g_cb_prefix_instruction_table[0xF4] = _cpu_set_6_h;
	g_cb_prefix_instruction_table[0xF5] = _cpu_set_6_l;
	g_cb_prefix_instruction_table[0xF6] = _cpu_set_6_imm_hl;
	g_cb_prefix_instruction_table[0xF7] = _cpu_set_6_a;
	g_cb_prefix_instruction_table[0xF8] = _cpu_set_7_b;
	g_cb_prefix_instruction_table[0xF9] = _cpu_set_7_c;
	g_cb_prefix_instruction_table[0xFA] = _cpu_set_7_d;
	g_cb_prefix_instruction_table[0xFB] = _cpu_set_7_e;
	g_cb_prefix_instruction_table[0xFC] = _cpu_set_7_h;
	g_cb_prefix_instruction_table[0xFD] = _cpu_set_7_l;
	g_cb_prefix_instruction_table[0xFE] = _cpu_set_7_imm_hl;
	g_cb_prefix_instruction_table[0xFF] = _cpu_set_7_a;

	registers_prepare(&g_registers);
}


void cpu_push8(u8 data)
{
	//Decrement Stack Pointer
	g_registers.SP -= 1;
	// Save data at the new location
	mem_write8(g_registers.SP, data);
}


void cpu_push16(u16 data)
{
	//Decrement Stack Pointer
	g_registers.SP -= 2;
	// Save data at the new location
	mem_write16(g_registers.SP, data);
}

bool cpu_get_halted()
{
	return g_cpu_halted;
}

void cpu_set_halted(bool val)
{
	g_cpu_halted = val;
}

bool cpu_get_stopped()
{
	return g_cpu_stopped;
}

void cpu_set_stopped(bool val)
{
	g_cpu_stopped = val;
}
