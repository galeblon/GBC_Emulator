#include"cpu.h"
#include"logger.h"
#include"mem.h"
#include"regs.h"

#define INSTRUCTIONS_NUMBER 256

#define _CPU_IS_HALF_CARRY(a, b) ((((a & 0x0F) + (b & 0x0F)) & 0x10) == 0x10)
#define _CPU_IS_CARRY(a, b) ((((a & 0xFF) + (b & 0xFF)) & 0x100) == 0x100)

typedef int (*cpu_instruction_t)(void);

static cpu_instruction_t g_instruction_table[INSTRUCTIONS_NUMBER];
static cpu_instruction_t g_cb_prefix_instruction_table[INSTRUCTIONS_NUMBER];

static struct cpu_registers g_registers;

static int cpu_halted = 0;
static int cpu_stopped = 0;

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
	cpu_stopped = 1;
	g_registers.PC += 2;
	return 4;
}

static int _cpu_halt(void)
{
	cpu_halted = 1;
	g_registers.PC += 1;
	return 4;
}

static int _cpu_prefix_cb(void)
{
	g_registers.PC += 1;
	d8 opcode = mem_read8(g_registers.PC);
	return g_cb_prefix_instruction_table[opcode];;
}

static int _cpu_di(void)
{
	//TODO waiting on #7
	// This should not disable immediately(after next instruction)
	g_registers.PC += 1;
	return 4;
}

static int _cpu_ei(void)
{
	//TODO waiting on #7
	// This should not enable immediately(after next instruction)
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
	a8 address = (a16)mem_read8(g_registers.PC) + 0xFF00;
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
	d8 index = mem_read8(g_registers.PC);
	g_registers.PC += 1;
	d16 result = g_registers.SP + index;
	g_registers.FLAGS.Z = 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(g_registers.SP, index);
	g_registers.FLAGS.C = _CPU_IS_CARRY(g_registers.SP, index);
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
		addr <<= 8;
		addr += mem_read8(g_registers.SP+1);
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
		addr <<= 8;
		addr += mem_read8(g_registers.SP+1);
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
		addr <<= 8;
		addr += mem_read8(g_registers.SP+1);
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
		addr <<= 8;
		addr += mem_read8(g_registers.SP+1);
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
	addr <<= 8;
	addr += mem_read8(g_registers.SP+1);
	g_registers.PC = addr;
	g_registers.SP += 2;
	return 16;
}

static int _cpu_reti(void)
{
	g_registers.PC += 1;
	a16 addr = 0x0000;
	addr = mem_read8(g_registers.SP);
	addr <<= 8;
	addr += mem_read8(g_registers.SP+1);
	g_registers.PC = addr;
	g_registers.SP += 2;
	//TODO Enable interrupts, waiting on #7
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
	d8 left = g_registers.B;
	d8 right = 0x01;
	g_registers.B = left + right;
	g_registers.FLAGS.Z = g_registers.B == 0;
	g_registers.FLAGS.N = 0;
	g_registers.FLAGS.H = _CPU_IS_HALF_CARRY(left, right);
	return 4;
}

