#ifndef __MEM_H_
#define __MEM_H_

#include"types.h"

u8 mem_read8(a16 addr);
u16 mem_read16(a16 addr);

void mem_write8(a16 addr, u8 data);
void mem_write16(a16 addr, u16 data);

int mem_prepare(const char *rom_path, const char *save_path);
void mem_destroy(const char *save_path);

void mem_step(int cycles_delta);

#endif // __MEM_H_
