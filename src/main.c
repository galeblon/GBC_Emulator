#include<stdlib.h>
#include"display.h"
#include"gpu.h"
#include"ints.h"
#include"input.h"
#include"joypad.h"
#include"logger.h"
#include"mem.h"
#include"regs.h"
#include"rom.h"
#include"timer.h"
#include"types.h"
#include"sys.h"

static struct sys_args g_args;

int main(int argc, char *argv[])
{
	char *save_path = NULL;
	struct input_bindings *input_bindings = NULL;

	if(!logger_prepare())
		return 1;

	if (!sys_parse_args(argc, argv, &g_args))
		return 1;

	if (g_args.save_path[0] != '\0')
		save_path = g_args.save_path;
	if (g_args.input_bindings.filled)
		input_bindings = &g_args.input_bindings;

	if (!mem_prepare(g_args.rom_path, save_path))
		return 1;

	logger_print(LOG_INFO, "Loaded ROM contents to memory.\n");

	if (!rom_checksum_validate())
		logger_print(LOG_WARN, "ROM header checksum failed.\n");

	logger_print(LOG_INFO, "ROM header checksum passed.\n");
	char title[16];
	rom_get_title(title);
	logger_print(LOG_INFO, "ROM title: %.16s\n", title);

	//TODO prepare memory and fill stack with data according to powerup sequence
	//TODO prepare sound
	cpu_prepare();
	ints_prepare();
	gpu_prepare(title, g_args.frame_rate, g_args.fullscreen);
	if(!input_prepare(input_bindings))
		return 1;
	joypad_prepare();
	timer_prepare();

	logger_print(LOG_INFO, "Starting emulation.\n");
	int cycles_delta = 0;

	// Main Loop
	while ( cycles_delta != -1 && !display_get_closed_status() ) {
		cycles_delta = cpu_single_step();
		gpu_step(cpu_is_double_speed() ? cycles_delta/2 : cycles_delta);
		mem_step(cycles_delta);
		// sound_step(cycles_delta)
		joypad_step();
		timer_step(cycles_delta);
		ints_check();
	}

	logger_print(LOG_INFO, "Halting emulation.\n");

	gpu_destroy();
	mem_destroy(save_path);
	logger_destroy();

	return 0;
}
