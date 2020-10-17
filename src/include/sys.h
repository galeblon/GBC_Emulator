#include "types.h"
#include "input.h"

#define PATH_LENGTH  260

struct sys_args {
	char rom_path[PATH_LENGTH];
	char save_path[PATH_LENGTH];
	struct input_bindings input_bindings;
	bool fullscreen;
	int frame_rate;
};

bool sys_parse_args(int argc, char *argv[], struct sys_args *opts);

