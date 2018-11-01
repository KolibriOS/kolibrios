#include <stdlib.h>
#include <string.h>

void* syscalloc (size_t num, size_t size)
{
    size_t bytes = num * size;
    void *p = sysmalloc(bytes);

    if(p)
        memset(p, 0, bytes);

    return p;
}
