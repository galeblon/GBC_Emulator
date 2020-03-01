#include"logger.h"
#include"cpu.h"

void emulator_log(enum log_type type, char *title, char *message) {
	_IO_FILE *output = stdout;
	if(type == LOG_FATAL)
		output = stderr;
	fprintf(output,
		"GBC_log %s",
		emulator_log_type_to_text(type));
	if(title != NULL)
		fprintf(output,
			":\n%s",
			title);
	fprintf(output, "\n");
	fprintf(output, "Registers:\n");
	cpu_register_print(output);
	if(message != NULL)
		fprintf(output, "Description:\n%s\n", message);
}


char *emulator_log_type_to_text(enum log_type type) {
	switch(type) {
	case LOG_INFO:
		return "Information";
	case LOG_WARN:
		return "Warning";
	case LOG_FATAL:
		return "Fatal error";
	default:
		return "Unkown type";
	}
}
