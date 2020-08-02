#ifndef GPU_H_
#define GPU_H_

#include"cpu.h"

#define CYCLES_PER_FRAME (CPU_CLOCK_SPEED/FRAME_RATE)

#define FRAME_RATE 60

#define SCALING_FACTOR 1

void gpu_prepare(char * rom_title);
bool gpu_step(int cycles_delta);
void _gpu_handle_drawing(void);
void gpu_destroy(void);

#endif /* GPU_H_ */
