#include <stdlib.h>
 
void * operator new(size_t n)
{
    return malloc(n);
}

void * operator new[](size_t n)
{
    return malloc(n);
}

void operator delete(void * ptr)
{
    free(ptr);
}

void operator delete[](void * ptr)
{
    free(ptr);
}
