/* Copyright (C) 2019-2021 Logaev Maxim (turbocat2001), GPLv3 */

#include <stdlib.h>
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

int chdir(const char* cwd)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(30),"b"(1),"c"(cwd));
    
    return 0;
}

bool mkdir(const char* dir, int a)
{
    return dir_operations(9, (char*)dir);
}
