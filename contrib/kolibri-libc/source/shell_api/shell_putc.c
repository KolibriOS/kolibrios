#include "shell.h"

void shell_putc(char c)
{
    __shell_init();
    *__shell_shm = SHELL_PUTC;
    *(__shell_shm+1) = c;
    SHELL_WAIT();
}