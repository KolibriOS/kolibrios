/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#ifndef _KSYS_FS_H_
#define _KSYS_FS_H_

#include <stdint.h>
#include <stddef.h>

#define asm_inline __asm__ __volatile__ 

#pragma pack(push,1)
typedef struct{
    unsigned            p00;
    union{
        uint64_t        p04;
        struct {
            unsigned    p04dw;
            unsigned    p08dw;
        };
    };
    unsigned            p12;
    union {
        unsigned        p16;
        const char     *new_name;
        void           *bdfe;
        void           *buf16;
        const void     *cbuf16;
    };
    char                p20;
    const char         *p21;
}ksys70_t;

typedef struct {
    unsigned attributes;
    unsigned name_cp;
    char creation_time[4];
    char creation_date[4];
    char last_access_time[4];
    char last_access_date[4];
    char last_modification_time[4];
    char last_modification_date[4];
    unsigned long long size;
    char name[0];
}ksys_bdfe_t;
#pragma pack(pop)

static inline
int _ksys_work_files(const ksys70_t *k)
{
    int status;
    asm_inline(
        "int $0x40"
        :"=a"(status)
        :"a"(70), "b"(k)
        :"memory"
    );
    return status;
}
 
static inline
int _ksys_file_delete(const char *name)
{
    ksys70_t k;
    k.p00 = 8;
    k.p20 = 0;
    k.p21 = name;
    return _ksys_work_files(&k);
}
 
static inline
int _ksys_mkdir(const char *path)
{
    ksys70_t dir_opt;
    dir_opt.p00 = 9;
    dir_opt.p21 = path;
    return _ksys_work_files(&dir_opt);
}

static inline
void _ksys_setcwd(char* dir){
    asm_inline(
        "int $0x40"
        ::"a"(30), "b"(1), "c"(dir)
    );
}

static inline
void* _ksys_alloc(size_t size){
    void  *val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(68),"b"(12),"c"(size)
    );
    return val;
}
 
static inline
int _ksys_free(void *mem)
{
    int val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(68),"b"(13),"c"(mem)
    );
    return val;
}

static inline
int _ksys_getcwd(char* buf, int bufsize){
    register int val;
    asm_inline(
        "int $0x40"
        :"=a"(val):"a"(30), "b"(2), "c"(buf), "d"(bufsize)
    );
    return val;
}

#endif
