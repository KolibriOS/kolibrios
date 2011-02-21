/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <limits.h>
#include <libc/file.h>

int
sprintf(char *str, const char *fmt, ...)
{
  FILE _strbuf;
  int len;
  va_list va;
  va_start(va, fmt);

  _strbuf._flag = _IOWRT|_IOSTRG;
  _strbuf._ptr = str;
  _strbuf._cnt = INT_MAX;
  len = _doprnt(fmt, va, &_strbuf);
  va_end(va);
  *_strbuf._ptr = 0;
  return len;
}
