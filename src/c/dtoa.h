#ifndef dtoa_h
#define dtoa_h

char * dtoa_r(double dd, int mode, int ndigits, int *decpt, int *sign, char **rve, char *buf, size_t blen);
char * dtoa(double dd, int mode, int ndigits, int *decpt, int *sign, char **rve);

#endif
