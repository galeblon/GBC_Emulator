#include<stdarg.h>
#include<stdio.h>
#include"cpu.h"
#include"debug.h"
#include"logger.h"

#ifdef DEBUG
static char g_log_buffer[16384];
#endif

static char *_logger_log_type_to_text(enum logger_log_type type)
{
	switch(type) {
	case LOG_INFO:
		return "Information";
	case LOG_WARN:
		return "Warning";
	case LOG_FATAL:
		return "Fatal error";
#ifdef DEBUG
	case LOG_ASSERT:
		return "Assertion error";
#endif
	default:
		return "Unkown type";
	}
}

static inline FILE *_logger_get_output(enum logger_log_type type)
{
	switch(type) {
		case LOG_INFO:
			return stdout;
		case LOG_WARN:
		case LOG_FATAL:
#ifdef DEBUG
		case LOG_ASSERT:
#endif
			return stderr;
	}

	return NULL;
}

void logger_log(enum logger_log_type type, char *title, const char *fmt, ...)
{
	FILE *output = _logger_get_output(type);
	fprintf(output,
		"GBC_log %s",
		_logger_log_type_to_text(type));
	if(title != NULL)
		fprintf(output,
			":\n%s",
			title);
	fprintf(output, "\n");
	fprintf(output, "Registers:\n");
	cpu_register_print(type);

	fprintf(output, "Description:\n");

	va_list args;
	va_start(args, fmt);
	vfprintf(output, fmt, args);
	va_end(args);

	fprintf(output, "\n");
}

void logger_print(enum logger_log_type type, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	FILE *out = _logger_get_output(type);

	vfprintf(out, fmt, args);

	va_end(args);
}

void logger_prepare(void)
{
#ifdef DEBUG
	//Buffered output is faster
	setvbuf(stdout, g_log_buffer, _IOFBF, sizeof(g_log_buffer));
#endif
}

void logger_destroy(void)
{
#ifdef DEBUG
	//If we buffer, we don't want to lose anything
	fflush(stdout);
#endif
}
