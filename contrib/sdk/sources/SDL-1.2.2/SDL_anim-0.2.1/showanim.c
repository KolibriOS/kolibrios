/*
	SDL_anim:  an animation library for SDL
	Copyright (C) 2001, 2002  Michael Leonhard

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
	
	Michael Leonhard
	mike@tamale.net
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>
#include "SDL_anim.h"

/* Draw a Gimpish background pattern to show transparency in the anim */
void draw_background( SDL_Surface *screen ) {
	Uint8 *dst = screen->pixels;
	int x, y;
	int bpp = screen->format->BytesPerPixel;
	Uint32 col[2];
	col[0] = SDL_MapRGB(screen->format, 0x66, 0x66, 0x66);
	col[1] = SDL_MapRGB(screen->format, 0x99, 0x99, 0x99);
	for(y = 0; y < screen->h; y++) {
		for(x = 0; x < screen->w; x++) {
			/* use an 8x8 checkerboard pattern */
			Uint32 c = col[((x ^ y) >> 3) & 1];
			switch(bpp) {
				case 1:
					dst[x] = c;
					break;
				case 2:
					((Uint16 *)dst)[x] = c;
					break;
				case 3:
					if(SDL_BYTEORDER == SDL_LIL_ENDIAN) {
						dst[x * 3] = c;
						dst[x * 3 + 1] = c >> 8;
						dst[x * 3 + 2] = c >> 16;
						}
					else {
						dst[x * 3] = c >> 16;
						dst[x * 3 + 1] = c >> 8;
						dst[x * 3 + 2] = c;
					}
					break;
				case 4:
					((Uint32 *)dst)[x] = c;
					break;
				}
			}
		dst += screen->pitch;
		}
	}

int app_main(int argc, char *argv[]) {
	SDL_Surface *screen;
	SDL_Animation *anim;
	SDL_Rect rect;
	SDL_Event event;
	SDL_KeyboardEvent *key;
	int depth, done;
	Uint32 start;
	
	argv[1]="ship.ani";
	/* Initialize the SDL library */
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		fprintf( stderr, "Couldn't initialize SDL: %s\n", SDL_GetError() );
		return 2;
		}

	/* Open the anim file */
	anim = Anim_Load(argv[1]);
	if( anim == NULL ) {
		fprintf( stderr, "Couldn't load %s: %s\n", argv[1], SDL_GetError() );
		SDL_Quit();
		return 3;
		}

	SDL_WM_SetCaption( argv[1], "showanim" );

	/* Create a display for the anim */
	depth = SDL_VideoModeOK( anim->w + 1, anim->h, 32, SDL_HWPALETTE );
	/* Use the deepest native mode, except that we emulate 32bpp for */
	/* viewing non-indexed anims on 8bpp screens */
	if( (anim->surface->format->BytesPerPixel > 1) && (depth == 8) ) {
		depth = 32;
		}
	screen = SDL_SetVideoMode( anim->w + 10, anim->h, 16, SDL_HWPALETTE );
	if( screen == NULL ) {
		fprintf( stderr, "Couldn't set %dx%dx%d video mode: %s\n",
				anim->w, anim->h, depth, SDL_GetError() );
		SDL_Quit();
		return 4;
		}

	/* Set the palette, if one exists */
	if( anim->surface->format->palette ) {
		SDL_SetColors( screen, anim->surface->format->palette->colors,
				0, anim->surface->format->palette->ncolors );
		}


	if( !Anim_DisplayFormat( anim ) ) {
		fprintf( stderr, "Anim_DisplayFormat() failed\n" );
		return 5;
		}

	done = 0;
	start = SDL_GetTicks();
	while( !done ) {
		while( SDL_PollEvent( &event ) ) {
			switch( event.type ) {
				case SDL_QUIT:
					done = 1;
					break;
				case SDL_KEYDOWN:
					key = (SDL_KeyboardEvent *)&event;
					if( key->keysym.sym == SDLK_ESCAPE ) done = 1;
					start = SDL_GetTicks();
					break;
				}
			}

		draw_background( screen );
		rect.x = 0;
		rect.y = 0;
		Anim_BlitFrame( anim, start, SDL_GetTicks(), screen, &rect );
		SDL_UpdateRect( screen, 0, 0, 0, 0 );
		SDL_Delay( 100 );
		}

	Anim_Free( anim );
	SDL_Quit();
	return 0;
	}
