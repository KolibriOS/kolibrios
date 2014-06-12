/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <string.h>

#undef bzero

void *
bzero(void *a, size_t b)
{
  return memset(a,0,b);
}
