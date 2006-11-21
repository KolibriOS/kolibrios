#include "stdio.h"
#include "stdlib.h"
#include "ctype.h"

/*
** itoab(n,s,b) - Convert "unsigned" n to characters in s using base b.
**                NOTE: This is a non-standard function.
*/
void itoab(int n,char* s,int  b)
{
  char *ptr;
  int lowbit;
  ptr = s;
  b >>= 1;
  do {
    lowbit = n & 1;
    n = (n >> 1) & 32767;
    *ptr = ((n % b) << 1) + lowbit;
    if(*ptr < 10) *ptr += '0'; else *ptr += 55;
    ++ptr;
    } while(n /= b);
  *ptr = 0;
  reverse (s);
}
