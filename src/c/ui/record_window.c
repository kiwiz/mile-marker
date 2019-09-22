#include "record_window.h"

record_t* s_record;
bool s_new;
Window* s_record_window;
TextLayer* s_label_layer;
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

    if(lat_tuple == NULL || lon_tuple == NULL) {
        return;
    }

    if(
        lat_tuple->type != TUPLE_BYTE_ARRAY || lat_tuple->length != 8 ||
        lon_tuple->type != TUPLE_BYTE_ARRAY || lon_tuple->length != 8
    ) {
        return;
    }

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

void record_window_load(Window* window) {
    if(s_new) {
        messages_callback_set(record_window_received_handler);
        record_window_location_query();
    }

    Layer* window_layer = window_get_root_layer(window);
    GRect frame = layer_get_bounds(window_layer);

    tm* time_parts = localtime(&s_record->timestamp);

    int16_t label_top = 10;
    s_label_layer = text_layer_create(GRect(0, label_top, frame.size.w, 15));
    text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
    text_layer_set_text_alignment(s_label_layer, GTextAlignmentCenter);
    text_layer_set_text(s_label_layer, "Add marker");
    layer_add_child(window_layer, text_layer_get_layer(s_label_layer));

    int16_t cal_top = label_top + 80;
    GRect cal_bounds = gbitmap_get_bounds(s_cal_bitmap_black);
    s_cal_layer = bitmap_layer_create(GRect(10, cal_top, cal_bounds.size.w, cal_bounds.size.h));
    bitmap_layer_set_compositing_mode(s_cal_layer, GCompOpSet);
    bitmap_layer_set_bitmap(s_cal_layer, s_cal_bitmap_black);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_cal_layer));

    strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", time_parts);
    s_date_layer = text_layer_create(GRect(30, cal_top - (cal_bounds.size.h / 2), 100, 15));
    text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text(s_date_layer, date_buf);

    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", time_parts);
    s_time_layer = text_layer_create(GRect(30, cal_top + (cal_bounds.size.h / 2), 100, 15));
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text(s_time_layer, time_buf);
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

    int16_t pin_top = cal_top + 35;
    GRect pin_bounds = gbitmap_get_bounds(s_pin_bitmap_black);
    s_pin_layer = bitmap_layer_create(GRect(10, pin_top, pin_bounds.size.w, pin_bounds.size.h));
    bitmap_layer_set_compositing_mode(s_pin_layer, GCompOpSet);
    bitmap_layer_set_bitmap(s_pin_layer, s_pin_bitmap_black);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_pin_layer));

    fmt_dms(lat_buf, sizeof(lat_buf), s_record->latitude);
    s_lat_layer = text_layer_create(GRect(30, pin_top - (pin_bounds.size.h / 2), 100, 15));
    text_layer_set_font(s_lat_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text(s_lat_layer, lat_buf);
    layer_add_child(window_layer, text_layer_get_layer(s_lat_layer));

    fmt_dms(lon_buf, sizeof(lon_buf), s_record->longitude);
    s_lon_layer = text_layer_create(GRect(30, pin_top + (pin_bounds.size.h / 2), 100, 15));
    text_layer_set_font(s_lon_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text(s_lon_layer, lon_buf);
    layer_add_child(window_layer, text_layer_get_layer(s_lon_layer));
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

Window* record_window_create(record_t* record, bool new) {
    if(s_record_window == NULL) {
        s_record_window = window_create();
        window_set_window_handlers(s_record_window, (WindowHandlers) {
            .load = record_window_load,
            .unload = record_window_unload,
        });
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
