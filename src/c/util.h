#ifndef util_h
#define util_h

#include <math.h>
#include <stdio.h>
#include <stdbool.h>

#define LOG(...) APP_LOG(APP_LOG_LEVEL_WARNING, __VA_ARGS__)

void fmt_dms(char* buf, size_t len, double val);
size_t char_len(char* buf, size_t len);

#endif
