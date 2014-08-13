#pragma once

#include <stdbool.h>


extern void cfg_init(void);

extern const char* cfg_str(const char* key);
extern int cfg_int(const char* key);
extern double cfg_double(const char* key);
extern bool cfg_bool(const char* key);
