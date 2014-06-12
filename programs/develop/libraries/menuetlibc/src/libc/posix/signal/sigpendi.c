/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <signal.h>

int
sigpending(sigset_t *_set)
{
  return sigemptyset(_set);
}
