/*!
  \file SDL_draw.c
  \author Mario Palomo Torrero <mpalomo@ihman.com>
  \author Jose M. de la Huerga Fernández
  \author Pepe González Mora
  \date 05-2002

  Drawing primitives for SDL. Main implementation file.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#include "SDL_draw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>  /*wmemset*/


/*==================== BEGIN of Draw_Pixel ======================*/

#define SDL_DRAW_BPP 1
#define SDL_DRAWFUNCTION  Draw_Pixel_1
#include "Draw_Pixel.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 2
#define SDL_DRAWFUNCTION  Draw_Pixel_2
#include "Draw_Pixel.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 3
#define SDL_DRAWFUNCTION  Draw_Pixel_3
#include "Draw_Pixel.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 4
#define SDL_DRAWFUNCTION  Draw_Pixel_4
#include "Draw_Pixel.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

static
void Draw_Pixel_error(SDL_Surface *super,
                      Sint16 x, Sint16 y, Uint32 color)
{
  SDL_printf("SDL_draw: Draw_Pixel ERROR!!."
                  " Have you call to SDL_Draw_Init()?\n");
  exit(-1);
}

void (*Draw_Pixel)(SDL_Surface *super,
                   Sint16 x, Sint16 y, Uint32 color) = Draw_Pixel_error;

/*===================== END of Draw_Pixel =======================*/

/*==================== BEGIN of Draw_Line ======================*/

#define SDL_DRAW_BPP 1
#define SDL_DRAWFUNCTION  Draw_Line_1
#include "Draw_Line.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 2
#define SDL_DRAWFUNCTION  Draw_Line_2
#include "Draw_Line.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 3
#define SDL_DRAWFUNCTION  Draw_Line_3
#include "Draw_Line.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 4
#define SDL_DRAWFUNCTION  Draw_Line_4
#include "Draw_Line.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

static
void Draw_Line_error(SDL_Surface *super,
                     Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,
                     Uint32 color)
{
  SDL_printf("SDL_draw: Draw_Line ERROR!!."
                  " Have you call to SDL_Draw_Init()?\n");
  exit(-1);
}

void (*Draw_Line)(SDL_Surface *super,
                  Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,
                  Uint32 color) = Draw_Line_error;

/*===================== END of Draw_Line =======================*/

/*=================== BEGIN of Draw_Circle =====================*/

#define SDL_DRAW_BPP 1
#define SDL_DRAWFUNCTION  Draw_Circle_1
#include "Draw_Circle.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 2
#define SDL_DRAWFUNCTION  Draw_Circle_2
#include "Draw_Circle.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 3
#define SDL_DRAWFUNCTION  Draw_Circle_3
#include "Draw_Circle.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 4
#define SDL_DRAWFUNCTION  Draw_Circle_4
#include "Draw_Circle.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

static
void Draw_Circle_error(SDL_Surface *super,
                       Sint16 x0, Sint16 y0, Uint16 r,
                       Uint32 color)
{
  SDL_printf("SDL_draw: Draw_Circle ERROR!!."
                  " Have you call to SDL_Draw_Init()?\n");
  exit(-1);
}


void (*Draw_Circle)(SDL_Surface *super,
                    Sint16 x0, Sint16 y0, Uint16 r,
                    Uint32 color) = Draw_Circle_error;

/*==================== END of Draw_Circle ======================*/

/*================= BEGIN of Draw_FillCircle ===================*/

#define SDL_DRAW_BPP 1
#define SDL_DRAWFUNCTION  Draw_FillCircle_1
#include "Draw_FillCircle.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 2
#define SDL_DRAWFUNCTION  Draw_FillCircle_2
#include "Draw_FillCircle.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 3
#define SDL_DRAWFUNCTION  Draw_FillCircle_3
#include "Draw_FillCircle.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 4
#define SDL_DRAWFUNCTION  Draw_FillCircle_4
#include "Draw_FillCircle.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

static
void Draw_FillCircle_error(SDL_Surface *super,
                           Sint16 x0, Sint16 y0, Uint16 r,
                           Uint32 color)
{
  SDL_printf("SDL_draw: Draw_FillCircle ERROR!!."
                  " Have you call to SDL_Draw_Init()?\n");
  exit(-1);
}


void (*Draw_FillCircle)(SDL_Surface *super,
                        Sint16 x0, Sint16 y0, Uint16 r,
                        Uint32 color) = Draw_FillCircle_error;

/*================== END of Draw_FillCircle ====================*/

/*=================== BEGIN of Draw_HLine =====================*/

