#include <syscall.h>

int create_file(const char *path)
{
    int err;
    ksys70_t  k;
    k.p00   = 2;
    k.p04dw = 0;
    k.p08dw = 0;
    k.p12   = 0;
    k.p20   = 0;
    k.p21   = path;
    return FS_Service(&k, &err);
};
