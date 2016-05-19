#include <kolibrisys.h>
#include <stdlib.h>
#include <stdio.h>

int vsnprintf(char *dest, size_t size, const char *format, va_list ap)
{
  return format_print(dest,size, format, ap);
}


int vsprintf (char * dest, const char * format, va_list ap )
{
  return format_print(dest, 4096, format, ap);
}
