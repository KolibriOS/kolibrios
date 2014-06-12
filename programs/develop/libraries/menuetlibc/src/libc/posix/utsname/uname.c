/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <errno.h>
#include <dos.h>

int uname(struct utsname *u)
{
  strncpy(u->sysname, "MenuetOS",8);
  u->sysname[sizeof(u->sysname) - 1] = '\0';
  sprintf(u->version, "0.47");
  sprintf(u->release, "1.0");
  strcpy(u->machine, "i386");
  strcpy(u->nodename, "(none)");
  return 0;
}
