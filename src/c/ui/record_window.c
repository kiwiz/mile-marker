#include "record_window.h"

record_t* s_record;
bool s_new;
size_t s_emoji_idx;
Window* s_record_window;
TextLayer* s_label_layer;
TextLayer* s_emoji_layer;
BitmapLayer* s_cal_layer;
TextLayer* s_date_layer;
TextLayer* s_time_layer;
BitmapLayer* s_pin_layer;
TextLayer* s_lat_layer;
TextLayer* s_lon_layer;
char date_buf[16];
char time_buf[16];
char lat_buf[18];
char lon_buf[18];

void record_window_received_handler(DictionaryIterator *iter, void *context) {
    Tuple *lat_tuple = dict_find(iter, MESSAGE_KEY_lat);
    Tuple *lon_tuple = dict_find(iter, MESSAGE_KEY_lon);

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
    layer_mark_dirty(text_layer_get_layer(s_lat_layer));
    layer_mark_dirty(text_layer_get_layer(s_lon_layer));
}

void record_window_location_query() {
    DictionaryIterator* iterator;
    if(app_message_outbox_begin(&iterator) != APP_MSG_OK) {
        return;
    }

    app_message_outbox_send();
}

void record_window_emoji_set(int idx) {
    s_emoji_idx = idx % (sizeof(action_emojis) / sizeof(action_emojis[0]));
    strncpy(s_record->emoji, action_emojis[s_emoji_idx], sizeof(((record_t*)0)->emoji));
    layer_mark_dirty(text_layer_get_layer(s_emoji_layer));
}

void record_window_click_handler(ClickRecognizerRef recognizer, void* context) {
    if(!s_new) {
        return;
    }

    switch(click_recognizer_get_button_id(recognizer)) {
        case BUTTON_ID_UP:
            record_window_emoji_set(s_emoji_idx + 14 - 1);
            break;
        case BUTTON_ID_DOWN:
            record_window_emoji_set(s_emoji_idx + 1);
            break;
        default:
            break;
    }
}

void record_window_load(Window* window) {
    if(s_new) {
        messages_callback_set(record_window_received_handler);
        record_window_location_query();
    }

    Layer* window_layer = window_get_root_layer(window);
    GRect frame = layer_get_bounds(window_layer);

    GFont text_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
    tm* time_parts = localtime(&s_record->timestamp);

    // add label layer
    int16_t label_top = frame.origin.y;
    s_label_layer = text_layer_create(GRect(frame.origin.x, label_top, frame.size.w, 24));
    text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(s_label_layer, GTextAlignmentCenter);
    text_layer_set_text(s_label_layer, s_new ? "Add marker":"Viewing marker");
    layer_add_child(window_layer, text_layer_get_layer(s_label_layer));

    // add emoji layer
    int16_t emoji_top = label_top + 24 + 11;
    s_emoji_layer = text_layer_create(GRect(frame.origin.x, emoji_top, frame.size.w, 40));
    text_layer_set_font(s_emoji_layer, s_emoji_font);
    text_layer_set_text_alignment(s_emoji_layer, GTextAlignmentCenter);
    text_layer_set_text(s_emoji_layer, s_record->emoji);
    layer_add_child(window_layer, text_layer_get_layer(s_emoji_layer));

    // add calendar icon
    int16_t cal_top = emoji_top + 50 + 3;
    GRect cal_bounds = gbitmap_get_bounds(s_cal_bitmap_black);
    s_cal_layer = bitmap_layer_create(GRect(frame.origin.x + 10, cal_top, cal_bounds.size.w, cal_bounds.size.h));
    bitmap_layer_set_compositing_mode(s_cal_layer, GCompOpSet);
    bitmap_layer_set_bitmap(s_cal_layer, s_cal_bitmap_black);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_cal_layer));

    // add date layer
    int16_t cal_text_left = frame.origin.x + 10 + cal_bounds.size.w + 10;
    strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", time_parts);
    s_date_layer = text_layer_create(GRect(cal_text_left, cal_top - 5, frame.size.w - cal_text_left, 18));
    text_layer_set_font(s_date_layer, text_font);
    text_layer_set_text(s_date_layer, date_buf);
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

    // add time layer
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", time_parts);
    s_time_layer = text_layer_create(GRect(cal_text_left, cal_top + 18 - 5, frame.size.w - cal_text_left, 18));
    text_layer_set_font(s_time_layer, text_font);
    text_layer_set_text(s_time_layer, time_buf);
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

    // add pin icon
    int16_t pin_top = cal_top + 18 + 18 + 3;
    GRect pin_bounds = gbitmap_get_bounds(s_pin_bitmap_black);
    s_pin_layer = bitmap_layer_create(GRect(frame.origin.x + 10, pin_top, pin_bounds.size.w, pin_bounds.size.h));
    bitmap_layer_set_compositing_mode(s_pin_layer, GCompOpSet);
    bitmap_layer_set_bitmap(s_pin_layer, s_pin_bitmap_black);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_pin_layer));

    int16_t pin_text_left = frame.origin.x + 10 + pin_bounds.size.w + 10;
    fmt_dms(lat_buf, sizeof(lat_buf), s_record->latitude);
    s_lat_layer = text_layer_create(GRect(pin_text_left, pin_top - 5, frame.size.w - pin_text_left, 18));
    text_layer_set_font(s_lat_layer, text_font);
    text_layer_set_text(s_lat_layer, lat_buf);
    layer_add_child(window_layer, text_layer_get_layer(s_lat_layer));

    fmt_dms(lon_buf, sizeof(lon_buf), s_record->longitude);
    s_lon_layer = text_layer_create(GRect(pin_text_left, pin_top + 18 - 5, frame.size.w - pin_text_left, 18));
    text_layer_set_font(s_lon_layer, text_font);
    text_layer_set_text(s_lon_layer, lon_buf);
    layer_add_child(window_layer, text_layer_get_layer(s_lon_layer));

    record_window_emoji_set(0);
}

void record_window_unload(Window* window) {
    if(s_new) {
        records_save(s_record);
    }

    messages_callback_clear();

    text_layer_destroy(s_label_layer);
    text_layer_destroy(s_date_layer);
    text_layer_destroy(s_time_layer);
    bitmap_layer_destroy(s_pin_layer);
    text_layer_destroy(s_lat_layer);
    text_layer_destroy(s_lon_layer);
}

void record_window_click_provider(void* context) {
    int key_repeat_delay = 250;

    window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler)record_window_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler)record_window_click_handler);
    window_single_repeating_click_subscribe(BUTTON_ID_UP, key_repeat_delay, (ClickHandler)record_window_click_handler);
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN, key_repeat_delay, (ClickHandler)record_window_click_handler);
}

Window* record_window_create(record_t* record, bool new) {
    if(s_record_window == NULL) {
        s_record_window = window_create();
        window_set_window_handlers(s_record_window, (WindowHandlers) {
            .load = record_window_load,
            .unload = record_window_unload,
        });
        window_set_click_config_provider(s_record_window, (ClickConfigProvider)record_window_click_provider);
    }

    s_new = new;
    s_record = record;

    return s_record_window;
}

void record_window_destroy(void) {
    free(s_record);
    s_record = NULL;

    window_destroy(s_record_window);
    s_record_window = NULL;
}
