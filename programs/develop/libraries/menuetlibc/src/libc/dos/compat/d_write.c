/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_WITE.C.
 *
 * Written by Peter Sulyok 1995 <sulyok@math.klte.hu>.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <libc/stubs.h>
#include <libc/dosio.h>
#include <errno.h>
#include <dos.h>

unsigned int _dos_write(int handle, const void *buffer, unsigned int count, unsigned int *result)
{
 int p;
 p=dosemu_write(handle,buffer,count);
 if(p==-1) return p;
 if(result) *result=p;
 return 0;
}

