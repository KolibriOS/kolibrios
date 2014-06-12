/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_GETDRV.C.
 *
 * Written by Peter Sulyok 1995 <sulyok@math.klte.hu>.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <dos.h>

void _dos_getdrive(unsigned int *p_drive)
{
 *p_drive = 0;
}
