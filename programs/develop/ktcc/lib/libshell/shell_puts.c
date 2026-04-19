#include <shell_api.h>
#include <string.h>

void shell_puts(const char *str)
{
    __shell_init();
    *__shell_shm = SHELL_PUTS;
    strcpy(__shell_shm+1, str);
    __SHELL_WAIT();
}