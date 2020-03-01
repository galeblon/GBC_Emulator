#ifndef INCLUDE_LOGGER_H_
#define INCLUDE_LOGGER_H_

enum log_type {
	LOG_INFO,
	LOG_WARN,
	LOG_FATAL,
};

/*
 * Title and message are optional
 * they are skipped if you pass NULL to them.
 */
void emulator_log(enum log_type type, char *title, char *message);

char *emulator_log_type_to_text(enum log_type type);

#endif /* INCLUDE_LOGGER_H_ */
