/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <ctype.h>
#include <string.h>

char *
strupr(char *_s)
{
  char *rv = _s;
  while (*_s)
  {
    *_s = toupper(*_s);
    _s++;
  }
  return rv;
}
