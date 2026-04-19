#include <stdarg.h>
#include <stdio.h>

int sscanf(const char* s, const char* format, ...)
{
    va_list arg;
    int n;
    va_start(arg, format);
    n = vsscanf(s, format, arg);
    va_end(arg);
    return n;
}
