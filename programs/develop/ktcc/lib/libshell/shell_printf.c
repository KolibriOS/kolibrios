#include <shell_api.h>
#include <stdio.h>

void shell_printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vsnprintf(__shell_shm + 1, SHELL_SHM_MAX, format, ap);
    *__shell_shm = SHELL_PUTS;
    va_end(ap);
    __SHELL_WAIT();
}
