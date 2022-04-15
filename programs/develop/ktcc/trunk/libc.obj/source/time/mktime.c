#include <time.h>

time_t mktime(struct tm* timeptr)
{
    //  int utcdiff = -3;
    const int mon_days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    unsigned long int tyears, tdays, leaps, utc_hrs;
    int i;

    tyears = timeptr->tm_year - 70; // tm->tm_year is from 1900.
    leaps = (tyears + 2) / 4; // no of next two lines until year 2100.
    i = (timeptr->tm_year - 100) / 100;
    leaps -= ((i / 4) * 3 + i % 4);
    tdays = 0;
    for (i = 0; i < timeptr->tm_mon; i++)
        tdays += mon_days[i];

    tdays += timeptr->tm_mday - 1; // days of month passed.
    tdays = tdays + (tyears * 365) + leaps;

    //  utc_hrs = timeptr->tm_hour + utcdiff; // for your time zone.
    return (tdays * 86400) + (timeptr->tm_hour * 3600) + (timeptr->tm_min * 60) + timeptr->tm_sec;
}