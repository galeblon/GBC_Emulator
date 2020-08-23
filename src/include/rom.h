#ifndef ROM_H_
#define ROM_H_

#define MAX_ROM_SIZE 8388608

// ROM Header special addresses
enum rom_header_addr {
	ROM_ENTRY_POINT   = 0x0100,  // 4 B
	ROM_NINTENDO_LOGO = 0x0104,  // 48 B
	ROM_TITLE         = 0x0134,  // 16 B
	ROM_CGB_MODE      = 0x0143,  // 1 B
	ROM_CART_TYPE     = 0x0147,  // 1 B
	ROM_ROM_BANK_SIZE = 0x0148,  // 1 B
	ROM_RAM_BANK_SIZE = 0x0149,  // 1 B
	ROM_CHECKSUM      = 0x014D,  // 1 B
};

enum rom_cgb_mode {
	NON_CGB,
	CGB_SUPPORT,
	CGB_ONLY
};

int rom_checksum_validate(void);
void rom_get_title(char * title_buffer);
void rom_print_title(void);

#endif /* ROM_H_ */
