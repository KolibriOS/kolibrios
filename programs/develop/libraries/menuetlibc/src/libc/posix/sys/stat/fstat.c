#include <libc/stubs.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libc/farptrgs.h>
#include <libc/bss.h>
#include <menuet/os.h>

#include "../../../dos/dos_emu/dosemuin.h"

#include "xstat.h"

int fstat(int handle, struct stat *statbuf)
{
	if (handle < 0 || handle >= _MAX_HANDLES)
	{
		errno = EBADF;
		return -1;
	}
	return stat(_io_handles[handle].filename, statbuf);
}
