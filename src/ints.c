#include"cpu.h"
#include"ints.h"
#include"logger.h"
#include"mem.h"
#include"regs.h"
#include"stdio.h"

#define IFAddress 0xFF0F
#define IEAddress 0xFFFF

#define readIF mem_read8(IFAddress)
#define readIE mem_read8(IEAddress)
#define writeIF(val) mem_write16(IFAddress, val)
#define writeIE(val) mem_write16(IEAddress, val)

#define isF0(reg) (reg & 0x01) == 1
#define isF1(reg) (reg & 0x02) == 1
#define isF2(reg) (reg & 0x04) == 1
#define isF3(reg) (reg & 0x08) == 1
#define isF4(reg) (reg & 0x10) == 1
#define isF5(reg) (reg & 0x20) == 1
#define isF6(reg) (reg & 0x40) == 1
#define isF7(reg) (reg & 0x80) == 1


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
	writeIF(0x00);
	writeIE(0x1F);
}


void ints_check(void)
{
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
	if (isF0(IF))
		cpu_jump_push(0x0040);
	else if (isF1(IF))
		cpu_jump_push(0x0048);
	else if (isF2(IF))
		cpu_jump_push(0x0050);
	else if (isF3(IF))
		cpu_jump_push(0x0058);
	else if (isF4(IF))
		cpu_jump_push(0x0060);
	else {
		// Logging the problem
		_ints_undefined_int_info(IE, IF);

		// Clearing IE to default value in attempt of recovery
		writeIE(0x1F);
	}

	// Clear interrupt register
	writeIF(0x00);
}
