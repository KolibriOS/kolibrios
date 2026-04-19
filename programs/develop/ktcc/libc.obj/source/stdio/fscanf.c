#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

int vfscanf(FILE* stream, const char* format, va_list arg)
{
    static char scanf_buffer[STDIO_MAX_MEM];
    fgets(scanf_buffer, STDIO_MAX_MEM-1, stream);
    return vsscanf(scanf_buffer, format, arg);
}

int fscanf(FILE* stream, const char* format, ...)
{
    va_list arg;
    int n;
    va_start(arg, format);

    n = vfscanf(stream, format, arg);

    va_end(arg);
    return n;
}
