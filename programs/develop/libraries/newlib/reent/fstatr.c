/* Reentrant versions of fstat system call.  This implementation just
   calls the fstat system call.  */

#include <reent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <_syslist.h>
#include <errno.h>
#include <string.h>


/* Some targets provides their own versions of these functions.  Those
   targets should define REENTRANT_SYSCALLS_PROVIDED in TARGET_CFLAGS.  */

#ifdef _REENT_ONLY
#ifndef REENTRANT_SYSCALLS_PROVIDED
#define REENTRANT_SYSCALLS_PROVIDED
#endif
#endif

#ifdef REENTRANT_SYSCALLS_PROVIDED

int _dummy_fstat_syscalls = 1;

#else

/* We use the errno variable used by the system dependent layer.  */

/*
FUNCTION
	<<_fstat_r>>---Reentrant version of fstat

INDEX
	_fstat_r

ANSI_SYNOPSIS
	#include <reent.h>
	int _fstat_r(struct _reent *<[ptr]>,
		     int <[fd]>, struct stat *<[pstat]>);

TRAD_SYNOPSIS
	#include <reent.h>
	int _fstat_r(<[ptr]>, <[fd]>, <[pstat]>)
	struct _reent *<[ptr]>;
	int <[fd]>;
	struct stat *<[pstat]>;

DESCRIPTION
	This is a reentrant version of <<fstat>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.
*/

#pragma pack(push, 1)
typedef struct
{
  char sec;
  char min;
  char hour;
  char rsv;
}detime_t;

typedef struct
{
  char  day;
  char  month;
  short year;
}dedate_t;

typedef struct
{
  unsigned    attr;
  unsigned    flags;
  union
  {
     detime_t  ctime;
     unsigned  cr_time;
  };
  union
  {
     dedate_t  cdate;
     unsigned  cr_date;
  };
  union
  {
     detime_t  atime;
     unsigned  acc_time;
  };
  union
  {
     dedate_t  adate;
     unsigned  acc_date;
  };
  union
  {
     detime_t  mtime;
     unsigned  mod_time;
  };
  union
  {
     dedate_t  mdate;
     unsigned  mod_date;
  };
  unsigned    size;
  unsigned    size_high;
} FILEINFO;

#pragma pack(pop)

extern unsigned  __NFiles;

#define __handle_check( __h, __r )                \
        if( (__h) < 0  ||  (__h) > __NFiles ) {   \
           ptr->_errno =  EBADF ;                 \
           return( __r );                         \
        }

int
_fstat_r (ptr, fd, pstat)
     struct _reent *ptr;
     int fd;
     struct stat *pstat;
{
    FILEINFO info;
    int ret;

    __file_handle *fh;

    __handle_check( fd, -1 );

    if (fd < 3)
    {
      pstat->st_mode = S_IFCHR;
      pstat->st_blksize = 0;
      return 0;
    }

    fh = (__file_handle*) __getOSHandle( fd );
    get_fileinfo(fh->name, &info);

    memset (pstat, 0, sizeof (* pstat));
    pstat->st_mode = S_IFREG;
    pstat->st_blksize = 4096;
    pstat->st_size = info.size;
    return 0;
}

int
_DEFUN (fstat, (fd, pstat),
     int fd _AND
     struct stat *pstat)
{
  return _fstat_r (_REENT, fd, pstat);
}

#endif /* ! defined (REENTRANT_SYSCALLS_PROVIDED) */
