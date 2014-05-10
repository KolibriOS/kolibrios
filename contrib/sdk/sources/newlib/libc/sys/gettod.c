#include <_ansi.h>
#include <reent.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

int
_DEFUN (gettimeofday, (ptimeval, ptimezone),
     struct timeval *ptimeval _AND
     void *ptimezone)
{
    unsigned int xtmp;
    struct   tm tmblk;

    if( ptimeval )
    {
        ptimeval->tv_usec = 0;

         __asm__ __volatile__("int $0x40":"=a"(xtmp):"0"(3));
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
        tmblk.tm_wday = tmblk.tm_yday = 0;
        tmblk.tm_isdst = -1;
        ptimeval->tv_sec = mktime(&tmblk);
        return 0;
    }
    else
    {
        errno = EINVAL;
        return -1;
    };

}

