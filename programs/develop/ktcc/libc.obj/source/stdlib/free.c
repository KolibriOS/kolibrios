#include <stdlib.h>
#include <sys/ksys.h>

void free(void* ptr)
{
    _ksys_free(ptr);
}