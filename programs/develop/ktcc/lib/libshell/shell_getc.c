#include <shell_api.h>
#include <string.h>

int shell_getc()
{
    __shell_init();

    if (__shell_is_init == __SHELL_INIT_OK)
    {
        __shell_lock(&(__shell_shm->shm_lock));
        __shell_shm->cmd = SHELL_GETC;
        __SHELL_WAIT();
        int charRead = *((int*)__shell_shm->data);
        __shell_unlock(&(__shell_shm->shm_lock));   
        return charRead;
    }
    else
    {
        return 0;
    }
}
