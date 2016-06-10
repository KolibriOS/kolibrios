#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*
** itoa(n,s) - Convert n to characters in s
*/
char* __itoa(int n,char* s)
{
  int sign;
  char *ptr;
  ptr = s;

  if(n == (int)0x80000000)
    return strcpy(s, "-2147483648");  // overflowed -n

  if ((sign = n) < 0) n = -n;
  do {
    *ptr++ = n % 10 + '0';
    } while ((n = n / 10) > 0);
  if (sign < 0) *ptr++ = '-';
  *ptr = '\0';
  return strrev(s);
}

