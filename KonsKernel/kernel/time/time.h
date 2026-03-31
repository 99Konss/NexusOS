#ifndef TIME_H
#define TIME_H

#include <stdint.h>

extern int timezone_offset;

void get_time(int *hour, int *min, int *sec);

#endif
