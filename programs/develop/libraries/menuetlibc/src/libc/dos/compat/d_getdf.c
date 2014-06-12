/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_GETDF.C.
 *
 * Written by Peter Sulyok 1995 <sulyok@math.klte.hu>.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <dos.h>
#include <errno.h>

unsigned int _dos_getdiskfree(unsigned int drive, struct _diskfree_t *diskspace)
{
 diskspace->sectors_per_cluster = 1;
 diskspace->avail_clusters      = 0xFFFF;
 diskspace->bytes_per_sector    = 512;
 diskspace->total_clusters      = 0xFFFF;
 return 0;
}
