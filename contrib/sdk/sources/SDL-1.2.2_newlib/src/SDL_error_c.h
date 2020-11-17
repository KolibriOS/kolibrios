/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@devolution.com
*/

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: SDL_error_c.h,v 1.2 2001/04/26 16:50:17 hercules Exp $";
#endif

/* This file defines a structure that carries language-independent
   error messages
*/

#ifndef _SDL_error_c_h
#define _SDL_error_c_h

#define ERR_MAX_STRLEN	128
#define ERR_MAX_ARGS	5

typedef struct {
	/* This is a numeric value corresponding to the current error */
	int error;

	/* This is a key used to index into a language hashtable containing
	   internationalized versions of the SDL error messages.  If the key
	   is not in the hashtable, or no hashtable is available, the key is
	   used directly as an error message format string.
	*/
	unsigned char key[ERR_MAX_STRLEN];

	/* These are the arguments for the error functions */
	int argc;
	union {
		void *value_ptr;
#if 0	/* What is a character anyway?  (UNICODE issues) */
		unsigned char value_c;
#endif
		int value_i;
		double value_f;
		unsigned char buf[ERR_MAX_STRLEN];
	} args[ERR_MAX_ARGS];
} SDL_error;

#endif /* _SDL_error_c_h */
