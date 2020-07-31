#include<stdlib.h>
#include<string.h>
#include"debug.h"
#include"logger.h"

struct instruction_info {
	int length;
	char* mnemonic_format;
};

static struct instruction_info extended_instruction_infos[256] = {
		{1, "RLC\tB"},
		{1, "RLC\tC"},
		{1, "RLC\tD"},
		{1, "RLC\tE"},
		{1, "RLC\tH"},
		{1, "RLC\tL"},
		{1, "RLC\t(HL)"},
		{1, "RLC\tA"},
		{1, "RRC\tB"},
		{1, "RRC\tC"},
		{1, "RRC\tD"},
		{1, "RRC\tE"},
		{1, "RRC\tH"},
		{1, "RRC\tL"},
		{1, "RRC\t(HL)"},
		{1, "RRC\tA"},
		{1, "RL\tB"},
		{1, "RL\tC"},
		{1, "RL\tD"},
		{1, "RL\tE"},
		{1, "RL\tH"},
		{1, "RL\tL"},
		{1, "RL\t(HL)"},
		{1, "RL\tA"},
		{1, "RR\tB"},
		{1, "RR\tC"},
		{1, "RR\tD"},
		{1, "RR\tE"},
		{1, "RR\tH"},
		{1, "RR\tL"},
		{1, "RR\t(HL)"},
		{1, "RR\tA"},
		{1, "SLA\tB"},
		{1, "SLA\tC"},
		{1, "SLA\tD"},
		{1, "SLA\tE"},
		{1, "SLA\tH"},
		{1, "SLA\tL"},
		{1, "SLA\t(HL)"},
		{1, "SLA\tA"},
		{1, "SRA\tB"},
		{1, "SRA\tC"},
		{1, "SRA\tD"},
		{1, "SRA\tE"},
		{1, "SRA\tH"},
		{1, "SRA\tL"},
		{1, "SRA\t(HL)"},
		{1, "SRA\tA"},
		{1, "SWAP\tB"},
		{1, "SWAP\tC"},
		{1, "SWAP\tD"},
		{1, "SWAP\tE"},
		{1, "SWAP\tH"},
		{1, "SWAP\tL"},
		{1, "SWAP\t(HL)"},
		{1, "SWAP\tA"},
		{1, "SRL\tB"},
		{1, "SRL\tC"},
		{1, "SRL\tD"},
		{1, "SRL\tE"},
		{1, "SRL\tH"},
		{1, "SRL\tL"},
		{1, "SRL\t(HL)"},
		{1, "SRL\tA"},
		{1, "BIT\t0, B"},
		{1, "BIT\t0, C"},
		{1, "BIT\t0, D"},
		{1, "BIT\t0, E"},
		{1, "BIT\t0, H"},
		{1, "BIT\t0, L"},
		{1, "BIT\t0, (HL)"},
		{1, "BIT\t0, A"},
		{1, "BIT\t1, B"},
		{1, "BIT\t1, C"},
		{1, "BIT\t1, D"},
		{1, "BIT\t1, E"},
		{1, "BIT\t1, H"},
		{1, "BIT\t1, L"},
		{1, "BIT\t1, (HL)"},
		{1, "BIT\t1, A"},
		{1, "BIT\t2, B"},
		{1, "BIT\t2, C"},
		{1, "BIT\t2, D"},
		{1, "BIT\t2, E"},
		{1, "BIT\t2, H"},
		{1, "BIT\t2, L"},
		{1, "BIT\t2, (HL)"},
		{1, "BIT\t2, A"},
		{1, "BIT\t3, B"},
		{1, "BIT\t3, C"},
		{1, "BIT\t3, D"},
		{1, "BIT\t3, E"},
		{1, "BIT\t3, H"},
		{1, "BIT\t3, L"},
		{1, "BIT\t3, (HL)"},
		{1, "BIT\t3, A"},
		{1, "BIT\t4, B"},
		{1, "BIT\t4, C"},
		{1, "BIT\t4, D"},
		{1, "BIT\t4, E"},
		{1, "BIT\t4, H"},
		{1, "BIT\t4, L"},
		{1, "BIT\t4, (HL)"},
		{1, "BIT\t4, A"},
		{1, "BIT\t5, B"},
		{1, "BIT\t5, C"},
		{1, "BIT\t5, D"},
		{1, "BIT\t5, E"},
		{1, "BIT\t5, H"},
		{1, "BIT\t5, L"},
		{1, "BIT\t5, (HL)"},
		{1, "BIT\t5, A"},
		{1, "BIT\t6, B"},
		{1, "BIT\t6, C"},
		{1, "BIT\t6, D"},
		{1, "BIT\t6, E"},
		{1, "BIT\t6, H"},
		{1, "BIT\t6, L"},
		{1, "BIT\t6, (HL)"},
		{1, "BIT\t6, A"},
		{1, "BIT\t7, B"},
		{1, "BIT\t7, C"},
		{1, "BIT\t7, D"},
		{1, "BIT\t7, E"},
		{1, "BIT\t7, H"},
		{1, "BIT\t7, L"},
		{1, "BIT\t7, (HL)"},
		{1, "BIT\t7, A"},
		{1, "RES\t0, B"},
		{1, "RES\t0, C"},
		{1, "RES\t0, D"},
		{1, "RES\t0, E"},
		{1, "RES\t0, H"},
		{1, "RES\t0, L"},
		{1, "RES\t0, (HL)"},
		{1, "RES\t0, A"},
		{1, "RES\t1, B"},
		{1, "RES\t1, C"},
		{1, "RES\t1, D"},
		{1, "RES\t1, E"},
		{1, "RES\t1, H"},
		{1, "RES\t1, L"},
		{1, "RES\t1, (HL)"},
		{1, "RES\t1, A"},
		{1, "RES\t2, B"},
		{1, "RES\t2, C"},
		{1, "RES\t2, D"},
		{1, "RES\t2, E"},
		{1, "RES\t2, H"},
		{1, "RES\t2, L"},
		{1, "RES\t2, (HL)"},
		{1, "RES\t2, A"},
		{1, "RES\t3, B"},
		{1, "RES\t3, C"},
		{1, "RES\t3, D"},
		{1, "RES\t3, E"},
		{1, "RES\t3, H"},
		{1, "RES\t3, L"},
		{1, "RES\t3, (HL)"},
		{1, "RES\t3, A"},
		{1, "RES\t4, B"},
		{1, "RES\t4, C"},
		{1, "RES\t4, D"},
		{1, "RES\t4, E"},
		{1, "RES\t4, H"},
		{1, "RES\t4, L"},
		{1, "RES\t4, (HL)"},
		{1, "RES\t4, A"},
		{1, "RES\t5, B"},
		{1, "RES\t5, C"},
		{1, "RES\t5, D"},
		{1, "RES\t5, E"},
		{1, "RES\t5, H"},
		{1, "RES\t5, L"},
		{1, "RES\t5, (HL)"},
		{1, "RES\t5, A"},
		{1, "RES\t6, B"},
		{1, "RES\t6, C"},
		{1, "RES\t6, D"},
		{1, "RES\t6, E"},
		{1, "RES\t6, H"},
		{1, "RES\t6, L"},
		{1, "RES\t6, (HL)"},
		{1, "RES\t6, A"},
		{1, "RES\t7, B"},
		{1, "RES\t7, C"},
		{1, "RES\t7, D"},
		{1, "RES\t7, E"},
		{1, "RES\t7, H"},
		{1, "RES\t7, L"},
		{1, "RES\t7, (HL)"},
		{1, "RES\t7, A"},
		{1, "SET\t0, B"},
		{1, "SET\t0, C"},
		{1, "SET\t0, D"},
		{1, "SET\t0, E"},
		{1, "SET\t0, H"},
		{1, "SET\t0, L"},
		{1, "SET\t0, (HL)"},
		{1, "SET\t0, A"},
		{1, "SET\t1, B"},
		{1, "SET\t1, C"},
		{1, "SET\t1, D"},
		{1, "SET\t1, E"},
		{1, "SET\t1, H"},
		{1, "SET\t1, L"},
		{1, "SET\t1, (HL)"},
		{1, "SET\t1, A"},
		{1, "SET\t2, B"},
		{1, "SET\t2, C"},
		{1, "SET\t2, D"},
		{1, "SET\t2, E"},
		{1, "SET\t2, H"},
		{1, "SET\t2, L"},
		{1, "SET\t2, (HL)"},
		{1, "SET\t2, A"},
		{1, "SET\t3, B"},
		{1, "SET\t3, C"},
		{1, "SET\t3, D"},
		{1, "SET\t3, E"},
		{1, "SET\t3, H"},
		{1, "SET\t3, L"},
		{1, "SET\t3, (HL)"},
		{1, "SET\t3, A"},
		{1, "SET\t4, B"},
		{1, "SET\t4, C"},
		{1, "SET\t4, D"},
		{1, "SET\t4, E"},
		{1, "SET\t4, H"},
		{1, "SET\t4, L"},
		{1, "SET\t4, (HL)"},
		{1, "SET\t4, A"},
		{1, "SET\t5, B"},
		{1, "SET\t5, C"},
		{1, "SET\t5, D"},
		{1, "SET\t5, E"},
		{1, "SET\t5, H"},
		{1, "SET\t5, L"},
		{1, "SET\t5, (HL)"},
		{1, "SET\t5, A"},
		{1, "SET\t6, B"},
		{1, "SET\t6, C"},
		{1, "SET\t6, D"},
		{1, "SET\t6, E"},
		{1, "SET\t6, H"},
		{1, "SET\t6, L"},
		{1, "SET\t6, (HL)"},
		{1, "SET\t6, A"},
		{1, "SET\t7, B"},
		{1, "SET\t7, C"},
		{1, "SET\t7, D"},
		{1, "SET\t7, E"},
		{1, "SET\t7, H"},
		{1, "SET\t7, L"},
		{1, "SET\t7, (HL)"},
		{1, "SET\t7, A"},
};

