/*
 * Copyright (C) KolibriOS team 2024. All rights reserved.
 * Distributed under terms of the GNU General Public License
*/

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ksys.h>

struct dirent *
_DEFUN(readdir, (dirp),
       register DIR *dirp)
{
    if (!dirp || !dirp->path)
    {
        errno = EBADF;
        return NULL;
    }

    ksys_dir_entry_t *entry = calloc(sizeof(ksys_dir_entry_t) +
                                     KSYS_FNAME_UTF8_SIZE, 1);

    int status = _ksys_file_read_dir(dirp->path, dirp->pos,
                                     KSYS_FILE_UTF8, 1, entry).status;
    if (status == KSYS_FS_ERR_EOF)
        return NULL;

    if (status)
    {
        errno = ENOENT;
        return NULL;
    }

    dirp->last_entry.d_ino = dirp->pos;
    strncpy(dirp->last_entry.d_name, entry->info.name, KSYS_FNAME_UTF8_SIZE-1);

    if (entry->info.attr & (KSYS_FILE_ATTR_DIR | KSYS_FILE_ATTR_VOL_LABEL))
        dirp->last_entry.d_type = DT_DIR;
    else
        dirp->last_entry.d_type = DT_REG;

    dirp->pos++;

    return &dirp->last_entry;
}
