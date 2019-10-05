#ifndef debug_h
#define debug_h

#include <pebble.h>

#define LOG(...) APP_LOG(APP_LOG_LEVEL_WARNING, __VA_ARGS__)

void timer_start(void);
int timer_end(void);

#endif
