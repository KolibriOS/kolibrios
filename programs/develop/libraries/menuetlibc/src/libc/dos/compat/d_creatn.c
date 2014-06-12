/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_CREATN.C.
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
#include<unistd.h>
#include <fcntl.h>

static char buf[1];

unsigned int _dos_creatnew(const char *filename, unsigned int attr, int *handle)
{
 int i;
 i=dosemu_open(filename,attr|O_CREAT|O_EXCL);
 if(i==-1) return -1;
 if(handle) *handle=i;
 return 0;
}
