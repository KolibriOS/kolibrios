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

//#include "SDL.h"

//#include "m_swap.h"
#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct SURFACE
{
	int w, h;		
	int pitch;		
	unsigned char *pixels;	
	int offset;		
} SURFACE;
 
SURFACE screen;

// Fake mouse handling.
boolean		grabMouse;

// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
static int	multiply=2;

void WinError(char *msg);


int BPP;
byte *hicolortable;
short hicolortransmask1,hicolortransmask2;
int		X_width;
int		X_height;
static int disableVerticalMouse = 0;
static int closed=0;
static int windowActive = 0;

HWND win;
static HINSTANCE inst;
static HDC dibDC;
static LOGPALETTE *palette;
static HPALETTE dibPal;
BITMAPINFO *bminfo;
static unsigned char *dibData;
static int bits8;
int palette_color[256];


static int	lastmousex = 0;
static int	lastmousey = 0;
boolean		mousemoved = false;
boolean		shmFinished;



//
//  Translates the key 
//

/*******
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
	
      case 'z':
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
**********/

void I_ShutdownGraphics(void)
{
 // SDL_Quit();
}



//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?

}

/* This processes SDL events */
/*****
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
      case SDL_QUIT:
	I_Quit();
	break;
    }
}

*******/




void I_GetEvent(void)
{
    MSG msg;
    POINT point;
    static LONG prevX, prevY;
    static int hadMouse = 0;
    event_t event;
    RECT rect;
    int lb, rb;
    static int prevlb = 0, prevrb = 0;
    
    /* Dispatch all messages: */
    while ( PeekMessage(&msg, NULL, 0, 0xFFFFFFFF, PM_REMOVE) )
    {
        TranslateMessage (&msg) ;
        DispatchMessage (&msg) ;
    }

    /* Check mouse and generate events if necessary: */
    if ( !GetCursorPos(&point) )
        WinError("GetCursorPos() failed");
    if ( hadMouse && windowActive)
    {
        lb = (GetAsyncKeyState(VK_LBUTTON) < 0);
        rb = (GetAsyncKeyState(VK_RBUTTON) < 0);
            
        if ( (prevX != point.x) || (prevY != point.y) ||
             (prevlb != lb) || (prevrb != rb) )
        {
            event.type = ev_mouse;
            event.data1 = lb | (rb << 1);
            event.data2 = (point.x - prevX)*9;
            if ( disableVerticalMouse )
                event.data3 = 0;
            else
                event.data3 = (prevY - point.y)*9;
            prevX = point.x;
            prevY = point.y;
            prevlb = lb;
            prevrb = rb;
            D_PostEvent(&event);
        }

        if ( grabMouse )
        {
            GetWindowRect(win, &rect);
            if ( !SetCursorPos((rect.left + rect.right) / 2,
                               (rect.top + rect.bottom) / 2) )
                WinError("SetCursorPos() failed");
            prevX = (rect.left + rect.right) / 2;
            prevY = (rect.top + rect.bottom) / 2;
        }
    }
    else
    {
        prevX = point.x;
        prevY = point.y;
        hadMouse = 1;
    }        
}



//
// I_StartTic
//
void I_StartTic (void)
{
//    SDL_Event Event;

//    while ( SDL_PollEvent(&Event) )
//	I_GetEvent(&Event);
    I_GetEvent();

}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
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

typedef struct SDL_Color
{
	byte r;
	byte g;
	byte b;
	byte unused;
} SDL_Color; 

SDL_Color colors[256];

void I_SetPalette (byte* palette)
{
    int i;
    RGBQUAD *rgb;

// 
//    for ( i=0; i<256; ++i ) {
//	colors[i].r = gammatable[usegamma][*palette++];
//	colors[i].g = gammatable[usegamma][*palette++];
//	colors[i].b = gammatable[usegamma][*palette++];
//	colors[i].unused = 0;
//    }
//    SDL_SetColors(screen, colors, 0, 256);

    rgb = bminfo->bmiColors;
    for ( i = 0; i < 256; i++ )
    {
      rgb->rgbRed = gammatable[usegamma][*palette++];
      rgb->rgbGreen = gammatable[usegamma][*palette++];
      rgb->rgbBlue = gammatable[usegamma][*palette++];
      rgb->rgbReserved = 0;
      rgb++;
    };


}



