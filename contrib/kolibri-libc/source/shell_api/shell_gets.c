#include "shell.h"

void shell_gets(char *str)
{
    __shell_init();
    *__shell_shm = SHELL_GETS;
    SHELL_WAIT();
    strcpy(str, __shell_shm+1);
}