#include<stdio.h>
#include<string.h>
#include"logger.h"
#include"sys.h"

/**
 * Parse commandline arguments to sys_args
 *
 * Available flags:
 *     -s <save path>  path to save file, which will be loaded at startup
 *                     and saved after emulation finishes
 *
 * @param argc  argument count from main
 * @param argv  argument vector form main
 * @param opts  struct to be filled
 *
 * @return	true if parsing successful, false on invalid arguments
 */
bool sys_parse_args(int argc, char *argv[], struct sys_args *opts)
{
	memset(opts, 0, sizeof(struct sys_args));

	char *arg;

	for (int i = 1; i < argc; i++) {
		arg = argv[i];

		if (arg[0] == '-') {
			switch (arg[1]) {
			case 's':
				strncpy(opts->save_path, argv[++i], PATH_LENGTH);
				break;
			}
		} else if (opts->rom_path[0] == '\0') {
			strncpy(opts->rom_path, argv[i], PATH_LENGTH);
		} else {
			logger_print(LOG_FATAL, "Invalid arguments.\n");
			return false;
		}
	}

	return true;
}
