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

extern int _FUNC(closedir)(DIR* dir);
extern DIR* _FUNC(opendir)(const char* path);
extern struct dirent* _FUNC(readdir)(DIR*);
extern void _FUNC(rewinddir)(DIR* dir);
extern void _FUNC(seekdir)(DIR* dir, unsigned pos);
extern unsigned _FUNC(telldir)(DIR* dir);

#endif // _DIRENT_H_
