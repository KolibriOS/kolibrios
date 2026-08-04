#include <shell_api.h>
#include <string.h>

void shell_write_string(const char *s, size_t len)
{
    __shell_init();
    if (__shell_is_init == __SHELL_INIT_OK)
    {
        if (len > SHELL_SHM_MAX - 1)
        {
            shell_write_string(s, SHELL_SHM_MAX - 1);                               // Outputs as much as it can.
            shell_write_string(s + (SHELL_SHM_MAX - 1), len - (SHELL_SHM_MAX - 1)); // Outputs the rest.
        }
        else
        {
            memset(__shell_shm, 0, SHELL_SHM_MAX);  // without int shell show \t, \n, lose chars and other trash
            memcpy(__shell_shm + 1, s, len);
            *__shell_shm = SHELL_PUTS;
            __SHELL_WAIT();
        }
    }
}
