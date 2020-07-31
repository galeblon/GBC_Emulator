#include<stddef.h>
#include<stdio.h>
#include<stdlib.h>
#include"debug.h"
#include"logger.h"
#include"mem.h"
#include"rom.h"

#define BASE_ADDR_CART_MEM       0x0000
#define BASE_ADDR_VRAM           0x8000
#define BASE_ADDR_RAM_SWITCH     0xA000
#define BASE_ADDR_INTER_RAM_8K   0xC000
#define BASE_ADDR_INTER_RAM_ECHO 0xE000
#define BASE_ADDR_SPRITE_ATTR    0xFE00
#define BASE_ADDR_EMPTY0         0xFEA0
#define BASE_ADDR_IO_PORTS       0xFF00
#define BASE_ADDR_EMPTY1         0xFF4C
#define BASE_ADDR_INTER_RAM      0xFF80
#define BASE_ADDR_INT_ENABLE     0xFFFF

#define SIZE_CART_MEM       0x8000
#define SIZE_VRAM           0x2000
#define SIZE_RAM_SWITCH     0x2000
#define SIZE_INTER_RAM_8K   0x2000
#define SIZE_INTER_RAM_ECHO 0x1E00
#define SIZE_SPRITE_ATTR    0x00A0
#define SIZE_EMPTY0         0x0060
#define SIZE_IO_PORTS       0x004C
#define SIZE_EMPTY1         0x0034
#define SIZE_INTER_RAM      0x007F
#define SIZE_INT_ENABLE     0x0001

#define NUM_MEM_BLOCKS  11

#define MAX_ROM_BANKS  0x200
#define MAX_RAM_BANKS  0x10

#define KB	1024

typedef u8 (*mem_block_read_t)(a16 addr);
typedef void (*mem_block_write_t)(a16 addr, u8 data);

struct mem_block {
	a16 base_addr;
	u16 size;
	mem_block_read_t read;
	mem_block_write_t write;
};

struct mem_bank {
	uint16_t size;
	u8 *mem;
};

enum mem_mbc {
	ROM_ONLY,
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

struct mem_cart_type {
	enum mem_mbc mbc;
	bool ram;
	bool battery;
	bool sram;
	bool rumble;
	bool timer;
	int num_rom_banks;
	int rom_bank_size;
	int num_ram_banks;
	int ram_bank_size;
};

enum mem_banking_mode {
	ROM_BANKING_MODE = 0,
	RAM_BANKING_MODE = 1
};

enum mem_rtc_state {
	RTC_NONE = 0,
	RTC_00,
	RTC_LATCHED,
	RTC_LATCHED_00
};

// Memory module state
static struct mem_cart_type g_mem_cart_type = {0};
static bool g_ram_enable = 0;
static u8 g_int_enable = true;
static u8 g_rom_bank = 1;
static u8 g_ram_bank = 0;
static enum mem_banking_mode g_banking_mode = ROM_BANKING_MODE;
static enum mem_rtc_state g_rtc_state = RTC_NONE;

static struct mem_bank g_rom[MAX_ROM_BANKS] = {0};
static struct mem_bank g_ram[MAX_RAM_BANKS] = {0};
static u8 g_rtc_latch[5];

static u8 g_inter_ram_8k[SIZE_INTER_RAM_8K] = {0};
static u8 g_inter_ram[SIZE_INTER_RAM] = {0};
static u8 g_vram[SIZE_VRAM] = {0};

static void _mem_cart_type_not_implemented(void)
{
	char *message = logger_get_msg_buffer();
	snprintf(message,
		LOG_MESSAGE_MAX_SIZE,
		"CARTRIDGE TYPE 0x%02X NOT IMPLEMENTED\n",
		g_mem_cart_type.mbc);
	logger_log(LOG_WARN,
		"MEM: CART TYPE NOT IMPLEMENTED",
		message);
}

static void _mem_not_implemented(const char *feature)
{
	char *message = logger_get_msg_buffer();
	snprintf(message,
		LOG_MESSAGE_MAX_SIZE,
		"%s NOT IMPLEMENTED\n",
		feature);
	logger_log(LOG_WARN,
		"MEM: NOT IMPLEMENTED",
		message);
}

static u8 _mem_read_rtc(u8 reg)
{
	if (g_rtc_state == RTC_LATCHED || g_rtc_state == RTC_LATCHED_00) {
		return g_rtc_latch[reg];
	}

	// TODO: can RTC registers be read without latching first?
	_mem_not_implemented("RTC");  // TODO: Implement RTC (#41)
	return 0;
}

static void _mem_latch_rtc(u8 data)
{
	switch (g_rtc_state) {
		case RTC_NONE:
			if (data == 0x00)
				g_rtc_state = RTC_00;
			break;
		case RTC_00:
			if (data == 0x01) {
				// TODO latch
			} else {
				g_rtc_state = RTC_NONE;
			}
			break;
		case RTC_LATCHED:
			if (data == 0x00) {
				g_rtc_state = RTC_LATCHED_00;
			}
			break;
		case RTC_LATCHED_00:
			if (data == 0x01) {
				// TODO latch
			} else {
				g_rtc_state = RTC_LATCHED;
			}
	}

	_mem_not_implemented("RTC");  // TODO: implement RTC (#41)
}


static u8 _mem_read_bank(struct mem_bank bank, a16 addr)
{
	debug_assert(addr < bank.size, "_mem_read_bank: address out of bounds");
	return bank.mem[addr];
}

static void _mem_write_bank(struct mem_bank bank, a16 addr, u8 data)
{
	debug_assert(addr < bank.size, "_mem_write_bank: address out of bounds");
	bank.mem[addr] = data;
}

static struct mem_cart_type _mem_parse_cart_type(struct mem_bank rom0)
{
	struct mem_cart_type cart_type = {0};

