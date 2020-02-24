#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>

typedef unsigned char byte;
byte* create_rom_buffer(char* path);

int main(int argc, char* argv[]){

	if(argc < 2){
		printf("ROM file not specified.\n");
		return 1;
	}

	byte* rom_buffer = create_rom_buffer(argv[1]);
	printf("Loaded ROM contents to memory.\n");

	byte header_checksum = *(byte*)(rom_buffer + 0x14d);
	byte header_checksum_verify = 0;
	for(int i=0x134; i<=0x14c; i++){
		header_checksum_verify = header_checksum_verify - *(byte*)(rom_buffer + i)-1;
	}
	if(header_checksum != header_checksum_verify){
		fprintf(stderr, "ROM header checksum failed.\n");
		return 1;
	} else {
		printf("ROM header checksum passed.\n");
	}

	//uint16_t first_instruction = *(uint16_t*)(rom_buffer + 0x102);

	free(rom_buffer);
	return 0;
}


byte* create_rom_buffer(char* path){
	FILE* rom_fileptr = fopen(path, "rb");
	if(rom_fileptr == NULL){
		fprintf(stderr, "Couldn't open rom file");
		return NULL;
	}
	fseek(rom_fileptr, 0, SEEK_END);
	long rom_len = ftell(rom_fileptr);
	rewind(rom_fileptr);

	char* rom_buffer = (char*) malloc(rom_len * sizeof(char));
	fread(rom_buffer, rom_len, 1, rom_fileptr);
	fclose(rom_fileptr);

	return rom_buffer;
}
