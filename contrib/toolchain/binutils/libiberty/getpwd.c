/* getpwd.c - get the working directory */

/*

@deftypefn Supplemental char* getpwd (void)

Returns the current working directory.  This implementation caches the
result on the assumption that the process will not call @code{chdir}
between calls to @code{getpwd}.

@end deftypefn

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#include <errno.h>
#ifndef errno
extern int errno;
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_LIMITS_H
#include <limits.h>
#endif

#include "libiberty.h"


#ifdef MAXPATHLEN
#define GUESSPATHLEN (MAXPATHLEN + 1)
#else
#define GUESSPATHLEN 100
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 255
#endif


static char *getccwd(char *buf, size_t size)
{
    int bsize;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(bsize)
    :"a"(30),"b"(2),"c"(buf), "d"(size)
    :"memory");

    return buf;
};

char *
getpwd (void)
{
  static char *pwd = 0;

  if (!pwd)
    pwd = getccwd (XNEWVEC (char, MAXPATHLEN + 1), MAXPATHLEN + 1);

  return pwd;
}


