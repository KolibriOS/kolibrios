/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <ctype.h>
#include <string.h>

char *
strlwr(char *_s)
{
  char *rv = _s;
  while (*_s)
  {
    *_s = tolower(*_s);
    _s++;
  }
  return rv;
}
