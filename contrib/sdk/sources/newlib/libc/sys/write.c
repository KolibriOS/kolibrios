/* write.c -- write bytes to an output device.
 *
 * Copyright (c) 1995 Cygnus Support
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */
#include <errno.h>
#include <unistd.h>
#include <alloca.h>
#include "io.h"

#undef erro
extern int errno;


#if 0
#define PAD_SIZE 512

static int zero_pad(int handle )           /* 09-jan-95 */
/*******************************/
{
    int         rc;
    long        curPos, eodPos;
    long        bytesToWrite;
    unsigned    writeAmt;
    char        zeroBuf[PAD_SIZE];

    // Pad with zeros due to lseek() past EOF (POSIX)
    curPos = lseek( handle, 0L, SEEK_CUR );   /* current offset */
    if( curPos == -1 )
        return( -1 );
    eodPos = lseek( handle, 0L, SEEK_END );   /* end of data offset */
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
#endif

static int os_write(int handle, const void *buffer, unsigned len, unsigned *amt )
{
    __io_handle *ioh;
    int          rc;

    rc   = 0;
    *amt = 0;

    ioh = &__io_tab[handle];

    rc = ioh->write(ioh->name,(const void*)buffer,ioh->offset,len,amt);

    ioh->offset+= *amt;

    if( *amt && *amt != len )
    {
        rc = ENOSPC;
        errno = rc;
    }

    return( rc );
}

/*
 * write -- write bytes to the serial port. Ignore fd, since
 *          stdout and stderr are the same. Since we have no filesystem,
 *          open will only return an error.
 */
ssize_t write(int fd, const void *buffer, size_t cnt)
{
    _ssize_t      ret;
    unsigned int  iomode_flags;
    unsigned      len_written, i, j;
    int           rc2;
    char         *buf;

    __io_handle  *ioh;

    if( (fd < 0) || (fd >=64) )
    {
        errno = EBADF;
        return -1;
    };

    ioh = &__io_tab[fd];

    iomode_flags = ioh->mode;

    if( iomode_flags == 0 )
    {
        errno = EBADF;
        return( -1 );
    }

    if( !(iomode_flags & _WRITE) )
    {
        errno = EACCES ;
        return( -1 );
    }

    if( (iomode_flags & _APPEND) && !(iomode_flags & _ISTTY) )
    {
        ioh->offset = lseek(fd, 0L, SEEK_END );   /* end of data offset */
    }

    len_written = 0;
    rc2 = 0;

    // Pad the file with zeros if necessary
    if( iomode_flags & _FILEEXT )
    {
        // turn off file extended flag
        ioh->mode = iomode_flags & (~_FILEEXT);

        // It is not required to pad a file with zeroes on an NTFS file system;
        // unfortunately it is required on FAT (and probably FAT32). (JBS)
//        rc2 = zero_pad( ptr, fd );
    }

    if( rc2 == 0 )
    {
        if( iomode_flags & _BINARY ) {  /* if binary mode */
            rc2 = os_write(fd, buffer, cnt, &len_written );
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
                        rc2 = os_write(fd, buf, buf_size, &j );
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
                    rc2 = os_write(fd, buf, buf_size, &j );
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
                rc2 = os_write(fd, buf, j, &i );
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

  return 0;
}

