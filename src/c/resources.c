#include "resources.h"

GBitmap* s_add_bitmap_black;
GBitmap* s_add_bitmap_white;
GBitmap* s_pin_bitmap_black;
GBitmap* s_cal_bitmap_black;

void resources_init(void) {
    s_add_bitmap_black = gbitmap_create_with_resource(RESOURCE_ID_ADD_BLACK);
    s_add_bitmap_white = gbitmap_create_with_resource(RESOURCE_ID_ADD_WHITE);
    s_pin_bitmap_black = gbitmap_create_with_resource(RESOURCE_ID_PIN_BLACK);
    s_cal_bitmap_black = gbitmap_create_with_resource(RESOURCE_ID_CAL_BLACK);
}

void resources_deinit(void) {
    gbitmap_destroy(s_add_bitmap_black);
    gbitmap_destroy(s_add_bitmap_white);
    gbitmap_destroy(s_pin_bitmap_black);
    gbitmap_destroy(s_cal_bitmap_black);
}
