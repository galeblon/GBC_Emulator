#include<stdio.h>
#include<stdlib.h>
#include"display.h"
#include"ints.h"
#include"gpu.h"
#include"mem.h"
#include"regs.h"
#include"rom.h"
#include"types.h"

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("ROM file not specified.\n");
		return 1;
	}

	if (!mem_prepare(argv[1]))
		return 1;

	printf("Loaded ROM contents to memory.\n");

	if (!rom_checksum_validate())
		fprintf(stderr, "ROM header checksum failed.\n");

	printf("ROM header checksum passed.\n");
	char title[16];
	rom_get_title(title);
	printf("ROM title: %.16s\n", title);

	//TODO prepare memory and fill stack with data according to powerup sequence
	//TODO prepare sound
	//TODO prepare joypads
	cpu_prepare();
	ints_prepare();
	gpu_prepare(title);

	printf("Starting emulation.\n");
	int cycles_delta = 0;

	// Main Loop
	while ( cycles_delta != -1 && !display_get_closed_status() ) {
		cycles_delta = cpu_single_step();
		gpu_step(cycles_delta);
		mem_step(cycles_delta);
		// sound_step(cycles_delta)
		// joypad
		ints_check();
	}

	printf("Halting emulation.\n");

	gpu_destroy();
	mem_destroy();

	return 0;
}
