/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <sys/stat.h>		/* For stat() */
#include <fcntl.h>		/* For O_RDONLY, etc. */
#include <unistd.h>		/* For read(), write(), etc. */
#include <limits.h>		/* For PATH_MAX */
#include <utime.h>		/* For utime() */
#include <errno.h>		/* For errno */

int link(const char *path1, const char *path2)
{
 return -1;
}
