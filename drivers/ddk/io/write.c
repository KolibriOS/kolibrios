#include <syscall.h>

int write_file(const char *path,const void *buff,
               unsigned offset,unsigned count,unsigned *writes)
{
    ksys70_t k;
    k.p00 = 3;
    k.p04 = offset;
    k.p12 = count;
    k.cbuf16 = buff;
    k.p20 = 0;
    k.p21 = path;
    return FS_Service(&k, writes);
}               