int makecol(int r, int g, int b)
{
  //  assert(BPP==2);
    return (b >> 3) | ((g >> 3) << 5) | ((r >> 3) << 10);
}

int TranslateKey(unsigned k)
{
/*wtf?    if ( (k >= VK_0) && (k <= VK_9) )*/
    if ( (k >= 0x30) && (k <= 0x39) )
        return (k - 0x30 + '0');
    if ( (k >= 0x41) && (k <= 0x5a) )
        return (k - 0x41 + 'a');

#define K(a,b) case a: return b;    
    switch ( k )
    {
        K(VK_LEFT, KEY_LEFTARROW);
        K(VK_RIGHT, KEY_RIGHTARROW);
        K(VK_UP, KEY_UPARROW);
        K(VK_DOWN, KEY_DOWNARROW);
        K(VK_BACK, KEY_BACKSPACE);
        K(VK_TAB, KEY_TAB);
        K(VK_RETURN, KEY_ENTER);
        K(VK_SHIFT, KEY_RSHIFT);
        K(VK_CONTROL, KEY_RCTRL);
        K(VK_MENU, KEY_RALT);
        K(VK_PAUSE, KEY_PAUSE);
        K(VK_ESCAPE, KEY_ESCAPE);
        K(VK_SPACE, ' ');
        K(VK_DELETE, KEY_BACKSPACE);
        K(VK_ADD, '+');
        K(VK_SUBTRACT, KEY_MINUS);
        K(0xBC, ',');
        K(0xBE, '.');
        K(VK_F1, KEY_F1);
        K(VK_F2, KEY_F2);
        K(VK_F3, KEY_F3);
        K(VK_F4, KEY_F4);
        K(VK_F5, KEY_F5);
        K(VK_F6, KEY_F6);
        K(VK_F7, KEY_F7);
        K(VK_F8, KEY_F8);
        K(VK_F9, KEY_F9);
        K(VK_F10, KEY_F10);
        K(VK_F11, KEY_F11);
        K(VK_F12, KEY_F12);
    }

    return 0;
}

void WinError(char *msg)
{
    printf("Windows Error: %s, GetLastError(): %u\n", msg, GetLastError());
    exit(EXIT_FAILURE);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message,
                                WPARAM wparam, LPARAM lparam);

void BlitDIB(void)
{
    RECT rect;

    GetClientRect(win, &rect);
    if ( StretchDIBits(dibDC, rect.left, rect.top, rect.right-rect.left,
                       rect.bottom-rect.top, 0, 0, SCREENWIDTH*2,
                       SCREENHEIGHT*2, dibData, bminfo, DIB_RGB_COLORS,
                         SRCCOPY)
             == GDI_ERROR )
          WinError("StrecthDIBits failed");
        
    GdiFlush();    
}
 

