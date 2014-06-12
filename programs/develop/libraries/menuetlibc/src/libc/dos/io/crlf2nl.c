/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <io.h>

ssize_t
crlf2nl(char *buf, ssize_t len)
{
  char *bp = buf;
  int i=0;
  while (len--)
  {
    if (*bp != 13)
    {
      *buf++ = *bp;
      i++;
    }
    bp++;
  }
  return i;
}
