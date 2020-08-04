#ifndef GPU_H_
#define GPU_H_

#include"cpu.h"

#define CYCLES_PER_FRAME (CPU_CLOCK_SPEED/FRAME_RATE)

#define FRAME_RATE 60


d8 gpu_read_bgpi();
void gpu_write_bgpi(d8 new_bgpi);
d8 gpu_read_bgpd();
void gpu_write_bgpd(d8 new_bgpd);
d8 gpu_read_spi();
void gpu_write_spi(d8 new_spi);
d8 gpu_read_spd();
void gpu_write_spd(d8 new_spd);


void gpu_prepare(char * rom_title);
bool gpu_step(int cycles_delta);
void gpu_destroy(void);

#endif /* GPU_H_ */
