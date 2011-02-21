/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <io.h>
#include <stdio.h>
#include <errno.h>
#include <libc/dosio.h>
#include <menuet/os.h>

int remove(const char *fn)
{
	struct systree_info2 inf;
	int res;
	_fixpath(fn,inf.name);
	inf.command = 8;
	inf.file_offset_low = inf.file_offset_high = 0;
	inf.size = inf.data_pointer = 0;
	res = __kolibri__system_tree_access2(&inf);
	if (res == 0) return 0;
	if (res == 5) errno = ENOENT;
	else errno = EACCES;
	return -1;
}
