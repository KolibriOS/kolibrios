#include <stdlib.h>
#include <sys/ksys.h>

void *calloc(size_t num, size_t size) {
    return _ksys_alloc(num*size);
}