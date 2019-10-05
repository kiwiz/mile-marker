#include "debug.h"

uint16_t ms;
time_t s;

void timer_start(void) {
    time_ms(&s, &ms);
}

int timer_end(void) {
    uint16_t end_ms;
    time_t end_s;
    time_ms(&end_s, &end_ms);

    // calculate delta
    int delta = end_ms - ms;
    if(delta < 0) {
        delta += 1000;
        end_s -= 1;
    }
    delta += (end_s - s) * 1000;

    return delta;
}
