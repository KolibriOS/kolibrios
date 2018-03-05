#include <time.h>
#include <kolibrisys.h>

struct tm __buffertime;



struct tm * localtime (const time_t * timer)
/* non-standard!  ignore parameter and return just time now */
{
	int kos_date, kos_time;
	kos_date = _ksys_get_date();
	kos_time = _ksys_get_system_clock();
	
	__buffertime.tm_mday = kos_date >> 16;
	__buffertime.tm_mon = ((kos_date & 0xFF00) >> 8) -1;
	__buffertime.tm_year = kos_date >> 16 + 100;
	
	__buffertime.tm_wday = __buffertime.tm_yday = __buffertime.tm_isdst = -1; /* temporary */
	
	__buffertime.tm_sec = kos_time >> 16;
	__buffertime.tm_min = (kos_time & 0xFF00) >> 8;
	__buffertime.tm_hour = kos_time & 0xFF;

	return &__buffertime;
}

time_t time (time_t* timer)
{
	time_t t = mktime(localtime(0));
	
	if (timer) *timer = t;
	
	return t;
}

time_t mktime (struct tm * timeptr)
{
	int y, m, d;
	time_t 	t;
	y = timeptr->tm_year + 1900;
	m = timeptr->tm_mon + 1;
	d = timeptr->tm_mday; /* to -1 or not to -1? */
	
	if (m < 3) { m += 12; y -= 1; }
	
	t = y * 365 + y / 4 + y /400 - y / 100; /* years - > days */
	t += 30 * m + 3 * (m + 1) / 5 + d;		/* add month days */
	
	t -= 719561;  /* 01 jan 1970 */
	t *= 86400;	
 
	t += 3600 * timeptr->tm_hour + 60 * timeptr->tm_min + timeptr->tm_sec;

	return t;
}

double difftime (time_t end, time_t beginning)
{
	return end - beginning;
}
