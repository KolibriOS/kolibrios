
/* Simple program:  Test bitmap blits */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"
#include "picture.xbm"

SDL_Surface *LoadXBM(SDL_Surface *screen, int w, int h, Uint8 *bits)
{
	SDL_Surface *bitmap;
	Uint8 *line;

	/* Allocate the bitmap */
	bitmap = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 1, 0, 0, 0, 0);
	if ( bitmap == NULL ) {
		fprintf(stderr, "Couldn't allocate bitmap: %s\n",
						SDL_GetError());
		return(NULL);
	}

	/* Copy the pixels */
	line = (Uint8 *)bitmap->pixels;
	w = (w+7)/8;
	while ( h-- ) {
		memcpy(line, bits, w);
		/* X11 Bitmap images have the bits reversed */
		{ int i, j; Uint8 *buf, byte;
			for ( buf=line, i=0; i<w; ++i, ++buf ) {
				byte = *buf;
				*buf = 0;
				for ( j=7; j>=0; --j ) {
					*buf |= (byte&0x01)<<j;
					byte >>= 1;
				}
			}
		}
		line += bitmap->pitch;
		bits += w;
	}
	return(bitmap);
}

void app_main(int argc, char *argv[])
{
	SDL_Surface *screen;
	SDL_Surface *bitmap;
	Uint8  video_bpp;
	Uint32 videoflags;
	Uint8 *buffer;
	int i, done;
	SDL_Event event;

	/* Initialize SDL */
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);

	video_bpp = 0;
	videoflags = SDL_SWSURFACE;
	/* Set 640x480 video mode */
	if ( (screen=SDL_SetVideoMode(640,480,video_bpp,videoflags)) == NULL ) {
		fprintf(stderr, "Couldn't set 640x480x%d video mode: %s\n",
						video_bpp, SDL_GetError());
		exit(2);
	}

	/* Set the surface pixels and refresh! */
	if ( SDL_LockSurface(screen) < 0 ) {
		fprintf(stderr, "Couldn't lock the display surface: %s\n",
							SDL_GetError());
		exit(2);
	}
	buffer=(Uint8 *)screen->pixels;
	for ( i=0; i<screen->h; ++i ) {
		memset(buffer,(i*255)/screen->h, screen->pitch);
		buffer += screen->pitch;
	}
	SDL_UnlockSurface(screen);
	SDL_UpdateRect(screen, 0, 0, 0, 0);

	/* Load the bitmap */
	bitmap = LoadXBM(screen, picture_width, picture_height,
					(Uint8 *)picture_bits);
	if ( bitmap == NULL ) {
		exit(1);
	}

	/* Wait for a keystroke */
	done = 0;
	while ( !done ) {
		/* Check for events */
		while ( SDL_PollEvent(&event) ) {
			switch (event.type) {
				case SDL_MOUSEBUTTONDOWN: {
					SDL_Rect dst;

					dst.x = event.button.x - bitmap->w/2;
					dst.y = event.button.y - bitmap->h/2;
					dst.w = bitmap->w;
					dst.h = bitmap->h;
					SDL_BlitSurface(bitmap, NULL,
								screen, &dst);
					SDL_UpdateRects(screen,1,&dst);
					}
					break;
				case SDL_KEYDOWN:
					/* Any key press quits the app... */
					done = 1;
					break;
				case SDL_QUIT:
					done = 1;
					break;
				default:
					break;
			}
		}
	}
	SDL_FreeSurface(bitmap);
	return(0);
}