static int _cpu_dec_b(void)
{
	g_registers.PC += 1;
	d8 left = g_registers.B;
	d8 right = -1;
	g_registers.B = left + right;
	g_registers.FLAGS.Z = g_registers.B == 0;
	g_registers.FLAGS.N = 1;
	g_registers.FLAGS.H = !_CPU_IS_HALF_CARRY(left, right);
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


int cpu_single_step(void)
{
	if(cpu_stopped) {
		//TODO check if anything was pressed
		// then remove stopped
	} else if(cpu_halted) {
		//TODO check if interrupt occured
		// then remove halted
	} else {
		// Fetch
		d8 instruction_code = mem_read8(g_registers.PC);
		// Decode & Execute
		return g_instruction_table[instruction_code]();
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
	g_instruction_table[0x10] = _cpu_stop;
	g_instruction_table[0x11] = _cpu_ld_de_d16;
	g_instruction_table[0x12] = _cpu_ld_imm_de_a;
	// Missing
	g_instruction_table[0x16] = _cpu_ld_d_d8;
	// Missing
	g_instruction_table[0x18] = _cpu_jr_r8;
	// Missing
	g_instruction_table[0x1A] = _cpu_ld_a_imm_de;
	// Missing
	g_instruction_table[0x1E] = _cpu_ld_e_d8;
	// Missing
	g_instruction_table[0x20] = _cpu_jr_nz_r8;
	g_instruction_table[0x21] = _cpu_ld_hl_d16;
	g_instruction_table[0x22] = _cpu_ld_imm_hl_inc_a;
	// Missing
	g_instruction_table[0x26] = _cpu_ld_h_d8;
	// Missing
	g_instruction_table[0x28] = _cpu_jr_z_r8;
	// Missing
	g_instruction_table[0x2A] = _cpu_ld_a_imm_hl_inc;
	// Missing
	g_instruction_table[0x2E] = _cpu_ld_l_d8;
	// Missing
	g_instruction_table[0x30] = _cpu_jr_nc_r8;
	g_instruction_table[0x31] = _cpu_ld_sp_d16;
	g_instruction_table[0x32] = _cpu_ld_imm_hl_dec_a;
	// Missing
	g_instruction_table[0x36] = _cpu_ld_imm_hl_d8;
	// Missing
	g_instruction_table[0x38] = _cpu_jr_c_r8;
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
	// Missing
	g_instruction_table[0xC0] = _cpu_ret_nz;
	g_instruction_table[0xC1] = _cpu_pop_bc;
	g_instruction_table[0xC2] = _cpu_jp_nz_a16;
	g_instruction_table[0xC3] = _cpu_jp_a16;
	g_instruction_table[0xC4] = _cpu_call_nz_a16;
	g_instruction_table[0xC5] = _cpu_push_bc;
	// Missing
	g_instruction_table[0xC7] = _cpu_rst_00H;
	g_instruction_table[0xC8] = _cpu_ret_z;
	g_instruction_table[0xC9] = _cpu_ret;
	g_instruction_table[0xCA] = _cpu_jp_z_a16;
	g_instruction_table[0xCB] = _cpu_prefix_cb;
	g_instruction_table[0xCC] = _cpu_call_z_a16;
	g_instruction_table[0xCD] = _cpu_call_a16;
	// Missing
	g_instruction_table[0xCF] = _cpu_rst_08H;
	// Missing
	g_instruction_table[0xD0] = _cpu_ret_nc;
	g_instruction_table[0xD1] = _cpu_pop_de;
	g_instruction_table[0xD2] = _cpu_jp_nc_a16;
	// Missing
	g_instruction_table[0xD4] = _cpu_call_nc_a16;
	g_instruction_table[0xD5] = _cpu_push_de;
	// Missing
	g_instruction_table[0xD7] = _cpu_rst_10H;
	g_instruction_table[0xD8] = _cpu_ret_c;
	g_instruction_table[0xD9] = _cpu_reti;
	g_instruction_table[0xDA] = _cpu_jp_c_a16;
	// Missing
	g_instruction_table[0xDC] = _cpu_call_c_a16;
	// Missing
	g_instruction_table[0xDF] = _cpu_rst_18H;
	// Missing
	g_instruction_table[0XE0] = _cpu_ldh_imm_a8_a;
	g_instruction_table[0xE1] = _cpu_pop_hl;
	g_instruction_table[0xE2] = _cpu_ld_imm_c_a;
	// Missing
	g_instruction_table[0xE5] = _cpu_push_hl;
	// Missing
	g_instruction_table[0xE7] = _cpu_rst_20H;
	// Missing
	g_instruction_table[0xE9] = _cpu_jp_imm_hl;
	g_instruction_table[0xEA] = _cpu_ld_imm_a16_a;
	// Missing
	g_instruction_table[0xEF] = _cpu_rst_28H;
	// Missing
	g_instruction_table[0XF0] = _cpu_ldh_a_imm_a8;
	g_instruction_table[0xF1] = _cpu_pop_af;
	g_instruction_table[0xF2] = _cpu_ld_a_imm_c;
	g_instruction_table[0xF3] = _cpu_di;
	g_instruction_table[0xF5] = _cpu_push_af;
	// Missing
	g_instruction_table[0xF7] = _cpu_rst_30H;
	g_instruction_table[0xF8] = _cpu_ld_hl_sp_add_d8;
	g_instruction_table[0xF9] = _cpu_ld_sp_hl;
	g_instruction_table[0xFA] = _cpu_ld_a_imm_a16;
	g_instruction_table[0xFB] = _cpu_ei;
	// Missing
	g_instruction_table[0xFF] = _cpu_rst_38H;

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
