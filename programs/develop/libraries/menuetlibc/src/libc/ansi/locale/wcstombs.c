/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>

size_t
wcstombs(char *s, const wchar_t *wcs, size_t n)
{
  int i;
  for (i=0; wcs[i] && (i<n-1); i++)
    s[i] = wcs[i];
  s[i] = 0;
  return i;
}
