#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

int vscanf(const char* format, va_list arg)
{
    static char scanf_buffer[STDIO_MAX_MEM];
    gets(scanf_buffer);
    return vsscanf(scanf_buffer, format, arg);
};

int scanf(const char* format, ...)
{
    va_list arg;
    va_start(arg, format);
    int n = vscanf(format, arg);
    va_end(arg);
    return n;
}
