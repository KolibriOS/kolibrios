#include <sys/types.h>
#include <errno.h>
#include <sys/kos_io.h>

int write_file(const char *path,const void *buff,
               size_t offset, size_t count, size_t *writes)
{
    int retval;
    __asm__ __volatile__(
    "pushl $0 \n\t"
    "pushl $0 \n\t"
    "movl %%eax, 1(%%esp) \n\t"
    "pushl %%ebx \n\t"
    "pushl %%edx \n\t"
    "pushl $0 \n\t"
    "pushl %%ecx \n\t"
    "pushl $3 \n\t"
    "movl %%esp, %%ebx \n\t"
    "mov $70, %%eax \n\t"
    "int $0x40 \n\t"
    "testl %%esi, %%esi \n\t"
    "jz 1f \n\t"
    "movl %%ebx, (%%esi) \n\t"
"1:"
    "addl $28, %%esp \n\t"
    :"=a" (retval)
    :"a"(path),"b"(buff),"c"(offset),"d"(count),"S"(writes));
    if(retval == 0)
        return 0;
    else if (retval == 8)
        return ENOSPC;
    return -1;
};
