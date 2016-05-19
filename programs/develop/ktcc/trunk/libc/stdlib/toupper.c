#include <stdlib.h>
/*
** return upper-case of c if it is lower-case, else c
*/
unsigned char toupper(unsigned char c)
{
  if(c<='z' && c>='a') return (c-32);
  return (c);
}
