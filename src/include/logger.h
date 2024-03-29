#ifndef INCLUDE_LOGGER_H_
#define INCLUDE_LOGGER_H_

#include"types.h"

#define LOG_BUFFER_SIZE       64
#define LOG_TITLE_MAX_SIZE    64
#define LOG_MESSAGE_MAX_SIZE 256
#define LOG_TOTAL_MAX_SIZE   LOG_TITLE_MAX_SIZE + LOG_MESSAGE_MAX_SIZE

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
void logger_log(enum logger_log_type type, char *title, const char *fmt, ...);
void logger_print(enum logger_log_type type, const char *fmt, ...);

bool logger_prepare(void);
void logger_destroy(void);

#endif /* INCLUDE_LOGGER_H_ */
