#include "messages.h"

bool s_ready;
AppMessageInboxReceived s_callback;

void messages_received_handler(DictionaryIterator* iterator, void* context) {
    Tuple *ready_tuple = dict_find(iterator, MESSAGE_KEY_ready);
    if(ready_tuple != NULL) {
        s_ready = true;
    }

    if(s_callback != NULL) {
        s_callback(iterator, context);
    }
}

bool messages_ready(void) {
    return s_ready;
}

void messages_init(void) {
    app_message_register_inbox_received((AppMessageInboxReceived) messages_received_handler);
    app_message_open(32, 4);
}

void messages_deinit(void) {
    app_message_deregister_callbacks();
    s_callback = NULL;
    s_ready = false;
}

void messages_callback_set(AppMessageInboxReceived callback) {
    s_callback = callback;
}

void messages_callback_clear(void) {
    s_callback = NULL;
}
