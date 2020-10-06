#include<pthread.h>
#include<stdarg.h>
#include<stdio.h>
#include<string.h>
#include"cpu.h"
#include"debug.h"
#include"logger.h"
#include"regs.h"

typedef struct log_info {
	enum logger_verbosity verbosity;
	enum logger_log_type  type;
	char                  title[LOG_TITLE_MAX_SIZE];
	struct cpu_registers  registers;
	char                  message[LOG_MESSAGE_MAX_SIZE];
} log_info;

static log_info g_log_buffer[LOG_BUFFER_SIZE];
static bool     g_full                        = false,
		        g_kill                        = false;
static s8       g_start_index                 = 0,
		        g_end_index                   = 0;

static pthread_t       g_logger_thread;
static pthread_cond_t  g_condition_not_empty = PTHREAD_COND_INITIALIZER;
static pthread_cond_t  g_condition_not_full  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t g_lock                = PTHREAD_MUTEX_INITIALIZER;

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
		return "Unknown type";
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

void logger_store(
	enum logger_verbosity verbosity,
	enum logger_log_type type,
	char *title,
	const char *fmt,
	...
)
{
	pthread_mutex_lock(&g_lock);

	//If not full, go
	if(g_full)
		pthread_cond_wait(&g_condition_not_full, &g_lock);

	g_log_buffer[g_end_index].verbosity = verbosity;
	g_log_buffer[g_end_index].type = type;

	if(title == NULL)
		strncpy(
			g_log_buffer[g_end_index].title,
			"",
			LOG_TITLE_MAX_SIZE
		);
	else
		strncpy(
			g_log_buffer[g_end_index].title,
			title,
			LOG_TITLE_MAX_SIZE
		);

	g_log_buffer[g_end_index].registers = cpu_register_get(type);

	va_list args;
	va_start(args, fmt);
	vsnprintf(
		g_log_buffer[g_end_index].message,
		LOG_MESSAGE_MAX_SIZE,
		fmt,
		args
	);
	va_end(args);

	g_end_index++;
	g_end_index %= LOG_BUFFER_SIZE;
	g_full = g_end_index == g_start_index;

	pthread_mutex_unlock(&g_lock);

	// We have filled one element of the buffer, so it's not empty
	pthread_cond_signal(&g_condition_not_empty);
}

void* logger_pop(__attribute__((unused)) void* args)
{
	while(true) {
		pthread_mutex_lock(&g_lock);

		// If not empty, print
		if(!g_full && g_start_index == g_end_index)
			pthread_cond_wait(&g_condition_not_empty, &g_lock);

		FILE *output = _logger_get_output(
			g_log_buffer[g_start_index].type
		);

		if(g_log_buffer[g_start_index].verbosity == VERBOSE) {
			fprintf(output,
				"GBC_log %s",
				_logger_log_type_to_text(g_log_buffer[g_start_index].type)
			);

			if(g_log_buffer[g_start_index].title != NULL)
				fprintf(output,
					":\n%s",
					g_log_buffer[g_start_index].title
				);

			fprintf(output, "\nRegisters:\n");
			fprintf(
				output,
				"\tA: 0x%02X F: 0x%02X\n"
				"\tB: 0x%02X C: 0x%02X\n"
				"\tD: 0x%02X E: 0x%02X\n"
				"\tH: 0x%02X L: 0x%02X\n"
				"\tSP: 0x%04X\n"
				"\tPC: 0x%04X\n"
				"\tZNHC\n"
				"\t%d%d%d%d\n",
				g_log_buffer[g_start_index].registers.A,
				g_log_buffer[g_start_index].registers.F,
				g_log_buffer[g_start_index].registers.B,
				g_log_buffer[g_start_index].registers.C,
				g_log_buffer[g_start_index].registers.D,
				g_log_buffer[g_start_index].registers.E,
				g_log_buffer[g_start_index].registers.H,
				g_log_buffer[g_start_index].registers.L,
				g_log_buffer[g_start_index].registers.SP,
				g_log_buffer[g_start_index].registers.PC,
				g_log_buffer[g_start_index].registers.FLAGS.Z,
				g_log_buffer[g_start_index].registers.FLAGS.N,
				g_log_buffer[g_start_index].registers.FLAGS.H,
				g_log_buffer[g_start_index].registers.FLAGS.C
			);

			fprintf(output, "Description:\n");
		}

		fprintf(
			output,
			g_log_buffer[g_start_index].message
		);

		if(g_log_buffer[g_start_index].verbosity == VERBOSE) {
			fprintf(output, "\n");
		}

		g_start_index++;
		g_start_index %= LOG_BUFFER_SIZE;
		g_full = false;

		// Should we die?
		if(g_kill) {
			pthread_mutex_unlock(&g_lock);
			pthread_cond_broadcast(&g_condition_not_full);
			pthread_cond_broadcast(&g_condition_not_empty);
			pthread_exit(NULL);
		} else {
			// We have printed one, so it's not full
			pthread_mutex_unlock(&g_lock);
			pthread_cond_signal(&g_condition_not_full);
		}

	}

	return NULL;
}

void logger_prepare(void)
{
	pthread_create(&g_logger_thread, NULL, logger_pop, NULL);
}

void logger_destroy(void)
{
	pthread_mutex_lock(&g_lock);
	g_kill = true;
	pthread_mutex_unlock(&g_lock);
	pthread_join(g_logger_thread, NULL);
}
