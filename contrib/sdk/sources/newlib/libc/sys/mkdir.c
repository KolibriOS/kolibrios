/* mkdir.c -- create a directory.
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
#include <sys/stat.h>
#include <sys/ksys.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include "glue.h"

/*
 * Create a directory. The KolibriOS create-folder syscall (function 70,
 * subfunction 9) rejects a path with a trailing '/', which the mkdir -p
 * pattern commonly produces (e.g. "/foo/"), so it is stripped here. The
 * permission mode is not meaningful on KolibriOS and is ignored.
 */
int
_DEFUN (mkdir, (path, mode),
        const char *path _AND
        mode_t mode)
{
    const char *target = path;
    char buf[PATH_MAX];
    size_t len = strlen(path);
    int err;

    (void)mode;

    if (len > 1 && path[len - 1] == '/')
    {
        if (len >= sizeof(buf))
            len = sizeof(buf) - 1;
        memcpy(buf, path, len - 1);
        buf[len - 1] = '\0';
        target = buf;
    }

    err = _ksys_mkdir(target);

    if (!err)
        return 0;

    if (err == 5)
        errno = ENOENT;
    else
        errno = EIO;

    return (-1);
}
