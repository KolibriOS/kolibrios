/*
showbdf - shows a BDF font on the screen
Placed in the public domain by Andre de Leiradella on 21-jan-2003.

You'll need SDL, SDLmain and SDL_gfxPrimitives to compile this program.
*/

#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include <SDL.h>
#include <SDL_main.h>
#include "SDL_bdf.h"
#include "SDL_gfxPrimitives.h"

/* Reads a byte from a rwops. */
static int ReadByteRwops(void *info) {
	unsigned char b;

	if (SDL_RWread((SDL_RWops *)info, &b, 1, 1) != 1)
		return -1;
	return b;
}

/* Put a pixel on a SDL surface. */
static void PutPixel(void *surface, int x, int y, unsigned int color) {
	pixelColor((SDL_Surface *)surface, (Sint16)x, (Sint16)y, (Uint32)color);
}

/* Put a pixel on a b&w surface with 1 bit per pixel. The first 4 bytes hold the width of the surface. */
static void PutBWPixel(void *surface, int x, int y, unsigned int color) {
	(void)color;
	y *= *(int *)surface;
	((unsigned char *)surface)[y + x / 8 + sizeof(int)] |= 1 << (x & 7);
}

/*
Example showing how to simulate anti-aliased text reducing the size of an
image. This function will render a text to a new SDL_Surface.
*/
static SDL_Surface *DrawAAH(BDF_Font *font, char *text, int entities, Uint8 r0, Uint8 g0, Uint8 b0, Uint8 r, Uint8 g, Uint8 b) {
	SDL_Surface   *surface;
	int	      x0, y0, w, h, a;
	unsigned char *pixels, *aux, *endimage, *endline;
	static Uint8  alpha[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

	if (entities)
		BDF_SizeEntitiesH(font, text, &x0, &y0, &w, &h);
	else
		BDF_SizeH(font, text, &x0, &y0, &w, &h);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	surface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, (w + 3) / 4, (h + 3) / 4, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
#else
	surface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, (w + 3) / 4, (h + 3) / 4, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
#endif
	if (surface == NULL)
		return NULL;
	boxRGBA(surface, 0, 0, surface->w - 1, surface->h - 1, r0, g0, b0, 255);
	w = (w + 7) / 8;
	h = ((h + 3) / 4) * 4;
	a = w * h * + sizeof(int);
	pixels = (unsigned char *)malloc(a);
	if (pixels == NULL) {
		SDL_OutOfMemory();
		SDL_FreeSurface(surface);
		return NULL;
	}
	memset(pixels, 0, a);
	*(int *)pixels = w;
	if (entities)
		BDF_DrawEntitiesH(pixels, PutBWPixel, font, text, x0, y0, 0);
	else
		BDF_DrawH(pixels, PutBWPixel, font, text, x0, y0, 0);
	aux = pixels + sizeof(int);
	endimage = aux + w * h;
	for (y0 = 0; aux < endimage; y0++, aux += w * 3)
		for (endline = aux + w, x0 = 0; aux < endline; x0 += 2, aux++) {
			a = alpha[aux[0] & 15];
			a += alpha[aux[w] & 15];
			a += alpha[aux[w * 2] & 15];
			a += alpha[aux[w * 3] & 15];
			pixelRGBA(surface, x0, y0, r, g, b, a * 255 / 16);
			a = alpha[(aux[0] >> 4) & 15];
			a += alpha[(aux[w] >> 4) & 15];
			a += alpha[(aux[w * 2] >> 4) & 15];
			a += alpha[(aux[w * 3] >> 4) & 15];
			pixelRGBA(surface, x0 + 1, y0, r, g, b, a * 255 / 16);
		}
	free(pixels);
	return surface;
}

/* Calls DrawAAH to render the text without support to entities. */
static SDL_Surface *RenderAAH(BDF_Font *font, char *text, Uint8 r0, Uint8 g0, Uint8 b0, Uint8 r, Uint8 g, Uint8 b) {
	return DrawAAH(font, text, 0, r0, g0, b0, r, g, b);
}

/* Calls DrawAAH to render the text with support to entities. */
static SDL_Surface *RenderEntitiesAAH(BDF_Font *font, char *text, Uint8 r0, Uint8 g0, Uint8 b0, Uint8 r, Uint8 g, Uint8 b) {
	return DrawAAH(font, text, 1, r0, g0, b0, r, g, b);
}

int main(int argc, char *argv[]) {
	SDL_RWops   *rwops;
	BDF_Font    *font;
	SDL_Surface *screen, *aatext;
	int	    x0, y0, w, h;
	SDL_Rect    pos;
	SDL_Event   event;
	char	    *text = "The quick fox jumped over the lazy dog";

	if (argc != 2) {
		fprintf(stderr, "Usage: showbdf <file.bdf>\n");
		return -1;
	}
	/* Reads the font. */
	rwops = SDL_RWFromFile(argv[1], "rb");
	font = BDF_OpenFont(ReadByteRwops, (void *)rwops, &w);
	SDL_RWclose(rwops);
	/* Check for error code. */
	switch (w) {
		case BDF_MEMORYERROR:
			fprintf(stderr, "Not enough memory reading BDF font\n");
			return -1;
		case BDF_READERROR:
			fprintf(stderr, "Error while reading BDF font\n");
			return -1;
		case BDF_WRONGVERSION:
			fprintf(stderr, "Wrong BDF font version, can only handle versions up to 2.2\n");
			return -1;
		case BDF_CANNOTHANDLEVERTICAL:
			fprintf(stderr, "Wrong BDF font direction, can only handle horizontal direction\n");
			return -1;
		case BDF_TOOMANYCHARACTERS:
		case BDF_TOOFEWCHARACTERS:
		case BDF_PARSEERROR:
			fprintf(stderr, "Invalid BDF font\n");
			return -1;
	}
	/* Init SDL. */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return -1;
	}
	atexit(SDL_Quit);
	/* Check the size of the image required to acomodate the rendered text. */
	BDF_SizeH(font, text, &x0, &y0, &w, &h);
	/* Set the video a little bit bigger than the minimum. */
	screen = SDL_SetVideoMode(w + 10, h + 15 + (h + 3) / 4, 0, SDL_SWSURFACE);
	if (screen == NULL) {
		fprintf(stderr, "Couldn't set %dx%d video mode: %s\n", w + 10, h + 15 + (h + 3) / 4, SDL_GetError());
		return -1;
	}
	/* Clear the screen to white. */
	boxColor(screen, 0, 0, screen->w - 1, screen->h - 1, 0xFFFFFFFF);
	/* Render the text to the screen with black. */
	BDF_DrawH((void *)screen, PutPixel, font, text, x0 + 5, y0 + 5, 0x000000FF);
	/* Render the same text with white blackgorund and black foreground to a new surface. */
	aatext = RenderAAH(font, text, 255, 255, 255, 0, 0, 0);
	if (aatext == NULL) {
		fprintf(stderr, "Couldn't render anti-aliased text: %s\n", SDL_GetError());
		return -1;
	}
	/* Blit it to the screen. */
	pos.x = (screen->w - aatext->w) / 2;
	pos.y = h + 5 + aatext->h / 2;
	SDL_BlitSurface(aatext, NULL, screen, &pos);
	SDL_FreeSurface(aatext);
	/* Update the screen. */
	SDL_UpdateRect(screen, 0, 0, 0, 0);
	/* Wait for something to happen... */
	for(;;) {
		if (SDL_WaitEvent(&event) < 0) {
			fprintf(stderr, "SDL_PullEvent() error: %s\n", SDL_GetError());
			return -1;
		}
		switch (event.type) {
			case SDL_MOUSEBUTTONDOWN:
			case SDL_KEYDOWN:
			case SDL_QUIT:
				BDF_CloseFont(font);
				return 0;
		}
	}
}
