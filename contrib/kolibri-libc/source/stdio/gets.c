#include <stdio.h>
#include <string.h>
#include "conio.h"
#include <errno.h>

char *gets(char* str)
{
    __con_init();
    if(__con_gets(str, 4096)==NULL){
        errno = EIO;
        return NULL;
    }
    str[strlen(str)-1]='\0';
    return str;
}