#include"cpu.h"
#include"debug.h"
#include"ints.h"
#include"logger.h"
#include"mem_priv.h"
#include"regs.h"
#include"types.h"

#define IFAddress 0xFF0F
#define IEAddress 0xFFFF
#define IFBase 0xE1
#define IEBase 0x00

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

static u8 g_if = 0;
static u8 g_ie = 1;
static u8 g_old_if = 0;

static void _ints_undefined_int_info(u8 i_e, u8 i_f)
{
	logger_log(
		LOG_WARN,
		"UNDEFINED INTERRUPT",
		"[INTERRUPT MODULE] UNDEFINED BEHAVIOUR ERROR - INTERRUPT TRIGGERED DOES NOT EXIST.\n    IF: 0x%X    IE: 0x%X\n",
		i_f,
		i_e
	);
}

static u8 _ints_read_handler(a16 addr)
{
	switch(addr) {
		case IFAddress:
			return g_if;
			break;
		case IEAddress:
			return g_ie;
			break;
	}

	debug_assert(false, "_ints_read_handler: invalid address");
	return 0;
}

static void _ints_write_handler(a16 addr, u8 data)
{
	switch(addr) {
		case IFAddress:
			g_if = data;
			break;
		case IEAddress:
			g_ie = data;
			break;
		default:
			debug_assert(false, "_ints_read_handler: invalid address");
	}
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
	g_if = IFBase;
	g_ie = IEBase;

	mem_register_handlers(IFAddress,
			_ints_read_handler, _ints_write_handler);
	mem_register_handlers(IEAddress,
			_ints_read_handler, _ints_write_handler);
}


void ints_check(void)
{
	// Resolve cpu halted/stopped state
	// Doesn't require interrupts service to be enabled
	if(g_if & g_ie) {
		cpu_set_halted(0);
		cpu_set_stopped(0);
	}

	g_old_if = g_if;

	// Are interrupts enabled at all?
	if (!g_ime)
		return;

	// Check which interrupts are enabled
	g_if &= g_ie;
	if (!g_if)
		return;
	ints_reset_ime();

	// Resolve interrupt
	if (isF0(g_if)) {
		cpu_call(0x0040);

		// Clear interrupt register
		g_if = noF0(g_if);
	}
	else if (isF1(g_if)) {
		cpu_call(0x0048);

		// Clear interrupt register
		g_if = noF1(g_if);
	}
	else if (isF2(g_if)) {
		cpu_call(0x0050);

		// Clear interrupt register
		g_if = noF2(g_if);
	}
	else if (isF3(g_if)) {
		cpu_call(0x0058);

		// Clear interrupt register
		g_if = noF3(g_if);
	}
	else if (isF4(g_if)) {
		cpu_call(0x0060);

		// Clear interrupt register
		g_if = noF4(g_if);
	}
	else {
		// Logging the problem
		_ints_undefined_int_info(g_ie, g_if);

		// Clearing IE to default value in attempt of recovery
		g_ie = IEBase;
	}
}


void ints_request(enum ints_interrupt_type interrupt)
{
	switch(interrupt) {
	case INT_V_BLANK:
		g_if |= B0;
		break;
	case INT_LCDC:
		g_if |= B1;
		break;
	case INT_TIMER_OVERFLOW:
		g_if |= B2;
		break;
	case INT_SERIAL_IO_TRANSFER_COMPLETE:
		g_if |= B3;
		break;
	case INT_HIGH_TO_LOW_P10_P13:
		g_if |= B4;
		break;
	default:
		break;
	}
}
