/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>

static unsigned long long next = 0;

int
rand(void)
{
  next = next * 1103515245L + 12345;
  next = (next<<15) ^ (next >> 27);
  return (int)((next >> 4) & RAND_MAX);
}

void
srand(unsigned seed)
{
  next = seed;
}
