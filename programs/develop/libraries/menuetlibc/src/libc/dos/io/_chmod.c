/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <io.h>
#include <errno.h>
#include <libc/dosio.h>
#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
 
int _chmod(const char *filename, int func, ...)
{
 int i;
 i=open(filename,O_RDONLY);
 if(i<0) return -1;
 close(i);
 return 0;
}
