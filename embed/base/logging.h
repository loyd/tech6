#pragma once

#include <stdint.h>


typedef enum {
  LOG_LEVEL_FATAL   = 1 << 0,
  LOG_LEVEL_ERROR   = 1 << 1,
  LOG_LEVEL_WARNING = 1 << 2,
  LOG_LEVEL_INFO    = 1 << 3,
  LOG_LEVEL_DEBUG   = 1 << 4
} log_level_t;


extern void* log__message(const char* file, int line, const char* func,
                          log_level_t level, const char* format, ...)
  __attribute__((format(printf, 5, 6)));


#ifdef NDEBUG
# define log_debug(...)   NULL
#else
# define log_debug(...)   log__message(__FILE__, __LINE__, __func__,  \
                                       LOG_LEVEL_DEBUG, __VA_ARGS__)
#endif

#define log_info(...)     log__message(__FILE__, __LINE__, __func__,  \
                                       LOG_LEVEL_INFO, __VA_ARGS__)
#define log_warning(...)  log__message(__FILE__, __LINE__, __func__,  \
                                       LOG_LEVEL_WARNING, __VA_ARGS__)
#define log_error(...)    log__message(__FILE__, __LINE__, __func__,  \
                                       LOG_LEVEL_ERROR, __VA_ARGS__)
#define log_fatal(...)    log__message(__FILE__, __LINE__, __func__,  \
                                       LOG_LEVEL_FATAL, __VA_ARGS__)
