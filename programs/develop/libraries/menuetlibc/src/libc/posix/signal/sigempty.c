/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <errno.h>
#include <signal.h>

int
sigemptyset(sigset_t *_set)
{
  int i;
  if (_set == 0)
  {
    errno = EINVAL;
    return -1;
  }
  for (i=0; i<10; i++)
    _set->__bits[i] = 0;
  return 0;
}
