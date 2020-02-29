#include"mem.h"
#include<stddef.h>

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

static u8 g_rom_bank0[SIZE_ROM_BANK0];
static u8 g_rom_switch[SIZE_ROM_SWITCH];  // todo: implement proper switchable blocks
static u8 g_vram[SIZE_VRAM];
static u8 g_ram_switch[SIZE_RAM_SWITCH];  // todo: implement proper switchable blocks
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

// todo: implement proper switchable blocks
MEM_GENERIC_BLOCK_RW(rom_bank0, BASE_ADDR_ROM_BANK0)
MEM_GENERIC_BLOCK_RW(rom_switch, BASE_ADDR_ROM_SWITCH)
MEM_GENERIC_BLOCK_RW(vram, BASE_ADDR_VRAM)
MEM_GENERIC_BLOCK_RW(ram_switch, BASE_ADDR_RAM_SWITCH)
MEM_GENERIC_BLOCK_RW(inter_ram_8k, BASE_ADDR_INTER_RAM_8K)
MEM_GENERIC_BLOCK_RW(sprite_attr, BASE_ADDR_SPRITE_ATTR)
MEM_GENERIC_BLOCK_RW(io_ports, BASE_ADDR_IO_PORTS)
MEM_GENERIC_BLOCK_RW(inter_ram, BASE_ADDR_INTER_RAM)

#undef MEM_GENERIC_BLOCK_RW

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

static struct mem_block *_mem_get_block(a16 addr)
{
	for (int i = 0; i < NUM_MEM_BLOCKS; i++) {
		if (addr >= g_mem_blocks[i].base_addr
			&& addr < g_mem_blocks[i].base_addr + g_mem_blocks[i].size)
			return &g_mem_blocks[i];
	}
	return NULL;
}

u8 mem_read8(a16 addr)
{
	struct mem_block *block;

	block = _mem_get_block(addr);

	if (block == NULL)  // todo: error
		return 0;

	return block->read(addr);
}

void mem_write8(a16 addr, u8 data)
{
	struct mem_block *block;

	block = _mem_get_block(addr);

	if (block == NULL) {}  // todo: error

	block->write(addr, data);
}

u16 mem_read16(a16 addr)
{
	struct mem_block *block;
	u16 ret;

	block = _mem_get_block(addr);

	if (block == NULL)  // todo: error
		return 0;

	ret = block->read(addr);

	if (addr + 1 == block->base_addr + block->size) {
		block = _mem_get_block(addr + 1);

		if (block == NULL)  // todo: error
			return 0;
	}

	ret = block->read(addr) << 8;

	return ret;
}

void mem_write16(a16 addr, u16 data)
{
	struct mem_block *block_1, *block_2;

	block_1 = _mem_get_block(addr);

	if (block_1 == NULL) {}  // todo: error

	if (addr + 1 == block_1->base_addr + block_1->size) {
		block_2 = _mem_get_block(addr + 1);
		if (block_2 == NULL) {}  // todo: error
	} else {
		block_2 = block_1;
	}

	block_1->write(addr, (u8)(data >> 8));
	block_2->write(addr, (u8)data);
}
