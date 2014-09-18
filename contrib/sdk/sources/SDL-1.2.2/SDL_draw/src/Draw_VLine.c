/*!
  \file Draw_VLine.c
  \author Mario Palomo <mpalomo@ihman.com>
  \author Jose M. de la Huerga Fernández
  \author Pepe González Mora
  \date 05-2002

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

#if SDL_DRAW_BPP == 1
#define SDL_DRAW_PUTPIXEL  \
  i = y1-y0+1;                             \
  switch( i % 4 ) {                        \
    do{                                    \
      case 0: *p = color; p+=super->pitch; \
      case 3: *p = color; p+=super->pitch; \
      case 2: *p = color; p+=super->pitch; \
      case 1: *p = color; p+=super->pitch; \
    }while( (i-=4) > 0 );                  \
  }

#elif SDL_DRAW_BPP == 2
#define SDL_DRAW_PUTPIXEL  \
  i = y1-y0+1;                                      \
  switch( i % 4 ) {                                 \
    do{                                             \
      case 0: *(Uint16*)p = color; p+=super->pitch; \
      case 3: *(Uint16*)p = color; p+=super->pitch; \
      case 2: *(Uint16*)p = color; p+=super->pitch; \
      case 1: *(Uint16*)p = color; p+=super->pitch; \
    }while( (i-=4) > 0 );                           \
  }

#elif SDL_DRAW_BPP == 3
#define SDL_DRAW_PUTPIXEL_BPP_3_AUX \
    if (SDL_BYTEORDER == SDL_BIG_ENDIAN) { \
      p[0] = colorbyte2;                   \
      p[1] = colorbyte1;                   \
      p[2] = colorbyte0;                   \
    } else {                               \
      p[0] = colorbyte0;                   \
      p[1] = colorbyte1;                   \
      p[2] = colorbyte2;                   \
    }

#define SDL_DRAW_PUTPIXEL \
  i = y1-y0+1;                                             \
  switch( i % 4 ) {                                        \
    do{                                                    \
      case 0: SDL_DRAW_PUTPIXEL_BPP_3_AUX p+=super->pitch; \
      case 3: SDL_DRAW_PUTPIXEL_BPP_3_AUX p+=super->pitch; \
      case 2: SDL_DRAW_PUTPIXEL_BPP_3_AUX p+=super->pitch; \
      case 1: SDL_DRAW_PUTPIXEL_BPP_3_AUX p+=super->pitch; \
    }while( (i-=4) > 0 );                                  \
  }


#elif SDL_DRAW_BPP == 4
#define SDL_DRAW_PUTPIXEL \
  i = y1-y0+1;                                      \
  switch( i % 4 ) {                                 \
    do{                                             \
      case 0: *(Uint32*)p = color; p+=super->pitch; \
      case 3: *(Uint32*)p = color; p+=super->pitch; \
      case 2: *(Uint32*)p = color; p+=super->pitch; \
      case 1: *(Uint32*)p = color; p+=super->pitch; \
    }while( (i-=4) > 0 );                           \
  }

#endif /*SDL_DRAW_BPP*/


void SDL_DRAWFUNCTION(SDL_Surface *super,
                      Sint16 x0,Sint16 y0, Sint16 y1,
                      Uint32 color)
{
#if SDL_DRAW_BPP == 3
  Uint8 colorbyte0 = (Uint8) (color & 0xff);
  Uint8 colorbyte1 = (Uint8) ((color >> 8) & 0xff);
  Uint8 colorbyte2 = (Uint8) ((color >> 16) & 0xff);
#endif

  register Uint8 *p;
  register Sint16 i;

  if (y0 > y1)  { i=y1; y1=y0; y0=i; }
  p = (Uint8*)super->pixels + y0 * super->pitch + x0 * SDL_DRAW_BPP;

  SDL_DRAW_PUTPIXEL

}/*Draw_VLine*/


#undef SDL_DRAW_PUTPIXEL
#undef SDL_DRAW_PUTPIXEL_BPP_3_AUX

