#include "record_window.h"

record_t* s_record;
size_t s_index;
Window* s_record_window;
TextLayer* s_label_layer;
SelectLayer* s_select_layer;
size_t s_emoji_idx;
BitmapLayer* s_cal_layer;
TextLayer* s_date_layer;
TextLayer* s_time_layer;
BitmapLayer* s_pin_layer;
TextLayer* s_lat_layer;
TextLayer* s_lon_layer;
TextLayer* s_loading_layer;
ActionBarLayer* s_action_bar_layer;
char date_buf[16];
char time_buf[16];
char lat_buf[18];
char lon_buf[18];

bool record_window_is_new(void) {
    return s_index == STORAGE_RECORDS_INDEX_NEW;
}

void record_window_location_query(void) {
    DictionaryIterator* iterator;
    if(app_message_outbox_begin(&iterator) != APP_MSG_OK) {
        return;
    }

    dict_write_uint8(iterator, MESSAGE_KEY_pos, 0);

    app_message_outbox_send();
}

void record_window_show_loading(bool show) {
    layer_set_hidden(text_layer_get_layer(s_loading_layer), !show);
    layer_set_hidden(text_layer_get_layer(s_lat_layer), show);
    layer_set_hidden(text_layer_get_layer(s_lon_layer), show);
}

void record_window_received_handler(DictionaryIterator* iter, void* context) {
    Tuple* lat_tuple = dict_find(iter, MESSAGE_KEY_lat);
    Tuple* lon_tuple = dict_find(iter, MESSAGE_KEY_lon);
    Tuple* ready_tuple = dict_find(iter, MESSAGE_KEY_ready);

    // kick off a query if the app just became ready
    if(ready_tuple != NULL) {
        record_window_location_query();
        return;
    }

    // check that keys exist
    if(lat_tuple == NULL || lon_tuple == NULL) {
        return;
    }

    // check that data is well formed
    if(
        lat_tuple->type != TUPLE_BYTE_ARRAY || lat_tuple->length != 8 ||
        lon_tuple->type != TUPLE_BYTE_ARRAY || lon_tuple->length != 8
    ) {
        return;
    }

    // render lat/lon to text buffers
    memcpy(&s_record->latitude, lat_tuple->value->data, sizeof(double));
    fmt_dms(lat_buf, sizeof(lat_buf), s_record->latitude);
    memcpy(&s_record->longitude, lon_tuple->value->data, sizeof(double));
    fmt_dms(lon_buf, sizeof(lon_buf), s_record->longitude);
    record_window_show_loading(false);
}

void record_window_click_handler(ClickRecognizerRef recognizer, void* context) {
    switch(click_recognizer_get_button_id(recognizer)) {
        case BUTTON_ID_SELECT:
            select_layer_save_selections(s_select_layer, s_record->emoji, sizeof(((record_t*)0)->emoji));
            records_save(s_record);
            window_stack_remove(s_record_window, true);
            break;
        case BUTTON_ID_BACK:
            window_stack_remove(s_record_window, true);
            break;
        default:
            break;
    }
}

void record_window_delete_record(void* context) {
    records_delete(s_index);
    window_stack_remove(s_record_window, true);
}

void record_window_click_provider(void* context) {
    window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler)record_window_delete_record);
}

