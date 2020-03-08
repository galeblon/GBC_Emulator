#ifndef ROM_H_
#define ROM_H_

#define MAX_ROM_SIZE 8388608

// ROM Header special addresses
// 4 bytes
#define ROM_ENTRY_POINT 0x100
// 48 bytes
#define ROM_NINTENDO_LOGO 0x104
// 16 bytes
#define ROM_TITLE 0x134
// 1 byte
#define ROM_CHECKSUM 0x14D

int rom_checksum_validate(void);
void rom_print_title(void);

#endif /* ROM_H_ */
