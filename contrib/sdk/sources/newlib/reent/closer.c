/* Reentrant version of close system call.  */

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


/*
FUNCTION
	<<_close_r>>---Reentrant version of close

INDEX
	_close_r

ANSI_SYNOPSIS
	#include <reent.h>
	int _close_r(struct _reent *<[ptr]>, int <[fd]>);

TRAD_SYNOPSIS
	#include <reent.h>
	int _close_r(<[ptr]>, <[fd]>)
	struct _reent *<[ptr]>;
	int <[fd]>;

DESCRIPTION
	This is a reentrant version of <<close>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.
*/
extern unsigned  __NFiles;


#define __handle_check( __h, __r )                \
        if( (__h) < 0  ||  (__h) > __NFiles ) {   \
           ptr->_errno =  EBADF ;                 \
           return( __r );                         \
        }




int
_DEFUN(_close_r, (ptr, fd),
     struct _reent *ptr _AND
     int fd)
{
    int ret;
    int h;

    __file_handle *fh;

    __handle_check( fd, -1 );

    fh = (__file_handle*) __getOSHandle( fd );

    if( fd > STDERR_FILENO )
    {
        _free_r(ptr, fh->name);
        _free_r(ptr, fh);
        __freePOSIXHandle( fd );
        __SetIOMode_nogrow( fd, 0 );
    }

    return 0;
}


int
_DEFUN( close,(fd),
     int fd)
{
    return _close_r(_REENT, fd);
}

#endif /* ! defined (REENTRANT_SYSCALLS_PROVIDED) */
