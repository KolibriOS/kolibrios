/* _rename.c -- Implementation of the low-level rename() routine
 *
 * Copyright (c) 2004 National Semiconductor Corporation
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
#include <stdio.h>
#include <unistd.h>
#include <alloca.h>
#include <errno.h>

int _rename (char *from, char *to)
{
    void* buf;
    int f_from;
    int f_to;
    int size;

    f_from = open(from,O_RDONLY);

    if (f_from < 0)
    {
        errno = ENOENT;
        return -1;
    };

    f_to = open(to,O_CREAT|O_WRONLY|O_EXCL);

    if (f_to < 0)
    {
        close(f_from);
        errno = EACCES;
        return -1;
    };

    buf = alloca(32768);

    do
    {
        size = read(f_from, buf, 32768);

        if (size >= 0)
            size = write(f_to, buf, size);

    }while (size == 32768);

    close(f_to);
    close(f_from);

    if (size == -1)
    {
        errno = EACCES;
        return -1;
    };

    remove(from);

    return (0);
};
