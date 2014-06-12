/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <sys/wait.h>
#include <errno.h>

/* ARGSUSED */
pid_t
wait(int *stat_loc)
{
  errno = ECHILD;
  return -1;
}
