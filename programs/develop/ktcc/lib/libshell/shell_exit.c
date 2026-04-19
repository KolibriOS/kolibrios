#include <shell_api.h>
#include <sys/ksys.h>

void shell_exit()
{
    if(__shell_is_init){
        *__shell_shm = SHELL_EXIT;
        __SHELL_WAIT();
        _ksys_shm_close(__shell_shm_name);
    }
}