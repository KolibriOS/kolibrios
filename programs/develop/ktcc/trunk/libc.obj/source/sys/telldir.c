/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#include <sys/dirent.h>

unsigned telldir(DIR *dir)
{
    if(dir!=NULL){
        return dir->pos;
    }else{
        return 0;
    }
}