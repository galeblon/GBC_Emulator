#include<stdio.h>
#include"types.h"
#include"rom.h"

int rom_load(char* path){
	FILE* rom_fileptr = fopen(path, "rb");
	if(rom_fileptr == NULL){
		fprintf(stderr, "Couldn't open rom file");
		return 0;
	}
	fseek(rom_fileptr, 0, SEEK_END);
	long rom_len = ftell(rom_fileptr);
	rewind(rom_fileptr);

	fread(ROM, rom_len, 1, rom_fileptr);
	fclose(rom_fileptr);
	return 1;
}

int rom_checksum_validate(){
	u8 header_checksum = read8ROM(ROM_H_CHECKSUM);
	u8 header_checksum_verify = 0;
	for(int i=0x134; i<=0x14C; i++)
		header_checksum_verify -= read8ROM(i) + 1;
	printf("ROM header checksum: 0x%X\nROM header checksum calculated: 0x%X\n",
			header_checksum,
			header_checksum_verify);
	return header_checksum == header_checksum_verify;
}

void rom_print_title(){
	char* title = (void*)(ROM+ROM_H_TITLE);
	printf("ROM title: %.16s\n", title);
}
