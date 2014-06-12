/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <time.h>
#include <libc/farptrgs.h>
#include <menuet/os.h>

unsigned long rawclock(void)
{
  static unsigned long base = 0;
  unsigned long rv;
  rv=__menuet__getsystemclock();
  if (base == 0)
    base = rv;
  return rv - base;
}
