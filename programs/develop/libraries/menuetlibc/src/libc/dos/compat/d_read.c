/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_READ.C.
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
#define FSLAYER
#include<menuet/os.h>

unsigned int _dos_read(int handle, void *buffer, unsigned int count, unsigned int *result)
{
 int p;
 p=dosemu_read(handle,buffer,count);
 if(p==-1) return p;
 if(result) *result=p;
 return 0;
}
  