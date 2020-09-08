#include<time.h>
#include"debug.h"
#include"rom.h"

enum mem_rtc_state {
	RTC_NONE = 0,
	RTC_00,
	RTC_LATCHED,
	RTC_LATCHED_00
};

struct mem_rtc_regs {
	u8 sec;
	u8 min;
	u8 hour;
	u8 day_lower;
	union {
		struct {
			u8 day_carry : 1;
			u8 unused : 5;
			u8 halt : 1;
			u8 day_bit_9 : 1;
		} __attribute__((packed));
		u8 day_higher;
	};
};

static enum mem_rtc_state g_rtc_state = RTC_NONE;
static bool g_rtc_halt = true;

static struct mem_rtc_regs g_rtc_regs = {0}, g_rtc_regs_on_halt = {0};
static time_t g_rtc_origin = 0;

static struct mem_rtc_regs _mem_rtc_make_regs(time_t current)
{
	struct mem_rtc_regs regs = {0};

	// If RTC is halted return register state saved on halt
	if (g_rtc_halt)
		return g_rtc_regs_on_halt;

	// Do not update RTC time-keeping registers when RTC is halted
	if (current == 0)
		current = time(NULL);

	time_t diff = difftime(current, g_rtc_origin);

	// Calculate values in RTC registers based on difference between
	// set origin time and current time
	regs.sec = diff % 60;
	diff /= 60;
	regs.min = diff % 60;
	diff /= 60;
	regs.hour = diff % 24;
	diff /= 24;
	regs.day_lower = diff & 0xFF;
	regs.day_bit_9 = (diff & 0x100) >> 8;
	regs.day_carry = (diff & 0x200) >> 9;
	regs.halt = g_rtc_halt ? 1 : 0;

	return regs;
}

static inline void _mem_rtc_halt(void)
{
	// Save register state as if it would be in the moment of halt.
	// Until the registers are latched, the old value must be shown, so we
	// can't update them right away. RTC registers are set to
	// g_rtc_regs_on_halt when RTC is halted and the registers are latched.
	g_rtc_regs_on_halt = _mem_rtc_make_regs(0);
	g_rtc_regs_on_halt.halt = 1;
	g_rtc_halt = true;
}

static inline void _mem_rtc_resume(time_t current)
{
	g_rtc_halt = false;

	if (current == 0)
		current = time(NULL);

	// Calculate new origin time based on current state of the RTC registers
	current -= g_rtc_regs_on_halt.sec;
	current -= g_rtc_regs_on_halt.min * 60;
	current -= g_rtc_regs_on_halt.hour * 60 * 60;
	current -= g_rtc_regs_on_halt.day_lower * 60 * 60 * 24;
	current -= g_rtc_regs_on_halt.day_bit_9 * 60 * 60 * 24 * (0x100);
	current -= g_rtc_regs_on_halt.day_carry * 60 * 60 * 24 * (0x200);
	g_rtc_origin = current;
}

/* Read memory from selected RTC register.
 *
 * RTC registers need to be latched before reading. The values accessible in
 * RTC registers are set when latched, so these never show current time.
 */
u8 mem_rtc_read(u8 reg)
{
	debug_assert(rom_get_header()->mbc == MBC3, "_mem_rtc_read: RTC unsupported on this MBC");

	if (g_rtc_state != RTC_LATCHED && g_rtc_state != RTC_LATCHED_00)
		// TODO: can RTC registers be read without latching first?
		return 0;

	switch(reg) {
		case 0x08:
			return g_rtc_regs.sec;
		case 0x09:
			return g_rtc_regs.min;
		case 0x0A:
			return g_rtc_regs.hour;
		case 0x0B:
			return g_rtc_regs.day_lower;
		case 0x0C:
			return g_rtc_regs.day_higher;
		default:
			debug_assert(false, "_mem_rtc_read: invalid RTC register");
	}

	return 0;
}

/* Write data into RTC registers.
 *
 * Writing is only allowed when RTC is halted. One exception is writing the
 * halt bit to 0x0C register.
 *
 * Writing values into RTC registers sets the time, which will become live
 * after resuming RTC.
 */
void mem_rtc_write(u8 reg, u8 data)
{
	debug_assert(rom_get_header()->mbc == MBC3, "_mem_rtc_write: RTC unsupported on this MBC");

	// ignore write if RTC is not halted and it is not halt flag written
	if (!g_rtc_halt && reg != 0x0C && !(data & 0x40))
		return;

	switch(reg) {
		case 0x08:
			g_rtc_regs_on_halt.sec = (data < 60) ? data : 0;
			break;
		case 0x09:
			g_rtc_regs_on_halt.min = (data < 60) ? data : 0;
			break;
		case 0x0A:
			g_rtc_regs_on_halt.hour = (data < 24) ? data : 0;
			break;
		case 0x0B:
			g_rtc_regs_on_halt.day_lower = data;
			break;
		case 0x0C:
			if (g_rtc_regs_on_halt.halt == 0 && (data & 0x40)) {
				_mem_rtc_halt();
			} else if (g_rtc_regs_on_halt.halt == 1 && !(data & 0x40)) {
				_mem_rtc_resume(0);
			} else {
				g_rtc_regs_on_halt.day_higher = data & 0xC1;
			}
			break;
		default:
			debug_assert(false, "_mem_rtc_write: invalid RTC register");
	}
}

/* Handle RTC latching.
 *
 * RTC is latched into readable registers when correct sequence of bytes is
 * written.
 *
 * data - byte written to memory area used for RTC latching
 */
void mem_rtc_latch(u8 data)
{
	debug_assert(rom_get_header()->mbc == MBC3, "_mem_rtc_latch: RTC unsupported on this MBC");

	switch (g_rtc_state) {
		case RTC_NONE:
			if (data == 0x00)
				g_rtc_state = RTC_00;
			break;
		case RTC_00:
			if (data == 0x01) {
				g_rtc_state = RTC_LATCHED;
				g_rtc_regs = _mem_rtc_make_regs(0);
			} else {
				g_rtc_state = RTC_NONE;
			}
			break;
		case RTC_LATCHED:
			if (data == 0x00) {
				g_rtc_state = RTC_LATCHED_00;
			}
			break;
		case RTC_LATCHED_00:
			if (data == 0x01) {
				g_rtc_regs = _mem_rtc_make_regs(0);
			}
			g_rtc_state = RTC_LATCHED;
			break;
		default:
			debug_assert(false, "_mem_rtc_latch: invalid RTC latch state");
	}
}

/* Initialize and unhalt RTC
 */
void mem_rtc_prepare(void)
{
	_mem_rtc_resume(0);
}
