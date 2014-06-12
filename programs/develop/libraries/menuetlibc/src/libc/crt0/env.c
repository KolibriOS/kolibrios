#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

static char **SDL_env = (char **)0;

int __libc_putenv(const char *variable)
{
 return -1;
}

char * __libc_getenv(const char *name)
{
 return NULL;
}
