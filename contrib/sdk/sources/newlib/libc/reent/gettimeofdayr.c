/* Reentrant version of gettimeofday system call
   This implementation just calls the times/gettimeofday system calls.
   Gettimeofday may not be available on all targets.  It's presence
   here is dubious.  Consider it for internal use only.  */

#include <reent.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <_syslist.h>
#include <errno.h>

/* Some targets provides their own versions of these functions.  Those
   targets should define REENTRANT_SYSCALLS_PROVIDED in TARGET_CFLAGS.  */

#ifdef _REENT_ONLY
#ifndef REENTRANT_SYSCALLS_PROVIDED
#define REENTRANT_SYSCALLS_PROVIDED
#endif
#endif

#ifdef REENTRANT_SYSCALLS_PROVIDED

int _dummy_gettimeofday_syscalls = 1;

#else

/* We use the errno variable used by the system dependent layer.  */
#undef errno
static int errno;

/*
FUNCTION
	<<_gettimeofday_r>>---Reentrant version of gettimeofday

INDEX
	_gettimeofday_r

ANSI_SYNOPSIS
	#include <reent.h>
	#include <time.h>
	int _gettimeofday_r(struct _reent *<[ptr]>,
		struct timeval *<[ptimeval]>,
		void *<[ptimezone]>);

TRAD_SYNOPSIS
	#include <reent.h>
	#include <time.h>
	int _gettimeofday_r(<[ptr]>, <[ptimeval]>, <[ptimezone]>)
	struct _reent *<[ptr]>;
	struct timeval *<[ptimeval]>;
	void *<[ptimezone]>;

DESCRIPTION
	This is a reentrant version of <<gettimeofday>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.

	This function is only available for a few targets.
	Check libc.a to see if its available on yours.
*/

int
_DEFUN (_gettimeofday_r, (ptr, ptimeval, ptimezone),
     struct _reent *ptr _AND
     struct timeval *ptimeval _AND
     void *ptimezone)
{
  int ret;

  errno = 0;
  if ((ret = _gettimeofday (ptimeval, ptimezone)) == -1 && errno != 0)
    ptr->_errno = errno;
  return ret;
}

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

int
_gettimeofday (struct timeval *tv, void *tz)
{
    unsigned int xtmp;
    struct   tm tmblk;

    if( tv )
    {
        tv->tv_usec = 0;

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
        tv->tv_sec = mktime(&tmblk);
        return 0;
    }
    else
    {
        errno = EINVAL;
        return -1;
    };
}


#endif /* ! defined (REENTRANT_SYSCALLS_PROVIDED) */
