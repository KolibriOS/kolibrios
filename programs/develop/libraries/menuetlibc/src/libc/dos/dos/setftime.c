/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dos.h>
#include <libc/dosio.h>
#include <errno.h>
#include <assert.h>

int setftime(int handle, struct ftime *ft)
{
 return -EPERM;
}
