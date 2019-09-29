#ifndef messages_h
#define messages_h

#include <pebble.h>
#include "record.h"

void messages_init(void);
void messages_deinit(void);
void messages_callback_set(AppMessageInboxReceived callback);
void messages_callback_clear(void);
bool messages_ready(void);

#endif
