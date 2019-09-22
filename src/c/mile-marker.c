#include <pebble.h>
#include "resources.h"
#include "messages.h"
#include "ui/main_window.h"

void prv_init(void) {
    resources_init();

    Window* main_window = main_window_create();
    window_stack_push(main_window, true);

    messages_init();
}

void prv_deinit(void) {
    messages_deinit();

    main_window_destroy();

    resources_deinit();
}

int main(void) {
    prv_init();
    app_event_loop();
    prv_deinit();
}
