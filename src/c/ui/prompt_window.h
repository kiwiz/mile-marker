#ifndef prompt_window_h
#define prompt_window_h

#include <pebble.h>
#include "../resources.h"

#define PDC_WINDOW_DELTA 33

Window* prompt_window_create(char* message, uint32_t resource, bool prompt, bool is_sequence);
void prompt_window_destroy();

#endif
