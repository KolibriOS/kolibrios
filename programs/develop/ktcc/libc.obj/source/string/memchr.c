#include <assert.h>
#include "unconst.h"
#include <string.h>

void* memchr(const void* s, int c, size_t n)
{
    int d0;
    register void* __res;
    if (!n)
        return NULL;
    __asm__ __volatile__(
        "repne\n\t"
        "scasb\n\t"
        "je 1f\n\t"
        "movl $1,%0\n"
        "1:\tdecl %0"
        : "=D"(__res), "=&c"(d0)
        : "a"(c), "0"(s), "1"(n));
    return __res;
}