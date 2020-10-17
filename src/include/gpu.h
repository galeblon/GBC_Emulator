#ifndef GPU_H_
#define GPU_H_

#include"cpu.h"

void gpu_prepare(char * rom_title, int frame_rate, bool fullscreen);
void gpu_step(int cycles_delta);
void gpu_destroy(void);

#endif /* GPU_H_ */
