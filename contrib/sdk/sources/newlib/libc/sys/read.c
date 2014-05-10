/* read.c -- read bytes from a input device.
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
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <alloca.h>
#include <sys/kos_io.h>
#include "glue.h"
#include "io.h"

#undef erro
extern int errno;

ssize_t read(int fd, void *buf, size_t cnt)
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

    __io_handle *ioh;

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

    if( !(iomode_flags & _READ) )
    {
        errno = EACCES;
        return( -1 );
    }

    if( iomode_flags & _BINARY )   /* if binary mode */
    {
        err = read_file(ioh->name, buffer, ioh->offset, cnt, &amount_read);
        ioh->offset+= amount_read;
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
            err=read_file(ioh->name,buffer, ioh->offset, cnt, &amount_read);
            ioh->offset+=amount_read;

            if( amount_read == 0 )
                break;                   /* EOF */

            reduce_idx = 0;
            finish_idx = reduce_idx;
            for( ; reduce_idx < amount_read; ++reduce_idx )
            {
                if( buffer[ reduce_idx ] == 0x1a )     /* EOF */
                {
                    lseek(fd, ((long)reduce_idx - (long)amount_read)+1L, SEEK_CUR );
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
                break;
            }
        } while( read_len != 0 );
    }
    return( total_len );
}
