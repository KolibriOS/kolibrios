#include <shell_api.h>
#include <sys/ksys.h>

void shell_exit()
{
    if (__shell_is_init == __SHELL_INIT_OK)
    {
        __shell_lock(&(__shell_shm->shm_lock));
        __shell_shm->cmd = SHELL_EXIT;
        __SHELL_WAIT();
        __shell_unlock(&(__shell_shm->shm_lock));
        _ksys_shm_close(__shell_shm_name);
    }
}
