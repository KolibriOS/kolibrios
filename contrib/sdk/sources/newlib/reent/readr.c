/* Reentrant versions of read system call. */

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

/* We use the errno variable used by the system dependent layer.  */

/*
FUNCTION
	<<_read_r>>---Reentrant version of read

INDEX
	_read_r

ANSI_SYNOPSIS
	#include <reent.h>
	_ssize_t _read_r(struct _reent *<[ptr]>,
		         int <[fd]>, void *<[buf]>, size_t <[cnt]>);

TRAD_SYNOPSIS
	#include <reent.h>
	_ssize_t _read_r(<[ptr]>, <[fd]>, <[buf]>, <[cnt]>)
	struct _reent *<[ptr]>;
	int <[fd]>;
	char *<[buf]>;
	size_t <[cnt]>;

DESCRIPTION
	This is a reentrant version of <<read>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.
*/


extern unsigned  __NFiles;

#define _READ   0x0001  /* file opened for reading */
#define _BINARY 0x0040  /* file is binary, skip CRLF processing */
#define _ISTTY  0x2000  /* is console device */

#define __handle_check( __h, __r )                \
        if( (__h) < 0  ||  (__h) > __NFiles ) {   \
           ptr->_errno =  EBADF;                  \
           return( __r );                         \
        }

_ssize_t
_DEFUN (_read, (fd, buf, cnt),
     int fd _AND
     _PTR buf _AND
     size_t cnt)
{

    return _read_r( _REENT, fd, buf, cnt);
}

_ssize_t
_DEFUN (_read_r, (ptr, fd, buf, cnt),
     struct _reent *ptr _AND
     int fd _AND
     _PTR buf _AND
     size_t cnt)
{
    _ssize_t ret;

    _ssize_t  read_len, total_len;
    unsigned  reduce_idx, finish_idx;
    unsigned  iomode_flags;
    char     *buffer = buf;
    int       rc;
    int       h;
    unsigned amount_read;
    int err;

    __file_handle *fh;

    __handle_check( fd, -1 );
    __ChkTTYIOMode( fd );
    iomode_flags = __GetIOMode( fd );
    if( iomode_flags == 0 )
    {
        ptr->_errno = EBADF;
        return( -1 );
    }
    if( !(iomode_flags & _READ) )
    {
        ptr->_errno = EACCES;      /* changed from EBADF to EACCES 23-feb-89 */
        return( -1 );
    }

    fh = (__file_handle*) __getOSHandle( fd );

    if( iomode_flags & _BINARY )   /* if binary mode */
    {
        err = read_file(fh->name, buffer, fh->offset, cnt, &amount_read);
        fh->offset+= amount_read;
        total_len  = amount_read;

        if(err)
            if ( amount_read == 0)
                return (-1);
    }
    else
    {
        total_len = 0;
        read_len = cnt;
        do
        {
            err=read_file(fh->name,buffer, fh->offset, cnt, &amount_read);
            fh->offset+=amount_read;

            if( amount_read == 0 )
                break;                   /* EOF */

            reduce_idx = 0;
            finish_idx = reduce_idx;
            for( ; reduce_idx < amount_read; ++reduce_idx )
            {
                if( buffer[ reduce_idx ] == 0x1a )     /* EOF */
                {
                    _lseek_r(ptr, fd, ((long)reduce_idx - (long)amount_read)+1L,
                           SEEK_CUR );
                    total_len += finish_idx;
                    return( total_len );
                }
                if( buffer[ reduce_idx ] != '\r' )
                {
                    buffer[ finish_idx++ ] = buffer[ reduce_idx ];
                };
            }

            total_len += finish_idx;
            buffer += finish_idx;
            read_len -= finish_idx;
            if( iomode_flags & _ISTTY )
            {
                break;  /* 04-feb-88, FWC */
            }
        } while( read_len != 0 );
    }
    return( total_len );
}


_ssize_t
_DEFUN (read, (fd, buf, cnt),
     int fd _AND
     _PTR buf _AND
     size_t cnt)
{

    return _read_r(_REENT, fd, buf, cnt);
};

#endif /* ! defined (REENTRANT_SYSCALLS_PROVIDED) */