	u8 type_byte = _mem_read_bank(rom0, 0x0147),
		rom_size_byte = _mem_read_bank(rom0, 0x0148),
		ram_size_byte = _mem_read_bank(rom0, 0x0149);

	switch(type_byte) {
		case 0x00: // ROM_ONLY
			cart_type.mbc = ROM_ONLY;
			break;
		case 0x01: // ROM+MBC1
			cart_type.mbc = MBC1;
			break;
		case 0x02: // ROM+MBC1+RAM
			cart_type.mbc = MBC1;
			cart_type.ram = true;
			break;
		case 0x03: // ROM+MBC1+RAM+BATT
			cart_type.mbc = MBC1;
			cart_type.ram = true;
			cart_type.battery = true;
			break;
		case 0x05: // ROM+MBC2
			cart_type.mbc = MBC2;
			break;
		case 0x06: // ROM+MBC2+BATT
			cart_type.mbc = MBC2;
			cart_type.battery = true;
			break;
		case 0x08: // ROM+RAM
			cart_type.mbc = ROM_ONLY;
			cart_type.ram = true;
			break;
		case 0x09: // ROM+RAM+BATT
			cart_type.mbc = ROM_ONLY;
			cart_type.ram = true;
			cart_type.battery = true;
			break;
		case 0x0B: // ROM+MMM01
			cart_type.mbc = MMM01;
			break;
		case 0x0C: // ROM+MMM01+SRAM
			cart_type.mbc = MMM01;
			cart_type.sram = true;
			break;
		case 0x0D: // ROM+MMM01+SRAM+BATT
			cart_type.mbc = MMM01;
			cart_type.sram = true;
			cart_type.battery = true;
			break;
		case 0x0F: // ROM+MBC3+TIMER+BATT
			cart_type.mbc = MBC3;
			cart_type.timer = true;
			cart_type.battery = true;
			break;
		case 0x10: // ROM+MBC3+TIMER+RAM+BATT
			cart_type.mbc = MBC3;
			cart_type.ram = true;
			cart_type.timer = true;
			cart_type.battery = true;
			break;
		case 0x11: // ROM+MBC3
			cart_type.mbc = MBC3;
			break;
		case 0x12: // ROM+MBC3+RAM
			cart_type.mbc = MBC3;
			cart_type.ram = true;
			break;
		case 0x13: // ROM+MBC3+RAM+BATT
			cart_type.mbc = MBC3;
			cart_type.ram = true;
			cart_type.battery = true;
			break;
		case 0x19: // ROM+MBC5
			cart_type.mbc = MBC5;
			break;
		case 0x1A: // ROM+MBC5+RAM
			cart_type.mbc = MBC5;
			cart_type.ram = true;
			break;
		case 0x1B: // ROM+MBC5+RAM+BATT
			cart_type.mbc = MBC5;
			cart_type.ram = true;
			cart_type.battery = true;
			break;
		case 0x1C: // ROM+MBC5+RUMBLE
			cart_type.mbc = MBC5;
			cart_type.rumble = true;
			break;
		case 0x1D: // ROM+MBC5+RUMBLE+SRAM
			cart_type.mbc = MBC5;
			cart_type.rumble = true;
			cart_type.sram = true;
			break;
		case 0x1E: // ROM+MBC5+RUMBLE+SRAM+BATT
			cart_type.mbc = MBC5;
			cart_type.rumble = true;
			cart_type.sram = true;
			cart_type.battery = true;
			break;
		case 0x1F:
		case 0xFD:
		case 0xFE:
		case 0xFF:
			_mem_cart_type_not_implemented();
			break;
		default:
			debug_assert(true, "_mem_parse_cart_type: unknown cartridge type");
			break;
	}

