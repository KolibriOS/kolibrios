/* close.c -- close a file descriptor.
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
#include <stdlib.h>
#include <sys/unistd.h>
#include <sys/kos_io.h>
#include "glue.h"
#include "io.h"

int
_DEFUN (close ,(fd),
       int fd)
{
    __io_handle *ioh;

    if( (fd < 0) || (fd >=64) )
    {
        errno = EBADF;
        return (-1);
    };

    ioh = &__io_tab[fd];

    if( fd > STDERR_FILENO )
    {
        free(ioh->name);
        __io_free(fd);
    }

    return (0);
}
