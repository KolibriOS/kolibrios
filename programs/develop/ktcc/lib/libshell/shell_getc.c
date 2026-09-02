#include <shell_api.h>

char shell_getc()
{
    __shell_init();

    if (__shell_is_init == __SHELL_INIT_OK)
    {
        __shell_shm->cmd = SHELL_GETC;
        __SHELL_WAIT();

        return __shell_shm->data[0];
    }
    else
    {
        return 0;
    }
}
