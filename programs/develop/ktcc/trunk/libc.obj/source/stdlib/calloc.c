#include <stdlib.h>
#include <errno.h>
#include <sys/ksys.h>

void *calloc(size_t num, size_t size) {
    void *ptr = _ksys_alloc(num*size);
    if(!ptr){
        errno = ENOMEM;
        return NULL;
    }
    memset(ptr, 0, num*size);
    return ptr;
}
