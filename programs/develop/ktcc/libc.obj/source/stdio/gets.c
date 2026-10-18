#include <stdio.h>
#include <string.h>
#include "conio.h"
#include <errno.h>
#include <limits.h>

char* gets(char* str)
{
    if (fgets(str, STDIO_MAX_MEM, stdin) == NULL) {
        return NULL;
    }

    int str_len = strlen(str);
    if (str[str_len - 1] == '\n') {
        str[str_len - 1] = '\0';
    }

    return str;
}
