#include "util.h"

void fmt_dms(char* buf, size_t len, double val) {
	int sign = val < 0 ? -1:1;
	int abs_val = fabs(round(val * 1000000));

	double dec = (abs_val % 1000000) / 1000000.0;
	int deg = abs_val / 1000000;
	double min = floor(dec * 60);
	int sec = (dec - min / 60) * 3600 * 100;

	snprintf(buf, len, "%dº %d' %d.%02d\"", sign * deg, (int) min, sec / 100, sec % 100);
}
