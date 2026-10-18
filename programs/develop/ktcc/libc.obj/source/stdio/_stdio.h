#ifndef __LIBC_STDIO_H
#define __LIBC_STDIO_H

#include <stdio.h>

extern char* stdin_buffer;
extern size_t stdin_buffer_size; //how much written
extern size_t stdin_buffer_len; // allocated
extern size_t stdin_buffer_position;

int stdin_buffer_init();
void stdin_buffer_free();
size_t stdin_read(char* restrict ptr, size_t bytes_count);

int fclose_r(FILE* stream);
FILE* fopen_r(const char* restrict name, const char* restrict mode, FILE* restrict stream);

#endif // __LIBC_STDIO_H
