#include <shell_api.h>
#include <sys/ksys.h>
#include <sys/ksys.h>

#define SHELL_PING_TIMEOUT 10 // 0.1 sec
#define SHELL_PING_MIN_DELAY 1

int shell_ping()
{
    *__shell_shm = SHELL_PING;

    _ksys_thread_yield(); // hope shell is fast enough

    size_t i = 0;
    while (*__shell_shm != SHELL_OK)
    {
        if (i > (SHELL_PING_TIMEOUT / SHELL_PING_MIN_DELAY))
        {
            return 0;
        }
        i++;
        _ksys_delay(SHELL_PING_MIN_DELAY);
    }

    return 1;
}
