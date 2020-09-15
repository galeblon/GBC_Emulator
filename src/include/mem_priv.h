#ifndef __MEM_PRIV_H_
#define __MEM_PRIV_H_

#include"mem.h"

typedef u8 (*mem_read_handler_t)(a16 addr);
typedef void (*mem_write_handler_t)(a16 addr, u8 data);

u8 mem_vram_read8(int bank, a16 addr);
void mem_vram_write8(int bank, a16 addr, u8 data);

void mem_register_handlers(a16 addr,
		mem_read_handler_t r, mem_write_handler_t w);

void mem_h_blank_notify(void);

#endif // __MEM_PRIV_H_

