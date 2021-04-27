//#include "format_scan.h"
#include <stdlib.h>
#include <errno.h>

int scanf ( const char * format, ...)
{
   va_list arg;
   int  n;
   va_start(arg, format);

   if(__scanf_buffer == NULL) __scanf_buffer = malloc(STDIO_MAX_MEM);
   if(__scanf_buffer == NULL) errno = ENOMEM; return ENOMEM;

   *__scanf_buffer = 0;
   n = vscanf(format, arg);

   va_end(arg);
   return n;
}


