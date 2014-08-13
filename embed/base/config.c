#include "base/config.h"

#include <iniparser.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>

#include "base/logging.h"


static const char* CONFIG_FILE = "config.ini";


static dictionary* dict;


void cfg_init(void) {
  if (!(dict = iniparser_load(CONFIG_FILE)))
    log_fatal("Failure while loading %s.", CONFIG_FILE);
}


#define NOT_FOUND_IF(expr)                                                    \
  if (expr) log_fatal("%s is required.", key);


const char* cfg_str(const char* key) {
  char* str = iniparser_getstring(dict, key, NULL);
  NOT_FOUND_IF(!str);
  return str;
}


int cfg_int(const char* key) {
  int res = iniparser_getint(dict, key, INT_MIN);
  NOT_FOUND_IF(res == INT_MIN);
  return res;
}


double cfg_double(const char* key) {
  double res = iniparser_getdouble(dict, key, NAN);
  NOT_FOUND_IF(isnan(res));
  return res;
}


bool cfg_bool(const char* key) {
  int res = iniparser_getboolean(dict, key, -1);
  NOT_FOUND_IF(res == -1);
  return res;
}
