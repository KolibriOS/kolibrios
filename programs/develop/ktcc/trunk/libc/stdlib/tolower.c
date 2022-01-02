#include <stdlib.h>
/*
** return lower-case of c if upper-case, else c
*/
unsigned char tolower(unsigned char c)
{
  if(c<='Z' && c>='A') return (c+32);
  return (c);
}
