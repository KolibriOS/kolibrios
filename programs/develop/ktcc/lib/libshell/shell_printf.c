#include <shell_api.h>
#include <stdio.h>

int shell_printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    __shell_init();
    if (__shell_is_init != __SHELL_INIT_OK) {
        va_end(ap);
        return 0;
    }

    __shell_lock(&(__shell_shm->shm_lock));
    int val = vsnprintf(__shell_shm->data, sizeof(__shell_shm->data), format, ap);
    __shell_shm->cmd = SHELL_PUTS;
    va_end(ap);
    __SHELL_WAIT();
    __shell_unlock(&(__shell_shm->shm_lock));

    return val;
}
