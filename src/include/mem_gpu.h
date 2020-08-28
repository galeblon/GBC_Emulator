#ifndef __MEM_GPU_H_
#define __MEM_GPU_H_

#include"mem.h"

u8 mem_vram_read8(int bank, a16 addr);
void mem_vram_write8(int bank, a16 addr, u8 data);

#endif // __MEM_GPU_H_

