/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>

size_t
mbstowcs(wchar_t *wcs, const char *s, size_t n)
{
  int i;
  for (i=0; s[i] && (i<n-1); i++)
    wcs[i] = s[i];
  wcs[i] = 0;
  return i;
}
