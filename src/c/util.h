#ifndef util_h
#define util_h

#include <math.h>
#include <stdio.h>
#include <stdbool.h>

#define MENU_CELL_BASIC_CELL_HEIGHT 44

void fmt_dms(char* buf, size_t len, double val);
size_t char_len(char* buf, size_t len);

#endif
