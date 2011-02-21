/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <errno.h>
#include <signal.h>

int
sigdelset(sigset_t *_set, int _signo)
{
  if (_set == 0)
  {
    errno = EINVAL;
    return -1;
  }
  if (_signo < 0 || _signo >= 320)
  {
    errno = EINVAL;
    return -1;
  }
  _set->__bits[_signo>>5] &= ~(1U << (_signo&31));
  return 0;
}
