// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for SDL library
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <stdlib.h>

#include "SDL.h"

#include "m_swap.h"
#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"


SDL_Surface *screen;

// DOOM always renders to a fixed 320x200 surface; SDL_SoftStretch scales
// it to the (arbitrary) window size each frame.
SDL_Surface *render;

// 24bpp truecolor twin of render: the 8->24 palette conversion is done once
// here at 320x200 (cheap), so the window-sized surface is truecolor - no
// per-pixel palette pass, and the display needs no 8->24 shadow conversion.
SDL_Surface *render24;

// Fake mouse handling.
boolean		grabMouse;


//
//  Translates the key 
//

int xlatekey(SDL_keysym *key)
{

    int rc;

    switch(key->sym)
    {
      case SDLK_LEFT:	rc = KEY_LEFTARROW;	break;
      case SDLK_RIGHT:	rc = KEY_RIGHTARROW;	break;
      case SDLK_DOWN:	rc = KEY_DOWNARROW;	break;
      case SDLK_UP:	rc = KEY_UPARROW;	break;
      case SDLK_ESCAPE:	rc = KEY_ESCAPE;	break;
      case SDLK_RETURN:	rc = KEY_ENTER;		break;
      case SDLK_TAB:	rc = KEY_TAB;		break;
      case SDLK_F1:	rc = KEY_F1;		break;
      case SDLK_F2:	rc = KEY_F2;		break;
      case SDLK_F3:	rc = KEY_F3;		break;
      case SDLK_F4:	rc = KEY_F4;		break;
      case SDLK_F5:	rc = KEY_F5;		break;
      case SDLK_F6:	rc = KEY_F6;		break;
      case SDLK_F7:	rc = KEY_F7;		break;
      case SDLK_F8:	rc = KEY_F8;		break;
      case SDLK_F9:	rc = KEY_F9;		break;
      case SDLK_F10:	rc = KEY_F10;		break;
      case SDLK_F11:	rc = KEY_F11;		break;
      case SDLK_F12:	rc = KEY_F12;		break;
	
      case SDLK_BACKSPACE:
      case SDLK_DELETE:	rc = KEY_BACKSPACE;	break;

      case SDLK_PAUSE:	rc = KEY_PAUSE;		break;

      case SDLK_EQUALS:	rc = KEY_EQUALS;	break;

      case SDLK_KP_MINUS:
      case SDLK_MINUS:	rc = KEY_MINUS;		break;

      case SDLK_LSHIFT:
      case SDLK_RSHIFT:
	rc = KEY_RSHIFT;
	break;
	
      case SDLK_LCTRL:
      case SDLK_RCTRL:
	rc = KEY_RCTRL;
	break;
	
      case SDLK_LALT:
      case SDLK_LMETA:
      case SDLK_RALT:
      case SDLK_RMETA:
	rc = KEY_RALT;
	break;
	
      default:
        rc = key->sym;
	break;
    }

    return rc;

}

void I_ShutdownGraphics(void)
{
  SDL_Quit();
}



//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?

}

/* This processes SDL events */
void I_GetEvent(SDL_Event *Event)
{
    Uint8 buttonstate;
    event_t event;

    switch (Event->type)
    {
      case SDL_KEYDOWN:
	event.type = ev_keydown;
	event.data1 = xlatekey(&Event->key.keysym);
	D_PostEvent(&event);
        break;

      case SDL_KEYUP:
	event.type = ev_keyup;
	event.data1 = xlatekey(&Event->key.keysym);
	D_PostEvent(&event);
	break;

      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
	buttonstate = SDL_GetMouseState(NULL, NULL);
	event.type = ev_mouse;
	event.data1 = 0
	    | (buttonstate & SDL_BUTTON(1) ? 1 : 0)
	    | (buttonstate & SDL_BUTTON(2) ? 2 : 0)
	    | (buttonstate & SDL_BUTTON(3) ? 4 : 0);
	event.data2 = event.data3 = 0;
	D_PostEvent(&event);
	break;

#if (SDL_MAJOR_VERSION >= 0) && (SDL_MINOR_VERSION >= 9)
      case SDL_MOUSEMOTION:
	/* Ignore mouse warp events */
	if ((Event->motion.x != screen->w/2)||(Event->motion.y != screen->h/2))
	{
	    /* Warp the mouse back to the center */
	    if (grabMouse) {
		SDL_WarpMouse(screen->w/2, screen->h/2);
	    }
	    event.type = ev_mouse;
	    event.data1 = 0
	        | (Event->motion.state & SDL_BUTTON(1) ? 1 : 0)
	        | (Event->motion.state & SDL_BUTTON(2) ? 2 : 0)
	        | (Event->motion.state & SDL_BUTTON(3) ? 4 : 0);
	    event.data2 = Event->motion.xrel << 2;
	    event.data3 = -Event->motion.yrel << 2;
	    D_PostEvent(&event);
	}
	break;
#endif

      case SDL_QUIT:
	I_Quit();
    }

}

