/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <sys/wait.h>
#include <errno.h>

/* ARGSUSED */
pid_t
waitpid(pid_t pid, int *stat_loc, int options)
{
  errno = ECHILD;
  return -1;
}
