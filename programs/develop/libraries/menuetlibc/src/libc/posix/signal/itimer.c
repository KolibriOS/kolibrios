/* Copyright (C) 1995 Charles Sandmann (sandmann@clio.rice.edu)
   setitimer implmentation - used for profiling and alarm
   BUGS: ONLY ONE AT A TIME, first pass code
   This software may be freely distributed, no warranty. */

#include <libc/stubs.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>

static struct itimerval real, prof;

/* not right, should compute from current tic count.  Do later */
int getitimer(int which, struct itimerval *value)
{
  errno = EINVAL;
  return -1;
}

int setitimer(int which, struct itimerval *value, struct itimerval *ovalue)
{
 return -EPERM;
}
