#include<allegro5/allegro5.h>
#include"debug.h"
#include"input.h"
#include"ints.h"
#include"joypad.h"
#include"mem_priv.h"
#include"types.h"

#define JOYPAD_INPUT_ADDR 0xFF00

static struct all_inputs g_all_inputs;

static enum joypad_reg_mode {
	MODE_INIT,
	MODE_BUTTON,
	MODE_DIRECTIONAL,
} g_joypad_mode = MODE_INIT;

static u8 _joypad_read_handler(a16 addr __attribute__((unused)))
{
	debug_assert(addr == JOYPAD_INPUT_ADDR,
			"_joypad_read_handler: invalid address");

	if (g_joypad_mode == MODE_INIT)
		return 0x3F;

	struct joypad_register_bits bits = {
		.RIGHT_A    = !((g_joypad_mode == MODE_BUTTON) ? g_all_inputs.A      : g_all_inputs.RIGHT),
		.LEFT_B     = !((g_joypad_mode == MODE_BUTTON) ? g_all_inputs.B      : g_all_inputs.LEFT),
		.DOWN_START = !((g_joypad_mode == MODE_BUTTON) ? g_all_inputs.START  : g_all_inputs.DOWN),
		.UP_SELECT  = !((g_joypad_mode == MODE_BUTTON) ? g_all_inputs.SELECT : g_all_inputs.UP),
		.SELECT_DIRECTIONS = !((g_joypad_mode == MODE_DIRECTIONAL)),
		.SELECT_BUTTONS = !((g_joypad_mode == MODE_BUTTON))
	};

	struct joypad_register reg;
	reg.BITS = bits;

	return reg.REG;
}

static void _joypad_write_handler(a16 addr __attribute__((unused)), u8 data)
{
	debug_assert(addr == JOYPAD_INPUT_ADDR,
			"_joypad_write_handler: invalid address");

	if (BV(data, 5) == 0 && BV(data, 4) == 1) {
		g_joypad_mode = MODE_BUTTON;
	} else if (BV(data, 5) == 1 && BV(data, 4) == 0) {
		g_joypad_mode = MODE_DIRECTIONAL;
	}
}

void _joypad_check_interrupt(struct all_inputs *prev)
{
	switch(g_joypad_mode) {
		case MODE_DIRECTIONAL:
			if ((prev->DOWN == 1 && g_all_inputs.DOWN == 0)
					|| (prev->UP    == 1 && g_all_inputs.UP    == 0)
					|| (prev->LEFT  == 1 && g_all_inputs.LEFT  == 0)
					|| (prev->RIGHT == 1 && g_all_inputs.RIGHT == 0))
				ints_request(INT_HIGH_TO_LOW_P10_P13);
			break;
		case MODE_BUTTON:
			if ((prev->START == 1 && g_all_inputs.START == 0)
					|| (prev->SELECT == 1 && g_all_inputs.SELECT == 0)
					|| (prev->A      == 1 && g_all_inputs.A      == 0)
					|| (prev->B      == 1 && g_all_inputs.B      == 0))
				ints_request(INT_HIGH_TO_LOW_P10_P13);
			break;
		default:
			break;
	}
}

void joypad_prepare(void)
{
	mem_register_handlers(JOYPAD_INPUT_ADDR,
			_joypad_read_handler, _joypad_write_handler);
}

void joypad_step(void)
{
	struct all_inputs prev_inputs = g_all_inputs;
	input_check_queue(&g_all_inputs);
	_joypad_check_interrupt(&prev_inputs);
}
