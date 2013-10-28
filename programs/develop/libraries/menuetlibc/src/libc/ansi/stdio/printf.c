/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <limits.h>
#include <libc/file.h>
#include <stdarg.h>

int
printf(const char *fmt, ...)
{
  int len;
  va_list va;
  va_start(va, fmt);

  len = _doprnt(fmt, va, stdout);
  va_end(va);
  return ferror(stdout) ? EOF : len;
}
