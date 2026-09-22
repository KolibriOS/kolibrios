#include <shell_api.h>
#include <stdio.h>
#include <string.h>

char *shell_gets(char *str, int n)
{
    __shell_init();

    char *ptr = NULL;
    if (__shell_is_init == __SHELL_INIT_OK)
    {
        __shell_shm->cmd = SHELL_GETS;
        __SHELL_WAIT();

        strncpy(str, __shell_shm->data, n);

        if (__shell_shm->return_value.str == EOF || !__shell_shm->return_value.str)
        {
            ptr = __shell_shm->return_value.str;
        }
        else
        {
            ptr = str;
        }
    }

    return ptr;
}
