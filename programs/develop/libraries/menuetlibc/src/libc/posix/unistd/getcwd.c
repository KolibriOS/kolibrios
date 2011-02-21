/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <libc/farptrgs.h>
#include <libc/dosio.h>

static char * __cur_cwd;

extern char* __get_curdir(void);
char * getcwd(char *buf, size_t size)
{
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
 size_t k;
 __cur_cwd=__get_curdir();
 k=min(size,strlen(__cur_cwd));
 memcpy(buf,__cur_cwd,k+1);
 return buf;
}

void __libc_set_cwd(char * p)
{
 __chdir(p);
}
