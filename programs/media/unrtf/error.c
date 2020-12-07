
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
 * Module name:    error
 * Author name:    Zach Smith
 * Create date:    01 Sep 00
 * Purpose:        Management of errors and warnings, when reporting
 *                 the source code file/line is not necessary.
 *----------------------------------------------------------------------
 * Changes
 * 10 Oct 00, tuorfa@yahoo.com: added usage()
 * 15 Oct 00, tuorfa@yahoo.com: improved output readability
 * 22 Sep 01, tuorfa@yahoo.com: removed mention of line number in handlers
 * 22 Sep 01, tuorfa@yahoo.com: added function-level comment blocks 
 *--------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
#include "main.h"



/*========================================================================
 * Name:	usage
 * Purpose:	Prints usage information and exits with an error.
 * Args:	None.
 * Returns:	None.
 *=======================================================================*/

void
usage ()
{
	fprintf (stdout, "Usage: %s\n", USAGE);
	exit(0);
}



/*========================================================================
 * Name:	error_handler
 * Purpose:	Prints error message and other useful info, then exits.
 * Args:	Message.
 * Returns:	None.
 *=======================================================================*/

void
error_handler (char* message)
{
	fprintf (stderr, "Error: %s\n", message);
	exit(10);
}


/*========================================================================
 * Name:	warning_handler
 * Purpose:	Prints useful info to stderr, but doesn't exit.
 * Args:	Message.
 * Returns:	None.
 *=======================================================================*/

void
warning_handler (char* message)
{
	fprintf (stderr, "Warning: %s\n", message);
}
