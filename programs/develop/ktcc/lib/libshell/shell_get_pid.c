#include <shell_api.h>
#include <string.h>

unsigned shell_get_pid()
{
    unsigned pid;
    __shell_init();
    *__shell_shm = SHELL_PID;
    __SHELL_WAIT();
    memcpy(&pid, __shell_shm+1, sizeof(unsigned));
    return pid;
}