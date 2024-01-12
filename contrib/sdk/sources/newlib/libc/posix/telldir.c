/*
 * Copyright (C) KolibriOS team 2024. All rights reserved.
 * Distributed under terms of the GNU General Public License
*/

#include <dirent.h>

long
_DEFUN(telldir, (dirp),
       DIR *dirp)
{
    return dirp->pos;
}
