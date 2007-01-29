#include "kolibc.h"

int _doprnt(char *dest, size_t maxlen, const char *fmt,
            va_list argp);


int sprintf(char *dest, const char *format,...)
{
  va_list arg;
  va_start (arg, format);
  return _doprnt(dest,512, format, arg);
}