void I_InitGraphics(void)
{
    static int firsttime=1;
    
    WNDCLASS wc;
    unsigned i, x, y, j;
    WORD *d;
    unsigned char *b;
    int bits;
    int frameX, frameY, capY;
    RECT rect;
    int width, height;
    RGBQUAD *rgb;
    int retval;

    if (!firsttime)
	return;
    firsttime = 0;

    if (M_CheckParm("-2"))
	multiply = 2;

    if (M_CheckParm("-3"))
	multiply = 3;

    if (M_CheckParm("-4"))
	multiply = 4;

    X_width = SCREENWIDTH * multiply;
    X_height = SCREENHEIGHT * multiply;

    // check if the user wants to grab the mouse (quite unnice)
    grabMouse = !!M_CheckParm("-grabmouse");

    /* [Petteri] New: Option to disable mouse vertical movement - useful
       for players used to Quake: */
    disableVerticalMouse = !!M_CheckParm("-novertmouse");

    /* Build and initialize the window: */
    
    inst = (HINSTANCE) GetModuleHandle(NULL);

    frameX = GetSystemMetrics(SM_CXFRAME);
    frameY = GetSystemMetrics(SM_CYFRAME);
    capY = GetSystemMetrics(SM_CYCAPTION);

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = inst;
    wc.hIcon = NULL;
    if ( grabMouse )
        wc.hCursor = LoadCursor( 0, IDC_ARROW );
    else
        wc.hCursor = LoadCursor( 0, IDC_ARROW );
    /*wc.hbrBackground = GetStockObject( WHITE_BRUSH );*/
    wc.hbrBackground = NULL;    
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "DoomWindowClass";

    retval= RegisterClass(&wc);

    width = X_width + 2*frameX;
    height = X_height + 2*frameY + capY;

    win = CreateWindow("DoomWindowClass", "NTDOOM",
                       WS_OVERLAPPEDWINDOW | WS_VISIBLE, 200, 200, width, height,
                       NULL, NULL, inst, NULL);

    /* Display the window: */
    ShowWindow(win, SW_SHOW);
    UpdateWindow(win);

    GetClientRect(win, &rect);
    fprintf(stderr, "I_InitGraphics: Client area: %ux%u\n",
            rect.right-rect.left, rect.bottom-rect.top);

    if ( (rect.right-rect.left) != X_width )
    {
        fprintf(stderr, "I_InitGraphics: Fixing width\n");
        width += X_width - (rect.right-rect.left);
        MoveWindow(win, 0, 0, width, height, TRUE);
    }
    if ( (rect.bottom-rect.top) != X_height )
    {
        fprintf(stderr, "I_InitGraphics: Fixing height\n");
        height += X_height - (rect.bottom-rect.top);
        MoveWindow(win, 0, 0, width, height, TRUE);
    }

    GetClientRect(win, &rect);
    fprintf(stderr, "I_InitGraphics: Client area: %ux%u\n",
            rect.right-rect.left, rect.bottom-rect.top);    
        
    dibDC = GetDC(win);
    BPP=1;
    bits = 8; //GetDeviceCaps(dibDC, BITSPIXEL);
    fprintf(stderr, "I_InitGraphics: %i bpp screen\n", bits);

    if ( BPP == 1 )
        bminfo = malloc(sizeof(BITMAPINFOHEADER) + 4*256);
    else
        bminfo = malloc(sizeof(BITMAPINFOHEADER));
        

    if ( BPP == 1 )
    {
       rgb = bminfo->bmiColors;
       for ( i = 0; i < 256; i++ )
       {
          rgb->rgbRed = i;
          rgb->rgbGreen = i;
          rgb->rgbBlue = i;
          rgb->rgbReserved = 0;
          rgb++;
       }
    }
    
    bminfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bminfo->bmiHeader.biWidth = 640; X_width;
    bminfo->bmiHeader.biHeight = -400 ;X_height;
    bminfo->bmiHeader.biPlanes = 1;
    if ( BPP == 1 )
        bminfo->bmiHeader.biBitCount = 8;
    else
        bminfo->bmiHeader.biBitCount = 16;
    bminfo->bmiHeader.biCompression = BI_RGB;
    bminfo->bmiHeader.biSizeImage = 0;
    bminfo->bmiHeader.biXPelsPerMeter = 0;
    bminfo->bmiHeader.biYPelsPerMeter = 0;
    bminfo->bmiHeader.biClrUsed = 0;
    bminfo->bmiHeader.biClrImportant = 0;
    
    dibData = malloc(640*400*BPP);

//    BlitDIB();

    screen.pixels=(unsigned char *) (dibData);
    screen.h=400;
    screen.w=640;
    screen.pitch=640;
       
  //  screens[0] = malloc(320*200);

    /* Build magic highcolor table: */
    if (BPP==2)
    {
        byte *tempptr, *tempptr2;

        tempptr=hicolortable=(byte *)malloc(256*32*9);
        
        for (i=0;i<32;i++)
        {
            for (j=0;j<256;j++)
            {
                *tempptr=j*gammatable[3][i*(256/32)]/256;
                tempptr++;
            }
        }
        for (i=1;i<=8;i++)
        {
            tempptr2=hicolortable;
            for (j=0;j<(256*32);j++)
            {
                *tempptr=(byte)(((int)(*tempptr2))*(8-i)/8);
                tempptr++; tempptr2++;
            }
        }
        hicolortransmask1=makecol(127,127,127);
        hicolortransmask2=makecol(63,63,63);
    }    
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message,
                                WPARAM wparam, LPARAM lparam)
{
    event_t event;
    RECT rect;
    
    switch ( message )
    {        
        case WM_DESTROY:
            if ( grabMouse )
            {
                ClipCursor(NULL);
                ShowCursor(TRUE);
            }
            fprintf(stderr, "WM_DESTROY\n");
            PostQuitMessage(0);
            closed = 1;
            break;

        case WM_MOVE:
            GetWindowRect(win, &rect);
            fprintf(stderr, "%u,%u - %u, %u\n",
                    rect.left,rect.top,rect.right,rect.bottom);
            ClipCursor(&rect);
            break;

        case WM_ACTIVATE:
            fprintf(stderr, "WM_ACTIVATE %u\n", (unsigned) LOWORD(wparam));
            if ( LOWORD(wparam) )
            {
                if ( !windowActive )
                {
                    if ( grabMouse )
                    {
                        ClipCursor(NULL); /* helps with Win95? */
                        GetWindowRect(win, &rect);
                        fprintf(stderr, "%u,%u - %u, %u\n",
                                rect.left,rect.top,rect.right,rect.bottom);
                        ClipCursor(&rect);
                        ShowCursor(FALSE);
                    }                    
                }
                windowActive = 1;
                if ( bits8 )
                {
                    if ( SetPaletteEntries(dibPal, 0, 256, palette->palPalEntry) != 256 )
                        WinError("SetPaletteEntries failed");
                    if ( !UnrealizeObject(dibPal) )
                        WinError("UnrealizeObject failed");
                    if ( SelectPalette(dibDC, dibPal, FALSE) == NULL )
                        WinError("SelectPalette failed");
                }
            }
            else
            {
                if ( grabMouse )
                {
                    ClipCursor(NULL);
                    ShowCursor(TRUE);
                }
                windowActive = 0;
            }
            return DefWindowProc(hwnd, message, wparam, lparam);

        case WM_KEYDOWN:
            event.type = ev_keydown;
            event.data1 = TranslateKey(wparam);
            if ( event.data1 != 0 )
                D_PostEvent(&event);
            break;

        case WM_KEYUP:
            event.type = ev_keyup;
            event.data1 = TranslateKey(wparam);
            if ( event.data1 != 0 )
                D_PostEvent(&event);
            break;            
            
        default:
            return(DefWindowProc(hwnd, message, wparam, lparam));
    }

    return 0;
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
    { i = I_GetTime();
	
	  tics = i - lasttic;
	  lasttic = i;
	  if (tics > 20)
	    tics = 20;

	  for (i=0 ; i<tics*2 ; i+=2)
	    screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0xff;
	  for ( ; i<20*2 ; i+=2)
	    screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0x0;
    }

    // scales the screen size before blitting it
    if (multiply == 1)
    {
	  unsigned char *olineptr;
	  unsigned char *ilineptr;
	  int y;

	  ilineptr = (unsigned char *) screens[0];
	  olineptr = (unsigned char *) screen.pixels;

	  y = SCREENHEIGHT;
	  while (y--)
	  {
	    memcpy(olineptr, ilineptr, screen.w);
	    ilineptr += SCREENWIDTH;
	    olineptr += screen.pitch;
	  }
    }
    else if (multiply == 2)
    {
	unsigned int *olineptrs[2];
	unsigned int *ilineptr;
	int x, y, i;
	unsigned int twoopixels;
	unsigned int twomoreopixels;
	unsigned int fouripixels;

	ilineptr = (unsigned int *) (screens[0]);
	for (i=0 ; i<2 ; i++) {
	    olineptrs[i] =
		(unsigned int *)&((byte *)screen.pixels)[i*screen.pitch];
        }

	y = SCREENHEIGHT;
	while (y--)
	{
	    x = SCREENWIDTH;
	    do
	    {
		fouripixels = *ilineptr++;
		twoopixels =	(fouripixels & 0xff000000)
		    |	((fouripixels>>8) & 0xffff00)
		    |	((fouripixels>>16) & 0xff);
		twomoreopixels =	((fouripixels<<16) & 0xff000000)
		    |	((fouripixels<<8) & 0xffff00)
		    |	(fouripixels & 0xff);
		*olineptrs[0]++ = twomoreopixels;
		*olineptrs[1]++ = twomoreopixels;
		*olineptrs[0]++ = twoopixels;
		*olineptrs[1]++ = twoopixels;
	    } while (x-=4);
	    olineptrs[0] += screen.pitch/4;
	    olineptrs[1] += screen.pitch/4;
	}

    }
    else if (multiply == 3)
    {
	unsigned int *olineptrs[3];
	unsigned int *ilineptr;
	int x, y, i;
	unsigned int fouropixels[3];
	unsigned int fouripixels;

	ilineptr = (unsigned int *) (screens[0]);
	for (i=0 ; i<3 ; i++) {
	    olineptrs[i] = 
		(unsigned int *)&((byte *)screen.pixels)[i*screen.pitch];
        }

	y = SCREENHEIGHT;
	while (y--)
	{
	    x = SCREENWIDTH;
	    do
	    {
		fouripixels = *ilineptr++;
		fouropixels[0] = (fouripixels & 0xff000000)
		    |	((fouripixels>>8) & 0xff0000)
		    |	((fouripixels>>16) & 0xffff);
		fouropixels[1] = ((fouripixels<<8) & 0xff000000)
		    |	(fouripixels & 0xffff00)
		    |	((fouripixels>>8) & 0xff);
		fouropixels[2] = ((fouripixels<<16) & 0xffff0000)
		    |	((fouripixels<<8) & 0xff00)
		    |	(fouripixels & 0xff);
		*olineptrs[0]++ = fouropixels[2];
		*olineptrs[1]++ = fouropixels[2];
		*olineptrs[2]++ = fouropixels[2];
		*olineptrs[0]++ = fouropixels[1];
		*olineptrs[1]++ = fouropixels[1];
		*olineptrs[2]++ = fouropixels[1];
		*olineptrs[0]++ = fouropixels[0];
		*olineptrs[1]++ = fouropixels[0];
		*olineptrs[2]++ = fouropixels[0];
	    } while (x-=4);
	    olineptrs[0] += 2*screen.pitch/4;
	    olineptrs[1] += 2*screen.pitch/4;
	    olineptrs[2] += 2*screen.pitch/4;
	}

    }
   BlitDIB();
}




