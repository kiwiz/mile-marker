#ifndef main_window_h
#define main_window_h

#include <pebble.h>
#include "../resources.h"
#include "../record.h"
#include "record_window.h"

Window* main_window_create();
void main_window_destroy();

#endif
