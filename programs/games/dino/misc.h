#ifndef MISC_H
#define MISC_H

#include <stdlib.h>
#include <sys/ksys.h>

// ktcc's ksys.h provides max as a macro, newlib's does not
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#define GAME_RAND_MAX 0x7FFFFFFF

unsigned gameRand(void);
void gameSeed(unsigned seed);
int getRandomNumber(int _min, int _max);
void intToStr(int num, int ndigits, char* result);
int getTimeStamp();

#endif
