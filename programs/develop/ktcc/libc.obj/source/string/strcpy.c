/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <string.h>

char* strcpy(char* to, const char* from)
{
    int d0, d1, d2;
    __asm__ __volatile__(
        "1:\tlodsb\n\t"
        "stosb\n\t"
        "testb %%al,%%al\n\t"
        "jne 1b"
        : "=&S"(d0), "=&D"(d1), "=&a"(d2)
        : "0"(from), "1"(to)
        : "memory");
    return to;
}
