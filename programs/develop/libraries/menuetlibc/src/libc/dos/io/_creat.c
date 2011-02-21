/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <fcntl.h>
#include <errno.h>
#include <io.h>
#include <libc/dosio.h>
#include <sys/fsext.h>

int _creat(const char* filename, int attrib)
{
 return _open(filename,attrib|O_CREAT);
}
