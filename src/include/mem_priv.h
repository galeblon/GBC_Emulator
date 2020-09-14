#ifndef __MEM_PRIV_H_
#define __MEM_PRIV_H_

#include"mem.h"

u8 mem_vram_read8(int bank, a16 addr);
void mem_vram_write8(int bank, a16 addr, u8 data);

void mem_set_io_ports(a16 addr, u8 data);

void mem_h_blank_notify(void);

#endif // __MEM_PRIV_H_
