#include "messages.h"

bool s_ready;
AppMessageInboxReceived s_callback;

#pragma pack(push, 2)
typedef struct {
    uint16_t record_size;
    uint16_t chunk;
    uint16_t count;
    uint16_t more;
    record_t records[STORAGE_RECORDS_PER_KEY];
} fetchdata_t;
#pragma pack(pop)

void messages_fetch_handler(size_t i) {
    DictionaryIterator* iterator;
    fetchdata_t buf;
    buf.record_size = sizeof(record_t);
    buf.chunk = i;

    // populate payload struct
    int delta = records_count() - STORAGE_RECORDS_PER_KEY * i;
    if(delta > 0) {
        persist_read_data(i, &buf.records, STORAGE_RECORDS_PER_KEY * sizeof(record_t));
        buf.more = delta > (int)STORAGE_RECORDS_PER_KEY;
        buf.count = buf.more ? (int)STORAGE_RECORDS_PER_KEY:delta;
    } else {
        buf.more = false;
        buf.count = 0;
    }

    // add to dict
    if(app_message_outbox_begin(&iterator) != APP_MSG_OK) {
        return;
    }
    dict_write_data(iterator, MESSAGE_KEY_data, (uint8_t*)&buf, sizeof(fetchdata_t));

    // send to phone
    app_message_outbox_send();
}

void messages_received_handler(DictionaryIterator* iterator, void* context) {
    Tuple* ready_tuple = dict_find(iterator, MESSAGE_KEY_ready);
    // handle ready message
    if(ready_tuple != NULL) {
        s_ready = true;
    }

    // handle fetch message
    Tuple* fetch_tuple = dict_find(iterator, MESSAGE_KEY_fetch);
    if(
        fetch_tuple != NULL &&
        fetch_tuple->type == TUPLE_INT &&
        fetch_tuple->length == 4
    ) {
        messages_fetch_handler(fetch_tuple->value->uint32);
    }

    // trigger callback if set
    if(s_callback != NULL) {
        s_callback(iterator, context);
    }
}

bool messages_ready(void) {
    return s_ready;
}

void messages_init(void) {
    app_message_register_inbox_received((AppMessageInboxReceived) messages_received_handler);
    // allocate enough space for messages
    // the additional bytes are somewhat arbitrary, but work
    app_message_open(8 + 2, sizeof(fetchdata_t) + 8);
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
