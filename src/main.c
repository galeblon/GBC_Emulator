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

int main(int argc, char *argv[])
{
	logger_prepare();
	if (argc < 2) {
		logger_print(LOG_FATAL, "ROM file not specified.\n");
		return 1;
		//if (!mem_prepare("./SML2_UE.gb"))
		//	return 1;
	}

	if (!mem_prepare(argv[1]))
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
	gpu_prepare(title);
	input_prepare();
	joypad_prepare();
	timer_prepare();

	logger_print(LOG_INFO, "Starting emulation.\n");
	int cycles_delta = 0;

	// Main Loop
	while ( cycles_delta != -1 && !display_get_closed_status() ) {
		cycles_delta = cpu_single_step();
		gpu_step(cycles_delta);
		mem_step(cycles_delta);
		// sound_step(cycles_delta)
		joypad_step();
		timer_step(cycles_delta);
		ints_check();
	}

	logger_print(LOG_INFO, "Halting emulation.\n");

	gpu_destroy();
	mem_destroy();
	logger_destroy();

	return 0;
}
