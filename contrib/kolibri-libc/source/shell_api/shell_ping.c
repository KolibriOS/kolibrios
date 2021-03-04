#include "shell.h"
#include <stdlib.h>
#include <ksys.h>

int shell_ping()
{
    __shell_init();
    *__shell_shm = SHELL_PING;
    _ksys_delay(10);
    if(*__shell_shm==SHELL_OK){
        return 1;
    }
    return 0;
}