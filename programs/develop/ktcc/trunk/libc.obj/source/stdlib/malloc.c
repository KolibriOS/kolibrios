#include <stdlib.h>
#include <sys/ksys.h>

void* malloc(size_t size)
{
    return _ksys_alloc(size);
}