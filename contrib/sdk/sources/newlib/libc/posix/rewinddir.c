/*
 * Copyright (C) KolibriOS team 2024. All rights reserved.
 * Distributed under terms of the GNU General Public License
*/

#include <dirent.h>

void
_DEFUN(rewinddir, (dirp),
    DIR *dirp)
{
    dirp->pos = 0;
}
