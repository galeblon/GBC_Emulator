#ifndef ROM_H_
#define ROM_H_

#include"types.h"

#define MAX_ROM_SIZE 8388608
#define KB 1024

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
	NON_CGB = 0,
	NON_CGB_UNINITIALIZED_PALETTES,
	CGB_SUPPORT,
	CGB_ONLY
};

enum rom_mbc {
	ROM_ONLY = 0,
	MBC1,
	MBC2,
	MBC3,
	MBC5,
	MMM01,
	POCKET_CAMERA,
	BANDAI_TAMA5,
	HUC1,
	HUC5
};

struct rom_header {
	enum rom_mbc mbc;
	bool ram;
	bool battery;
	bool sram;
	bool rumble;
	bool timer;
	enum rom_cgb_mode cgb_mode;
	int num_rom_banks;
	int rom_bank_size;
	int num_ram_banks;
	int ram_bank_size;
};

int  rom_checksum_validate(void);
bool rom_is_licensee(void);
void rom_get_title(char * title_buffer);
void rom_print_title(void);
void rom_parse_header(u8 *rom0);
const struct rom_header *rom_get_header(void);
bool rom_is_cgb(void);

#endif /* ROM_H_ */
