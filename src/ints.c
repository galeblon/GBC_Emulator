#include"cpu.h"
#include"ints.h"
#include"logger.h"
#include"mem.h"
#include"regs.h"
#include"stdio.h"
#include"types.h"

#define IFAddress 0xFF0F
#define IEAddress 0xFFFF
#define IFBase 0x00
#define IEBase 0x1F

#define readIF mem_read8(IFAddress)
#define readIE mem_read8(IEAddress)
#define writeIF(val) mem_write8(IFAddress, val)
#define writeIE(val) mem_write8(IEAddress, val)

#define B0 0x01
#define B1 0x02
#define B2 0x04
#define B3 0x08
#define B4 0x10
#define B5 0x20
#define B6 0x40
#define B7 0x80

#define isF0(reg) ((reg & B0) != 0)
#define isF1(reg) ((reg & B1) != 0)
#define isF2(reg) ((reg & B2) != 0)
#define isF3(reg) ((reg & B3) != 0)
#define isF4(reg) ((reg & B4) != 0)
#define isF5(reg) ((reg & B5) != 0)
#define isF6(reg) ((reg & B6) != 0)
#define isF7(reg) ((reg & B7) != 0)

#define noF0(reg) (reg & ~B0)
#define noF1(reg) (reg & ~B1)
#define noF2(reg) (reg & ~B2)
#define noF3(reg) (reg & ~B3)
#define noF4(reg) (reg & ~B4)
#define noF5(reg) (reg & ~B5)
#define noF6(reg) (reg & ~B6)
#define noF7(reg) (reg & ~B7)

static bool g_ime;

static d8 g_old_if = 0;

static void _ints_undefined_int_info(d8 i_e, d8 i_f)
{
	char *message = logger_get_msg_buffer();
	snprintf(
		message,
		LOG_MESSAGE_MAX_SIZE,
		"[INTERRUPT MODULE] UNDEFINED BEHAVIOUR ERROR - INTERRUPT TRIGGERED DOES NOT EXIST.\n    IF: 0x%X    IE: 0x%X\n",
		i_f,
		i_e
	);
	logger_log(
		LOG_WARN,
		"UNDEFINED INTERRUPT",
		message
	);
}


void ints_set_ime(void)
{
	g_ime = 1;
}


void ints_reset_ime(void)
{
	g_ime = 0;
}


void ints_prepare(void)
{
	ints_set_ime();
	writeIF(IFBase);
	writeIE(IEBase);
}


void ints_check(void)
{
	// Resolve cpu halted/stopped state
	// Doesn't require interrupts service to be enabled
	if (g_old_if != readIF) {
		cpu_set_halted(0);
		cpu_set_stopped(0);
	}

	g_old_if = readIF;

	// Are interrupts enabled at all?
	if (!g_ime)
		return;

	// Check which interrupts are enabled
	d8 IF = readIF;
	d8 IE = readIE;
	IF &= IE;
	if (!IF)
		return;
	ints_reset_ime();

	// Resolve interrupt
	if (isF0(IF)) {
		cpu_call(0x0040);

		// Clear interrupt register
		writeIF(noF0(IF));
	}
	else if (isF1(IF)) {
		cpu_call(0x0048);

		// Clear interrupt register
		writeIF(noF1(IF));
	}
	else if (isF2(IF)) {
		cpu_call(0x0050);

		// Clear interrupt register
		writeIF(noF2(IF));
	}
	else if (isF3(IF)) {
		cpu_call(0x0058);

		// Clear interrupt register
		writeIF(noF3(IF));
	}
	else if (isF4(IF)) {
		cpu_call(0x0060);

		// Clear interrupt register
		writeIF(noF4(IF));
	}
	else {
		// Logging the problem
		_ints_undefined_int_info(IE, IF);

		// Clearing IE to default value in attempt of recovery
		writeIE(IEBase);
	}
}


void ints_request(enum ints_interrupt_type interrupt)
{
	d8 IF = readIF;
	switch(interrupt) {
	case 0:
		IF |= B0;
		break;
	case 1:
		IF |= B1;
		break;
	case 2:
		IF |= B2;
		break;
	case 3:
		IF |= B3;
		break;
	case 4:
		IF |= B4;
		break;
	case 5:
		IF |= B5;
		break;
	case 6:
		IF |= B6;
		break;
	case 7:
		IF |= B7;
		break;
	}
	writeIF(IF);
}
