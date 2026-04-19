/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#include <sys/dirent.h>

void rewinddir(DIR *dir){
    if(dir!=NULL){
        dir->pos=0;
    }
}