#ifndef GPU_H_
#define GPU_H_

#include"cpu.h"

#define CYCLES_PER_FRAME (CPU_CLOCK_SPEED/FRAME_RATE)

#define FRAME_RATE 60


void gpu_prepare(char * rom_title);
void gpu_step(int cycles_delta);
void gpu_destroy(void);

#endif /* GPU_H_ */
