#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"logger.h"
#include"sys.h"

#define DEFAULT_FRAME_RATE 30

int _sys_load_custom_bindings(const char *input_config_path, struct sys_args *opts) {
	logger_print(LOG_INFO, "SYS MODULE: loading custom bindings from %s.\n", input_config_path);

	FILE *bindings_fileptr = fopen(input_config_path, "r");

	if(bindings_fileptr == NULL) {
		logger_print(LOG_FATAL, "Couldn't open input bindings file.\n");
		return 0;
	}

	char line[256];

	int binding_values[INPUT_BINDINGS_TO_READ];
	int current_index = 0;
	while(fgets(line, sizeof(line), bindings_fileptr)) {
		if(line[0] == '#' || line[0] == '\n')
			continue;
		int value = atoi(line);
		binding_values[current_index++] = value;
		if(current_index == INPUT_BINDINGS_TO_READ)
			break;
	}

	fclose(bindings_fileptr);
	if(current_index != INPUT_BINDINGS_TO_READ) {
		logger_print(LOG_FATAL, "Config file doesn't contain all sections.\n");
		return 0;
	}

	// Fill from sections
	struct keyboard_bindings bindings = {
			binding_values[0],
			binding_values[1],
			binding_values[2],
			binding_values[3],
			binding_values[4],
			binding_values[5],
			binding_values[6],
			binding_values[7]
	};

	opts->input_bindings.keyboard = bindings;

	struct gamepad_bindings pad_bindings = {
			binding_values[8],
			binding_values[9],
			binding_values[10],
			binding_values[11],
			binding_values[12],
			binding_values[13],
			binding_values[14]
	};

	opts->input_bindings.gamepad = pad_bindings;
	opts->input_bindings.filled = true;
	return 1;
}


/**
 * Parse commandline arguments to sys_args
 *
 * Available flags:
 *     -s <save path>  path to save file, which will be loaded at startup
 *                     and saved after emulation finishes
 *     -c <input bindings> path to input bindings file. For reference
 *                     check the provided input.config file
 *     -f              run in fulscreen window
 *     -r <frame rate> adjust display frame rate
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

	opts->frame_rate = DEFAULT_FRAME_RATE;

	char *arg;

	for (int i = 1; i < argc; i++) {
		arg = argv[i];

		if (arg[0] == '-') {
			switch (arg[1]) {
			case 's':
				strncpy(opts->save_path, argv[++i], PATH_LENGTH);
				break;
			case 'c':
				_sys_load_custom_bindings(argv[++i], opts);
				break;
			case 'f':
				opts->fullscreen = true;
				break;
			case 'r':
				opts->frame_rate = atoi(argv[++i]);
				if (opts->frame_rate <= 0)
					opts->frame_rate = DEFAULT_FRAME_RATE;
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

