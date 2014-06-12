/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>

static char dev_con[] = "con";

/* ARGSUSED */
char *
ttyname(int fildes)
{
  if (isatty(fildes))
    return dev_con;
  else
    return 0;
}    
