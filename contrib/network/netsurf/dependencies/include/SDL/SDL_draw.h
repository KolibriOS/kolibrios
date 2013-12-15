/*!
  \file SDL_draw.h
  \author Mario Palomo Torrero <mpalomo@ihman.com>
  \author Jose M. de la Huerga Fernández
  \author Pepe González Mora
  \date 05-2002

  Drawing primitives for SDL. Main header file.

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
#ifndef SDL_DRAW_H
#define SDL_DRAW_H

#include "SDL.h"
#include "begin_code.h"

#ifdef __cplusplus
extern "C" {
#endif


extern DECLSPEC void Draw_Init(void);

extern DECLSPEC
void (*Draw_Pixel)(SDL_Surface *super,
                   Sint16 x, Sint16 y, Uint32 color);

extern DECLSPEC
void (*Draw_Line)(SDL_Surface *super,
                  Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,
                  Uint32 color);

extern DECLSPEC
void (*Draw_Circle)(SDL_Surface *super,
                    Sint16 x0, Sint16 y0, Uint16 r,
                    Uint32 color);

extern DECLSPEC
void (*Draw_FillCircle)(SDL_Surface *super,
                        Sint16 x0, Sint16 y0, Uint16 r,
                        Uint32 color);

extern DECLSPEC
void (*Draw_HLine)(SDL_Surface *super,
                      Sint16 x0,Sint16 y0, Sint16 x1,
                      Uint32 color);

extern DECLSPEC
void (*Draw_VLine)(SDL_Surface *super,
                      Sint16 x0,Sint16 y0, Sint16 y1,
                      Uint32 color);

extern DECLSPEC
void (*Draw_Rect)(SDL_Surface *super,
                  Sint16 x,Sint16 y, Uint16 w,Uint16 h,
                  Uint32 color);

/* We wrap SDL_FillRect with the SDL_draw name convention */
#define Draw_FillRect(SUPER, X, Y, W, H, COLOR) \
  do {                                          \
    SDL_Rect r = {(X), (Y), (W), (H)};          \
    SDL_FillRect((SUPER), &r, (COLOR));         \
  }while(0)


extern DECLSPEC
void (*Draw_Ellipse)(SDL_Surface *super,
                        Sint16 x0, Sint16 y0,
                        Uint16 Xradius, Uint16 Yradius,
                        Uint32 color);

extern DECLSPEC
void (*Draw_FillEllipse)(SDL_Surface *super,
                        Sint16 x0, Sint16 y0,
                        Uint16 Xradius, Uint16 Yradius,
                        Uint32 color);

extern DECLSPEC
void (*Draw_Round)(SDL_Surface *super,
                   Sint16 x0,Sint16 y0, Uint16 w,Uint16 h,
                   Uint16 corner, Uint32 color);

extern DECLSPEC
void (*Draw_FillRound)(SDL_Surface *super,
                       Sint16 x0,Sint16 y0, Uint16 w,Uint16 h,
                       Uint16 corner, Uint32 color);


/* We'll use SDL for reporting errors */
#define Draw_SetError  SDL_SetError
#define Draw_GetError  SDL_GetError


#ifdef __cplusplus
} /* extern "C" */
#endif

#include "close_code.h"

#endif /* SDL_DRAW_H */

