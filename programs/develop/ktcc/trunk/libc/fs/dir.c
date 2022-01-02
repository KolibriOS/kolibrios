/* Copyright (C) 2019-2021 Logaev Maxim (turbocat2001), GPLv3 */

#include <stdlib.h>
#include <dir.h>
#include <stdbool.h>

#pragma pack(push,1)
typedef struct {
    unsigned	p00;
    unsigned long long	p04;
    unsigned	p12;
    unsigned	p16;
    char		p20;
    char		*p21;
} kol_struct70;
#pragma pack(pop)

int kol_file_70(kol_struct70 *k)
{
    asm volatile ("int $0x40"::"a"(70), "b"(k));
}

bool dir_operations(unsigned char fun_num, char *path)
{
    kol_struct70 inf;
    inf.p00 = fun_num;
    inf.p04 = 0;
    inf.p12 = 0;
    inf.p16 = 0;
    inf.p20 = 0;
    inf.p21 = path;
    if(!kol_file_70(&inf)){
        return true;
    }
    else {
	    return false;
    }
}

int lsdir(const char* dir, short_file_info **list)
{
    int num_of_file=0;
    kol_struct70 inf;
    
    inf.p00 = 1;
    inf.p04 = 0;
    inf.p12 = 2;
    inf.p16 = (unsigned) malloc(32+inf.p12*560);
    inf.p20 = 0;
    inf.p21 = (char*)dir;
    
    if(kol_file_70(&inf))
    {
        free((void*)inf.p16);
        return FS_ERROR;
    }
    
    num_of_file = *(unsigned*)(inf.p16+8);
    inf.p12 = num_of_file;
    free((void*)inf.p16);
    inf.p16 = (unsigned) malloc(32+inf.p12*560);
    *list = (short_file_info*)malloc(num_of_file*sizeof(short_file_info));
    
    if(kol_file_70(&inf))
    {
        free((void*)inf.p16);
        return FS_ERROR;
    }
  
    for(int i=0; i<num_of_file; i++)
    {
       (*list)[i].type = *(unsigned*)(inf.p16+32+(264+40)*i);
       (*list)[i].name =(char*)(inf.p16+32+40+(264+40)*i);
    }
    return num_of_file;
}


char *getcwd(char *buf, unsigned size)
{
    if(buf == NULL){
       if((buf = malloc(size))==NULL)
       {
            return NULL;
       }
    }
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(30),"b"(2),"c"(buf), "d"(size));
    return(buf);
}

void setcwd(const char* cwd)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(30),"b"(1),"c"(cwd));
}

bool rmdir(const char* dir)
{
    return dir_operations(8, (char*)dir);
}

bool mkdir(const char* dir)
{
    return dir_operations(9, (char*)dir);
}
