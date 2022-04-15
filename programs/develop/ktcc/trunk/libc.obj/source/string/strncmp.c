/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <string.h>

int strncmp(const char* s1, const char* s2, size_t n)
{
    register int __res;
    int d0, d1, d2;
    __asm__ __volatile__(
        "1:\tdecl %3\n\t"
        "js 2f\n\t"
        "lodsb\n\t"
        "scasb\n\t"
        "jne 3f\n\t"
        "testb %%al,%%al\n\t"
        "jne 1b\n"
        "2:\txorl %%eax,%%eax\n\t"
        "jmp 4f\n"
        "3:\tsbbl %%eax,%%eax\n\t"
        "orb $1,%%al\n"
        "4:"
        : "=a"(__res), "=&S"(d0), "=&D"(d1), "=&c"(d2)
        : "1"(s1), "2"(s2), "3"(n));
    return __res;
}
