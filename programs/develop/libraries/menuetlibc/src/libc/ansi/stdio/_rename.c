/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <errno.h>
#include <libc/dosio.h>
#include<assert.h>

int _rename(const char *old, const char *new)
{
 return -1;
}
