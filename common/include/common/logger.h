#ifndef RP2040_LOGGER_H
#define RP2040_LOGGER_H

#include <stdio.h>
#include <stdarg.h>

#include "pico/time.h"

// #define LOG_VERSION "0.1.0"

// #define LOG_USE_COLOR

typedef void (*log_LockFn)(void *udata, int lock);

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define LOG_TRACE(...) log_log(LOG_TRACE, __FILENAME__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) log_log(LOG_DEBUG, __FILENAME__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...)  log_log(LOG_INFO,  __FILENAME__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...)  log_log(LOG_WARN,  __FILENAME__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) log_log(LOG_ERROR, __FILENAME__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) log_log(LOG_FATAL, __FILENAME__, __LINE__, __VA_ARGS__)

void log_set_udata(void *udata);
void log_set_lock(log_LockFn fn);
void log_set_fp(FILE *fp);
void log_set_level(int level);
void log_set_quiet(int enable);

int log_level();

void log_log(int level, const char *file, int line, const char *fmt, ...);



#endif // RP2040_LOGGER_H