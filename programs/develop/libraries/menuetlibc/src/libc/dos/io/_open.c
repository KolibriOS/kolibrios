/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <io.h>
#include <libc/dosio.h>
#include <sys/fsext.h>
#define FSLAYER
#include <menuet/os.h>

int _open(const char* filename, int oflag)
{
 int rv;
 if (__FSEXT_call_open_handlers(__FSEXT_open, &rv, &filename))
  return rv;
 return dosemu_open(filename,oflag);
}
