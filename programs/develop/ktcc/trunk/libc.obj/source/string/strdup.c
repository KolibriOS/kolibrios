#include <stdlib.h>
#include <string.h>

char* strdup(const char* str)
{
    char* buf = malloc(strlen(str) + 1);
    buf[strlen(str)] = '\0';
    strcpy(buf, str);
    return buf;
}