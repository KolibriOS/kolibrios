/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>

void
swab(const void *from, void *to, int n)
{
  unsigned long temp;
  const char* fromc = from;
  char* toc = to;

  n >>= 1; n++;
#define	STEP	temp = *fromc++,*toc++ = *fromc++,*toc++ = temp
  /* round to multiple of 8 */
  while ((--n) & 07)
    STEP;
  n >>= 3;
  while (--n >= 0) {
    STEP; STEP; STEP; STEP;
    STEP; STEP; STEP; STEP;
  }
}
