/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* This is file GETDINFO.C */
/*
 * Get device info word by calling IOCTL Function 0.
 *
 * Copyright (c) 1994 Eli Zaretskii <eliz@is.elta.co.il>
 *
 * This software may be used freely so long as this copyright notice is
 * left intact.  There is no warranty on this software.
 *
 */

#include <errno.h>
#include <libc/dosio.h>
#include <assert.h>

short _get_dev_info(int);

short _get_dev_info(int fhandle)
{
}
