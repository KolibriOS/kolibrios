/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#include <sys/dirent.h>

void seekdir(DIR *dir, unsigned pos)
{
    if(dir==NULL || pos>dir->num_objs){
        return;
    }
    dir->pos=pos;
    return;
}