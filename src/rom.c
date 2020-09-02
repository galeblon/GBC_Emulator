#include<stdio.h>
#include"debug.h"
#include"logger.h"
#include"mem.h"
#include"rom.h"
#include"types.h"

static struct rom_header g_header = {0};

static void _rom_cart_type_not_implemented(u8 type_byte, u8 rom_size_byte)
{
	char *message = logger_get_msg_buffer();
	snprintf(message,
		LOG_MESSAGE_MAX_SIZE,
		"CARTRIDGE TYPE 0x%02X ROM SIZE 0x%02X NOT IMPLEMENTED\n",
		type_byte, rom_size_byte);
	logger_log(LOG_WARN,
		"MEM: CART TYPE NOT IMPLEMENTED",
		message);
}

static void _rom_log_cart_type(void)
{
	char *message = logger_get_msg_buffer();
	snprintf(message,
		LOG_MESSAGE_MAX_SIZE,
		"ROM Header parsed:\n"
		"  MBC:       0x%02X\n"
		"  RAM:       %d\n"
		"  BATT:      %d\n"
		"  SRAM:      %d\n"
		"  RUMBLE:    %d\n"
		"  TIMER:     %d\n"
		"  CGB:       %d\n"
		"  ROM BANKS: %04d x 0x%04X B\n"
		"  RAM BANKS: %04d x 0x%04X B\n",
		g_header.mbc, g_header.ram, g_header.battery,
		g_header.sram, g_header.rumble, g_header.timer,
		g_header.cgb_mode,
		g_header.num_rom_banks, g_header.rom_bank_size,
		g_header.num_ram_banks, g_header.ram_bank_size);
	logger_log(LOG_INFO,
		"ROM: HEADER INFO PARSED",
		message);
}

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

