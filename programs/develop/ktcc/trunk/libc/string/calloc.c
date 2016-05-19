#include <stdlib.h>
#include <string.h>

void* calloc (size_t num, size_t size)
{
    size_t bytes = num * size;
    void *p = malloc(bytes);

    if(p)
        memset(p, 0, bytes);

    return p;
}
