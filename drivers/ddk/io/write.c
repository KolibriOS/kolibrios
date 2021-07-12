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
    int status;
    unsigned bytes_written_v;
    FS_Service(&k, &bytes_written_v);
    if (!status){
        *writes = bytes_written_v;
    }
    return status;
}               
