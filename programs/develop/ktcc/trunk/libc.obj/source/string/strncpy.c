/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <string.h>

char* strncpy(char* dst, const char* src, size_t n)
{
    int d0, d1, d2, d3;
    __asm__ __volatile__(
        "1:\tdecl %2\n\t"
        "js 2f\n\t"
        "lodsb\n\t"
        "stosb\n\t"
        "testb %%al,%%al\n\t"
        "jne 1b\n\t"
        "rep\n\t"
        "stosb\n"
        "2:"
        : "=&S"(d0), "=&D"(d1), "=&c"(d2), "=&a"(d3)
        : "0"(src), "1"(dst), "2"(n)
        : "memory");
    return dst;
}