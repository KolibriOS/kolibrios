#include <stdio.h>
#include <string.h>
#include "conio.h"
#include <errno.h>
#include <limits.h>

char* gets(char* str)
{
    if (console_gets(str, STDIO_MAX_MEM) == NULL) {
        errno = EIO;
        return NULL;
    }

    int str_len = strlen(str);
    if (str[str_len - 1] == '\n') {
        str[str_len - 1] = '\0';
    }
    return str;
}
