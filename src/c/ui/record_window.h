#ifndef record_window_h
#define record_window_h

#include <pebble.h>
#include "select_layer.h"
#include "../resources.h"
#include "../record.h"
#include "../messages.h"
#include "../util.h"
#include "../emoji.h"

Window* record_window_create(record_t* record, bool new);
void record_window_destroy();

#endif
