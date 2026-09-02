#include <shell_api.h>

void shell_putc(char c)
{
    __shell_init();

    if (__shell_is_init == __SHELL_INIT_OK)
    {
        __shell_shm->data[0] = c;
        __shell_shm->cmd = SHELL_PUTC;
        __SHELL_WAIT();
    }
}