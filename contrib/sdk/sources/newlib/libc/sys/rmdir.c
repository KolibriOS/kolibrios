/* rmdir.c -- remove a directory.
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
#include <_ansi.h>
#include <sys/ksys.h>
#include <errno.h>

#include "glue.h"

/*
 * Remove a directory. Maps onto the KolibriOS delete syscall (function 70,
 * subfunction 8). Like POSIX rmdir, the directory must already be empty.
 */
int
_DEFUN (rmdir, (path),
        const char *path)
{
    int err = _ksys_rmdir(path);

    if (!err)
        return 0;

    if (err == 5)
        errno = ENOENT;
    else
        errno = EIO;

    return (-1);
}
