#include <shell_api.h>

void shell_cls()
{
    __shell_init();
    if (__shell_is_init == __SHELL_INIT_OK)
    {
        __shell_shm->cmd = SHELL_CLS;
        __SHELL_WAIT();
    }
}
