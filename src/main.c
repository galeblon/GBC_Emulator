#include<stdio.h>
#include<stdlib.h>
#include"types.h"
#include"rom.h"

int main(int argc, char* argv[]){

	if(argc < 2){
		printf("ROM file not specified.\n");
		return 1;
	}

	if(!rom_load(argv[1])){
		return 1;
	}
	printf("Loaded ROM contents to memory.\n");

	if(!rom_checksum_validate()){
		fprintf(stderr, "ROM header checksum failed.\n");
	}
	printf("ROM header checksum passed.\n");

	rom_print_title();

	// ROM entry point is typically 4 bytes: NOP, JMP optcode, 2 byt address
	// this doesn't have to be always true
	// It should be safe to just interpret the instructions with cpu emulation
	a16 first_instruction = read16ROM(ROM_H_ENTRY_POINT+2);
	printf("ROM first instruction address: 0x%X\n", first_instruction);

	return 0;
}
