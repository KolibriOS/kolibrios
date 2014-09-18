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

/* This a stretch blit implementation based on ideas given to me by
   Tomasz Cejner - thanks! :)

   April 27, 2000 - Sam Lantinga
*/

#include "SDL_error.h"
#include "SDL_video.h"
#include "SDL_blit.h"

/* This isn't ready for general consumption yet - it should be folded
   into the general blitting mechanism.
*/

#if (defined(WIN32UNDEFINED) && !defined(_M_ALPHA) && !defined(_WIN32_WCE)) || \
    defined(i386) && defined(__GNUC__) && defined(USE_ASMBLIT)
#define USE_ASM_STRETCH
#endif

#ifdef USE_ASM_STRETCH

#if defined(WIN32UNDEFINED) || defined(i386)
#define PREFIX16	0x66
#define STORE_BYTE	0xAA
#define STORE_WORD	0xAB
#define LOAD_BYTE	0xAC
#define LOAD_WORD	0xAD
#define RETURN		0xC3
#else
#error Need assembly opcodes for this architecture
#endif

#if defined(__ELF__) && defined(__GNUC__)
extern unsigned char _copy_row[4096] __attribute__ ((alias ("copy_row")));
#endif
static unsigned char copy_row[4096];

static int generate_rowbytes(int src_w, int dst_w, int bpp)
{
	static struct {
		int bpp;
		int src_w;
		int dst_w;
	} last;

	int i;
	int pos, inc;
	unsigned char *eip;
	unsigned char load, store;

	/* See if we need to regenerate the copy buffer */
	if ( (src_w == last.src_w) &&
	     (dst_w == last.src_w) && (bpp == last.bpp) ) {
		return(0);
	}
	last.bpp = bpp;
	last.src_w = src_w;
	last.dst_w = dst_w;

	switch (bpp) {
	    case 1:
		load = LOAD_BYTE;
		store = STORE_BYTE;
		break;
	    case 2:
	    case 4:
		load = LOAD_WORD;
		store = STORE_WORD;
		break;
	    default:
		SDL_SetError("ASM stretch of %d bytes isn't supported\n", bpp);
		return(-1);
	}
	pos = 0x10000;
	inc = (src_w << 16) / dst_w;
	eip = copy_row;
	for ( i=0; i<dst_w; ++i ) {
		while ( pos >= 0x10000L ) {
			if ( bpp == 2 ) {
				*eip++ = PREFIX16;
			}
			*eip++ = load;
			pos -= 0x10000L;
		}
		if ( bpp == 2 ) {
			*eip++ = PREFIX16;
		}
		*eip++ = store;
		pos += inc;
	}
	*eip++ = RETURN;

	/* Verify that we didn't overflow (too late) */
	if ( eip > (copy_row+sizeof(copy_row)) ) {
		SDL_SetError("Copy buffer overflow");
		return(-1);
	}
	return(0);
}

#else

#define DEFINE_COPY_ROW(name, type)			\
void name(type *src, int src_w, type *dst, int dst_w)	\
{							\
	int i;						\
	int pos, inc;					\
	type pixel = 0;					\
							\
	pos = 0x10000;					\
	inc = (src_w << 16) / dst_w;			\
	for ( i=dst_w; i>0; --i ) {			\
		while ( pos >= 0x10000L ) {		\
			pixel = *src++;			\
			pos -= 0x10000L;		\
		}					\
		*dst++ = pixel;				\
		pos += inc;				\
	}						\
}
DEFINE_COPY_ROW(copy_row1, Uint8)
DEFINE_COPY_ROW(copy_row2, Uint16)
DEFINE_COPY_ROW(copy_row4, Uint32)

#endif /* USE_ASM_STRETCH */

/* The ASM code doesn't handle 24-bpp stretch blits */
void copy_row3(Uint8 *src, int src_w, Uint8 *dst, int dst_w)
{
	int i;
	int pos, inc;
	Uint8 pixel[3];

	pos = 0x10000;
	inc = (src_w << 16) / dst_w;
	for ( i=dst_w; i>0; --i ) {
		while ( pos >= 0x10000L ) {
			pixel[0] = *src++;
			pixel[1] = *src++;
			pixel[2] = *src++;
			pos -= 0x10000L;
		}
		*dst++ = pixel[0];
		*dst++ = pixel[1];
		*dst++ = pixel[2];
		pos += inc;
	}
}

