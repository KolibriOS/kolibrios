/* Reentrant versions of lseek system call. */

#include <reent.h>
#include <unistd.h>
#include <_syslist.h>
#include <errno.h>

/* Some targets provides their own versions of this functions.  Those
   targets should define REENTRANT_SYSCALLS_PROVIDED in TARGET_CFLAGS.  */

#ifdef _REENT_ONLY
#ifndef REENTRANT_SYSCALLS_PROVIDED
#define REENTRANT_SYSCALLS_PROVIDED
#endif
#endif

#ifndef REENTRANT_SYSCALLS_PROVIDED

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


/*
FUNCTION
	<<_lseek_r>>---Reentrant version of lseek

INDEX
	_lseek_r

ANSI_SYNOPSIS
	#include <reent.h>
	off_t _lseek_r(struct _reent *<[ptr]>,
		       int <[fd]>, off_t <[pos]>, int <[whence]>);

TRAD_SYNOPSIS
	#include <reent.h>
	off_t _lseek_r(<[ptr]>, <[fd]>, <[pos]>, <[whence]>)
	struct _reent *<[ptr]>;
	int <[fd]>;
	off_t <[pos]>;
	int <[whence]>;

DESCRIPTION
	This is a reentrant version of <<lseek>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.
*/

_off_t
_DEFUN (_lseek_r, (ptr, fd, pos, whence),
     struct _reent *ptr _AND
     int fd _AND
     _off_t pos _AND
     int whence)
{
    _off_t ret;
    __file_handle *fh;

    __handle_check( fd, -1 );
    fh = (__file_handle*) __getOSHandle( fd );

    switch(whence)
    {
        case SEEK_SET:
            ret = pos;
            break;
        case SEEK_CUR:
            ret = fh->offset + pos;
            break;
        case SEEK_END:
        {
            FILEINFO info;
            get_fileinfo(fh->name, &info);
            ret = pos + info.size;
            break;
        }
        default:
            ptr->_errno = EINVAL;
            return -1;
    };

    fh->offset = ret;

    return( ret );
}

_off_t
_DEFUN (lseek, (fd, pos, whence),
     int fd _AND
     _off_t pos _AND
     int whence)

{
    return _lseek_r(_REENT, fd, pos, whence);
};

#endif /* ! defined (REENTRANT_SYSCALLS_PROVIDED) */
