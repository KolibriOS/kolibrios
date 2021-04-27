#include <stdio.h>

int fscanf(FILE* stream, const char* format, ...)
{
   va_list arg;
   int n;
   va_start(arg, format);
   n = vfscanf(stream, format, arg);
   va_end(arg);
   return n;
}


