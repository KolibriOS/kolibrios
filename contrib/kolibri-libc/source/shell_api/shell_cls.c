#include "shell.h"

void shell_cls()
{
    __shell_init();
    *__shell_shm = SHELL_CLS;
    SHELL_WAIT();
}