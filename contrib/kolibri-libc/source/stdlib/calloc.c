#include <stdlib.h>
#include <ksys.h>

void *calloc(size_t num, size_t size) {
    return _ksys_alloc(num*size);
}