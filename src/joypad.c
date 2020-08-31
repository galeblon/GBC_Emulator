#include<allegro5/allegro5.h>
#include<stdio.h>
#include"ints.h"
#include"joypad.h"
#include"player_input.h"
#include"types.h"

static struct all_inputs g_all_inputs;

static struct joypad_register g_input_presses;

void _joypad_update_register(void);

void joypad_prepare(void)
{
	g_input_presses.LEFT_B = 1;
	g_input_presses.RIGHT_A = 1;
	g_input_presses.UP_SELECT = 1;
	g_input_presses.DOWN_START = 1;

	g_input_presses.SELECT_BUTTONS = 1;
	g_input_presses.SELECT_DIRECTIONS = 1;
}

void joypad_step(void)
{
	player_input_check_queue(&g_all_inputs);

	_joypad_update_register();
}

void _joypad_update_register(void)
{
	struct joypad_register old_state = g_input_presses;

	g_input_presses.DOWN_START = !((g_all_inputs.DOWN && !g_input_presses.SELECT_DIRECTIONS)
							|| (g_all_inputs.START && !g_input_presses.SELECT_BUTTONS));
	g_input_presses.LEFT_B = !((g_all_inputs.LEFT && !g_input_presses.SELECT_DIRECTIONS)
							|| (g_all_inputs.B && !g_input_presses.SELECT_BUTTONS));
	g_input_presses.RIGHT_A = !((g_all_inputs.RIGHT && !g_input_presses.SELECT_DIRECTIONS)
							|| (g_all_inputs.A && !g_input_presses.SELECT_BUTTONS));
	g_input_presses.UP_SELECT = !((g_all_inputs.UP && !g_input_presses.SELECT_DIRECTIONS)
							|| (g_all_inputs.SELECT && !g_input_presses.SELECT_BUTTONS));

	bool ask_for_interrupt = false;
	if(old_state.DOWN_START > g_input_presses.DOWN_START)
		ask_for_interrupt = true;
	if(old_state.UP_SELECT > g_input_presses.UP_SELECT)
		ask_for_interrupt = true;
	if(old_state.LEFT_B > g_input_presses.LEFT_B)
		ask_for_interrupt = true;
	if(old_state.RIGHT_A > g_input_presses.RIGHT_A)
		ask_for_interrupt = true;

	if(ask_for_interrupt)
		ints_request(INT_HIGH_TO_LOW_P10_P13);
}


d8 joypad_get_register_state(void)
{
	d8 reg = 0;
	reg |= g_input_presses.RIGHT_A << 0;
	reg |= g_input_presses.LEFT_B << 1;
	reg |= g_input_presses.UP_SELECT << 2;
	reg |= g_input_presses.DOWN_START << 3;
	reg |= g_input_presses.SELECT_DIRECTIONS << 4;
	reg |= g_input_presses.SELECT_BUTTONS << 5;

	return reg;
}

void joypad_set_register_selection(bool select_buttons, bool select_directions)
{
	g_input_presses.SELECT_BUTTONS = !select_buttons;
	g_input_presses.SELECT_DIRECTIONS = !select_directions;
}

