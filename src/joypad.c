#include<allegro5/allegro5.h>
#include <input.h>
#include<stdio.h>
#include"ints.h"
#include"joypad.h"
#include"types.h"

static struct all_inputs g_all_inputs;

static struct joypad_register g_input_presses;

void _joypad_update_register(void);

void joypad_prepare(void)
{
	g_input_presses.REG = 0x3F;
}

void joypad_step(void)
{
	input_check_queue(&g_all_inputs);

	//TODO: g_input_presses.REG = mem_private_read8(0xFF00);
	_joypad_update_register();
	//TODO mem_private_write8(0xFF00, g_input_presses.REG);
}

void _joypad_update_register(void)
{
	struct joypad_register old_state = g_input_presses;

	g_input_presses.BITS.DOWN_START = !((g_all_inputs.DOWN && !g_input_presses.BITS.SELECT_DIRECTIONS)
							|| (g_all_inputs.START && !g_input_presses.BITS.SELECT_BUTTONS));
	g_input_presses.BITS.LEFT_B = !((g_all_inputs.LEFT && !g_input_presses.BITS.SELECT_DIRECTIONS)
							|| (g_all_inputs.B && !g_input_presses.BITS.SELECT_BUTTONS));
	g_input_presses.BITS.RIGHT_A = !((g_all_inputs.RIGHT && !g_input_presses.BITS.SELECT_DIRECTIONS)
							|| (g_all_inputs.A && !g_input_presses.BITS.SELECT_BUTTONS));
	g_input_presses.BITS.UP_SELECT = !((g_all_inputs.UP && !g_input_presses.BITS.SELECT_DIRECTIONS)
							|| (g_all_inputs.SELECT && !g_input_presses.BITS.SELECT_BUTTONS));

	if(old_state.BITS.DOWN_START > g_input_presses.BITS.DOWN_START
			|| old_state.BITS.UP_SELECT > g_input_presses.BITS.UP_SELECT
			|| old_state.BITS.LEFT_B > g_input_presses.BITS.LEFT_B
			|| old_state.BITS.RIGHT_A > g_input_presses.BITS.RIGHT_A)
		ints_request(INT_HIGH_TO_LOW_P10_P13);
}
