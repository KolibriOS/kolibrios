#include <syscall.h>

int set_file_size(const char *path, unsigned size)
{
     ksys70_t  k;
     int err;
     k.p00   = 4;
     k.p04dw = size;
     k.p08dw = 0;
     k.p20   = 0;
     k.p21   = path;
     return FS_Service(&k, &err);
}