	if (rom_size_byte == 0x00) {
		cart_type.rom_bank_size = 32 * KB;
		cart_type.num_rom_banks = 1;
	} else if (rom_size_byte <= 0x08) {
		cart_type.rom_bank_size = 16 * KB;
		cart_type.num_rom_banks = (2 << rom_size_byte);
	} else if (0x52 <= rom_size_byte && rom_size_byte < 0x55) {
		// TODO: find some reliable info on 0x52-0x54
		_mem_cart_type_not_implemented();
	} else {
		debug_assert(true, "_mem_parse_cart_type: unknown cartridge type");
	}

	if (cart_type.mbc == MBC2) {
		cart_type.ram_bank_size = 512;
		cart_type.num_ram_banks = 1;
	} else {
		switch (ram_size_byte) {
			case 0x00:
				cart_type.ram_bank_size = 0;
				cart_type.num_ram_banks = 0;
				break;
			case 0x01:
				cart_type.ram_bank_size = 2 * KB;
				cart_type.num_ram_banks = 1;
				break;
			case 0x02:
				cart_type.ram_bank_size = 8 * KB;
				cart_type.num_ram_banks = 1;
				break;
			case 0x03:
				cart_type.ram_bank_size = 8 * KB;
				cart_type.num_ram_banks = 4;
				break;
			case 0x04:
				cart_type.ram_bank_size = 8 * KB;
				cart_type.num_ram_banks = 16;
				break;
			case 0x05:
				cart_type.ram_bank_size = 8 * KB;
				cart_type.num_ram_banks = 8;
				break;
			default:
				debug_assert(true, "_mem_parse_cart_type: unknown cartridge type");
				break;
		}
	}

