#include "shell_api.h"

char shell_getc()
{
    __shell_init();
    *__shell_shm = SHELL_GETC;
    __SHELL_WAIT();
    return *(__shell_shm+1);
}