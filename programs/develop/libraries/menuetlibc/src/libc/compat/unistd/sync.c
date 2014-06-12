/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>

int
sync(void)
{
  int i;
  /* Update files with handles 0 thru 254 (incl).  */
  for (i = 0; i < 255; i++)
    fsync (i);
  return 0;
}
