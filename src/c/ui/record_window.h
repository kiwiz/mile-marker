#ifndef record_window_h
#define record_window_h

#include <pebble.h>
#include "../resources.h"
#include "../record.h"
#include "../messages.h"
#include "../g_fmt.h"

Window* record_window_create(record_t* record, bool new);
void record_window_destroy();

#endif
