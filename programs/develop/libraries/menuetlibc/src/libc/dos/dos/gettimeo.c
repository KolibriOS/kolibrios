#include <time.h>
#include <assert.h>

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
 struct tm tmblk;
 struct timeval tv_tmp;
 unsigned long xtmp;
 if (!tv) tv = &tv_tmp;
 tv->tv_usec=0;
 xtmp=__menuet__getsystemclock();
 tmblk.tm_sec = (xtmp>>16)&0xff;
 tmblk.tm_min = (xtmp>>8)&0xff;
 tmblk.tm_hour = xtmp&0xff;
 BCD_TO_BIN(tmblk.tm_sec);
 BCD_TO_BIN(tmblk.tm_min);
 BCD_TO_BIN(tmblk.tm_hour);
 __asm__ __volatile__("int $0x40":"=a"(xtmp):"0"(29)); 
 tmblk.tm_mday = (xtmp>>16)&0xff;
 tmblk.tm_mon = ((xtmp>>8)&0xff)-1;
 tmblk.tm_year = ((xtmp&0xff)+2000)-1900;
 tmblk.tm_wday = tmblk.tm_yday = tmblk.tm_gmtoff = 0;
 tmblk.tm_zone = 0;
 tmblk.tm_isdst = -1;
 tv->tv_sec = mktime(&tmblk);
 if(tz)
 {
  struct tm *tmloc = localtime(&(tv->tv_sec));
  tz->tz_minuteswest = - tmloc->tm_gmtoff / 60;
  tz->tz_dsttime = tmloc->tm_isdst;
 }
 return 0;
}
