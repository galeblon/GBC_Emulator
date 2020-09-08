#include"types.h"

u8 mem_rtc_read(u8 reg);
void mem_rtc_write(u8 reg, u8 data);
void mem_rtc_latch(u8 data);
void mem_rtc_prepare(void);
