#ifndef INCLUDE_LOGGER_H_
#define INCLUDE_LOGGER_H_

#include"types.h"

#define LOG_BUFFER_SIZE       64
#define LOG_TITLE_MAX_SIZE    64
#define LOG_MESSAGE_MAX_SIZE 256
#define LOG_TOTAL_MAX_SIZE   LOG_TITLE_MAX_SIZE + LOG_MESSAGE_MAX_SIZE

enum logger_verbosity {
	CONCISE,
	VERBOSE
};

enum logger_log_type {
	LOG_INFO,
	LOG_WARN,
	LOG_FATAL,
#ifdef DEBUG
	LOG_ASSERT,
#endif
};


/*
 * Title and message are optional
 * they are skipped if you pass NULL to them.
 */
void logger_store(enum logger_verbosity verbosity, enum logger_log_type type, char *title, const char *fmt, ...);
void* logger_pop(void* args);

void logger_prepare(void);
void logger_destroy(void);

#endif /* INCLUDE_LOGGER_H_ */
