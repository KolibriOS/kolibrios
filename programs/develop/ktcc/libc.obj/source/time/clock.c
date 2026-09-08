#include <time.h>
#include <sys/ksys.h>

clock_t clock(void)
{
    return _ksys_get_tick_count();
}
