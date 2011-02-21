/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdarg.h>
#include <libc/file.h>
#include <libc/unconst.h>

int
vsscanf(const char *str, const char *fmt, va_list ap)
{
  FILE _strbuf;

  _strbuf._flag = _IOREAD|_IOSTRG|_IONTERM;
  _strbuf._ptr = _strbuf._base = unconst(str, char *);
  _strbuf._cnt = 0;
  while (*str++)
    _strbuf._cnt++;
  _strbuf._bufsiz = _strbuf._cnt;
  return _doscan(&_strbuf, fmt, ap);
}
