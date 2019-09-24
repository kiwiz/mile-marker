#ifndef resources_h
#define resources_h

#include <pebble.h>

extern GBitmap* s_pin_bitmap_black;
extern GBitmap* s_cal_bitmap_black;
extern GFont s_emoji_font;

void resources_init(void);
void resources_deinit(void);

#endif
