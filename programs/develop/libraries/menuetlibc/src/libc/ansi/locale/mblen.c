/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>

int
mblen(const char *s, size_t n)
{
  if (s)
  {
    if (n == 0 || *s == 0)
      return 0;
    return 1;
  }
  else
    return 1;
}
