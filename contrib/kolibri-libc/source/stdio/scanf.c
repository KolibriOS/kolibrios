#include "format_scan.h"
#include <stdlib.h>

int scanf ( const char * format, ...)
{
   va_list arg;
   int  n;
   va_start(arg, format);

   if(__scanf_buffer == NULL) __scanf_buffer = malloc(4096);
   if(__scanf_buffer == NULL) return -3;

   *__scanf_buffer = 0;
   n = vscanf(format, arg);

   va_end(arg);
   return n;
}


