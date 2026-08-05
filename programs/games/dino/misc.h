#ifndef MISC_H
#define MISC_H

#include <stdlib.h>
#include <sys/ksys.h>

// ktcc's ksys.h provides max as a macro, newlib's does not
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

int getRandomNumber(int _min, int _max);
void intToStr(int num, int ndigits, char* result);
int getTimeStamp();

#endif
