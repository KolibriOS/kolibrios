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
 "@(#) $Id: SDL_memops.h,v 1.2 2001/04/26 16:50:18 hercules Exp $";
#endif

#ifndef _SDL_memops_h
#define _SDL_memops_h

/* System dependent optimized memory manipulation routines:
*/
#include <string.h>

#if defined(__GNUC__) && defined(i386)
/* Thanks to Brennan "Bas" Underwood, for the inspiration. :)
 */
#define SDL_memcpy(dst, src, len)					  \
do {									  \
	int u0, u1, u2;						  \
	__asm__ __volatile__ (						  \
		"cld\n\t"						  \
		"rep ; movsl\n\t"					  \
		"testb $2,%b4\n\t"					  \
		"je 1f\n\t"						  \
		"movsw\n"						  \
		"1:\ttestb $1,%b4\n\t"					  \
		"je 2f\n\t"						  \
		"movsb\n"						  \
		"2:"							  \
		: "=&c" (u0), "=&D" (u1), "=&S" (u2)			  \
		: "0" ((unsigned)(len)/4), "q" (len), "1" (dst),"2" (src) \
		: "memory" );						  \
} while(0)

#define SDL_memcpy4(dst, src, len)				\
do {								\
	int ecx, edi, esi;					\
	__asm__ __volatile__ (					\
		"cld\n\t"					\
		"rep ; movsl"					\
		: "=&c" (ecx), "=&D" (edi), "=&S" (esi)		\
		: "0" ((unsigned)(len)), "1" (dst), "2" (src)	\
		: "memory" );					\
} while(0)

#define SDL_revcpy(dst, src, len)			\
do {							\
	int u0, u1, u2;					\
	char *dstp = (char *)(dst);			\
	char *srcp = (char *)(src);			\
	int n = (len);					\
	if ( n >= 4 ) {					\
	__asm__ __volatile__ (				\
		"std\n\t"				\
		"rep ; movsl\n\t"			\
		: "=&c" (u0), "=&D" (u1), "=&S" (u2)	\
		: "0" (n >> 2),				\
		  "1" (dstp+(n-4)), "2" (srcp+(n-4))	\
		: "memory" );				\
	}						\
	switch (n & 3) {				\
		case 3: dstp[2] = srcp[2];		\
		case 2: dstp[1] = srcp[1];		\
		case 1: dstp[0] = srcp[0];		\
			break;				\
		default:				\
			break;				\
	}						\
} while(0)

#define SDL_memmove(dst, src, len)			\
do {							\
	if ( (dst) < (src) ) {				\
		SDL_memcpy((dst), (src), (len));	\
	} else {					\
		SDL_revcpy((dst), (src), (len));	\
	}						\
} while(0)

#define SDL_memset4(dst, val, len)				\
do {								\
	int u0, u1, u2;					\
	__asm__ __volatile__ (					\
		"cld\n\t"					\
		"rep ; stosl\n\t"				\
		: "=&D" (u0), "=&a" (u1), "=&c" (u2)		\
		: "0" (dst), "1" (val), "2" ((Uint32)(len))	\
		: "memory" );					\
} while(0)

#endif /* GNU C and x86 */

/* If there are no optimized versions, define the normal versions */
#ifndef SDL_memcpy
#define SDL_memcpy(dst, src, len)	memcpy(dst, src, len)
#endif

#ifndef SDL_memcpy4
#define SDL_memcpy4(dst, src, len)	memcpy(dst, src, (len) << 2)
#endif

#ifndef SDL_revcpy
#define SDL_revcpy(dst, src, len)	memmove(dst, src, len)
#endif

#ifndef SDL_memset4
#define SDL_memset4(dst, val, len)		\
do {						\
	unsigned _count = (len);		\
	unsigned _n = (_count + 3) / 4;		\
	Uint32 *_p = (Uint32 *)(dst);		\
	Uint32 _val = (val);			\
        switch (_count % 4) {			\
        case 0: do {    *_p++ = _val;		\
        case 3:         *_p++ = _val;		\
        case 2:         *_p++ = _val;		\
        case 1:         *_p++ = _val;		\
		} while ( --_n );		\
	}					\
} while(0)
#endif

#endif /* _SDL_memops_h */
