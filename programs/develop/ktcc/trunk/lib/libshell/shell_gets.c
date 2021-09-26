#include <shell_api.h>
#include <string.h>

void shell_gets(char *str, int n)
{
    __shell_init();
    *__shell_shm = SHELL_GETS;
    __SHELL_WAIT();
    strncpy(str, __shell_shm+1, n);
}
