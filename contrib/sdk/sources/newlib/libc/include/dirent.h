/*
 * Copyright (C) KolibriOS team 2024. All rights reserved.
 * Distributed under terms of the GNU General Public License
*/

#ifndef _DIRENT_H_
#define _DIRENT_H_

#include <sys/types.h>
#include <sys/cdefs.h>
#include <sys/ksys.h>

_BEGIN_STD_C

#define DT_UNKNOWN      0
#define DT_DIR          4
#define DT_REG          8

struct dirent {
    ino_t    d_ino;
    unsigned d_type;
    char     d_name[KSYS_FNAME_UTF8_SIZE];
};

typedef struct {
    char*           path;
    uint32_t        pos;
    struct dirent   last_entry;
} DIR;

DIR* _DEFUN(opendir, (name), const char *name);
int  _DEFUN(closedir, (dirp), register DIR *dirp);
void _DEFUN(seekdir, (dirp, loc), DIR *dirp _AND long loc);
void _DEFUN(rewinddir, (dirp), DIR *dirp);
long _DEFUN(telldir, (dirp), DIR *dirp);

struct dirent *_DEFUN(readdir, (dirp), register DIR *dirp);

_END_STD_C

#endif /* _DIRENT_H_ */
