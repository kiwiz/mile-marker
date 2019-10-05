#include <pebble.h>
#include "record.h"
#include "resources.h"
#include "messages.h"
#include "ui/main_window.h"

void prv_init(void) {
    resources_init();

    Window* main_window = main_window_create();
    window_stack_push(main_window, true);

    // show record window if launched normally
    if(launch_reason() != APP_LAUNCH_PHONE) {
        record_t* record = record_create();
        Window* record_window = record_window_create(record, STORAGE_RECORDS_INDEX_NEW);
        window_stack_push(record_window, true);
    }

    messages_init();
}

void prv_deinit(void) {
    messages_deinit();

    prompt_window_destroy();
    record_window_destroy();
    main_window_destroy();

    resources_deinit();
}

int main(void) {
    prv_init();
    app_event_loop();
    prv_deinit();
}
