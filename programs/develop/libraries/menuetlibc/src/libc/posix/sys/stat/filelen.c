/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* This is file FILELEN.C */
/*
 * Copyright (c) 1994 Eli Zaretskii <eliz@is.elta.co.il>
 *
 * This software may be used freely so long as this copyright notice is
 * left intact.  There is no warranty on this software.
 *
 */

#include <errno.h>
#include <libc/dosio.h>

long __filelength(int);

long __filelength(int fhandle)
{
 return dosemu_iosize(fhandle);
}