//
// I_StartTic
//
void I_StartTic (void)
{
    SDL_Event Event;

    while ( SDL_PollEvent(&Event) )
	I_GetEvent(&Event);
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
}

//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{

    static int	lasttic;
    int		tics;
    int		i;

    // draws little dots on the bottom of the screen
    if (devparm)
    {

	i = I_GetTime();
	tics = i - lasttic;
	lasttic = i;
	if (tics > 20) tics = 20;

	for (i=0 ; i<tics*2 ; i+=2)
	    screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0xff;
	for ( ; i<20*2 ; i+=2)
	    screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0x0;
    }

    // The 8bpp frame is converted to truecolor once here at 320x200; every blit
    // below is 24bpp, so the window-sized surface gets no per-pixel palette pass
    // and the display needs no 8->24 shadow conversion on update.
    SDL_BlitSurface(render, NULL, render24, NULL);

    if ( SDL_MUSTLOCK(screen) ) {
        if ( SDL_LockSurface(screen) < 0 ) {
            return;
        }
    }
    // Stretch the whole 320x200 frame to the window. Aspect may not match the
    // window shape, but nothing is cropped and there are no seams.
    SDL_SoftStretch(render24, NULL, screen, NULL);
    if ( SDL_MUSTLOCK(screen) ) {
        SDL_UnlockSurface(screen);
    }
    SDL_UpdateRect(screen, 0, 0, 0, 0);
}


//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}


//
// I_SetPalette
//
void I_SetPalette (byte* palette)
{
    int i;
    SDL_Color colors[256];

    for ( i=0; i<256; ++i ) {
	colors[i].r = gammatable[usegamma][*palette++];
	colors[i].g = gammatable[usegamma][*palette++];
	colors[i].b = gammatable[usegamma][*palette++];
	colors[i].unused = 0;
    }
    SDL_SetColors(render, colors, 0, 256);
}


void I_InitGraphics(void)
{

    static int	firsttime=1;
    Uint16 video_w, video_h;
    Uint32 video_flags;
    int i;

    if (!firsttime)
	return;
    firsttime = 0;

    video_flags = SDL_SWSURFACE;
    if (!!M_CheckParm("-fullscreen"))
        video_flags |= SDL_FULLSCREEN;

    // check if the user wants to grab the mouse (quite unnice)
    grabMouse = !!M_CheckParm("-grabmouse");

    // Window size: "-winsize <w> <h>"; defaults to 2x of 320x200.
    video_w = SCREENWIDTH * 2;
    video_h = SCREENHEIGHT * 2;
    i = M_CheckParm("-winsize");
    if (i && i < myargc-2) {
	video_w = atoi(myargv[i+1]);
	video_h = atoi(myargv[i+2]);
    }

    // 24bpp truecolor window: the kolibri framebuffer is 24bpp, so a truecolor
    // surface avoids SDL's per-frame 8->24 shadow conversion across the whole
    // window (the 8->24 step is done once at 320x200 in I_FinishUpdate).
    screen = SDL_SetVideoMode(video_w, video_h, 24, video_flags);
    if ( screen == NULL ) {
        I_Error("Could not set %dx%d video mode: %s", video_w, video_h,
							SDL_GetError());
    }
    SDL_ShowCursor(0);
    SDL_WM_SetCaption("SDL DOOM! v1.11", "doom");

    /* DOOM renders to a fixed 320x200 paletted surface; each frame it is
       scaled to the window with SDL_SoftStretch (see I_FinishUpdate). */
    render = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREENWIDTH, SCREENHEIGHT,
                                  8, 0, 0, 0, 0);
    if ( render == NULL )
        I_Error("Cannot create render surface: %s", SDL_GetError());
    // 24bpp truecolor copy of render, scaled to the window (see I_FinishUpdate)
    render24 = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREENWIDTH, SCREENHEIGHT,
                                    screen->format->BitsPerPixel,
                                    screen->format->Rmask, screen->format->Gmask,
                                    screen->format->Bmask, screen->format->Amask);
    if ( render24 == NULL )
        I_Error("Cannot create render24 surface: %s", SDL_GetError());
    screens[0] = (unsigned char *) render->pixels;
}
