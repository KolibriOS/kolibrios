#include <sys/ksys.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <shell_api.h>

char app_name[13];
char __shell_shm_name[32];
char *__shell_shm = NULL;
enum __SHELL_INIT_STATE __shell_is_init = __SHELL_NOT_LOADED;

int __shell_shm_init()
{
    __shell_is_init = __SHELL_LOADING;
    ksys_thread_t proc_info;
    unsigned PID;

    _ksys_thread_info(&proc_info, -1);
    PID = proc_info.pid;
    strncpy(app_name, (&proc_info)->name, 12);

    itoa(PID, __shell_shm_name);
    strcat(__shell_shm_name, "-SHELL");

    return _ksys_shm_open(__shell_shm_name, KSYS_SHM_OPEN_ALWAYS | KSYS_SHM_WRITE, SHELL_SHM_MAX, &__shell_shm);
}

void __shell_init()
{
    switch (__shell_is_init)
    {
    case __SHELL_NOT_LOADED:
        if (__shell_shm_init())
        {
            debug_printf("%s: shell problems detected!\n", app_name);
            goto __shell_init_err;
        }

        if (!shell_ping())
        {
            goto __shell_init_err;
        }

        __shell_is_init = __SHELL_INIT_OK; // The shell is being pinged, so it's working.
        break;
    case __SHELL_LOADING:
        while (__shell_is_init == __SHELL_LOADING)
        {
            _ksys_thread_yield();
        }
    case __SHELL_INIT_OK:
        if (!shell_ping())
        {
            goto __shell_init_err;
        }
        break;
    default:
        break;
    }

    return;

__shell_init_err:
    __shell_is_init = __SHELL_INIT_FAILED;
    shell_exit();
    return;
}
