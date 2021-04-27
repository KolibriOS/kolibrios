/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

//#include "format_print.h"
#include <stdio.h>

int snprintf(char* buffer, size_t count, const char* format, ...)
{
  va_list va;
  va_start(va, format);
  const int ret = _vsnprintf(_out_buffer, buffer, count, format, va);
  va_end(va);
  return ret;
}