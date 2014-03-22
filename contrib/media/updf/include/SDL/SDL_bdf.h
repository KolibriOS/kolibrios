/*
SDL_bdf - renders BDF fonts
Copyright (C) 2002-2003 Andre de Leiradella

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

For information about SDL_bdf contact leiradella@bigfoot.com

Version 1.0: first public release.
Version 1.1: removed SDL dependecies, now SDL_bdf can be used with any graphics
	     library.
Version 1.2: fixed BDF_SizeH and BDF_SizeEntitiesH to return the correct sizes.
*/
#ifndef __SDL_bdf_h__
#define __SDL_bdf_h__

#ifdef __cplusplus
extern "C" {
#endif

/* Error codes. */

/* No error. */
#define BDF_OK			 0
/* Not enough memory reading BDF font. */
#define BDF_MEMORYERROR 	 1
/* Error reading BDF font. */
#define BDF_READERROR		 2
/* Can only handle BDF font varsions up to 2.2. */
#define BDF_WRONGVERSION	 3
/* Can only handle horizontal BDF fonts. */
#define BDF_CANNOTHANDLEVERTICAL 4
/* Character found past end of BDF font. */
#define BDF_TOOMANYCHARACTERS	 5
/* BDF font is missing characters. */
#define BDF_TOOFEWCHARACTERS	 6
/* Error parsing BDF font. */
#define BDF_PARSEERROR		 7

/* A BDF character. */
typedef struct {
	char	      *name;
	int	      code;
	int	      dwx0, dwy0;
	int	      dwx1, dwy1;
	int	      bbw, bbh, bbxoff0x, bbyoff0y, wbytes;
	unsigned char *bits;
} BDF_Char;

/* A BDF font. */
typedef struct {
	int	 metricsSet, numChars;
	BDF_Char *chars;
	BDF_Char *code[256];
} BDF_Font;

/*
Function to put a pixel on the surface, it receives a pointer to the surface
(whatever format it may be), the x and y coordinates and the color.
*/
typedef void (*BDF_PutPixel)(void *, int, int, unsigned int);

/*
Function to read a byte, it receives an user defined void pointer and must
return a value in the range [0..255] or -1 to indicate EOF.
*/
typedef int (*BDF_ReadByte)(void *);

/*
Opens a BDF font, it receives the function that will produce the stream of
bytes, the user defined void pointer that will be passed to getbyte and a
pointer to an int that will receive the error code. Returns the BDF font.
*/
extern BDF_Font *BDF_OpenFont(BDF_ReadByte getbyte, void *info, int *error);
/*
Closes the font and frees all associated memory.
*/
extern void BDF_CloseFont(BDF_Font *font);
/*
Determines the size of the horizontal text, returns the width and height of the
smallest rectangle that can acomodate the rendered text and the start position
in x0 and y0 on where the text must be rendered to exactly fit the rectangle.
This is because the render functions take the y parameter as the baseline of
the text to allow different fonts (e.g. normal and italic) to be mixed in the
same line. It handles NULL pointers for pieces of information you don't want.
*/
extern void BDF_SizeH(BDF_Font *font, char *text, int *x0, int *y0, int *width, int *height);
/*
Same as above but accepts entities in the form &...;
*/
extern void BDF_SizeEntitiesH(BDF_Font *font, char *text, int *x0, int *y0, int *width, int *height);
/*
Draws the text at the given surface starting at position (x, y). It calls
putpixel with the surface, coordinates and color to draw the pixel (doesn't
clip). Returns the next x coordinate to continue to render more text. Only
accepts characters in the range [0..255].
*/
extern int BDF_DrawH(void *surface, BDF_PutPixel putpixel, BDF_Font *font, char *text, int x, int y, unsigned int color);
/*
Same as above but accepts entities in the form &...;
*/
extern int BDF_DrawEntitiesH(void *, BDF_PutPixel, BDF_Font *, char *, int, int, unsigned int);

#ifdef __cplusplus
};
#endif

#endif
