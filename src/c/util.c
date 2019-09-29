#include "util.h"

void fmt_dms(char* buf, size_t len, double val) {
	int sign = val < 0 ? -1:1;
	int abs_val = fabs(round(val * 1000000));

	double dec = (abs_val % 1000000) / 1000000.0;
	int deg = abs_val / 1000000;
	double min = floor(dec * 60);
	int sec = (dec - min / 60) * 3600 * 100;

	snprintf(buf, len, "%dº%d'%d.%02d\"", sign * deg, (int) min, sec / 100, sec % 100);
}

size_t char_len(char* buf, size_t len) {
    size_t l = 0;
    bool uni = false;

    while(l < len) {
		if(buf[l] == '\0') {
			break;
		}

		if(!uni) {
			if((buf[l++] & 0xc0) == 0xc0) {
				uni = true;
				continue;
			}
		} else {
			if((buf[l] & 0xc0) == 0x80) {
				++l;
				continue;
			}
		}

		break;
    }

    return l;
}