/* Perform a stretch blit between two surfaces of the same format.
   NOTE:  This function is not safe to call from multiple threads!
*/
int SDL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect,
                    SDL_Surface *dst, SDL_Rect *dstrect)
{
	int pos, inc;
	int dst_width;
	int dst_maxrow;
	int src_row, dst_row;
	Uint8 *srcp = NULL;
	Uint8 *dstp;
	SDL_Rect full_src;
	SDL_Rect full_dst;
#if defined(USE_ASM_STRETCH) && defined(__GNUC__)
	int u1, u2;
#endif
	const int bpp = dst->format->BytesPerPixel;

	if ( src->format->BitsPerPixel != dst->format->BitsPerPixel ) {
		SDL_SetError("Only works with same format surfaces");
		return(-1);
	}

	/* Verify the blit rectangles */
	if ( srcrect ) {
		if ( (srcrect->x < 0) || (srcrect->y < 0) ||
		     ((srcrect->x+srcrect->w) > src->w) ||
		     ((srcrect->y+srcrect->h) > src->h) ) {
			SDL_SetError("Invalid source blit rectangle");
			return(-1);
		}
	} else {
		full_src.x = 0;
		full_src.y = 0;
		full_src.w = src->w;
		full_src.h = src->h;
		srcrect = &full_src;
	}
	if ( dstrect ) {
		if ( (dstrect->x < 0) || (dstrect->y < 0) ||
		     ((dstrect->x+dstrect->w) > dst->w) ||
		     ((dstrect->y+dstrect->h) > dst->h) ) {
			SDL_SetError("Invalid destination blit rectangle");
			return(-1);
		}
	} else {
		full_dst.x = 0;
		full_dst.y = 0;
		full_dst.w = dst->w;
		full_dst.h = dst->h;
		dstrect = &full_dst;
	}

	/* Set up the data... */
	pos = 0x10000;
	inc = (srcrect->h << 16) / dstrect->h;
	src_row = srcrect->y;
	dst_row = dstrect->y;
	dst_width = dstrect->w*bpp;

#ifdef USE_ASM_STRETCH
	/* Write the opcodes for this stretch */
	if ( (bpp != 3) &&
	     (generate_rowbytes(srcrect->w, dstrect->w, bpp) < 0) ) {
		return(-1);
	}
#endif

	/* Perform the stretch blit */
	for ( dst_maxrow = dst_row+dstrect->h; dst_row<dst_maxrow; ++dst_row ) {
		dstp = (Uint8 *)dst->pixels + (dst_row*dst->pitch)
		                            + (dstrect->x*bpp);
		while ( pos >= 0x10000L ) {
			srcp = (Uint8 *)src->pixels + (src_row*src->pitch)
			                            + (srcrect->x*bpp);
			++src_row;
			pos -= 0x10000L;
		}
#ifdef USE_ASM_STRETCH
		switch (bpp) {
		    case 3:
			copy_row3(srcp, srcrect->w, dstp, dstrect->w);
			break;
		    default:
#ifdef __GNUC__
			__asm__ __volatile__ ("call _copy_row"
			: "=&D" (u1), "=&S" (u2)
			: "0" (dstp), "1" (srcp)
			: "memory" );
#else
#ifdef WIN32UNDEFINED
		{ void *code = &copy_row;
			__asm {
				push edi
				push esi
	
				mov edi, dstp
				mov esi, srcp
				call dword ptr code

				pop esi
				pop edi
			}
		}
#else
#error Need inline assembly for this compiler
#endif
#endif /* __GNUC__ */
			break;
		}
#else
		switch (bpp) {
		    case 1:
			copy_row1(srcp, srcrect->w, dstp, dstrect->w);
			break;
		    case 2:
			copy_row2((Uint16 *)srcp, srcrect->w,
			          (Uint16 *)dstp, dstrect->w);
			break;
		    case 3:
			copy_row3(srcp, srcrect->w, dstp, dstrect->w);
			break;
		    case 4:
			copy_row4((Uint32 *)srcp, srcrect->w,
			          (Uint32 *)dstp, dstrect->w);
			break;
		}
#endif
		pos += inc;
	}
	return(0);
}

