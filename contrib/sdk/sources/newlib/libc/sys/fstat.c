/* fstat.c -- get status of a file.
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
#include <string.h>
#include <sys/unistd.h>
#include <sys/kos_io.h>
#include <sys/stat.h>
#include "glue.h"
#include "io.h"

int
_DEFUN (fstat, (fd, buf),
       int fd _AND
       struct stat *buf)
{
    fileinfo_t info;
    __io_handle *ioh;

    if( (fd < 0) || (fd >=64) )
    {
        errno = EBADF;
        return (-1);
    };

    memset (buf, 0, sizeof (* buf));

    if (fd <= STDERR_FILENO)
    {
        buf->st_mode = S_IFCHR;
        buf->st_blksize = 0;
    }
    else
    {

        ioh = &__io_tab[fd];
        get_fileinfo(ioh->name, &info);

        buf->st_mode = S_IFREG;
        buf->st_blksize = 4096;
        buf->st_size = info.size;
    };

    return (0);
}



