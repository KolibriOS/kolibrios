/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>

int isatty(int fd)
{
 if(fd==0 || fd==1) return 1;
 return 0;
}
