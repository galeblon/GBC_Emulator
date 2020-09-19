#include"debug.h"
#include"ints.h"
#include"mem_priv.h"
#include"types.h"

#define DIV_ADDR  0xFF04
#define TIMA_ADDR 0xFF05
#define TMA_ADDR  0xFF06
#define TAC_ADDR  0xFF07

static struct {
	u16 div;
	u8 tima;
	u8 tma;
	union {
		struct {
			u8 clk_select : 2;
			u8 timer_en   : 1;
			u8 unused     : 5;
		};
		u8 tac;
	};
} g_timer_reg = {0};

static void _timer_inc_tima(void)
{
	if (g_timer_reg.tima == 0xFF) {
		ints_request(INT_TIMER_OVERFLOW);
		g_timer_reg.tima = g_timer_reg.tma;
	} else {
		g_timer_reg.tima++;
	}
}

static u8 _timer_clock_bit(void)
{
	switch(g_timer_reg.clk_select) {
		case 0x00:
			return 9;
		case 0x01:
			return 3;
		case 0x02:
			return 5;
		case 0x03:
			return 7;
	}

	debug_assert(false, "_timer_clock_bit: invalid clock select");
	return 0;
}

static inline u8 _timer_get_tima_driver(void)
{
	return BV(g_timer_reg.div, _timer_clock_bit());
}

static void _timer_reset_div(void)
{
	// If the bit that's clocking the TIMA is set, then 1 -> 0 transition will
	// be detected and TIMA will be increased
	if (_timer_get_tima_driver() == 1)
		_timer_inc_tima();

	g_timer_reg.div = 0;
}

static void _timer_update_tac(u8 val)
{
	u8 driver_prev = _timer_get_tima_driver();
	g_timer_reg.tac = val;

	// If the bit that's driving TIMA is changed (1 -> 0) increment TIMA
	if (driver_prev == 1 && _timer_get_tima_driver() == 0)
		_timer_inc_tima();
}

static u8 _timer_read_handler(a16 addr)
{
	switch(addr) {
		case DIV_ADDR:
			return g_timer_reg.div & 0xFF;
		case TIMA_ADDR:
			return g_timer_reg.tima;
		case TMA_ADDR:
			return g_timer_reg.tma;
		case TAC_ADDR:
			return g_timer_reg.tac;
	}

	debug_assert(false, "_timer_read_handler: invalid address");
	return 0;
}

static void _timer_write_handler(a16 addr, u8 data)
{
	switch(addr) {
		case DIV_ADDR:
			_timer_reset_div();
			break;
		case TIMA_ADDR:
			g_timer_reg.tima = data;
			break;
		case TMA_ADDR:
			g_timer_reg.tma = data;
			break;
		case TAC_ADDR:
			_timer_update_tac(data);
			break;
		default:
			debug_assert(false, "_timer_write_handler: invalid address");
	}
}

static void _timer_update(int cycles)
{
	u8 clock_bit = _timer_clock_bit();

	// TODO: [optimization] Update TIMA combinatorically instead of iteratively
	for (int i = 0; i < cycles; i++) {
		g_timer_reg.div++;

		// Increment TIMA when the clock bit changes 1 -> 0
		if (BV(g_timer_reg.div - 1, clock_bit) == 1
				&& _timer_get_tima_driver() == 0)
			_timer_inc_tima();
	}
}

void timer_step(int cycles_delta)
{
	if (g_timer_reg.timer_en)
		_timer_update(cycles_delta);
	else
		g_timer_reg.div += cycles_delta;
}

void timer_prepare(void)
{
	mem_register_handlers(DIV_ADDR,  _timer_read_handler, _timer_write_handler);
	mem_register_handlers(TIMA_ADDR, _timer_read_handler, _timer_write_handler);
	mem_register_handlers(TMA_ADDR,  _timer_read_handler, _timer_write_handler);
	mem_register_handlers(TAC_ADDR,  _timer_read_handler, _timer_write_handler);
}
