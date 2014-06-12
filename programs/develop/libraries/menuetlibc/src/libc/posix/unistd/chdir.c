/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <libc/dosio.h>

void __chdir(char * path);

int chdir (const char *dirname)
{
 __chdir((char *)dirname);
 return 0;
}
