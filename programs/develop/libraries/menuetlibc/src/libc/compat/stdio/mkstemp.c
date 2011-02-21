/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

int
mkstemp (char *_template)
{
  if (mktemp (_template))
    return creat (_template, 0666);
  else {
    errno = ENOENT;
    return -1;
  }
}
