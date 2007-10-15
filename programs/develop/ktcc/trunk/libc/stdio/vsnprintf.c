#include <kolibrisys.h>
#include <stdlib.h>
#include <stdio.h>

int format_print(char *dest, size_t maxlen, const char *fmt,
            va_list argp);


int vsnprintf(char *dest, size_t size,const char *format,va_list ap)
{

  return format_print(dest,size, format, ap);
}


