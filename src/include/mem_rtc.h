#include"types.h"

struct mem_rtc_save {
	uint32_t sec;
	uint32_t min;
	uint32_t hour;
	uint32_t day_lower;
	uint32_t day_higher;
	uint32_t latched_sec;
	uint32_t latched_min;
	uint32_t latched_hour;
	uint32_t latched_day_lower;
	uint32_t latched_day_higher;
	uint64_t epoch;
} __attribute__((packed));

u8 mem_rtc_read(u8 reg);
void mem_rtc_write(u8 reg, u8 data);
void mem_rtc_latch(u8 data);
void mem_rtc_prepare(struct mem_rtc_save *save);
struct mem_rtc_save mem_rtc_get_save(void);
