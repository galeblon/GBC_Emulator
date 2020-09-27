#include<stddef.h>
#include<stdio.h>
#include<stdlib.h>
#include"cpu.h"
#include"debug.h"
#include"logger.h"
#include"mem_priv.h"
#include"mem_rtc.h"
#include"rom.h"

#define BASE_ADDR_CART_MEM       0x0000
#define BASE_ADDR_VRAM           0x8000
#define BASE_ADDR_RAM_SWITCH     0xA000
#define BASE_ADDR_WRAM0          0xC000
#define BASE_ADDR_WRAM           0xD000
#define BASE_ADDR_WRAM_ECHO      0xE000
#define BASE_ADDR_SPRITE_ATTR    0xFE00
#define BASE_ADDR_EMPTY0         0xFEA0
#define BASE_ADDR_IO_PORTS       0xFF00
#define BASE_ADDR_HRAM           0xFF80
#define BASE_ADDR_INT_ENABLE     0xFFFF

#define SIZE_CART_MEM       0x8000
#define SIZE_VRAM           0x2000
#define SIZE_RAM_SWITCH     0x2000
#define SIZE_WRAM0          0x1000
#define SIZE_WRAM           0x1000
#define SIZE_WRAM_ECHO      0x1E00
#define SIZE_SPRITE_ATTR    0x00A0
#define SIZE_EMPTY0         0x0060
#define SIZE_IO_PORTS       0x0080
#define SIZE_HRAM           0x007F
#define SIZE_INT_ENABLE     0x0001

#define NUM_MEM_BLOCKS  11

#define MAX_ROM_BANKS  0x200
#define MAX_RAM_BANKS  0x10
#define NUM_WRAM_BANKS 0x08
#define NUM_VRAM_BANKS 0x02

#define DMA_CYCLES_PER_10H	8

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

enum mem_banking_mode {
	ROM_BANKING_MODE = 0,
	RAM_BANKING_MODE = 1
};

enum mem_dma_state {
	DMA_NONE                = 1 << 0,
	DMA_GENERAL_IN_PROGRESS = 1 << 1,
	DMA_H_BLANK_IN_PROGRESS = 1 << 2,
	DMA_VRAM_SUCCESS        = 1 << 3,
	DMA_H_BLANK_TERMINATED  = 1 << 4,
	DMA_AVAILIBLE           = DMA_NONE | DMA_VRAM_SUCCESS | DMA_H_BLANK_TERMINATED,
};

// Memory module state
static bool g_ram_enable = 0;
static u16 g_rom_bank = 1;
static u8 g_ram_bank = 0;
static u8 g_wram_bank = 1;
static u8 g_vram_bank = 0;
static int g_dma_lock = 0;
static u16 g_dma_length = 0, g_dma_remaining = 0;
static a16 g_dma_src = 0, g_dma_dst = 0;
static enum mem_banking_mode g_banking_mode = ROM_BANKING_MODE;
static enum mem_dma_state g_dma_state = DMA_NONE;

static u8 g_hram[SIZE_HRAM] = {0};
static u8 g_io_ports[SIZE_IO_PORTS] = {0};

static mem_read_handler_t g_io_ports_read[SIZE_IO_PORTS] = {0};
static mem_write_handler_t g_io_ports_write[SIZE_IO_PORTS] = {0};
static mem_read_handler_t g_int_enable_read = NULL;
static mem_write_handler_t g_int_enable_write = NULL;

static u8 g_sprite_attr[SIZE_SPRITE_ATTR] = {0};

static struct mem_bank g_rom[MAX_ROM_BANKS] = {0};
static struct mem_bank g_ram[MAX_RAM_BANKS] = {0};
static struct mem_bank g_wram[NUM_WRAM_BANKS] = {0};
static struct mem_bank g_vram[NUM_VRAM_BANKS] = {0};

static void _mem_not_implemented(const char *feature, a16 addr)
{
	logger_log(LOG_WARN,
		"MEM: NOT IMPLEMENTED",
		"%s NOT IMPLEMENTED (0x%04X)\n",
		feature, addr);
}

