/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#include <stdio.h>
#include <stdarg.h>
#include <limits.h>

void debug_printf(const char *format,...)
{
    va_list ap;
    static char log_board[STDIO_MAX_MEM];
    va_start (ap, format);
    vsnprintf(log_board, STDIO_MAX_MEM, format, ap);
    va_end(ap);
    _ksys_debug_puts(log_board);
}
