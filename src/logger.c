#include"logger.h"
#include"cpu.h"

static char logger_message_buffer[LOG_MESSAGE_MAX_SIZE];

static char *_logger_log_type_to_text(enum logger_log_type type) {
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

void logger_log(enum logger_log_type type, char *title, char *message) {
	FILE *output = stdout;
	if(type == LOG_FATAL)
		output = stderr;
	fprintf(output,
		"GBC_log %s",
		_logger_log_type_to_text(type));
	if(title != NULL)
		fprintf(output,
			":\n%s",
			title);
	fprintf(output, "\n");
	fprintf(output, "Registers:\n");
	cpu_register_print(output);
	if(message != NULL)
		fprintf(output,
			"Description:\n%.*s\n",
			LOG_MESSAGE_MAX_SIZE,
			message);
}

char *logger_get_msg_buffer(void) {
	return logger_message_buffer;
}
