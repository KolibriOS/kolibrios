/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#ifndef _DIRENT_H_
#define _DIRENT_H_

#include <limits.h>
#include <stddef.h>

#define IS_FOLDER 16
#define IS_FILE   0

typedef unsigned ino_t;

struct dirent {
    ino_t d_ino;           //File serial number.
    char d_name[PATH_MAX]; // Name of entry.
    unsigned d_type;
};

typedef struct {
    struct dirent* objs;
    ino_t pos;
    ino_t num_objs;
} DIR;

DLLAPI int closedir(DIR* dir);
DLLAPI DIR* opendir(const char* path);
DLLAPI struct dirent* readdir(DIR*);
DLLAPI void rewinddir(DIR* dir);
DLLAPI void seekdir(DIR* dir, unsigned pos);
DLLAPI unsigned telldir(DIR* dir);

#endif // _DIRENT_H_
