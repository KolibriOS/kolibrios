/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#include <stdarg.h>
#include <stdio.h>
#include <limits.h>

int vsprintf(char* s, const char* format, va_list arg)
{
    return vsnprintf(s, STDIO_MAX_MEM, format, arg);
}

int vprintf(const char* format, va_list arg)
{
    return vfprintf(stdout, format, arg);
}
