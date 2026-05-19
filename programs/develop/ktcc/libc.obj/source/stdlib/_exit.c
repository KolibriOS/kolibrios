#include <conio.h>
#include <sys/ksys.h>

void _exit(int status)
{
    if (__con_is_load) {
        con_exit(0);
    }

    _ksys_exit();
}
