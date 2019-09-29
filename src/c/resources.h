#ifndef resources_h
#define resources_h

#include <pebble.h>

#define ACCENT_COLOR GColorFolly

extern GBitmap* s_pin_bitmap_black;
extern GBitmap* s_cal_bitmap_black;
extern GBitmap* s_check_bitmap_white;
extern GFont s_emoji_font;

void resources_init(void);
void resources_deinit(void);


#endif