#define SDL_DRAW_BPP 1
#define SDL_DRAWFUNCTION  Draw_HLine_1
#include "Draw_HLine.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 2
#define SDL_DRAWFUNCTION  Draw_HLine_2
#include "Draw_HLine.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 3
#define SDL_DRAWFUNCTION  Draw_HLine_3
#include "Draw_HLine.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 4
#define SDL_DRAWFUNCTION  Draw_HLine_4
#include "Draw_HLine.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

static
void Draw_HLine_error(SDL_Surface *super,
                      Sint16 x0,Sint16 y0, Sint16 x1,
                      Uint32 color)
{
  SDL_printf("SDL_draw: Draw_HLine ERROR!!."
                  " Have you call to SDL_Draw_Init()?\n");
  exit(-1);
}

void (*Draw_HLine)(SDL_Surface *super,
                   Sint16 x0,Sint16 y0, Sint16 x1,
                   Uint32 color) = Draw_HLine_error;

/*==================== END of Draw_HLine ======================*/

/*=================== BEGIN of Draw_VLine =====================*/
#define SDL_DRAW_BPP 1
#define SDL_DRAWFUNCTION  Draw_VLine_1
#include "Draw_VLine.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 2
#define SDL_DRAWFUNCTION  Draw_VLine_2
#include "Draw_VLine.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 3
#define SDL_DRAWFUNCTION  Draw_VLine_3
#include "Draw_VLine.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 4
#define SDL_DRAWFUNCTION  Draw_VLine_4
#include "Draw_VLine.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

static
void Draw_VLine_error(SDL_Surface *super,
                      Sint16 x0,Sint16 y0, Sint16 y1,
                      Uint32 color)
{
  SDL_printf("SDL_draw: Draw_VLine ERROR!!."
                  " Have you call to SDL_Draw_Init()?\n");
  exit(-1);
}


void (*Draw_VLine)(SDL_Surface *super,
                   Sint16 x0,Sint16 y0, Sint16 y1,
                   Uint32 color) = Draw_VLine_error;

/*==================== END of Draw_VLine ======================*/

/*==================== BEGIN of Draw_Rect ======================*/

#define SDL_DRAW_BPP 1
#define SDL_DRAWFUNCTION  Draw_Rect_1
#include "Draw_Rect.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 2
#define SDL_DRAWFUNCTION  Draw_Rect_2
#include "Draw_Rect.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 3
#define SDL_DRAWFUNCTION  Draw_Rect_3
#include "Draw_Rect.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 4
#define SDL_DRAWFUNCTION  Draw_Rect_4
#include "Draw_Rect.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

static
void Draw_Rect_error(SDL_Surface *super,
                  Sint16 x,Sint16 y, Uint16 w,Uint16 h,
                  Uint32 color)
{
  SDL_printf("SDL_draw: Draw_Rect ERROR!!."
                  " Have you call to SDL_Draw_Init()?\n");
  exit(-1);
}

void (*Draw_Rect)(SDL_Surface *super,
                  Sint16 x,Sint16 y, Uint16 w,Uint16 h,
                  Uint32 color) = Draw_Rect_error;

/*===================== END of Draw_Rect =======================*/

/*=================== BEGIN of Draw_Ellipse ====================*/

#define SDL_DRAW_BPP 1
#define SDL_DRAWFUNCTION  Draw_Ellipse_1
#include "Draw_Ellipse.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 2
#define SDL_DRAWFUNCTION  Draw_Ellipse_2
#include "Draw_Ellipse.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 3
#define SDL_DRAWFUNCTION  Draw_Ellipse_3
#include "Draw_Ellipse.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 4
#define SDL_DRAWFUNCTION  Draw_Ellipse_4
#include "Draw_Ellipse.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

static
void Draw_Ellipse_error(SDL_Surface *super,
                        Sint16 x0, Sint16 y0,
                        Uint16 Xradius, Uint16 Yradius,
                        Uint32 color)
{
  SDL_printf("SDL_draw: Draw_Ellipse ERROR!!."
                  " Have you call to SDL_Draw_Init()?\n");
  exit(-1);
}


void (*Draw_Ellipse)(SDL_Surface *super,
                     Sint16 x0, Sint16 y0,
                     Uint16 Xradius, Uint16 Yradius,
                     Uint32 color) = Draw_Ellipse_error;

/*==================== END of Draw_Ellipse =====================*/

/*================= BEGIN of Draw_FillEllipse ==================*/

#define SDL_DRAW_BPP 1
#define SDL_DRAWFUNCTION  Draw_FillEllipse_1
#include "Draw_FillEllipse.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 2
#define SDL_DRAWFUNCTION  Draw_FillEllipse_2
#include "Draw_FillEllipse.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 3
#define SDL_DRAWFUNCTION  Draw_FillEllipse_3
#include "Draw_FillEllipse.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 4
#define SDL_DRAWFUNCTION  Draw_FillEllipse_4
#include "Draw_FillEllipse.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

