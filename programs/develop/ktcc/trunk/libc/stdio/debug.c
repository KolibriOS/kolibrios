#include <stdarg.h>
#include <stdio.h>
#include <kolibrisys.h>

void debug_printf(const char *format,...)
{
        va_list ap;
        char log_board[300];
        va_start (ap, format);
        vsnprintf(log_board, sizeof log_board, format, ap);
        va_end(ap);
        debug_out_str(log_board);
}
