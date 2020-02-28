#ifndef ROM_H_
#define ROM_H_

#include"types.h"

#define MAX_ROM_SIZE 8388608

// ROM Header special addresses
// 4 bytes
#define ROM_H_ENTRY_POINT 0x100
// 48 bytes
#define ROM_H_NINTENDO_LOGO 0x104
// 16 bytes
#define ROM_H_TITLE 0x134
// 1 byte
#define ROM_H_CHECKSUM 0x14D

#define read8ROM(addr) (*(d8 *)(ROM + addr))
#define read16ROM(addr) (*(d16 *)(ROM + addr))

u8 ROM[MAX_ROM_SIZE];

int rom_load(char *path);

int rom_checksum_validate();

void rom_print_title();

#endif /* ROM_H_ */
