/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

//#include "format_print.h"

int printf(const char *format, ...)
{
  va_list arg;
  va_start(arg, format);
  return vprintf(format, arg);
}
