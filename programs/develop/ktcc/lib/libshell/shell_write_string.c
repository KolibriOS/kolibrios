#include <shell_api.h>
#include <string.h>

size_t shell_write_string(const char *s, size_t len)
{
    __shell_init();
    if (__shell_is_init == __SHELL_INIT_OK)
    {
        size_t written_bytes = 0;
        if (len > sizeof(__shell_shm->data) - 1)
        {
            written_bytes += shell_write_string(s, sizeof(__shell_shm->data) - 1);                                           // Outputs as much as it can.
            written_bytes += shell_write_string(s + (sizeof(__shell_shm->data) - 1), len - (sizeof(__shell_shm->data) - 1)); // Outputs the rest.
        }
        else
        {
            memset(__shell_shm->data, 0, sizeof(__shell_shm->data)); // without int shell show \t, \n, lose chars and other trash
            memcpy(__shell_shm->data, s, len);
            __shell_shm->cmd = SHELL_PUTS;
            __SHELL_WAIT();
            written_bytes += __shell_shm->return_value.written_bytes;
        }

        return written_bytes;
    }
    return 0;
}
