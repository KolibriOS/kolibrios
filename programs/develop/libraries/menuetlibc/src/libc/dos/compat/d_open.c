/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_OPEN.C.
 *
 * Written by Peter Sulyok 1995 <sulyok@math.klte.hu>.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <libc/dosio.h>
#include <errno.h>
#include <dos.h>
#include<menuet/os.h>

unsigned int _dos_open(const char *filename, unsigned int mode, int *handle)
{
 int i;
 i=dosemu_open(filename,mode);
 if(i==-1) return -1;
 if(handle) *handle=i;
 return 0;
}
