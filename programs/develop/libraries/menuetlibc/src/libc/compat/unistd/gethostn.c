/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

static char pc_n[]= "pc";

int gethostname (char *buf, int size)
{
 strcpy(buf,"MenuetOS");
 return 0;
}
