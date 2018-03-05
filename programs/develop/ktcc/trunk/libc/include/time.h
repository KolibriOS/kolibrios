#ifndef _TIME_H
#define _TIME_H


typedef unsigned long int clock_t;
typedef unsigned long int time_t;
#define clock() get_tick_count()
#define CLOCKS_PER_SEC 100

struct tm {
	int tm_sec;	 /*	seconds after the minute	0-61*/
	int tm_min;	 /* minutes after the hour	0-59 */
	int tm_hour; /*	hours since midnight	0-23 */
	int tm_mday; /* day of the month	1-31 */
	int tm_mon;	 /* months since January	0-11 */
	int tm_year; /* years since 1900 */	
	int tm_wday; /* days since Sunday	0-6		*/
	int tm_yday; /* days since January 1	0-365 	*/
	int tm_isdst; /* Daylight Saving Time flag	*/
};

time_t mktime (struct tm * timeptr);
time_t time (time_t* timer);
struct tm * localtime (const time_t * timer); /* non-standard!  ignore parameter and return just time now, not generate tm_isdst, tm_yday, tm_wday == -1  */
double difftime (time_t end, time_t beginning);

extern struct tm __buffertime;

#endif