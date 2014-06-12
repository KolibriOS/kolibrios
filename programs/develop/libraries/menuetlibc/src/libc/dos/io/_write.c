/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <io.h>
#include <sys/fsext.h>

#include <libc/dosio.h>
#define FSLAYER
#include <menuet/os.h>

int _write(int handle, const void* buffer, size_t count)
{
 __FSEXT_Function *func = __FSEXT_get_function(handle);
 if (func)
 {
  int rv;
  if (func(__FSEXT_write, &rv, &handle)) return rv;
 }
 return dosemu_write(handle,buffer,count);
}
