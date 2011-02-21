/*
SDL_flic - renders FLIC animations
Copyright (C) 2003 Andre de Leiradella

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

For information about SDL_flic contact leiradella@bigfoot.com

Version 1.0: first public release.
Version 1.1: fixed bug to set *error to FLI_OK when returning successfully from FLI_Open
             added function FLI_Reset to reset the animation to the first frame
Version 1.2: added function FLI_Skip to skip the current frame without rendering
             FLI_Animation->surface is now correctly locked and unlocked
             the rwops stream is now part of the FLI_Animation structure and is closed inside FLI_Close
             renamed FLI_Reset to FLI_Rewind
             added function FLI_Version that returns the library version
*/
#ifndef __SDL_flic_h__
#define __SDL_flic_h__

#include <SDL.h>
#include <setjmp.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Supported formats. */
#define FLI_FLI 0xAF11
#define FLI_FLC 0xAF12

/* Error codes. */

/* No error. */
#define FLI_OK            0
/* Error reading the file. */
#define FLI_READERROR     1
/* Invalid frame size (corrupted file). */
#define FLI_CORRUPTEDFILE 2
/* Error in SDL operation. */
#define FLI_SDLERROR      3
/* Out of memory. */
#define FLI_OUTOFMEMORY   4

/*
The animation structure, all members are read-only, don't try to longjmp to
error.
*/
typedef struct {
        Uint32      format, numframes, width, height, depth, delay, offframe1, nextframe, offnextframe;
        /* rwops is where the animation is read from. */
        SDL_RWops   *rwops;
        /* surface is where the frames is rendered to. */
        SDL_Surface *surface;
        /* error is used to longjmp in case of error so to avoid a chain of if's. */
        jmp_buf     error;
} FLI_Animation;

/*
Returns the library version in the format MAJOR << 16 | MINOR.
*/
extern int FLI_Version(void);
/*
Opens a FLIC animation and return a pointer to it. rwops is left at the same
point it was before the the call. error receives the result of the call.
*/
extern FLI_Animation *FLI_Open(SDL_RWops *rwops, int *error);
/*
Closes the animation, closes the stream and frees all used memory.
*/
extern void          FLI_Close(FLI_Animation *flic);
/*
Renders the next frame of the animation returning an int to indicate if it was
successfull or not.
*/
extern int           FLI_NextFrame(FLI_Animation *flic);
/*
Rewinds the animation to the first frame.
*/
extern int           FLI_Rewind(FLI_Animation *flic);
/*
Skips the current frame of the animation without rendering it.
*/
extern int           FLI_Skip(FLI_Animation *flic);

#ifdef __cplusplus
};
#endif

#endif
