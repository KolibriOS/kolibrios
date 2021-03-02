#include <time.h>
#include <ksys.h>

struct tm buffertime;

struct tm * localtime (const time_t * timer)
/* non-standard!  ignore parameter and return just time now */
{
	int kos_date, kos_time;
    kos_date = _ksys_get_date();
    kos_time = _ksys_get_clock();
   
    int bcd_day = (kos_date >> 16);
    int bcd_mon = ((kos_date & 0xFF00) >> 8);
    int bcd_year = (kos_date & 0xFF);
    buffertime.tm_mday = ((bcd_day & 0xF0)>>4)*10 + (bcd_day & 0x0F);
    buffertime.tm_mon = ((bcd_mon & 0xF0)>>4)*10 + (bcd_mon & 0x0F) - 1;
    buffertime.tm_year = ((bcd_year & 0xF0)>>4)*10 + (bcd_year & 0x0F) + 100;
   
    buffertime.tm_wday = buffertime.tm_yday = buffertime.tm_isdst = -1; /* temporary */
   
    int bcd_sec = (kos_time >> 16);
    int bcd_min = ((kos_time & 0xFF00) >> 8);
    int bcd_hour = (kos_time & 0xFF);

    buffertime.tm_sec = ((bcd_sec & 0xF0)>>4)*10 + (bcd_sec & 0x0F);
    buffertime.tm_min = ((bcd_min & 0xF0)>>4)*10 + (bcd_min & 0x0F);
    buffertime.tm_hour = ((bcd_hour & 0xF0)>>4)*10 + (bcd_hour & 0x0F);

	return &buffertime;
}
 
