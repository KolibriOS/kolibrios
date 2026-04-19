#include <sys/ksys.h>
#include <time.h>

time_t time(time_t* timer)
{
    static struct tm __buffertime;
    int kos_date, kos_time;
    kos_date = _ksys_get_date().val;
    kos_time = _ksys_get_time().val;

    int bcd_day = (kos_date >> 16);
    int bcd_mon = ((kos_date & 0xFF00) >> 8);
    int bcd_year = (kos_date & 0xFF);
    __buffertime.tm_mday = ((bcd_day & 0xF0) >> 4) * 10 + (bcd_day & 0x0F);
    __buffertime.tm_mon = ((bcd_mon & 0xF0) >> 4) * 10 + (bcd_mon & 0x0F) - 1;
    __buffertime.tm_year = ((bcd_year & 0xF0) >> 4) * 10 + (bcd_year & 0x0F) + 100;

    __buffertime.tm_wday = __buffertime.tm_yday = __buffertime.tm_isdst = -1; /* temporary */

    int bcd_sec = (kos_time >> 16);
    int bcd_min = ((kos_time & 0xFF00) >> 8);
    int bcd_hour = (kos_time & 0xFF);

    __buffertime.tm_sec = ((bcd_sec & 0xF0) >> 4) * 10 + (bcd_sec & 0x0F);
    __buffertime.tm_min = ((bcd_min & 0xF0) >> 4) * 10 + (bcd_min & 0x0F);
    __buffertime.tm_hour = ((bcd_hour & 0xF0) >> 4) * 10 + (bcd_hour & 0x0F);

    time_t ret = mktime(&__buffertime);
    if (timer) {
        *timer = ret;
    }
    return ret;
}
