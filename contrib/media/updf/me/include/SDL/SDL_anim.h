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

#ifndef _SDLanim_h
#define _SDLanim_h

#include "SDL.h"
#include "begin_code.h"

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

struct SDL_Animation;
typedef struct SDL_Animation {
	SDL_Surface *surface;
	int frames, w, h;
	Uint32 duration;
	} SDL_Animation;

extern DECLSPEC struct SDL_Animation *Anim_Load( const char *file );
extern DECLSPEC void Anim_Free( SDL_Animation *anim );
extern DECLSPEC int Anim_GetFrameNum( SDL_Animation *anim, Uint32 start, Uint32 now );
extern DECLSPEC int Anim_BlitFrame( SDL_Animation *anim, Uint32 start, Uint32 now, SDL_Surface *dest, SDL_Rect *dr );
extern DECLSPEC void Anim_GetFrameRect( SDL_Animation *anim, int frame, SDL_Rect *rect );
extern DECLSPEC int Anim_BlitFrameNum( SDL_Animation *anim, int frame, SDL_Surface *dest, SDL_Rect *dr );
extern DECLSPEC int Anim_DisplayFormat( SDL_Animation *anim );
	
/* We'll use SDL for reporting errors */
#define Anim_SetError	SDL_SetError
#define Anim_GetError	SDL_GetError

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
};
#endif

#include "SDL/close_code.h"

#endif /* _SDL_anim_h */
