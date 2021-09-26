#include <shell_api.h>

void shell_cls()
{
    __shell_init();
    *__shell_shm = SHELL_CLS;
    __SHELL_WAIT();
}