void record_window_load(Window* window) {
    if(record_window_is_new()) {
        messages_callback_set(record_window_received_handler);
        record_window_location_query();
    }

    Layer* window_layer = window_get_root_layer(window);
    GRect frame = layer_get_bounds(window_layer);

    GFont text_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
    tm* time_parts = localtime(&s_record->timestamp);

    // add action bar layer
    GRect action_bar_frame = GRect(0, 0, 0, 0);
    s_action_bar_layer = action_bar_layer_create();
    if(!record_window_is_new()) {
        action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_DOWN, s_delete_bitmap_white);
        action_bar_layer_add_to_window(s_action_bar_layer, window);
        action_bar_layer_set_click_config_provider(s_action_bar_layer, (ClickConfigProvider)record_window_click_provider);
        action_bar_frame = layer_get_bounds(action_bar_layer_get_layer(s_action_bar_layer));
    }

    // add label layer
    int16_t label_top = frame.origin.y;
    s_label_layer = text_layer_create(GRect(frame.origin.x, label_top, frame.size.w - action_bar_frame.size.w, 24));
    text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(s_label_layer, GTextAlignmentCenter);
    text_layer_set_text(s_label_layer, record_window_is_new() ? "Add marker":"View marker");
    layer_add_child(window_layer, text_layer_get_layer(s_label_layer));

    // add emoji layer
    int16_t emoji_top = label_top + 24 + 11;
    s_select_layer = select_layer_create(GRect(frame.origin.x, emoji_top, frame.size.w - action_bar_frame.size.w, 40));
    select_layer_add_choices(s_select_layer, location_emojis, LOCATION_EMOJI_COUNT);
    select_layer_add_choices(s_select_layer, action_emojis, ACTION_EMOJI_COUNT);
    select_layer_set_disabled(s_select_layer, !record_window_is_new());
    select_layer_set_click_config_onto_window(s_select_layer, window);
    select_layer_set_callbacks(s_select_layer, NULL, (SelectLayerCallbacks){
        .back_click=record_window_click_handler,
        .select_click=record_window_click_handler,
    });
    select_layer_load_selections(s_select_layer, s_record->emoji);
    layer_add_child(window_layer, select_layer_get_layer(s_select_layer));

    // add calendar icon
    int16_t cal_top = emoji_top + 50 + 3;
    GRect cal_bounds = gbitmap_get_bounds(s_cal_bitmap_black);
    s_cal_layer = bitmap_layer_create(GRect(frame.origin.x + 6, cal_top, cal_bounds.size.w, cal_bounds.size.h));
    bitmap_layer_set_compositing_mode(s_cal_layer, GCompOpSet);
    bitmap_layer_set_bitmap(s_cal_layer, s_cal_bitmap_black);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_cal_layer));

    // add date layer
    int16_t cal_text_left = frame.origin.x + 6 + cal_bounds.size.w + 6;
    strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", time_parts);
    s_date_layer = text_layer_create(GRect(cal_text_left, cal_top - 5, frame.size.w - cal_text_left - action_bar_frame.size.w, 20));
    text_layer_set_font(s_date_layer, text_font);
    text_layer_set_text(s_date_layer, date_buf);
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

    // add time layer
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", time_parts);
    s_time_layer = text_layer_create(GRect(cal_text_left, cal_top + 20 - 7, frame.size.w - cal_text_left - action_bar_frame.size.w, 20));
    text_layer_set_font(s_time_layer, text_font);
    text_layer_set_text(s_time_layer, time_buf);
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

    // add pin icon
    int16_t pin_top = cal_top + 18 + 18 + 3;
    GRect pin_bounds = gbitmap_get_bounds(s_pin_bitmap_black);
    s_pin_layer = bitmap_layer_create(GRect(frame.origin.x + 6, pin_top, pin_bounds.size.w, pin_bounds.size.h));
    bitmap_layer_set_compositing_mode(s_pin_layer, GCompOpSet);
    bitmap_layer_set_bitmap(s_pin_layer, s_pin_bitmap_black);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_pin_layer));

    // add loading layer
    int16_t pin_text_left = frame.origin.x + 6 + pin_bounds.size.w + 6;
    s_loading_layer = text_layer_create(GRect(pin_text_left, pin_top - 5, frame.size.w - pin_text_left - action_bar_frame.size.w, 20));
    text_layer_set_font(s_loading_layer, text_font);
    text_layer_set_text(s_loading_layer, record_window_is_new() ? "Loading...":"Unavailable");
    layer_add_child(window_layer, text_layer_get_layer(s_loading_layer));

    // add lat layer
    fmt_dms(lat_buf, sizeof(lat_buf), s_record->latitude);
    s_lat_layer = text_layer_create(GRect(pin_text_left, pin_top - 5, frame.size.w - pin_text_left - action_bar_frame.size.w, 20));
    text_layer_set_font(s_lat_layer, text_font);
    text_layer_set_text(s_lat_layer, lat_buf);
    layer_add_child(window_layer, text_layer_get_layer(s_lat_layer));

    // add lat layer
    fmt_dms(lon_buf, sizeof(lon_buf), s_record->longitude);
    s_lon_layer = text_layer_create(GRect(pin_text_left, pin_top + 20 - 7, frame.size.w - pin_text_left - action_bar_frame.size.w, 20));
    text_layer_set_font(s_lon_layer, text_font);
    text_layer_set_text(s_lon_layer, lon_buf);
    layer_add_child(window_layer, text_layer_get_layer(s_lon_layer));

    record_window_show_loading(record_window_is_new() || (s_record->latitude != s_record->latitude));
}

void record_window_unload(Window* window) {
    messages_callback_clear();

    text_layer_destroy(s_label_layer);
    select_layer_destroy(s_select_layer);
    bitmap_layer_destroy(s_cal_layer);
    text_layer_destroy(s_date_layer);
    text_layer_destroy(s_time_layer);
    bitmap_layer_destroy(s_pin_layer);
    text_layer_destroy(s_lat_layer);
    text_layer_destroy(s_lon_layer);
    text_layer_destroy(s_loading_layer);
    action_bar_layer_destroy(s_action_bar_layer);
}

Window* record_window_create(record_t* record, size_t idx) {
    if(s_record_window != NULL) {
        record_window_destroy();
    }

    s_record_window = window_create();
    window_set_window_handlers(s_record_window, (WindowHandlers) {
        .load = record_window_load,
        .unload = record_window_unload,
    });

    s_index = idx;
    s_record = record;

    return s_record_window;
}

void record_window_destroy(void) {
    free(s_record);
    s_record = NULL;

    window_destroy(s_record_window);
    s_record_window = NULL;
}
