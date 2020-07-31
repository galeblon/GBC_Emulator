#ifndef INCLUDE_LOGGER_H_
#define INCLUDE_LOGGER_H_

#define LOG_MESSAGE_MAX_SIZE 250

enum logger_log_type {
	LOG_INFO,
	LOG_WARN,
	LOG_FATAL,
	LOG_ASSERT,
};


/*
 * Title and message are optional
 * they are skipped if you pass NULL to them.
 */
void logger_log(enum logger_log_type type, char *title, char *message);

char *logger_get_msg_buffer(void);

#endif /* INCLUDE_LOGGER_H_ */