/**
void I_InitGraphics(void)
{

    static int	firsttime=1;
    int video_w, video_h, w, h;
    byte video_bpp;
    unsigned int video_flags;

    if (!firsttime)
	return;
    firsttime = 0;

    video_flags = (SDL_SWSURFACE|SDL_HWPALETTE);
    if (!!M_CheckParm("-fullscreen"))
        video_flags |= SDL_FULLSCREEN;

    if (M_CheckParm("-2"))
	multiply = 2;

    if (M_CheckParm("-3"))
	multiply = 3;

    // check if the user wants to grab the mouse (quite unnice)
    grabMouse = !!M_CheckParm("-grabmouse");

    video_w = w = SCREENWIDTH * multiply;
    video_h = h = SCREENHEIGHT * multiply;
    video_bpp = 8;

 
    if ( multiply > 3 ) {
        I_Error("Smallest available mode (%dx%d) is too large!",
						video_w, video_h);
    }
    screen = SDL_SetVideoMode(video_w, video_h, 8, video_flags);
    if ( screen == NULL ) {
        I_Error("Could not set %dx%d video mode: %s", video_w, video_h,
							SDL_GetError());
    }
    SDL_ShowCursor(0);
    SDL_WM_SetCaption("MenuetOS-DOOM", "doom");

     w = SCREENWIDTH * multiply;
    h = SCREENHEIGHT * multiply;
    if (multiply == 1 && !SDL_MUSTLOCK(screen) ) {
	screens[0] = (unsigned char *) screen->pixels;
    } else {
	screens[0] = (unsigned char *) malloc (SCREENWIDTH * SCREENHEIGHT);
        if ( screens[0] == NULL )
            I_Error("Couldn't allocate screen memory");
    }
}
*****/