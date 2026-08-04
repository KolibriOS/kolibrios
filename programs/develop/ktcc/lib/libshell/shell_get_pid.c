#include <shell_api.h>
#include <string.h>

unsigned shell_get_pid()
{
    unsigned pid = 0;

    __shell_init();
    if (__shell_is_init == __SHELL_INIT_OK)
    {
        *__shell_shm = SHELL_PID;
        __SHELL_WAIT();
        memcpy(&pid, __shell_shm + 1, sizeof(unsigned));
    }

    return pid;
}
