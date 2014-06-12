/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>

int
mbtowc(wchar_t *pwc, const char *s, size_t n)
{
  int x = 0;
  if (s == 0)
    return 0;
  if (*s)
    x = 1;

  if (pwc)
    *pwc = *s;

  return x;
}
