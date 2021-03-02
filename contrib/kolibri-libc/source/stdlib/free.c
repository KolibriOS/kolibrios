#include <stdlib.h>
#include <ksys.h>

void free(void *ptr) {
    _ksys_free(ptr);
}