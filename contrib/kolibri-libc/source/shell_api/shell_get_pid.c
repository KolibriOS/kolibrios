#include "shell.h"
#include <stdlib.h>

unsigned shell_get_pid()
{
    unsigned pid;
    __shell_init();
    *__shell_shm = SHELL_PID;
    SHELL_WAIT();
    memcpy(&pid, __shell_shm+1, sizeof(unsigned));
    return pid;
}