	return cart_type;
}

static u8 _mem_read_cart_mem(a16 addr)
{
	switch (g_mem_cart_type.mbc) {
		case ROM_ONLY:
			return _mem_read_bank(g_rom[0], addr - BASE_ADDR_CART_MEM);
			break;
		case MBC1:
		case MBC2:
		case MBC3:
			if (addr < 0x4000) {
				return _mem_read_bank(g_rom[0], addr - BASE_ADDR_CART_MEM);
			} else if (addr < 0x8000) {
				return _mem_read_bank(g_rom[g_rom_bank], addr - BASE_ADDR_CART_MEM - g_mem_cart_type.rom_bank_size * g_rom_bank);
			}
			break;
		case MBC5:
		case MMM01:
		case POCKET_CAMERA:
		case BANDAI_TAMA5:
		case HUC1:
		case HUC5:
			// TODO: implement other MBCs
			break;
	}

	debug_assert(true, "_mem_read_cart_mem: invalid cartridge type");
	return -1;
}


static void _mem_write_cart_mem(a16 addr, u8 data)
{
	switch(g_mem_cart_type.mbc) {
		case ROM_ONLY:
			// read only
			break;
		case MBC1:
			if (addr < 0x2000) {
				// RAM Enable:
				// RAM is enabled when the lower 4 bytes of data written
				// to 0x0000-0x1FFFF are equal to 0x0A, otherwise RAM is disabled

				g_ram_enable = (data & 0xF) == 0x0A;
			} else if (addr < 0x4000) {
				// ROM Bank number lower:
				// data written to 0x2000-0x3FFF selects the lower 5 bits of the
				// ROM Bank number
				// If the lower bits are set to all zeros, a data bank one higher
				// is used (eg. if Bank 0 is selected Bank 1 will be used,
				// Bank 20 -> Bank 21, etc.

				g_rom_bank = (g_rom_bank & 0xE0) | (data & 0x1F);
				if ((data & 0x1F) == 0)
					g_rom_bank += 1;
			} else if (addr < 0x6000) {
				// depending on currently set ROM/RAM banking mode

				if (g_banking_mode == ROM_BANKING_MODE) {
					// ROM Bank number upper:
					// bits 0-1 of data are bits 5-6 of ROM Bank number

					g_rom_bank = (g_rom_bank & 0x1F) | ((data & 0x03) << 5);
				} else {
					// RAM Bank number:
					// bits 0-1 of data select RAM Bank number used

					g_ram_bank = data & 0x03;
				}
			} else if (addr < 0x8000) {
				// ROM/RAM banking mode select
				// bit 0 of data selects banking mode:
				//		* 0x00: ROM Banking Mode (default)
				//		* 0x01: RAM Banking Mode
				g_banking_mode = (enum mem_banking_mode)(data & 0x01);

				// reset the bank values
				if (g_banking_mode == ROM_BANKING_MODE) {
					g_ram_bank = 0x01;
				} else {
					g_rom_bank = g_rom_bank & 0x1F;
				}
			}
			break;
		case MBC2:
			if (addr < 0x2000) {
				// RAM Enable:
				// To enable RAM, bit 8 of address must be cleared
				// TODO: is 0x0A used same as in MBC1? Pan Docs does not specify

				if ((addr & 0x100) == 0) {
					g_ram_enable = (data & 0xF) == 0x0A;
				}
			} else if (addr < 0x4000) {
				// ROM Bank number:
				// To set ROM Bank number, bit 8 of address must be set
				// Bits 0-3 of data select the ROM Bank exposed
				// on addresses 0x4000-0x7FFFF
				// ROM Bank 0 cannot be selected
				// TODO: assuming that the logic is the same as MBC1 and
				//       ROM Bank is set to 1 in case 0 is written here

				if ((addr & 0x100) != 0) {
					g_rom_bank = data & 0x0F;

					if ((data & 0x0F) == 0)
						g_rom_bank += 1;
				}
			}
			break;
		case MBC3:
			if (addr < 0x2000) {
				// RAM Enable:
				// RAM and RTC Register is enabled when the lower 4 bytes of
				// data written to 0x0000-0x1FFFF are equal to 0x0A, otherwise
				// RAM and RTC Register are disabled

				g_ram_enable = (data & 0xF) == 0x0A;
			} else if (addr < 0x4000) {
				// ROM Bank number:
				// data written to 0x2000-0x3FFF selects the 7 bit ROM Bank
				// number
				// If bits 6-0 are all cleared, ROM Bank 1 will be selected

				if ((data & 0x7F) != 0) {
					g_rom_bank = data & 0x7F;
				} else {
					g_rom_bank = 1;
				}
			} else if (addr < 0x6000) {
				// RAM Bank number / RTC register select
				// If data is 0x00-0x03, then the respective RAM Bank is
				// mapped to 0xA000-0xBFFF.
				// If data is 0x08-0x0C, then the respective RTC register
				// is mapped to 0xA000-0xBFFF.

				if (data < 0x04 || (0x08 <= data && data < 0x0D))
					g_ram_bank = data;

			} else if (addr < 0x8000) {
				_mem_latch_rtc(data);
			}
			break;
		case MBC5:
		case MMM01:
		case POCKET_CAMERA:
		case BANDAI_TAMA5:
		case HUC1:
		case HUC5:
			// TODO: implement other MBCs
			break;
		default:
			debug_assert(true, "_mem_write_cart_mem: invalid cartridge type");
	}
}

static inline u8 _mem_read_vram(a16 addr)
{
	return g_vram[addr - BASE_ADDR_VRAM];
}

static inline void _mem_write_vram(a16 addr, u8 data)
{
	g_vram[addr - BASE_ADDR_VRAM] = data;
}

static u8 _mem_read_ram_switch(a16 addr)
{
	switch(g_mem_cart_type.mbc) {
		case ROM_ONLY:
			return _mem_read_bank(g_ram[0], addr - BASE_ADDR_RAM_SWITCH);
			break;
		case MBC1:
			return _mem_read_bank(g_ram[g_ram_bank], addr - BASE_ADDR_RAM_SWITCH - g_mem_cart_type.ram_bank_size * g_ram_bank);
			break;
		case MBC2:
		{
			u8 data = _mem_read_bank(g_ram[g_ram_bank], addr - BASE_ADDR_RAM_SWITCH - g_mem_cart_type.ram_bank_size * g_ram_bank);
#ifdef DEBUG
			// in debug build run additional check if nothing was written
			// to 4 upper bits of RAM cell on read
			if (data & 0xF0) {
				debug_assert(true, "_mem_read_ram_switch: data in higher bits on MBC2 RAM");
			}
#endif
			return data;
			break;
		}
		case MBC3:
			if (g_ram_bank < 0x04) {
				return _mem_read_bank(g_ram[g_ram_bank], addr - BASE_ADDR_RAM_SWITCH - g_mem_cart_type.ram_bank_size * g_ram_bank);
			} else if (0x08 <= g_ram_bank  && g_ram_bank < 0x0D) {
				return _mem_read_rtc(g_ram_bank);
			}
			break;
		case MBC5:
		case MMM01:
		case POCKET_CAMERA:
		case BANDAI_TAMA5:
		case HUC1:
		case HUC5:
			break;
	}

	debug_assert(true, "_mem_read_ram_switch: invalid cartridge type");
	return 0;
}

static void _mem_write_ram_switch(a16 addr, u8 data)
{
	switch(g_mem_cart_type.mbc) {
		case ROM_ONLY:
			_mem_write_bank(g_ram[0], addr - BASE_ADDR_RAM_SWITCH, data);
			break;
		case MBC1:
			_mem_write_bank(g_ram[g_ram_bank], addr - BASE_ADDR_RAM_SWITCH - g_mem_cart_type.ram_bank_size * g_ram_bank, data);
			break;
		case MBC2:
			// in MBC2 each addressable memory cell is 4 b only
			_mem_write_bank(g_ram[g_ram_bank], addr - BASE_ADDR_RAM_SWITCH - g_mem_cart_type.ram_bank_size * g_ram_bank, data & 0x0F);
			break;
		case MBC3:
			if (g_ram_bank < 0x04) {
				_mem_write_bank(g_ram[g_ram_bank], addr - BASE_ADDR_RAM_SWITCH - g_mem_cart_type.ram_bank_size * g_ram_bank, data);
			} else {
				// TODO: RTC write (#41)
				_mem_not_implemented("RTC");
			}
			break;
		case MBC5:
		case MMM01:
		case POCKET_CAMERA:
		case BANDAI_TAMA5:
		case HUC1:
		case HUC5:
			break;
		default:
			debug_assert(true, "_mem_write_ram_switch: invalid cartridge type");
	}
}

static inline u8 _mem_read_inter_ram_8k(a16 addr)
{
	// TODO: add switchable internal RAM banks
#if DEBUG
	if (addr >= 0xD000)
		_mem_not_implemented("Switchable internal RAM Banks");  // TODO: implement switchable internal RAM (#42)
#endif
	return g_inter_ram_8k[addr - BASE_ADDR_INTER_RAM_8K];
}

static inline void _mem_write_inter_ram_8k(a16 addr, u8 data)
{
	// TODO: add switchable internal RAM banks
#if DEBUG
	if (addr >= 0xD000)
		_mem_not_implemented("Switchable internal RAM Banks");  // TODO: implement switchable internal RAM (#42)
#endif
	g_inter_ram_8k[addr - BASE_ADDR_INTER_RAM_8K] = data;
}

static inline u8 _mem_read_inter_ram_echo(a16 addr)
{
	return _mem_read_inter_ram_8k(addr);
}

static inline void _mem_write_inter_ram_echo(a16 addr, u8 data)
{
	_mem_write_inter_ram_8k(addr, data);
}

static inline u8 _mem_read_sprite_attr(a16 addr __attribute__((unused)))
{
	// TODO: Sprite attribute table support (#43)
	_mem_not_implemented("Sprite attribute table");
	return 0;
}

static inline void _mem_write_sprite_attr(a16 addr __attribute__((unused)),
	u8 data __attribute__((unused)))
{
	// TODO: Sprite attribute table support (#43)
	_mem_not_implemented("Sprite attribute table");
}

static inline u8 _mem_read_empty0(a16 addr __attribute__((unused)))
{
	return 0;
}

static inline void _mem_write_empty0(a16 addr __attribute__((unused)),
	u8 data __attribute__((unused)))
{
}


static inline u8 _mem_read_io_ports(a16 addr __attribute__((unused)))
{
	// TODO: IO ports (#44)
	_mem_not_implemented("IO ports");
	return 0;
}

static inline void _mem_write_io_ports(a16 addr __attribute__((unused)),
	u8 data __attribute__((unused)))
{
	// TODO: IO ports (#44)
	_mem_not_implemented("IO ports");
}

static inline u8 _mem_read_empty1(a16 addr __attribute__((unused)))
{
	return 0;
}

static inline void _mem_write_empty1(a16 addr __attribute__((unused)),
	u8 data __attribute__((unused)))
{
}

static inline u8 _mem_read_inter_ram(a16 addr)
{
	return g_inter_ram[addr - BASE_ADDR_INTER_RAM];
}

static inline void _mem_write_inter_ram(a16 addr, u8 data)
{
	g_inter_ram[addr - BASE_ADDR_INTER_RAM] = data;
}

static inline u8 _mem_read_int_enable(a16 addr __attribute__((unused)))
{
	return g_int_enable;
}

static inline void _mem_write_int_enable(a16 addr __attribute__((unused)),
	u8 data)
{
	g_int_enable = data;
}


#define MEM_BLOCK_DEF(LOWER, UPPER)	\
{	\
	.base_addr = BASE_ADDR_ ##UPPER ,	\
	.size = SIZE_ ##UPPER ,			\
	.read = _mem_read_ ##LOWER ,		\
	.write = _mem_write_ ##LOWER ,		\
},	\

struct mem_block g_mem_blocks[NUM_MEM_BLOCKS] = {
	MEM_BLOCK_DEF(cart_mem, CART_MEM)
	MEM_BLOCK_DEF(vram, VRAM)
	MEM_BLOCK_DEF(ram_switch, RAM_SWITCH)
	MEM_BLOCK_DEF(inter_ram_8k, INTER_RAM_8K)
	MEM_BLOCK_DEF(inter_ram_echo, INTER_RAM_ECHO)
	MEM_BLOCK_DEF(sprite_attr, SPRITE_ATTR)
	MEM_BLOCK_DEF(empty0, EMPTY0)
	MEM_BLOCK_DEF(io_ports, IO_PORTS)
	MEM_BLOCK_DEF(empty1, EMPTY1)
	MEM_BLOCK_DEF(inter_ram, INTER_RAM)
	MEM_BLOCK_DEF(int_enable, INT_ENABLE)
};

#undef MEM_BLOCK_DEF

static inline u8 _mem_u16_lower(u16 data)
{
	return (u8)(data & 0x00FF);
}

static inline u8 _mem_u16_higher(u16 data)
{
	return (u8)((data & 0xFF00) >> 8);
}

static struct mem_block *_mem_get_block(a16 addr)
{
	for (int i = 0; i < NUM_MEM_BLOCKS; i++) {
		if (addr >= g_mem_blocks[i].base_addr
			&& addr < g_mem_blocks[i].base_addr + g_mem_blocks[i].size)
			return &g_mem_blocks[i];
	}
	return NULL;
}

static u8 _mem_read_error(a16 addr)
{
	// TODO #15: determine proper way to handle error
	char *msg_buff = logger_get_msg_buffer();
	snprintf(msg_buff, LOG_MESSAGE_MAX_SIZE,
		"MEMORY READ ERROR AT ADDRESS 0x%04X\n", addr);
	return 0;
}

static void _mem_write8_error(a16 addr, u8 data)
{
	// TODO #15: determine proper way to handle error
	char *msg_buff = logger_get_msg_buffer();
	snprintf(msg_buff, LOG_MESSAGE_MAX_SIZE,
		"MEMORY U8 WRITE ERROR AT ADDRESS 0x%04X\n, DATA: 0x%02X",
		addr, data);
}

static void _mem_write16_error(a16 addr, u16 data)
{
	// TODO #15: determine proper way to handle error
	char *msg_buff = logger_get_msg_buffer();
	snprintf(msg_buff, LOG_MESSAGE_MAX_SIZE,
		"MEMORY U16 WRITE ERROR AT ADDRESS 0x%04X\n, DATA: 0x%04X",
		addr, data);
}

u8 mem_read8(a16 addr)
{
	struct mem_block *block;
	block = _mem_get_block(addr);

	if (block == NULL)
		return _mem_read_error(addr);

	return block->read(addr);
}

void mem_write8(a16 addr, u8 data)
{
	struct mem_block *block;
	block = _mem_get_block(addr);

	if (block == NULL) {
		_mem_write8_error(addr, data);
		return;
	}

	block->write(addr, data);
}

u16 mem_read16(a16 addr)
{
	struct mem_block *block;
	u16 ret = 0;
	block = _mem_get_block(addr);

	if (block == NULL)
		return _mem_read_error(addr);

	ret = (u16)block->read(addr);

	if (addr + 1 == block->base_addr + block->size) {
		block = _mem_get_block(addr + 1);

		if (block == NULL)
			return _mem_read_error(addr + 1);
	}

	ret |= ((u16)block->read(addr + 1)) << 8;
	return ret;
}

void mem_write16(a16 addr, u16 data)
{
	struct mem_block *block_1, *block_2;

	block_1 = _mem_get_block(addr);

	if (block_1 == NULL) {
		_mem_write16_error(addr, data);
		return;
	}

	if (addr + 1 == block_1->base_addr + block_1->size) {
		block_2 = _mem_get_block(addr + 1);
		if (block_2 == NULL) {
			_mem_write16_error(addr, data);
			return;
		}
	} else {
		block_2 = block_1;
	}

	block_1->write(addr, _mem_u16_lower(data));
	block_2->write(addr + 1, _mem_u16_higher(data));
}

int _mem_load_rom(char *path)
{
	long rom_len;
	u8 rom0_mem[0x4000];
	struct mem_bank rom0 = { .size = 0x4000, .mem = rom0_mem };

	FILE *rom_fileptr = fopen(path, "rb");

	if(rom_fileptr == NULL) {
		fprintf(stderr, "Couldn't open rom file");
		return 0;
	}

	fseek(rom_fileptr, 0, SEEK_END);
	rom_len = ftell(rom_fileptr);

	if (rom_len > MAX_ROM_SIZE) {
		fclose(rom_fileptr);
		return 0;
	}

	rewind(rom_fileptr);

	fread(rom0.mem, 0x4000, 1, rom_fileptr);

	g_mem_cart_type = _mem_parse_cart_type(rom0);

	if (g_mem_cart_type.rom_bank_size == 0
			|| rom_len / g_mem_cart_type.rom_bank_size != g_mem_cart_type.num_rom_banks)
		return 0;

	rewind(rom_fileptr);

	for (int i = 0; i < g_mem_cart_type.num_rom_banks; i++) {
		g_rom[i].mem = (u8 *)calloc(1, g_mem_cart_type.rom_bank_size);
		g_rom[i].size = g_mem_cart_type.rom_bank_size;
		fread(g_rom[i].mem, g_rom[i].size, 1, rom_fileptr);
	}

	fclose(rom_fileptr);
	return 1;
}

int mem_prepare(char *rom_path)
{
	int rc;

	rc = _mem_load_rom(rom_path);

	if (rc != 1)
		return rc;

	for (int i = 0; i < g_mem_cart_type.num_ram_banks; i++) {
		g_ram[i].mem = (u8 *)calloc(1, g_mem_cart_type.ram_bank_size);
		g_ram[i].size = g_mem_cart_type.ram_bank_size;
	}

	return 1;
}

void mem_destroy(void)
{
	for (int i = 0; i < g_mem_cart_type.num_rom_banks; i++) {
		free(g_rom[i].mem);
	}

	for (int i = 0; i < g_mem_cart_type.num_ram_banks; i++) {
		free(g_ram[i].mem);
	}
}
