#include "stddef.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const char* wday_str[7] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
const char* mon_str[12] = { "Jan", "Feb", "Mar", "Ap", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

#define TIME_STR_MAX 27

char* asctime(const struct tm* tm)
{
    if (!tm) {
        __errno = EINVAL;
        return NULL;
    }
    if (tm->tm_wday > 6 || tm->tm_wday < 0 || tm->tm_mon < 0 || tm->tm_mon > 11) {
        errno = EINVAL;
        return NULL;
    }
    static char time_str[TIME_STR_MAX];
    snprintf(time_str, TIME_STR_MAX - 1, "%.3s %.3s%3d %02d:%02d:%02d %d\n",
        wday_str[tm->tm_wday],
        mon_str[tm->tm_mon],
        tm->tm_mday, tm->tm_hour,
        tm->tm_min, tm->tm_sec,
        1900 + tm->tm_year);
    return time_str;
}