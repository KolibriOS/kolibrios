/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#include <sys/dirent.h>
#include <stdlib.h>

struct dirent* readdir(DIR *dir)
{
    if(dir->num_objs>dir->pos){
        dir->pos++;
        return &dir->objs[dir->pos-1];
    }else{
        return NULL;
    }
}
