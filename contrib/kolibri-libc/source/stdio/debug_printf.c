/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#include <stdio.h>
#include <stdarg.h>

void debug_printf(const char *format,...)
{
    va_list ap;
    char log_board[300];
    va_start (ap, format);
    vsnprintf(log_board, 300, format, ap);
    va_end(ap);
    _ksys_debug_puts(log_board);
}
