/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <sys/exceptn.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>

#include <libc/dosio.h>
#include <assert.h>

void (*__setmode_stdio_hook)(int fd, int mode); /* BSS to zero */

int setmode(int handle, int mode)
{
 return 0;
}
