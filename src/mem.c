#include"mem.h"
#include"rom.h"
#include"logger.h"
#include<stddef.h>
#include<stdio.h>

#define BASE_ADDR_ROM_BANK0      0x0000
#define BASE_ADDR_ROM_SWITCH     0x4000
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

#define SIZE_ROM_BANK0      0x4000
#define SIZE_ROM_SWITCH     0x4000
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

#define NUM_MEM_BLOCKS 12

typedef u8 (*mem_block_read_t)(a16 addr);
typedef void (*mem_block_write_t)(a16 addr, u8 data);

struct mem_block {
	a16 base_addr;
	mem_block_read_t read;
	mem_block_write_t write;
	u16 size;
};

static u8 g_rom[MAX_ROM_SIZE];
static u8 g_vram[SIZE_VRAM];
// TODO #13: implement proper switchable RAM
static u8 g_ram_switch[SIZE_RAM_SWITCH];
static u8 g_inter_ram_8k[SIZE_INTER_RAM_8K];
static u8 g_sprite_attr[SIZE_SPRITE_ATTR];
static u8 g_io_ports[SIZE_IO_PORTS];
static u8 g_inter_ram[SIZE_INTER_RAM];
static u8 g_int_enable;


// Memory block access function definitions

#define MEM_GENERIC_BLOCK_RW(BLOCK, BASE_ADDR)	\
static inline u8 _mem_read_##BLOCK (a16 addr)	\
{	\
	return g_##BLOCK [addr - BASE_ADDR];	\
}	\
	\
static inline void _mem_write_##BLOCK (a16 addr, u8 data)	\
{	\
	g_##BLOCK [addr - BASE_ADDR] = data;	\
}

MEM_GENERIC_BLOCK_RW(vram, BASE_ADDR_VRAM)
// TODO #13: implement proper switchable RAM blocks
MEM_GENERIC_BLOCK_RW(ram_switch, BASE_ADDR_RAM_SWITCH)
MEM_GENERIC_BLOCK_RW(inter_ram_8k, BASE_ADDR_INTER_RAM_8K)
MEM_GENERIC_BLOCK_RW(sprite_attr, BASE_ADDR_SPRITE_ATTR)
MEM_GENERIC_BLOCK_RW(io_ports, BASE_ADDR_IO_PORTS)
MEM_GENERIC_BLOCK_RW(inter_ram, BASE_ADDR_INTER_RAM)

#undef MEM_GENERIC_BLOCK_RW

static inline u8 _mem_read_rom_bank0(a16 addr)
{
	return g_rom[addr - BASE_ADDR_ROM_BANK0];
}

static inline void _mem_write_rom_bank0(a16 addr, u8 data)
{
	g_rom[addr - BASE_ADDR_ROM_BANK0] = data;
}

static inline u8 _mem_read_rom_switch(a16 addr)
{
	return g_rom[addr];  // TODO #13: implement proper switchable rom banks
}

static inline void _mem_write_rom_switch(a16 addr, u8 data)
{
	g_rom[addr] = data;  // TODO #13: implement proper switchable rom banks
}

static inline u8 _mem_read_inter_ram_echo(a16 addr)
{
	return g_inter_ram_8k[addr - BASE_ADDR_INTER_RAM_ECHO];
}

static inline void _mem_write_inter_ram_echo(a16 addr, u8 data)
{
	g_inter_ram_8k[addr - BASE_ADDR_INTER_RAM_ECHO] = data;
}

static inline u8 _mem_read_empty0(a16 addr __attribute__((unused)))
{
	return 0;
}

static inline void _mem_write_empty0(a16 addr __attribute__((unused)),
	u8 data __attribute__((unused)))
{
}

static inline u8 _mem_read_empty1(a16 addr __attribute__((unused)))
{
	return 0;
}

static inline void _mem_write_empty1(a16 addr __attribute__((unused)),
	u8 data __attribute__((unused)))
{
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
	MEM_BLOCK_DEF(rom_bank0, ROM_BANK0)
	MEM_BLOCK_DEF(rom_switch, ROM_SWITCH)
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

	ret = block->read(addr) << 8;

	if (addr + 1 == block->base_addr + block->size) {
		block = _mem_get_block(addr + 1);

		if (block == NULL)
			return _mem_read_error(addr + 1);
	}

	ret |= block->read(addr + 1);
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

	block_1->write(addr, _mem_u16_higher(data));
	block_2->write(addr + 1, _mem_u16_lower(data));
}

int mem_load_rom(char *path)
{
	long rom_len;
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
	fread(g_rom, MAX_ROM_SIZE, 1, rom_fileptr);
	fclose(rom_fileptr);
	return 1;
}
