#include "stdio.h"
#include "stdlib.h"
#include "ctype.h"


/*
** atoi(s) - convert s to integer.
*/
int atoi(char *s)
{
  int sign, n;
  while(isspace(*s)) ++s;
  sign = 1;
  switch(*s) {
    case '-': sign = -1;
    case '+': ++s;
    }
  n = 0;
  while(isdigit(*s)) n = 10 * n + *s++ - '0';
  return (sign * n);
}
