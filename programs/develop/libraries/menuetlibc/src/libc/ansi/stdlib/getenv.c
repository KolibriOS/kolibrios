/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>
#include <string.h>

extern char * __libc_getenv(const char *name); // from crt0/env.c
char * getenv(const char *name)
{
 return __libc_getenv(name);
}
