#ifndef MISC_H
#define MISC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ksys.h>

int getRandomNumber(int _min, int _max);
void intToStr(int num, int ndigits, char* result);
int getTimeStamp();

#endif
