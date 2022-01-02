#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

int format_print(char *dest, size_t maxlen, const char *fmt,va_list argp);


int snprintf(char *dest, size_t size,const char *format,...)
{
  va_list arg;
  va_start (arg, format);
  return format_print(dest,size, format, arg);
}


