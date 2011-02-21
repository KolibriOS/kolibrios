/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
#include <io.h>
#include <unistd.h>

int
chsize(int handle, long size)
{
  return ftruncate(handle, size);
}
