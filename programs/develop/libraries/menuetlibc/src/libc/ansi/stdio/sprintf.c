/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <limits.h>
#include <libc/file.h>
#include <stdarg.h>

int
vsnprintf(char *str, size_t size, const char *fmt, va_list ap)
{
  FILE _strbuf;
  int len;

  _strbuf._flag = _IOWRT|_IOSTRG;
  _strbuf._ptr = str;
  _strbuf._cnt = size;
  len = _doprnt(fmt, ap, &_strbuf);
  *_strbuf._ptr = 0;
  return len;
}

int
snprintf(char *str, size_t size, const char *fmt, ...)
{
  int len;
  va_list va;
  va_start(va, fmt);
  len = vsnprintf(str, size, fmt, va);
  va_end(va);
  return len;
}

int
sprintf(char *str, const char *fmt, ...)
{
  int len;
  va_list va;
  va_start(va, fmt);
  len = vsnprintf(str, INT_MAX, fmt, va);
  va_end(va);
  return len;
}

