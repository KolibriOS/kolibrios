#include <shell_api.h>

int shell_getc()
{
    __shell_init();

    if (__shell_is_init == __SHELL_INIT_OK)
    {
        __shell_shm->cmd = SHELL_GETC;
        __SHELL_WAIT();

        return *((int*)__shell_shm->data);
    }
    else
    {
        return 0;
    }
}
