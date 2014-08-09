#include "base/logging.h"

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>


static const log_level_t LOG_ERRMASK = LOG_LEVEL_FATAL
                                     | LOG_LEVEL_ERROR
                                     | LOG_LEVEL_WARNING;


/*
 * 123456 test.c:45 (do_smthngs)         debug> Debug message
 * 123465 foo/test.c:45 (do_smthngs)      info> Information
 * 123645 too/long/path/to/test.c:4... warning> Warning
 * 126345 test.c:45 (do_smthngs)         error> Error message
 * 162345 test.c:45 (do_smthngs)         fatal> Very long long long long me...
 * |<----------------- prefix ---------------->|<--------- message --------->|-
 * |<------------------------- full (including \n) -------------------------->|
 */

static const int PREFIX_SIZE  = 45;
static const int MESSAGE_SIZE = 80;


static const char* level_str(log_level_t level) {
  switch (level) {
    case LOG_LEVEL_FATAL:   return "fatal";
    case LOG_LEVEL_ERROR:   return "error";
    case LOG_LEVEL_WARNING: return "warning";
    case LOG_LEVEL_INFO:    return "info";
    case LOG_LEVEL_DEBUG:   return "debug";

    default:
      assert(0);
  }
}


void* log__message(const char* file, int line, const char* func,
                  log_level_t level, const char* format, ...) {
  const int FULL_SIZE = PREFIX_SIZE + MESSAGE_SIZE + 1;
  assert(file && func && format);

  // Remove prefix 'embed/'
  assert(strncmp(file, "embed/", 6) == 0);
  file += 6;

  va_list arg;
  FILE* log_file = level & LOG_ERRMASK ? stderr : stdout;

  char message[FULL_SIZE];
  int timestamp = uv_now(uv_default_loop()) % 1000000;
  int offset = snprintf(message, PREFIX_SIZE, "%6d %s:%d (%s)",
                        timestamp, file, line, func);

  if (offset > PREFIX_SIZE-10) {
    message[PREFIX_SIZE-10] = ' ';
    memset(message + PREFIX_SIZE-13, '.', 3);
  } else {
    memset(message + offset, ' ', PREFIX_SIZE-6 - offset);
  }

  snprintf(message + PREFIX_SIZE-9, MESSAGE_SIZE+10, "%7s> ", level_str(level));

  va_start(arg, format);
  offset = vsnprintf(message + PREFIX_SIZE, MESSAGE_SIZE+1, format, arg);
  va_end(arg);

  if (offset > MESSAGE_SIZE) {
    memset(message + FULL_SIZE - 4, '.', 3);
    message[FULL_SIZE-1] = '\n';
    fwrite(message, 1, FULL_SIZE, log_file);
  } else {
    message[PREFIX_SIZE + offset] = '\n';
    fwrite(message, 1, PREFIX_SIZE + offset + 1, log_file);
  }

#ifdef NDEBUG
  if (level & LOG_ERRMASK)
#endif
    fflush(stdout);

  if (level == LOG_LEVEL_FATAL) abort();

  return NULL;
}
