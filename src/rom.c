#include"debug.h"
#include"logger.h"
#include"mem.h"
#include"rom.h"
#include"types.h"

static struct rom_header g_header = {0};

static void _rom_cart_type_not_implemented(u8 type_byte, u8 rom_size_byte)
{
	logger_log(
		LOG_WARN,
		"MEM: CART TYPE NOT IMPLEMENTED",
		"CARTRIDGE TYPE 0x%02X ROM SIZE 0x%02X NOT IMPLEMENTED\n",
		type_byte, rom_size_byte);
}

static void _rom_log_cart_type(void)
{
	logger_log(
		LOG_INFO,
		"ROM: HEADER INFO PARSED",
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
}

int rom_checksum_validate(void)
{
	u8 header_checksum = mem_read8(ROM_CHECKSUM);
	u8 header_checksum_verify = 0;
	for (int i=0x134; i<=0x14C; i++)
		header_checksum_verify -= mem_read8(i) + 1;
	logger_print(LOG_INFO, NULL, "ROM header checksum: 0x%X\n"
			"ROM header checksum calculated: 0x%X\n", header_checksum,
			header_checksum_verify);
	return header_checksum == header_checksum_verify;
}

bool rom_is_licensee(void)
{
	d8 old_licensee_code = mem_read8(0x014B);

	if(old_licensee_code == 0x33) {
		d16 new_licensee_code = mem_read16(0x0144);
		u8  translated_code   = (
			 ((new_licensee_code & 0x0F00) >> 4)
			| (new_licensee_code & 0x000F)
		);

		switch(translated_code) {
		case 0x00:
		case 0x01:
		case 0x08:
		case 0x13:
		case 0x18:
		case 0x19:
		case 0x20:
		case 0x22:
		case 0x24:
		case 0x25:
		case 0x28:
		case 0x29:
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x37:
		case 0x38:
		case 0x39:
		case 0x41:
		case 0x42:
		case 0x44:
		case 0x46:
		case 0x47:
		case 0x49:
		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57:
		case 0x58:
		case 0x59:
		case 0x60:
		case 0x61:
		case 0x64:
		case 0x67:
		case 0x69:
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x75:
		case 0x78:
		case 0x79:
		case 0x80:
		case 0x83:
		case 0x86:
		case 0x87:
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x95:
		case 0x96:
		case 0x97:
		case 0x99:
		case 0xA4:
			return true;
		default:
			return false;
		}
	} else {
		switch(old_licensee_code) {
		case 0x00:
		case 0x01:
		case 0x08:
		case 0x09:
		case 0x0A:
		case 0x0B:
		case 0x0C:
		case 0x13:
		case 0x18:
		case 0x19:
		case 0x1A:
		case 0x1D:
		case 0x1F:
		case 0x24:
		case 0x25:
		case 0x28:
		case 0x29:
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x38:
		case 0x39:
		case 0x3C:
		case 0x3E:
		case 0x41:
		case 0x42:
		case 0x44:
		case 0x46:
		case 0x47:
		case 0x49:
		case 0x4A:
		case 0x4D:
		case 0x4F:
		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57:
		case 0x59:
		case 0x5A:
		case 0x5B:
		case 0x5C:
		case 0x5D:
		case 0x60:
		case 0x61:
		case 0x67:
		case 0x69:
		case 0x6E:
		case 0x6F:
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x75:
		case 0x78:
		case 0x79:
		case 0x7A:
		case 0x7C:
		case 0x7F:
		case 0x80:
		case 0x83:
		case 0x86:
		case 0x8B:
		case 0x8C:
		case 0x8E:
		case 0x8F:
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x95:
		case 0x96:
		case 0x97:
		case 0x99:
		case 0x9A:
		case 0x9B:
		case 0x9C:
		case 0x9D:
		case 0x9F:
		case 0xA1:
		case 0xA2:
		case 0xA4:
		case 0xA6:
		case 0xA7:
		case 0xA9:
		case 0xAA:
		case 0xAC:
		case 0xAD:
		case 0xAF:
		case 0xB0:
		case 0xB1:
		case 0xB2:
		case 0xB4:
		case 0xB6:
		case 0xB7:
		case 0xB9:
		case 0xBA:
		case 0xBB:
		case 0xBD:
		case 0xBF:
		case 0xC0:
		case 0xC2:
		case 0xC3:
		case 0xC4:
		case 0xC5:
		case 0xC6:
		case 0xC8:
		case 0xC9:
		case 0xCA:
		case 0xCB:
		case 0xCC:
		case 0xCD:
		case 0xCE:
		case 0xCF:
		case 0xD0:
		case 0xD1:
		case 0xD2:
		case 0xD3:
		case 0xD4:
		case 0xD6:
		case 0xD7:
		case 0xD9:
		case 0xDA:
		case 0xDB:
		case 0xDD:
		case 0xDE:
		case 0xDF:
		case 0xE0:
		case 0xE1:
		case 0xE2:
		case 0xE3:
		case 0xE5:
		case 0xE7:
		case 0xE8:
		case 0xE9:
		case 0xEA:
		case 0xEB:
		case 0xEC:
		case 0xEE:
		case 0xF0:
		case 0xF3:
		case 0xFF:
			return true;
		default:
			return false;
		}
	}
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
