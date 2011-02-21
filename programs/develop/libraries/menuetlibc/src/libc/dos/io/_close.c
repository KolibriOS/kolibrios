/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <errno.h>
#include <io.h>
#include <sys/fsext.h>

#include <libc/dosio.h>

#include <menuet/os.h>

int _close(int handle)
{
 __FSEXT_Function *func = __FSEXT_get_function(handle);
 if (func)
 {
  int rv;
  if (func(__FSEXT_close, &rv, &handle))
  {
   __FSEXT_set_function(handle, 0);
   return rv;
  }
  __FSEXT_set_function(handle, 0);
 }
 return dosemu_close(handle);
}
