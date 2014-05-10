/* open.c -- open a file.
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
#include <sys/kos_io.h>
#include "glue.h"
#include "io.h"

#undef erro
extern int errno;

int open (const char * filename, int flags, ...)
{
    __io_handle *ioh;
    fileinfo_t   info;
    int iomode, rwmode, offset;
    int hid;
    int err;

    hid = __io_alloc();
    if(hid < 0)
    {
        errno = EMFILE;
        return (-1);
    };

//    path = getfullpath(name);

    err = get_fileinfo(filename, &info);

    if( flags & O_EXCL &&
        flags & O_CREAT )
    {
        if( !err )
        {
            errno = EEXIST;
            return (-1);
        };
    }

    if( err )
    {
        if(flags & O_CREAT)
            err=create_file(filename);
        if( err )
        {
            errno = EACCES;
            return -1;
        };
    };

    if( flags & O_TRUNC )
        set_file_size(filename, 0);

    ioh = &__io_tab[hid];

    rwmode = flags & ( O_RDONLY | O_WRONLY | O_RDWR );

    iomode = 0;
    offset = 0;

    if( rwmode == O_RDWR )
        iomode |= _READ | _WRITE;
    else if( rwmode == O_RDONLY)
        iomode |= _READ;
    else if( rwmode == O_WRONLY)
        iomode |= _WRITE;

    if( flags & O_APPEND )
    {
        iomode |= _APPEND;
        offset = info.size;
    };

    if( flags & (O_BINARY|O_TEXT) )
    {
        if( flags & O_BINARY )
            iomode |= _BINARY;
    } else
        iomode |= _BINARY;

    ioh->name   = strdup(filename);
    ioh->offset = offset;
    ioh->mode   = iomode;
    ioh->read   = read_file;
    ioh->write  = write_file;

    return hid;
};


