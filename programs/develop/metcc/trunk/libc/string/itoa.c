#include "stdio.h"
#include "stdlib.h"
#include "ctype.h"

/*
** itoa(n,s) - Convert n to characters in s 
*/
void itoa(int n,char* s)
{
  int sign;
  char *ptr;
  ptr = s;
  if ((sign = n) < 0) n = -n;
  do {
    *ptr++ = n % 10 + '0';
    } while ((n = n / 10) > 0);
  if (sign < 0) *ptr++ = '-';
  *ptr = '\0';
  reverse(s);
}

