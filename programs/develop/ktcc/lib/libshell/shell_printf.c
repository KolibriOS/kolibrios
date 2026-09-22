#include <shell_api.h>
#include <stdio.h>

int shell_printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int val = vsnprintf(__shell_shm->data, sizeof(__shell_shm->data), format, ap);
    __shell_shm->cmd = SHELL_PUTS;
    va_end(ap);
    __SHELL_WAIT();
    return val;
}
