/*
 * Copyright (C) KolibriOS team 2024. All rights reserved.
 * Distributed under terms of the GNU General Public License
*/

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ksys.h>

/* TODO: Add thread safety */

DIR *
_DEFUN(opendir, (name),
       const char *name)
{
    ksys_file_info_t info;
    if (_ksys_file_info(name, &info))
    {
        errno = ENOENT;
        return NULL;
    }

    if (!(info.attr & (KSYS_FILE_ATTR_DIR | KSYS_FILE_ATTR_VOL_LABEL)))
    {
        errno = ENOTDIR;
        return NULL;
    }

    DIR* dir = malloc(sizeof(DIR));
    if (!dir)
        return NULL;

    dir->path = strdup(name);
    if (!dir->path)
        return NULL;

    dir->pos = 0;

    return dir;
}
