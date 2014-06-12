/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */

#include <signal.h>
#include <stdio.h>

extern char *sys_siglist[];

void
psignal (int sig, const char *msg)
{
  if (sig >= 0 && sig < NSIG)
    __libclog_printf( "%s: %s\n", msg, sys_siglist[sig]);
  else
    __libclog_printf( "%s: signal %d\n", msg, sig);
}
