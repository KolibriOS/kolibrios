#include <shell_api.h>
#include <string.h>

unsigned shell_get_pid()
{
    unsigned pid = 0;

    __shell_init();
    if (__shell_is_init == __SHELL_INIT_OK)
    {
        __shell_lock(&(__shell_shm->shm_lock));
        __shell_shm->cmd = SHELL_PID;
        __SHELL_WAIT();
        memcpy(&pid, __shell_shm->data, sizeof(unsigned));
        __shell_unlock(&(__shell_shm->shm_lock));
    }

    return pid;
}
