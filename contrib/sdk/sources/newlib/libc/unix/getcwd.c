#ifndef _NO_GETCWD

/*
 * Copyright (C) KolibriOS team 2024. All rights reserved.
 * Distributed under terms of the GNU General Public License
*/

#include <stdlib.h>
#include <limits.h>

#include <sys/errno.h>
#include <sys/unistd.h>
#include <sys/ksys.h>

#ifndef _REENT_ONLY

char *
_DEFUN (getcwd, (buf, size),
    char *buf _AND
    size_t size)
{
    if (buf != NULL && size == 0)
    {
        errno = EINVAL;
        return NULL;
    }

    if (buf == NULL)
    {
        if (size == 0)
            size = PATH_MAX;

        buf = malloc(size);
        if (buf == NULL)
        {
            errno = ENOMEM;
            return NULL;
        }
    }

    _ksys_getcwd(buf, size);

    if (access(buf, R_OK) == -1)
    {
        errno = EACCES;
        return NULL;
    }

    return buf;
}

#endif /* _REENT_ONLY */
#endif /* !_NO_GETCWD  */
