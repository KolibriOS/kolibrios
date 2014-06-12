/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <io.h>
#include <errno.h>

int ftruncate(int fd, off_t where)
{
	int res = dosemu_truncate(fd, where);
	if (res) {errno = res; return -1;}
	return 0;
}
