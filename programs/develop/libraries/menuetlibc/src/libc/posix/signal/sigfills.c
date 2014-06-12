/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <errno.h>
#include <signal.h>

int
sigfillset(sigset_t *_set)
{
  if (_set == 0)
  {
    errno = EINVAL;
    return -1;
  }
  sigemptyset(_set);
  _set->__bits[9] = 0x7fff; /* SIGABRT (288) .. SIGTRAP (302) */
  return 0;
}
