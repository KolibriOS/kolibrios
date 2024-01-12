/*
 * Copyright (C) KolibriOS team 2024. All rights reserved.
 * Distributed under terms of the GNU General Public License
*/

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>

int
_DEFUN(closedir, (dirp),
       register DIR *dirp)
{
    if (!dirp)
    {
        errno = EBADF;
        return -1;
    }

    if (dirp->path)
        free(dirp->path);

    free(dirp);
    return 0;
}
