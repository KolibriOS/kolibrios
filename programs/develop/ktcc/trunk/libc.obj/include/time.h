#ifndef _TIME_H_
#define _TIME_H_

#include <sys/ksys.h>

typedef unsigned long int clock_t;
typedef unsigned long int time_t;
#define clock()        _ksys_get_clock()
#define CLOCKS_PER_SEC 100

#pragma pack(push, 1)
struct tm {
    int tm_sec;   /* seconds after the minute	0-61*/
    int tm_min;   /* minutes after the hour	0-59 */
    int tm_hour;  /* hours since midnight	0-23 */
    int tm_mday;  /* day of the month	1-31 */
    int tm_mon;   /* months since January	0-11 */
    int tm_year;  /* years since 1900 */
    int tm_wday;  /* days since Sunday	0-6		*/
    int tm_yday;  /* days since January 1	0-365 	*/
    int tm_isdst; /* Daylight Saving Time flag	*/
};
#pragma pack(pop)

DLLAPI time_t mktime(struct tm* timeptr);
DLLAPI time_t time(time_t* timer);
DLLAPI struct tm* localtime(const time_t* timer);
DLLAPI double difftime(time_t end, time_t beginning);
DLLAPI char* asctime(const struct tm* tm);

#endif