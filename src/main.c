#include<stdio.h>
#include<stdlib.h>
#include"cpu.h"
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

	if (!mem_load_rom(argv[1]))
		return 1;

	printf("Loaded ROM contents to memory.\n");

	if (!rom_checksum_validate())
		fprintf(stderr, "ROM header checksum failed.\n");

	printf("ROM header checksum passed.\n");
	rom_print_title();

	//TODO prepare memory and fill stack with data according to powerup sequence
	//TODO prepare gpu
	//TODO prepare sound
	//TODO prepare joypads
	cpu_prepare();

	printf("Starting emulation.\n");
	int cycles_delta = 0;

	// Main Loop
	while (cycles_delta != -1) {
		cycles_delta = cpu_single_step();
		// gpu_step(cycles_delta)
		// sound_step(cycles_delta)
		// joypad
		// interrupts_handling
	}

	printf("Halting emulation.\n");

	return 0;
}
