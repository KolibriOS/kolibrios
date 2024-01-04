/*
 * Copyright (C) KolibriOS team 2004-2024. All rights reserved.
 * Distributed under terms of the GNU General Public License
*/

#include <stdlib.h>
#include <sys/ksys.h>

char *getcwd(char *buf, unsigned size){
    if(!buf){
       if((buf = malloc(size))==NULL){
           return NULL;
       }
    }
    _ksys_getcwd(buf, size);
    return(buf);
}