static struct instruction_info instruction_infos[256] = {
		{1, "NOP"},
		{3, "LD\tBC, 0x%04X"},
		{1, "LD\t(BC), A"},
		{1, "INC\tBC"},
		{1, "INC\tB"},
		{1, "DEC\tB"},
		{2, "LD\tB, 0x%02X"},
		{1, "RLCA"},
		{3, "LD\t(0x%04X), SP"},
		{1, "ADD\tHLC, BC"},
		{1, "LD\tA, (BC)"},
		{1, "DEC\tBC"},
		{1, "INC\tC"},
		{1, "DEC\tC"},
		{2, "LD\tC, 0x%02X"},
		{1, "RRCA"},
		{2, "STOP\t0"},
		{3, "LD\tDE, 0x%04X"},
		{1, "LD\t(DE), A"},
		{1, "INC\tDE"},
		{1, "INC\tD"},
		{1, "DEC\tD"},
		{2, "LD\tD, 0x%02X"},
		{1, "RLA"},
		{2, "JR\t0x%02X"},
		{1, "ADD\tHL, DE"},
		{1, "LD\tA, (DE)"},
		{1, "DEC\tDE"},
		{1, "INC\tE"},
		{1, "DEC\tE"},
		{2, "LD\tE, 0x%02X"},
		{1, "RRA"},
		{2, "JR\tNZ, 0x%02X"},
		{3, "LD\tHL, 0x%04X"},
		{1, "LD\t(HL+), A"},
		{1, "INC\tHL"},
		{1, "INC\tH"},
		{1, "DEC\tH"},
		{2, "LD\tH, 0x%02X"},
		{1, "DAA"},
		{2, "JR\tZ, 0x%02X"},
		{1, "ADD\tHL, HL"},
		{1, "LD\tA, (HL+)"},
		{1, "DEC\tHL"},
		{1, "INC\tL"},
		{1, "DEC\tL"},
		{2, "LD\tL, 0x%02X"},
		{1, "CPL"},
		{2, "JR\tNC, 0x%02X"},
		{3, "LD\tSP, 0x%04X"},
		{1, "LD\t(HL-), A"},
		{1, "INC\tSP"},
		{1, "INC\t(HL)"},
		{1, "DEC\t(HL)"},
		{2, "LD\t(HL), 0x%02X"},
		{1, "SCF"},
		{2, "JR\tC, 0x%02X"},
		{1, "ADD\tHL, SP"},
		{1, "LD\tA, (HL-)"},
		{1, "DEC\tSP"},
		{1, "INC\tA"},
		{1, "DEC\tA"},
		{2, "LD\tA, 0x%02X"},
		{1, "CCF"},
		{1, "LD\tB, B"},
		{1, "LD\tB, C"},
		{1, "LD\tB, D"},
		{1, "LD\tB, E"},
		{1, "LD\tB, H"},
		{1, "LD\tB, L"},
		{1, "LD\tB, (HL)"},
		{1, "LD\tB, A"},
		{1, "LD\tC, B"},
		{1, "LD\tC, C"},
		{1, "LD\tC, D"},
		{1, "LD\tC, E"},
		{1, "LD\tC, H"},
		{1, "LD\tC, L"},
		{1, "LD\tC, (HL)"},
		{1, "LD\tC, A"},
		{1, "LD\tD, B"},
		{1, "LD\tD, C"},
		{1, "LD\tD, D"},
		{1, "LD\tD, E"},
		{1, "LD\tD, H"},
		{1, "LD\tD, L"},
		{1, "LD\tD, (HL)"},
		{1, "LD\tD, A"},
		{1, "LD\tE, B"},
		{1, "LD\tE, C"},
		{1, "LD\tE, D"},
		{1, "LD\tE, E"},
		{1, "LD\tE, H"},
		{1, "LD\tE, L"},
		{1, "LD\tE, (HL)"},
		{1, "LD\tE, A"},
		{1, "LD\tH, B"},
		{1, "LD\tH, C"},
		{1, "LD\tH, D"},
		{1, "LD\tH, E"},
		{1, "LD\tH, H"},
		{1, "LD\tH, L"},
		{1, "LD\tH, (HL)"},
		{1, "LD\tH, A"},
		{1, "LD\tL, B"},
		{1, "LD\tL, C"},
		{1, "LD\tL, D"},
		{1, "LD\tL, E"},
		{1, "LD\tL, H"},
		{1, "LD\tL, L"},
		{1, "LD\tL, (HL)"},
		{1, "LD\tL, A"},
		{1, "LD\t(HL), B"},
		{1, "LD\t(HL), C"},
		{1, "LD\t(HL), D"},
		{1, "LD\t(HL), E"},
		{1, "LD\t(HL), H"},
		{1, "LD\t(HL), L"},
		{1, "HALT"},
		{1, "LD\t(HL), A"},
		{1, "LD\tA, B"},
		{1, "LD\tA, C"},
		{1, "LD\tA, D"},
		{1, "LD\tA, E"},
		{1, "LD\tA, H"},
		{1, "LD\tA, L"},
		{1, "LD\tA, (HL)"},
		{1, "LD\tA, A"},
		{1, "ADD\tA, B"},
		{1, "ADD\tA, C"},
		{1, "ADD\tA, D"},
		{1, "ADD\tA, E"},
		{1, "ADD\tA, H"},
		{1, "ADD\tA, L"},
		{1, "ADD\tA, (HL)"},
		{1, "ADD\tA, A"},
		{1, "ADC\tA, B"},
		{1, "ADC\tA, C"},
		{1, "ADC\tA, D"},
		{1, "ADC\tA, E"},
		{1, "ADC\tA, H"},
		{1, "ADC\tA, L"},
		{1, "ADC\tA, (HL)"},
		{1, "ADC\tA, A"},
		{1, "SUB\tB"},
		{1, "SUB\tC"},
		{1, "SUB\tD"},
		{1, "SUB\tE"},
		{1, "SUB\tH"},
		{1, "SUB\tL"},
		{1, "SUB\t(HL)"},
		{1, "SUB\tA"},
		{1, "SBC\tA, B"},
		{1, "SBC\tA, C"},
		{1, "SBC\tA, D"},
		{1, "SBC\tA, E"},
		{1, "SBC\tA, H"},
		{1, "SBC\tA, L"},
		{1, "SBC\tA, (HL)"},
		{1, "SBC\tA, A"},
		{1, "AND\tB"},
		{1, "AND\tC"},
		{1, "AND\tD"},
		{1, "AND\tE"},
		{1, "AND\tH"},
		{1, "AND\tL"},
		{1, "AND\t(HL)"},
		{1, "AND\tA"},
		{1, "XOR\tB"},
		{1, "XOR\tC"},
		{1, "XOR\tD"},
		{1, "XOR\tE"},
		{1, "XOR\tH"},
		{1, "XOR\tL"},
		{1, "XOR\t(HL)"},
		{1, "XOR\tA"},
		{1, "OR\tB"},
		{1, "OR\tC"},
		{1, "OR\tD"},
		{1, "OR\tE"},
		{1, "OR\tH"},
		{1, "OR\tL"},
		{1, "OR\t(HL)"},
		{1, "OR\tA"},
		{1, "CP\tB"},
		{1, "CP\tC"},
		{1, "CP\tD"},
		{1, "CP\tE"},
		{1, "CP\tH"},
		{1, "CP\tL"},
		{1, "CP\t(HL)"},
		{1, "CP\tA"},
		{1, "RET\tNZ"},
		{1, "POP\tBC"},
		{3, "JP\tNZ, 0x%04X"},
		{3, "JP\t0x%04X"},
		{3, "CALL\tNZ, 0x%04X"},
		{1, "PUSH\tBC"},
		{2, "ADD\tA, 0x%02X"},
		{1, "RST\t0x00"},
		{1, "RET\tZ"},
		{1, "RET"},
		{3, "JP\tZ, 0x%04X"},
		{4, "CB"},
		{3, "CALL\tZ, 0x%04X"},
		{3, "CALL\t0x%04X"},
		{2, "ADC\tA, 0x%02X"},
		{1, "RST\t0x08"},
		{1, "RET\tNC"},
		{1, "POP\tDE"},
		{3, "JP\tNC, 0x%04X"},
		{1, "NIL"},
		{3, "CALL\tNC, 0x%04X"},
		{1, "PUSH\tDE"},
		{2, "SUB\t0x%02X"},
		{1, "RST\t0x10"},
		{1, "RET\tC"},
		{1, "RETI"},
		{3, "JP\tC, 0x%04X"},
		{1, "NIL"},
		{3, "CALL\tC, 0x%04X"},
		{1, "NIL"},
		{2, "SBC\tA, 0x%02X"},
		{1, "RST\t0x18"},
		{2, "LDH\t(0x%02X), A"},
		{1, "POP\tHL"},
		{2, "LD\t(C), A"},
		{1, "NIL"},
		{1, "NIL"},
		{1, "PUSH\tHL"},
		{2, "AND\t0x%02X"},
		{1, "RST\t0x20"},
		{2, "ADD\tSP, %d"},
		{1, "JP\t(HL)"},
		{3, "LD\t(0x%04X), A"},
		{1, "NIL"},
		{1, "NIL"},
		{1, "NIL"},
		{2, "XOR\t0x%02X"},
		{1, "RST\t0x28"},
		{2, "LDH\tA, (0x%02X)"},
		{1, "POP\tAF"},
		{2, "LD\tA, (C)"},
		{1, "DI"},
		{1, "NIL"},
		{1, "PUSH\tAF"},
		{2, "OR\t0x%02X"},
		{1, "RST\t0x30"},
		{2, "LD\tHL, SP + %d"},
		{1, "LD\tSP, HL"},
		{3, "LD\tA, (0x%04X)"},
		{1, "EI"},
		{1, "NIL"},
		{1, "NIL"},
		{2, "CP\t0x%02X"},
		{1, "RST\t0x38"}
};


int debug_op_length(d8 opcode)
{
	return instruction_infos[opcode].length;
}


char* debug_op_mnemonic_format(d8 opcode)
{
	return instruction_infos[opcode].mnemonic_format;
}

char* debug_op_extended_mnemonic_format(d8 opcode)
{
	return extended_instruction_infos[opcode].mnemonic_format;
}

void debug_assert(bool expr, const char *msg)
{
#ifdef DEBUG
	if (!expr) {
		char *msg_buf = logger_get_msg_buffer();
		strncpy(msg_buf, msg, LOG_MESSAGE_MAX_SIZE);
		logger_log(LOG_ASSERT, "ASSERT", msg_buf);
		exit(1);
	}
#endif
}
