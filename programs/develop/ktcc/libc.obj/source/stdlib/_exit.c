#include <sys/ksys.h>
#include "../sys/_conio.h"

void _exit(int status)
{
    console_exit();

    _ksys_exit();
}
