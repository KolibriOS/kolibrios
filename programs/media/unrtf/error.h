
/*=============================================================================
   GNU UnRTF, a command-line program to convert RTF documents to other formats.
   Copyright (C) 2000,2001 Zachary Thayer Smith

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   The author is reachable by electronic mail at tuorfa@yahoo.com.
=============================================================================*/


/*----------------------------------------------------------------------
 * Module name:    error.h
 * Author name:    Zach Smith
 * Create date:    1 Sept 2000
 * Purpose:        Macros to be executed at the start of a function,
 *                 when reporting source code file/line is useful.
 *----------------------------------------------------------------------
 * Changes
 *--------------------------------------------------------------------*/

#include <stdlib.h>

#define CHECK_PARAM_NOT_NULL(XX) { if ((XX)==NULL) { fprintf (stderr, "internal error: null pointer param in %s at %d\n", __FILE__, __LINE__); exit (1); }}

#define CHECK_MALLOC_SUCCESS(XX) { if ((XX)==NULL) { fprintf (stderr, "internal error: cannot allocate memory in %s at %d\n", __FILE__, __LINE__); exit (1); }}


extern void usage(void);
extern void error_handler (char*);
extern void warning_handler (char*);


