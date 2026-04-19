#include <stdlib.h>
#include <sys/ksys.h>

void* realloc(void* ptr, size_t newsize)
{
    return _ksys_realloc(ptr, newsize);
}