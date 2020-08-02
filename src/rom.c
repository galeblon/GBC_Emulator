#include<stdio.h>
#include"mem.h"
#include"rom.h"
#include"types.h"

int rom_checksum_validate(void)
{
	u8 header_checksum = mem_read8(ROM_CHECKSUM);
	u8 header_checksum_verify = 0;
	for (int i=0x134; i<=0x14C; i++)
		header_checksum_verify -= mem_read8(i) + 1;
	printf("ROM header checksum: 0x%X\nROM header checksum calculated: 0x%X\n",
		header_checksum, header_checksum_verify);
	return header_checksum == header_checksum_verify;
}

void rom_get_title(char * title_buffer)
{
	for (a16 i = 0; i < 16; i++)
		title_buffer[i] = mem_read8(ROM_TITLE + i);
}

void rom_print_title(void)
{
	char title[16];
	rom_get_title(title);
	printf("ROM title: %.16s\n", title);
}
