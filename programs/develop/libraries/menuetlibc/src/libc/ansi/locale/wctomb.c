/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>

int
wctomb(char *s, wchar_t wchar)
{
  if (s)
    s[0] = wchar;
  return 1;
}
