#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

const char *wday_str[7]={"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const char *mon_str[12]={"Jan", "Feb", "Mar", "Ap", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

#pragma GCC push_options
#pragma GCC optimize("O0")

char *asctime(const struct tm *tm){
	static char time_str[30];
    if(tm->tm_wday>7 || tm->tm_wday<1 || tm->tm_mon<1 || tm->tm_mon>12){
        errno = EINVAL;
        return NULL;
    }
    snprintf(time_str, 26, "%.3s %.3s%3d %2d:%2d:%2d %d\n", 
        wday_str[tm->tm_wday-1], 
        mon_str[tm->tm_mon-1], 
		tm->tm_mday, tm->tm_hour, 
		tm->tm_min, tm->tm_sec, 
		1900 + tm->tm_year
    );
    return time_str;
}
#pragma GCC pop_options
