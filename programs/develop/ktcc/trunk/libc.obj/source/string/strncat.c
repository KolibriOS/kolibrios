#include <string.h>

char* strncat(char* dst, const char* src, size_t n)
{
    int d0, d1, d2, d3;
    __asm__ __volatile__(
        "repne\n\t"
        "scasb\n\t"
        "decl %1\n\t"
        "movl %8,%3\n"
        "1:\tdecl %3\n\t"
        "js 2f\n\t"
        "lodsb\n\t"
        "stosb\n\t"
        "testb %%al,%%al\n\t"
        "jne 1b\n"
        "2:\txorl %2,%2\n\t"
        "stosb"
        : "=&S"(d0), "=&D"(d1), "=&a"(d2), "=&c"(d3)
        : "0"(src), "1"(dst), "2"(0), "3"(0xffffffff), "g"(n)
        : "memory");
    return dst;
}