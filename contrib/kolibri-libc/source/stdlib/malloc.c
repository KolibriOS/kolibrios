#include <stdlib.h>
#include <ksys.h>

void *malloc(size_t size) {
    return _ksys_alloc(size);
}