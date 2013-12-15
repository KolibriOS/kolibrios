/* Reentrant versions of write system call. */

#include <reent.h>
#include <unistd.h>
#include <_syslist.h>
#include <alloca.h>
#include <errno.h>
#include <string.h>


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
	<<_write_r>>---Reentrant version of write

INDEX
	_write_r

ANSI_SYNOPSIS
	#include <reent.h>
	_ssize_t _write_r(struct _reent *<[ptr]>,
		          int <[fd]>, const void *<[buf]>, size_t <[cnt]>);

TRAD_SYNOPSIS
	#include <reent.h>
	_ssize_t _write_r(<[ptr]>, <[fd]>, <[buf]>, <[cnt]>)
	struct _reent *<[ptr]>;
	int <[fd]>;
	char *<[buf]>;
	size_t <[cnt]>;

DESCRIPTION
	This is a reentrant version of <<write>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.
*/


#define _WRITE  0x0002  /* file opened for writing */
#define _APPEND 0x0080  /* file opened for append */
#define _BINARY 0x0040  /* file is binary, skip CRLF processing */
#define _ISTTY  0x2000  /* is console device */
#define _FILEEXT 0x8000 /* lseek with positive offset has been done */

#define __handle_check( __h, __r )                \
        if( (__h) < 0  ||  (__h) > __NFiles ) {   \
           ptr->_errno =  EBADF;                  \
           return( __r );                         \
        }

extern unsigned  __NFiles;

#define PAD_SIZE 512

static int zero_pad(struct _reent *ptr, int handle )           /* 09-jan-95 */
/*******************************/
{
    int         rc;
    long        curPos, eodPos;
    long        bytesToWrite;
    unsigned    writeAmt;
    char        zeroBuf[PAD_SIZE];

    // Pad with zeros due to lseek() past EOF (POSIX)
    curPos = _lseek_r( ptr, handle, 0L, SEEK_CUR );   /* current offset */
    if( curPos == -1 )
        return( -1 );
    eodPos = _lseek_r( ptr, handle, 0L, SEEK_END );   /* end of data offset */
    if( eodPos == -1 )
        return( -1 );

    if( curPos > eodPos ) {
        bytesToWrite = curPos - eodPos;         /* amount to pad by */

        if( bytesToWrite > 0 ) {                /* only write if needed */
            memset( zeroBuf, 0x00, PAD_SIZE );  /* zero out a buffer */
            do {                                /* loop until done */
                if( bytesToWrite > PAD_SIZE )
                    writeAmt = 512;
                else
                    writeAmt = (unsigned)bytesToWrite;
                rc = _write_r(ptr, handle, zeroBuf, writeAmt );
                if( rc < 0 )
                    return( rc );
                bytesToWrite -= writeAmt;       /* more bytes written */
            } while( bytesToWrite != 0 );
        }
    } else {
        curPos = _lseek_r( ptr, handle, curPos, SEEK_SET );
        if( curPos == -1 ) {
            return( -1 );
        }
    }

    return( 0 );                /* return success code */
}


static int os_write(struct _reent *ptr, int handle,
                    const void *buffer, unsigned len, unsigned *amt )
/********************************************************************************/
{
    __file_handle *fh;
    int         rc;

    rc   = 0;
    *amt = 0;

    fh = (__file_handle*) __getOSHandle( handle );

    rc = fh->write(fh->name,buffer,fh->offset,len,amt);

    fh->offset+= *amt;

    if( *amt != len )
    {
        rc = ENOSPC;
        ptr->_errno = ENOSPC;
    }
    return( rc );
}
_ssize_t
_DEFUN (_write_r, (ptr, fd, buffer, cnt),
     struct _reent *ptr _AND
     int fd _AND
     _CONST _PTR buffer _AND
     size_t cnt)
{
    _ssize_t      ret;
    unsigned int  iomode_flags;
    unsigned      len_written, i, j;
    int           rc2;
    char        *buf;

    __file_handle  *fh;

    __handle_check( fd, -1 );

    iomode_flags = __GetIOMode( fd );
    if( iomode_flags == 0 )
    {
        ptr->_errno = EBADF;
        return( -1 );
    }

    if( !(iomode_flags & _WRITE) ) {
        ptr->_errno = EACCES ;     /* changed from EBADF to EACCES 23-feb-89 */
        return( -1 );
    }

    if( (iomode_flags & _APPEND) && !(iomode_flags & _ISTTY) )
    {
        fh->offset = _lseek_r(ptr, fd, 0L, SEEK_END );   /* end of data offset */
    }

    len_written = 0;
    rc2 = 0;

    // Pad the file with zeros if necessary
    if( iomode_flags & _FILEEXT )
    {
        // turn off file extended flag
        __SetIOMode_nogrow( fd, iomode_flags&(~_FILEEXT) );

        // It is not required to pad a file with zeroes on an NTFS file system;
        // unfortunately it is required on FAT (and probably FAT32). (JBS)
        rc2 = zero_pad( ptr, fd );
    }

    if( rc2 == 0 )
    {
        if( iomode_flags & _BINARY ) {  /* if binary mode */
            rc2 = os_write(ptr, fd, buffer, cnt, &len_written );
            /* end of binary mode part */
        } else {    /* text mode */

            int buf_size = 512;

            buf = (char*)alloca( buf_size );

            j = 0;
            for( i = 0; i < cnt; )
            {
                if( ((const char*)buffer)[i] == '\n' )
                {
                    buf[j] = '\r';
                    ++j;
                    if( j == buf_size )
                    {
                        rc2 = os_write(ptr, fd, buf, buf_size, &j );
                        if( rc2 == -1 )
                            break;
                        len_written += j;
                        if( rc2 == ENOSPC )
                            break;
                        len_written = i;
                        j = 0;
                    }
                }
                buf[j] = ((const char*)buffer)[i];
                ++i;
                ++j;
                if( j == buf_size ) {
                    rc2 = os_write(ptr, fd, buf, buf_size, &j );
                    if( rc2 == -1 )
                        break;
                    len_written += j;
                    if( rc2 == ENOSPC )
                        break;
                    len_written = i;
                    j = 0;
                }
            }
            if( j ) {
                rc2 = os_write(ptr, fd, buf, j, &i );
                if( rc2 == ENOSPC ) {
                    len_written += i;
                } else {
                    len_written = cnt;
                }
            }
            /* end of text mode part */
        }
    }

    if( rc2 == -1 ) {
        return( rc2 );
    } else {
        return( len_written );
    }
}

_ssize_t
_DEFUN (write, ( fd, buffer, cnt),
     int fd _AND
     _CONST _PTR buffer _AND
     size_t cnt)

{

    return _write_r(_REENT, fd, buffer, cnt);

}

#endif /* ! defined (REENTRANT_SYSCALLS_PROVIDED) */
