#ifndef __LIBC_STDIO_H
#define __LIBC_STDIO_H

#include <stdio.h>

int fclose_r(FILE* stream);
FILE* fopen_r(const char* restrict name, const char* restrict mode, FILE* restrict stream);

#endif // __LIBC_STDIO_H
