/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/dosio.h>
#include <libc/farptrgs.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

void
_put_path(const char *path)
{
  _put_path2(path, 0);
}

void
_put_path2(const char *path, int offset)
{
 unimpl();
}
