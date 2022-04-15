/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#include <sys/dirent.h>
#include <sys/ksys.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_DIR_ERR() if(_ksys70(&inf).status){ \
                            free((void*)inf.p16); \
                            errno = ENOTDIR; \
                            return NULL; \
                        } 

DIR* opendir(const char* path)
{
    DIR* list = malloc(sizeof(DIR));
    if(list==NULL){
        errno = ENOMEM;
        return NULL;
    }

    list->pos=0;
    unsigned num_of_file=0;
    ksys70_t inf;

    inf.p00 = 1;
    inf.p04 = 0;
    inf.p12 = 2;
    inf.p16 = (unsigned) malloc(32+inf.p12*560);
    inf.p20 = 0;
    inf.p21 = (char*)path;
   
    CHECK_DIR_ERR();
   
    num_of_file = *(unsigned*)(inf.p16+8);
    inf.p12 = num_of_file;
    free((void*)inf.p16);
    inf.p16 = (unsigned) malloc(32+inf.p12*560);
    list->objs = (struct dirent*)malloc(num_of_file*sizeof(struct dirent));
   
    CHECK_DIR_ERR();
 
    for(int i=0; i<num_of_file; i++){
       list->objs[i].d_ino  = i;
       list->objs[i].d_type = *(unsigned*)(inf.p16+32+(264+40)*i);
       strcpy(list->objs[i].d_name,(char*)(inf.p16+32+40+(264+40)*i));
    }
    list->num_objs = num_of_file;
    return list;
}


