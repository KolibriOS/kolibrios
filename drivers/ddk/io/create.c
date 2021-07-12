#include <syscall.h>

int create_file(const char *path)
{
    int err=0;
    ksys70_t k;
    k.p00 = 2;
    k.p12 = 0;
    k.p20 = 0;
    k.p21 = path;
    return FS_Service(&k, &err);
};