static inline void _mem_fatal(char *msg) {
	logger_log(LOG_FATAL, "MEM: ERROR", msg);
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

static inline void _mem_dma_start(a16 total_length)
{
	debug_assert(g_dma_state & DMA_AVAILIBLE, "_mem_dma_start: DMA in progress");

	g_dma_length = g_dma_remaining = total_length;
}

static void _mem_dma(a16 length)
{
	u8 data;
	a16 offset = g_dma_length - g_dma_remaining;

	debug_assert(length <= g_dma_remaining, "_mem_dma: invalid DMA length");

	for (u16 i = 0; i < length; i++) {
		data = mem_read8(g_dma_src + offset + i);
		mem_write8(g_dma_dst + offset + i, data);
	}

	g_dma_remaining -= length;

	if (g_dma_remaining == 0) {
		g_dma_state = DMA_VRAM_SUCCESS;
	}
}

static u8 _mem_read_cart_mem(a16 addr)
{
	if (g_dma_lock)
		return 0;

	const struct rom_header *header = rom_get_header();

	switch (header->mbc) {
		case ROM_ONLY:
			return _mem_read_bank(g_rom[0], addr - BASE_ADDR_CART_MEM);
			break;
		case MBC1:
		case MBC2:
		case MBC3:
		case MBC5:
			if (addr < 0x4000 || header->num_rom_banks == 1) {
				return _mem_read_bank(g_rom[0], addr - BASE_ADDR_CART_MEM);
			} else if (addr < 0x8000) {
				return _mem_read_bank(g_rom[g_rom_bank], addr - BASE_ADDR_CART_MEM - header->rom_bank_size);
			}
			break;
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
	if (g_dma_lock)
		return;

	switch(rom_get_header()->mbc) {
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

				g_rom_bank = (g_rom_bank & 0x00E0) | (data & 0x1F);
				if ((data & 0x1F) == 0)
					g_rom_bank += 1;
			} else if (addr < 0x6000) {
				// depending on currently set ROM/RAM banking mode

				if (g_banking_mode == ROM_BANKING_MODE) {
					// ROM Bank number upper:
					// bits 0-1 of data are bits 5-6 of ROM Bank number

					g_rom_bank = (g_rom_bank & 0x001F) | ((data & 0x03) << 5);
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
					g_rom_bank = g_rom_bank & 0x001F;
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
					g_rom_bank = data & 0x000F;

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
				mem_rtc_latch(data);
			}
			break;
		case MBC5:
			if (addr < 0x2000) {
				// RAM Enable:
				// RAM is enabled when the data written to 0x0000-0x1FFFF is equal
				// to 0x0A, otherwise RAM is disabled

				g_ram_enable = data == 0x0A;
			} else if (addr < 0x3000) {
				// ROM Bank number lower:
				// data written to 0x2000-0x3FFF selects the lower 8 bits of the
				// ROM Bank number
				g_rom_bank = (g_rom_bank & 0xFF00) | data;
			} else if (addr < 0x4000) {
				// ROM Bank number higher:
				// highest bit of the ROM Bank selection

				g_rom_bank = (g_rom_bank & 0x00FF) | ((data & 1) << 8);
			} else if (addr < 0x6000) {
				// RAM Bank number:
				// bits 0-3 of data select RAM Bank number used

				g_ram_bank = data & 0x0F;
			}
		break;
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
	if (g_dma_lock)
		return 0;

	return _mem_read_bank(g_vram[g_vram_bank], addr - BASE_ADDR_VRAM);
}

static inline void _mem_write_vram(a16 addr, u8 data)
{
	if (g_dma_lock)
		return;

	_mem_write_bank(g_vram[g_vram_bank], addr - BASE_ADDR_VRAM, data);
}

static u8 _mem_read_ram_switch(a16 addr)
{
	if (g_dma_lock)
		return 0;

	const struct rom_header *header = rom_get_header();

	switch(header->mbc) {
		case ROM_ONLY:
			return _mem_read_bank(g_ram[0], addr - BASE_ADDR_RAM_SWITCH);
			break;
		case MBC1:
		case MBC5:
			return _mem_read_bank(g_ram[g_ram_bank], addr - BASE_ADDR_RAM_SWITCH);
			break;
		case MBC2:
		{
			u8 data = _mem_read_bank(g_ram[g_ram_bank], addr - BASE_ADDR_RAM_SWITCH);
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
				return _mem_read_bank(g_ram[g_ram_bank], addr - BASE_ADDR_RAM_SWITCH);
			} else if (0x08 <= g_ram_bank  && g_ram_bank < 0x0D) {
				return mem_rtc_read(g_ram_bank);
			}
			break;
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
	if (g_dma_lock)
		return;

	const struct rom_header *header = rom_get_header();

	switch(header->mbc) {
		case ROM_ONLY:
			_mem_write_bank(g_ram[0], addr - BASE_ADDR_RAM_SWITCH, data);
			break;
		case MBC1:
		case MBC5:
			_mem_write_bank(g_ram[g_ram_bank],
					addr - BASE_ADDR_RAM_SWITCH, data);
			break;
		case MBC2:
			// in MBC2 each addressable memory cell is 4 b only
			_mem_write_bank(g_ram[g_ram_bank],
					addr - BASE_ADDR_RAM_SWITCH, data & 0x0F);
			break;
		case MBC3:
			if (g_ram_bank < 0x04) {
				_mem_write_bank(g_ram[g_ram_bank],
						addr - BASE_ADDR_RAM_SWITCH, data);
			} else {
				mem_rtc_write(g_ram_bank, data);
			}
			break;
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

static inline u8 _mem_read_wram0(a16 addr)
{
	if (g_dma_lock)
		return 0;

	return _mem_read_bank(g_wram[0], addr - BASE_ADDR_WRAM0);
}

static inline void _mem_write_wram0(a16 addr, u8 data)
{
	if (g_dma_lock)
		return;

	_mem_write_bank(g_wram[0], addr - BASE_ADDR_WRAM0, data);
}

static inline u8 _mem_read_wram(a16 addr)
{
	debug_assert(0 < g_wram_bank && g_wram_bank < NUM_WRAM_BANKS,
			"_mem_read_wram: incorrect WRAM Bank number");

	if (g_dma_lock)
		return 0;

	if (rom_is_cgb())
		return _mem_read_bank(g_wram[g_wram_bank], addr - BASE_ADDR_WRAM);

	return _mem_read_bank(g_wram[1], addr - BASE_ADDR_WRAM);
}

static inline void _mem_write_wram(a16 addr, u8 data)
{
	debug_assert(0 < g_wram_bank && g_wram_bank < NUM_WRAM_BANKS,
			"_mem_write_wram: incorrect WRAM Bank number");

	if (g_dma_lock)
		return;

	if (rom_is_cgb()) {
		_mem_write_bank(g_wram[g_wram_bank],
				addr - BASE_ADDR_WRAM,
				data);
	} else {
		_mem_write_bank(g_wram[1], addr - BASE_ADDR_WRAM, data);
	}
}

static inline u8 _mem_read_wram_echo(a16 addr)
{
	if (addr < BASE_ADDR_WRAM)
		return _mem_read_wram0(addr);

	return _mem_read_wram(addr);
}

static inline void _mem_write_wram_echo(a16 addr, u8 data)
{
	if (addr < BASE_ADDR_WRAM)
		_mem_write_wram0(addr, data);

	_mem_write_wram(addr, data);
}

static inline u8 _mem_read_sprite_attr(a16 addr)
{
	if (g_dma_lock)
		return 0;

	return g_sprite_attr[addr - BASE_ADDR_SPRITE_ATTR];
}

static inline void _mem_write_sprite_attr(a16 addr, u8 data)
{
	if (g_dma_lock)
		return;

	g_sprite_attr[addr - BASE_ADDR_SPRITE_ATTR] = data;
}

static inline u8 _mem_read_empty0(a16 addr __attribute__((unused)))
{
	return 0;
}

static inline void _mem_write_empty0(a16 addr __attribute__((unused)),
	u8 data __attribute__((unused)))
{
}


static inline u8 _mem_io_ports_read_handler(a16 addr)
{
	switch (addr) {
	case 0xFF4F:
		// VBK: VRAM Bank selection
		if (rom_is_cgb())
			return 0xFE | g_vram_bank;
		break;
	case 0xFF55:
		// VRAM DMA transfer remaining
		// Returns:
		//   0xFF: when transfer completed
		//   (length_remaining / 0x10) - 1: while transfer in progress
		//   0x80 + (length_remaining / 0x10) - 1:
		//		after terminated H-blank transfer
		// Bit 7 signifies if transfer is active (0) or not (1).
		//
		// Right now all VRAM DMA is instant, so we always return 0xFF.
		if (rom_is_cgb()) {
			switch (g_dma_state) {
			case DMA_NONE:
			case DMA_VRAM_SUCCESS:
				return 0xFF;
			case DMA_GENERAL_IN_PROGRESS:
			case DMA_H_BLANK_IN_PROGRESS:
				return (g_dma_remaining / 0x10) - 1;
			case DMA_H_BLANK_TERMINATED:
				return 0x80 | ((g_dma_remaining / 0x10) - 1);
			default:
				debug_assert(false, "_mem_read_io_ports: invalid DMA state");
				g_dma_state = DMA_NONE;
			}
		}
		break;
	case 0xFF70:
		// SVBK: WRAM Bank selection
		if (rom_is_cgb())
			return g_wram_bank;
		break;
	case 0xFF46: // OAM DMA
	case 0xFF51: // HDMA1
	case 0xFF52: // HDMA2
	case 0xFF53: // HDMA3
	case 0xFF54: // HDMA4
		break;
	default:
		_mem_not_implemented("IO ports read", addr);
		return g_io_ports[addr - BASE_ADDR_IO_PORTS];
	}

	return 0xFF;
}

static inline void _mem_io_ports_write_handler(a16 addr, u8 data)
{
	switch (addr) {
	case 0xFF46:
		// DMA: OAM DMA transfer address
		if (data <= 0xF1) {
			g_dma_dst = BASE_ADDR_SPRITE_ATTR;
			g_dma_src = (a16)data << 8;
			_mem_dma_start(SIZE_SPRITE_ATTR);
			_mem_dma(SIZE_SPRITE_ATTR);
			g_dma_lock = 160;
		} else {
			debug_assert(true, "_mem_write_empty1: invalid OAM DMA address");
		}
		break;
	case 0xFF4F:
		// VBK: VRAM Bank selection
		if (rom_is_cgb())
			g_vram_bank = data & 0x01;
		break;
	case 0xFF51:
		// HDMA1: VRAM DMA Source address, higher
		if (rom_is_cgb())
			g_dma_src = (g_dma_src & 0x00FF) | ((a16)data << 8);
		break;
	case 0xFF52:
		// HDMA2: VRAM DMA Source address, lower
		// Bits 0-3 are ignored (zeros are used)
		if (rom_is_cgb())
			g_dma_src = (g_dma_src & 0xFF00) | data;
		break;
	case 0xFF53:
		// HDMA3: VRAM DMA Destination address, higher
		// Bits 5-7 are ignored (0x80 is used)
		if (rom_is_cgb())
			g_dma_dst = (g_dma_dst & 0x00FF) | ((a16)data << 8);
		break;
	case 0xFF54:
		// HDMA4: VRAM DMA Destination address, lower
		// Bits 0-3 are ignored (zeros are used)
		if (rom_is_cgb())
			g_dma_dst = (g_dma_dst & 0xFF00) | data;
		break;
	case 0xFF55:
		// HDMA5: VRAM DMA mode/length
		// Bit 7 is DMA Mode:
		//     0 - General DMA - blocks execution and transfers right away
		//     1 - H-blank DMA - tranfers 0x10 bytes on each H-blank
		// Bits 0-6 are (transfer_length / 0x10) - 1
		if (rom_is_cgb()) {
			a16 length;

			switch (g_dma_state) {
			case DMA_NONE:
			case DMA_VRAM_SUCCESS:
			case DMA_H_BLANK_TERMINATED:
				g_dma_dst &= 0x1FF0;
				g_dma_src &= 0xFFF0;
				length = ((data & 0x7F) + 1) << 8;

				// Prevent DMA from overflowing beyond VRAM
				if (g_dma_dst + length > SIZE_VRAM)
					length = SIZE_VRAM - g_dma_dst;

				_mem_dma_start(length);

				if ((data & 0x80) == 0) {
					// Execute General DMA all at once
					g_dma_state = DMA_GENERAL_IN_PROGRESS;
					g_dma_lock = DMA_CYCLES_PER_10H * (g_dma_length / 0x10);
					cpu_set_halted(true);
					_mem_dma(length);
				} else {
					g_dma_state = DMA_H_BLANK_IN_PROGRESS;
				}
				break;
			case DMA_H_BLANK_IN_PROGRESS:
				// Assuming writes with bit 7 = 1 are ignored
				if ((data & 0x80) == 0)
					g_dma_state = DMA_H_BLANK_TERMINATED;
				break;
			default:
				debug_assert(false, "_mem_write_io_ports: invalid DMA state");
				g_dma_state = DMA_NONE;
			}
		}
		break;
	case 0xFF70:
		// SVBK: WRAM Bank selection
		if (rom_is_cgb()) {
			g_wram_bank = data & 0x03;
			if (g_wram_bank == 0)
				g_wram_bank = 1;
		}
		break;
	default:
		// If an address within IO ports does not have any special handling
		// implemented just write the data, so that it can be read through
		// mem_readX later
		g_io_ports[addr - BASE_ADDR_IO_PORTS] = data;

		_mem_not_implemented("IO ports write", addr);
		break;
	}
}

static inline u8 _mem_read_io_ports(a16 addr)
{
	if (g_dma_lock)
		return 0;

	mem_read_handler_t r = g_io_ports_read[addr - BASE_ADDR_IO_PORTS];

	if (r)
		return r(addr);

	return _mem_io_ports_read_handler(addr);
}

static inline void _mem_write_io_ports(a16 addr, u8 data)
{
	if (g_dma_lock)
		return;

	mem_write_handler_t w = g_io_ports_write[addr - BASE_ADDR_IO_PORTS];

	if (w) {
		w(addr, data);
	} else {
		_mem_io_ports_write_handler(addr, data);
	}
}

static inline u8 _mem_read_hram(a16 addr)
{
	return g_hram[addr - BASE_ADDR_HRAM];
}

static inline void _mem_write_hram(a16 addr, u8 data)
{
	g_hram[addr - BASE_ADDR_HRAM] = data;
}

static inline u8 _mem_read_int_enable(a16 addr)
{
	if (g_dma_lock)
		return 0;

	if (g_int_enable_read)
		return g_int_enable_read(addr);

	return 0;
}

static inline void _mem_write_int_enable(a16 addr, u8 data)
{
	if (g_dma_lock)
		return;

	if (g_int_enable_write)
		g_int_enable_write(addr, data);
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
	MEM_BLOCK_DEF(wram0, WRAM0)
	MEM_BLOCK_DEF(wram, WRAM)
	MEM_BLOCK_DEF(wram_echo, WRAM_ECHO)
	MEM_BLOCK_DEF(sprite_attr, SPRITE_ATTR)
	MEM_BLOCK_DEF(empty0, EMPTY0)
	MEM_BLOCK_DEF(io_ports, IO_PORTS)
	MEM_BLOCK_DEF(hram, HRAM)
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
	logger_log(LOG_WARN, "MEM: READ ERROR",
		"MEMORY READ ERROR AT ADDRESS 0x%04X\n", addr);
	return 0;
}

static void _mem_write8_error(a16 addr, u8 data)
{
	// TODO #15: determine proper way to handle error
	logger_log(LOG_WARN, "MEM: WRITE8 ERROR",
		"MEMORY U8 WRITE ERROR AT ADDRESS 0x%04X\n, DATA: 0x%02X",
		addr, data);
}

static void _mem_write16_error(a16 addr, u16 data)
{
	// TODO #15: determine proper way to handle error
	logger_log(LOG_WARN, "MEM: WRITE16 ERROR",
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

int _mem_load_rom(const char *path)
{
	long rom_len;
	u8 rom0[0x4000];

	FILE *rom_fileptr = fopen(path, "rb");

	if(rom_fileptr == NULL) {
		_mem_fatal("Couldn't open rom file");
		return 0;
	}

	fseek(rom_fileptr, 0, SEEK_END);
	rom_len = ftell(rom_fileptr);

	if (rom_len > MAX_ROM_SIZE) {
		_mem_fatal("ROM file size is larger than allowed maximum");
		fclose(rom_fileptr);
		return 0;
	}

	rewind(rom_fileptr);

	fread(rom0, 0x4000, 1, rom_fileptr);

	rom_parse_header(rom0);
	const struct rom_header *header = rom_get_header();

	if (header->rom_bank_size == 0
			|| rom_len / header->rom_bank_size != header->num_rom_banks) {
		_mem_fatal("Incoherent declared ROM bank size");
		return 0;
	}

	rewind(rom_fileptr);

	for (int i = 0; i < header->num_rom_banks; i++) {
		g_rom[i].mem = (u8 *)calloc(1, header->rom_bank_size);
		g_rom[i].size = header->rom_bank_size;
		fread(g_rom[i].mem, g_rom[i].size, 1, rom_fileptr);
	}

	fclose(rom_fileptr);
	return 1;
}

static int _mem_save_ram(const char *save_path)
{
	FILE *file = fopen(save_path, "wb+");

	if(file == NULL) {
		_mem_fatal("Couldn't open save file");
		return 0;
	}

	const struct rom_header *header = rom_get_header();

	for (int i = 0; i < header->num_ram_banks; i++) {
		debug_assert(g_ram[i].mem != NULL,
				"mem_destroy: null RAM Bank memory");
		fwrite(g_ram[i].mem, g_ram[i].size, 1, file);
	}

	if (header->mbc == MBC3) {
		struct mem_rtc_save rtc = mem_rtc_get_save();
		fwrite(&rtc, sizeof(rtc), 1, file);
	}

	fclose(file);
	return 1;
}

static int _mem_load_ram(const char *save_path)
{
	long file_len;
	struct mem_rtc_save rtc;
	FILE *file = fopen(save_path, "rb");
	const struct rom_header *header = rom_get_header();

	if(file == NULL) {
		logger_print(LOG_INFO, "Save file does not exist, skipping save load");
		if (header->mbc == MBC3)
			mem_rtc_prepare(NULL);
		return 1;
	}

	fseek(file, 0, SEEK_END);
	file_len = ftell(file);
	rewind(file);

	int expected_len = header->num_ram_banks * header->ram_bank_size;

	if (header->mbc == MBC3)
		expected_len += sizeof(struct mem_rtc_save);

	if (file_len != expected_len) {
		_mem_fatal("Save file size is incorrect for this cartridge type");
		fclose(file);
		return 0;
	}

	for (int i = 0; i < header->num_ram_banks; i++) {
		debug_assert(g_ram[i].mem != NULL,
				"mem_destroy: null RAM Bank memory");
		fread(g_ram[i].mem, g_ram[i].size, 1, file);
	}

	if (header->mbc == MBC3)
		fread(&rtc, sizeof(struct mem_rtc_save), 1, file);

	fclose(file);

	mem_rtc_prepare(&rtc);

	return 1;
}

/**
 * Initialize memory module:
 *	- parse cartridge header into rom_header
 *	- allocate memory for ROM and RAM
 *	- load ROM contents from ROM file
 *	- optionally load RAM content from save file
 *
 *	@param  rom_path    path to file containing ROM data to load
 *	@param	save_path   path to file containing saved RAM data to load. If NULL
 *	                    is passed, then RAM is ininitialized to 0
 *
 *	@return	1 if everything succeeded, 0 if an error occurred during module
 *          initialization
 */
int mem_prepare(const char *rom_path, const char *save_path)
{
	int rc = 1;

	rc = _mem_load_rom(rom_path);

	if (rc != 1)
		return rc;

	const struct rom_header *header = rom_get_header();

	for (int i = 0; i < header->num_ram_banks; i++) {
		g_ram[i].mem = (u8 *)calloc(1, header->ram_bank_size);
		g_ram[i].size = header->ram_bank_size;
	}

	g_vram[0].mem = (u8 *)calloc(1, SIZE_VRAM);
	g_vram[0].size = SIZE_VRAM;

	if (rom_is_cgb()) {
		for (int i = 0; i < NUM_WRAM_BANKS; i++) {
			g_wram[i].mem = (u8 *)calloc(1, SIZE_WRAM);
			g_wram[i].size = SIZE_WRAM;
		}

		g_vram[1].mem = (u8 *)calloc(1, SIZE_VRAM);
		g_vram[1].size = SIZE_VRAM;
	} else {
		g_wram[0].mem = (u8 *)calloc(1, SIZE_WRAM0);
		g_wram[0].size = SIZE_WRAM0;
		g_wram[1].mem = (u8 *)calloc(1, SIZE_WRAM);
		g_wram[1].size = SIZE_WRAM;
	}

	if (save_path) {
		rc = _mem_load_ram(save_path);
		if (rc != 1) {
			mem_destroy(NULL);
			return rc;
		}
	} else if (header->mbc == MBC3) {
		mem_rtc_prepare(NULL);
	}

	return 1;
}

/**
 * Safely terminate the module and optionally save RAM contents to given path.
 *
 * @param   save_path   path to file to dump RAM to. If NULL is passed, RAM is
 *                      not saved.
 */
void mem_destroy(const char *save_path)
{
	if (save_path)
		_mem_save_ram(save_path);

	const struct rom_header *header = rom_get_header();

	for (int i = 0; i < header->num_rom_banks; i++) {
		debug_assert(g_rom[i].mem != NULL,
				"mem_destroy: null ROM Bank memory");
		free(g_rom[i].mem);
	}

	for (int i = 0; i < header->num_ram_banks; i++) {
		debug_assert(g_rom[i].mem != NULL,
				"mem_destroy: null RAM Bank memory");
		free(g_ram[i].mem);
	}

	for (int i = 0; i < NUM_WRAM_BANKS; i++) {
		if (g_wram[i].mem)
			free(g_wram[i].mem);
	}

	free(g_vram[0].mem);

	if (g_vram[1].mem)
		free(g_vram[1].mem);
}

void mem_step(int cycles_delta)
{
	if (g_dma_lock > cycles_delta) {
		g_dma_lock -= cycles_delta;
	} else {
		g_dma_lock = 0;
	}

	if (g_dma_state == DMA_GENERAL_IN_PROGRESS && g_dma_lock == 0) {
		cpu_set_halted(false);
		g_dma_state = DMA_VRAM_SUCCESS;
	}
}

/* Read from arbitrary VRAM bank
 *
 * NOTE: addr parameter is an address that would be used in normal mem_read8
 *       if this bank was selected.
 */
u8 mem_vram_read8(int bank, a16 addr)
{
	return _mem_read_bank(g_vram[bank], addr - BASE_ADDR_VRAM);
}

/* Write to arbitrary VRAM bank
 *
 * NOTE: addr parameter is an address that would be used in normal mem_write8
 *       if this bank was selected.
 */
void mem_vram_write8(int bank, a16 addr, u8 data)
{
	_mem_write_bank(g_vram[bank], addr - BASE_ADDR_VRAM, data);
}

/* Register read/write handler function for address within IO Ports range
 * (0xFF00-0xFF79) or Interrupt Enable (0xFFFF).
 */
void mem_register_handlers(a16 addr,
		mem_read_handler_t r, mem_write_handler_t w)
{
	if (addr == BASE_ADDR_INT_ENABLE) {
		debug_assert(g_int_enable_read == NULL && g_int_enable_write == NULL,
				"mem_register_handlers: handlers already registered");
		g_int_enable_read = r;
		g_int_enable_write = w;
	} else {
		debug_assert(addr >= BASE_ADDR_IO_PORTS
				&& addr < BASE_ADDR_IO_PORTS + SIZE_IO_PORTS,
				"mem_register_io_port_handlers: "
				"this address is handled within mem");
		debug_assert(g_io_ports_read[addr - BASE_ADDR_IO_PORTS] == NULL
				&& g_io_ports_write[addr - BASE_ADDR_IO_PORTS] == NULL,
				"mem_register_handlers: handlers already registered");

		g_io_ports_read[addr - BASE_ADDR_IO_PORTS] = r;
		g_io_ports_write[addr - BASE_ADDR_IO_PORTS] = w;
	}
}

/* Notify mem module about H-blank interval start.
 *
 * The purpose of this function is to drive the emulation of the H-blank DMA.
 */
void mem_h_blank_notify(void)
{
	if (g_dma_state != DMA_H_BLANK_IN_PROGRESS)
		return;

	_mem_dma(0x10);
}