void rom_parse_header(u8 *rom0)
{
	u8 cgb_mode_byte = rom0[ROM_CGB_MODE],
		type_byte = rom0[ROM_CART_TYPE],
		rom_size_byte = rom0[ROM_ROM_BANK_SIZE],
		ram_size_byte = rom0[ROM_RAM_BANK_SIZE];

	switch(cgb_mode_byte) {
		case 0x80:
			g_header.cgb_mode = CGB_SUPPORT;
			break;
		case 0xC0:
			g_header.cgb_mode = CGB_ONLY;
			break;
		default:
			if (cgb_mode_byte & 0x8C) {
				g_header.cgb_mode = NON_CGB_UNINITIALIZED_PALETTES;
			} else {
				g_header.cgb_mode = NON_CGB;
			}
	}

	switch(type_byte) {
		case 0x00: // ROM_ONLY
			g_header.mbc = ROM_ONLY;
			break;
		case 0x01: // ROM+MBC1
			g_header.mbc = MBC1;
			break;
		case 0x02: // ROM+MBC1+RAM
			g_header.mbc = MBC1;
			g_header.ram = true;
			break;
		case 0x03: // ROM+MBC1+RAM+BATT
			g_header.mbc = MBC1;
			g_header.ram = true;
			g_header.battery = true;
			break;
		case 0x05: // ROM+MBC2
			g_header.mbc = MBC2;
			break;
		case 0x06: // ROM+MBC2+BATT
			g_header.mbc = MBC2;
			g_header.battery = true;
			break;
		case 0x08: // ROM+RAM
			g_header.mbc = ROM_ONLY;
			g_header.ram = true;
			break;
		case 0x09: // ROM+RAM+BATT
			g_header.mbc = ROM_ONLY;
			g_header.ram = true;
			g_header.battery = true;
			break;
		case 0x0B: // ROM+MMM01
			g_header.mbc = MMM01;
			break;
		case 0x0C: // ROM+MMM01+SRAM
			g_header.mbc = MMM01;
			g_header.sram = true;
			break;
		case 0x0D: // ROM+MMM01+SRAM+BATT
			g_header.mbc = MMM01;
			g_header.sram = true;
			g_header.battery = true;
			break;
		case 0x0F: // ROM+MBC3+TIMER+BATT
			g_header.mbc = MBC3;
			g_header.timer = true;
			g_header.battery = true;
			break;
		case 0x10: // ROM+MBC3+TIMER+RAM+BATT
			g_header.mbc = MBC3;
			g_header.ram = true;
			g_header.timer = true;
			g_header.battery = true;
			break;
		case 0x11: // ROM+MBC3
			g_header.mbc = MBC3;
			break;
		case 0x12: // ROM+MBC3+RAM
			g_header.mbc = MBC3;
			g_header.ram = true;
			break;
		case 0x13: // ROM+MBC3+RAM+BATT
			g_header.mbc = MBC3;
			g_header.ram = true;
			g_header.battery = true;
			break;
		case 0x19: // ROM+MBC5
			g_header.mbc = MBC5;
			break;
		case 0x1A: // ROM+MBC5+RAM
			g_header.mbc = MBC5;
			g_header.ram = true;
			break;
		case 0x1B: // ROM+MBC5+RAM+BATT
			g_header.mbc = MBC5;
			g_header.ram = true;
			g_header.battery = true;
			break;
		case 0x1C: // ROM+MBC5+RUMBLE
			g_header.mbc = MBC5;
			g_header.rumble = true;
			break;
		case 0x1D: // ROM+MBC5+RUMBLE+SRAM
			g_header.mbc = MBC5;
			g_header.rumble = true;
			g_header.sram = true;
			break;
		case 0x1E: // ROM+MBC5+RUMBLE+SRAM+BATT
			g_header.mbc = MBC5;
			g_header.rumble = true;
			g_header.sram = true;
			g_header.battery = true;
			break;
		case 0x1F:
		case 0xFD:
		case 0xFE:
		case 0xFF:
			_rom_cart_type_not_implemented(type_byte, rom_size_byte);
			break;
		default:
			debug_assert(true, "rom_parse_header: unknown cartridge type");
			break;
	}

	if (rom_size_byte == 0x00) {
		g_header.rom_bank_size = 32 * KB;
		g_header.num_rom_banks = 1;
	} else if (rom_size_byte <= 0x08) {
		g_header.rom_bank_size = 16 * KB;
		g_header.num_rom_banks = (2 << rom_size_byte);
	} else if (0x52 <= rom_size_byte && rom_size_byte < 0x55) {
		// TODO: find some reliable info on 0x52-0x54
		_rom_cart_type_not_implemented(type_byte, rom_size_byte);
	} else {
		debug_assert(true, "rom_parse_header: unknown cartridge type");
	}

	if (g_header.mbc == MBC2) {
		g_header.ram_bank_size = 512;
		g_header.num_ram_banks = 1;
	} else {
		switch (ram_size_byte) {
			case 0x00:
				g_header.ram_bank_size = 0;
				g_header.num_ram_banks = 0;
				break;
			case 0x01:
				g_header.ram_bank_size = 2 * KB;
				g_header.num_ram_banks = 1;
				break;
			case 0x02:
				g_header.ram_bank_size = 8 * KB;
				g_header.num_ram_banks = 1;
				break;
			case 0x03:
				g_header.ram_bank_size = 8 * KB;
				g_header.num_ram_banks = 4;
				break;
			case 0x04:
				g_header.ram_bank_size = 8 * KB;
				g_header.num_ram_banks = 16;
				break;
			case 0x05:
				g_header.ram_bank_size = 8 * KB;
				g_header.num_ram_banks = 8;
				break;
			default:
				debug_assert(true, "rom_parse_header: unknown cartridge type");
				break;
		}
	}

	_rom_log_cart_type();
}

const struct rom_header *rom_get_header(void)
{
	return &g_header;
}

bool rom_is_cgb(void)
{
	return g_header.cgb_mode != NON_CGB;
}
