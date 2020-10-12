#include "types.h"

#define PATH_LENGTH  260

struct sys_args {
	char rom_path[PATH_LENGTH];
	char save_path[PATH_LENGTH];
	char input_config_path[PATH_LENGTH];
};

bool sys_parse_args(int argc, char *argv[], struct sys_args *opts);

