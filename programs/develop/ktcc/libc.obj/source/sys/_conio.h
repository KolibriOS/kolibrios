#ifndef __LIBC_SYS_CONIO_H_
#define __LIBC_SYS_CONIO_H

#include <stddef.h>

char* console_gets(char* buff, size_t len);

void console_write(const char* ptr, size_t len);

void console_exit();

#endif // __LIBC_SYS_CONIO_H
