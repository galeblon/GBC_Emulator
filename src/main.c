#include<SDL2/SDL_main.h>
#include<stdlib.h>
#include<time.h>
#include"display.h"
#include"gpu.h"
#include"ints.h"
#include"events.h"
#include"joypad.h"
#include"logger.h"
#include"mem.h"
#include"regs.h"
#include"rom.h"
#include"timer.h"
#include"types.h"
#include"sys.h"

static struct sys_args g_args;

#if defined(__x86_64__)
#define SEC (1000000000)
#define NSEC_PER_CLOCK  (SEC / CPU_CLOCK_SPEED)

static inline long timespec_diff(struct timespec *t_end, struct timespec *t_start)
{
	return (t_end->tv_sec * SEC + t_end->tv_nsec) - (t_start->tv_sec * SEC + t_start->tv_nsec);
}

static inline void wait_clock(struct timespec *t_start, int cycles)
{
	int clock_div = cpu_is_double_speed() ? 4 : 2;

	struct timespec t_end;
	clock_gettime(CLOCK_MONOTONIC, &t_end);

	// for some reason the theoretical cycle time divided by 2
	// (by 4 in double speed mode) is about the right speed
	while (timespec_diff(&t_end, t_start) < NSEC_PER_CLOCK / clock_div * cycles) {
		clock_gettime(CLOCK_MONOTONIC, &t_end);
	}
}
#endif // defined(__x86_64__)

int main(int argc, char *argv[])
{
	char *save_path = NULL;
	struct input_bindings *input_bindings = NULL;

	if(!logger_prepare())
		return 1;

	if (!sys_parse_args(argc, argv, &g_args))
		return 1;

	if (g_args.save_path[0] != '\0')
		save_path = g_args.save_path;
	if (g_args.input_bindings.filled)
		input_bindings = &g_args.input_bindings;

	if (!mem_prepare(g_args.rom_path, save_path))
		return 1;

	logger_print(LOG_INFO, "Loaded ROM contents to memory.\n");

	if (!rom_checksum_validate())
		logger_print(LOG_WARN, "ROM header checksum failed.\n");

	logger_print(LOG_INFO, "ROM header checksum passed.\n");
	char title[16];
	rom_get_title(title);
	logger_print(LOG_INFO, "ROM title: %.16s\n", title);

	//TODO prepare memory and fill stack with data according to powerup sequence
	//TODO prepare sound
	cpu_prepare();
	ints_prepare();
	gpu_prepare(title, g_args.frame_rate, g_args.fullscreen);
	if(!events_prepare(input_bindings))
		return 1;
	joypad_prepare();
	timer_prepare();

	logger_print(LOG_INFO, "Starting emulation.\n");
	int cycles_delta = 0;

#if defined(__x86_64__)
	struct timespec t_start;
#endif // defined(__x86_64__)

	// Main Loop
	while ( cycles_delta != -1 && !display_get_closed_status() ) {
#if defined(__x86_64__)
		clock_gettime(CLOCK_MONOTONIC, &t_start);
#endif // defined(__x86_64__)

		cycles_delta = cpu_single_step();
		gpu_step(cpu_is_double_speed() ? cycles_delta/2 : cycles_delta);
		mem_step(cycles_delta);
		// sound_step(cycles_delta)
		joypad_step();
		timer_step(cycles_delta);
		ints_check();

#if defined(__x86_64__)
		wait_clock(&t_start, cycles_delta);
#endif // defined(__x86_64__)
	}

	logger_print(LOG_INFO, "Halting emulation.\n");

	events_destroy();
	gpu_destroy();
	mem_destroy(save_path);
	logger_destroy();

	return 0;
}