static
void Draw_FillEllipse_error(SDL_Surface *super,
                            Sint16 x0, Sint16 y0,
                            Uint16 Xradius, Uint16 Yradius,
                            Uint32 color)
{
  SDL_printf("SDL_draw: Draw_FillEllipse ERROR!!."
                  " Have you call to SDL_Draw_Init()?\n");
  exit(-1);
}


void (*Draw_FillEllipse)(SDL_Surface *super,
                         Sint16 x0, Sint16 y0,
                         Uint16 Xradius, Uint16 Yradius,
                         Uint32 color) = Draw_FillEllipse_error;

/*================== END of Draw_FillEllipse ===================*/

/*==================== BEGIN of Draw_Round =====================*/

#define SDL_DRAW_BPP 1
#define SDL_DRAWFUNCTION  Draw_Round_1
#include "Draw_Round.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 2
#define SDL_DRAWFUNCTION  Draw_Round_2
#include "Draw_Round.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 3
#define SDL_DRAWFUNCTION  Draw_Round_3
#include "Draw_Round.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 4
#define SDL_DRAWFUNCTION  Draw_Round_4
#include "Draw_Round.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

static
void Draw_Round_error(SDL_Surface *super,
                      Sint16 x0,Sint16 y0, Uint16 w,Uint16 h,
                      Uint16 corner, Uint32 color)
{
  SDL_printf("SDL_draw: Draw_Round ERROR!!."
                  " Have you call to SDL_Draw_Init()?\n");
  exit(-1);
}

void (*Draw_Round)(SDL_Surface *super,
                   Sint16 x0,Sint16 y0, Uint16 w,Uint16 h,
                   Uint16 corner, Uint32 color) = Draw_Round_error;

/*===================== END of Draw_Round ======================*/

/*================== BEGIN of Draw_FillRound ===================*/

#define SDL_DRAW_BPP 1
#define SDL_DRAWFUNCTION  Draw_FillRound_1
#include "Draw_FillRound.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 2
#define SDL_DRAWFUNCTION  Draw_FillRound_2
#include "Draw_FillRound.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 3
#define SDL_DRAWFUNCTION  Draw_FillRound_3
#include "Draw_FillRound.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

#define SDL_DRAW_BPP 4
#define SDL_DRAWFUNCTION  Draw_FillRound_4
#include "Draw_FillRound.c"
#undef SDL_DRAWFUNCTION
#undef SDL_DRAW_BPP

static
void Draw_FillRound_error(SDL_Surface *super,
                          Sint16 x0,Sint16 y0, Uint16 w,Uint16 h,
                          Uint16 corner, Uint32 color)
{
  SDL_printf("SDL_draw: Draw_FillRound ERROR!!."
                  " Have you call to SDL_Draw_Init()?\n");
  exit(-1);
}

void (*Draw_FillRound)(SDL_Surface *super,
                       Sint16 x0,Sint16 y0, Uint16 w,Uint16 h,
                       Uint16 corner, Uint32 color) = Draw_FillRound_error;

/*=================== END of Draw_FillRound ====================*/


/*Assignment of function pointers:*/
#define SDL_DRAW_FUNCTIONS_BPP(x) \
      Draw_Pixel       = Draw_Pixel_##x;       \
      Draw_Line        = Draw_Line_##x;        \
      Draw_Circle      = Draw_Circle_##x;      \
      Draw_FillCircle  = Draw_FillCircle_##x;  \
      Draw_HLine       = Draw_HLine_##x;       \
      Draw_VLine       = Draw_VLine_##x;       \
      Draw_Rect        = Draw_Rect_##x;        \
      Draw_Ellipse     = Draw_Ellipse_##x;     \
      Draw_FillEllipse = Draw_FillEllipse_##x; \
      Draw_Round       = Draw_Round_##x;       \
      Draw_FillRound   = Draw_FillRound_##x

/*IMPORTANT: Call this function AFTER the call to 'SDL_SetVideoMode'*/
void Draw_Init(void)
{
  SDL_Surface *screen = SDL_GetVideoSurface();
  if (!screen) {
    SDL_printf("SDL_draw: SDL_Draw_Init ERROR!!."
                  " Video Surface not found\n");
    exit(-2);
  }


  switch(screen->format->BytesPerPixel) {
    case 1:
      SDL_DRAW_FUNCTIONS_BPP(1);
    break;

    case 2:
      SDL_DRAW_FUNCTIONS_BPP(2);
    break;

    case 3:
      SDL_DRAW_FUNCTIONS_BPP(3);
    break;

    case 4:
      SDL_DRAW_FUNCTIONS_BPP(4);
    break;
  }/*switch*/

}/*Draw_Init*/

#undef SDL_DRAW_FUNCTIONS_BPP

