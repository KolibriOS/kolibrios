/*!
  \file Draw_Rect.c
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
#define SDL_DRAW_PUTPIXEL \
  memset(p0, color, w);                \
  memset(p1, color, w);                \
                                       \
  if (h<3)  return;                    \
  p0 = (Uint8*)super->pixels + (y+1)*super->pitch + x;       \
  p1 = (Uint8*)super->pixels + (y+1)*super->pitch + (x+w-1); \
  i = h-2;                             \
  switch( i % 4 ) {                    \
    do{                                \
      case 0:                          \
        *p0 = color; p0+=super->pitch; \
        *p1 = color; p1+=super->pitch; \
      case 3:                          \
        *p0 = color; p0+=super->pitch; \
        *p1 = color; p1+=super->pitch; \
      case 2:                          \
        *p0 = color; p0+=super->pitch; \
        *p1 = color; p1+=super->pitch; \
      case 1:                          \
        *p0 = color; p0+=super->pitch; \
        *p1 = color; p1+=super->pitch; \
    }while( (i-=4) > 0 );              \
  }


#elif SDL_DRAW_BPP == 2
#define SDL_DRAW_PUTPIXEL \
  i = w;                                        \
  switch( i % 4 ) {                             \
    do{                                         \
      case 0:                                   \
        *(Uint16*)p0 = color; p0+=2;            \
        *(Uint16*)p1 = color; p1+=2;            \
      case 3:                                   \
        *(Uint16*)p0 = color; p0+=2;            \
        *(Uint16*)p1 = color; p1+=2;            \
      case 2:                                   \
        *(Uint16*)p0 = color; p0+=2;            \
        *(Uint16*)p1 = color; p1+=2;            \
      case 1:                                   \
        *(Uint16*)p0 = color; p0+=2;            \
        *(Uint16*)p1 = color; p1+=2;            \
    }while( (i-=4) > 0 );                       \
  }                                             \
  if (h<3)  return;                             \
  p0 = (Uint8*)super->pixels + (y+1)*super->pitch + x*2;       \
  p1 = (Uint8*)super->pixels + (y+1)*super->pitch + (x+w-1)*2; \
  i = h-2;                                      \
  switch( i % 4 ) {                             \
    do{                                         \
      case 0:                                   \
        *(Uint16*)p0 = color; p0+=super->pitch; \
        *(Uint16*)p1 = color; p1+=super->pitch; \
      case 3:                                   \
        *(Uint16*)p0 = color; p0+=super->pitch; \
        *(Uint16*)p1 = color; p1+=super->pitch; \
      case 2:                                   \
        *(Uint16*)p0 = color; p0+=super->pitch; \
        *(Uint16*)p1 = color; p1+=super->pitch; \
      case 1:                                   \
        *(Uint16*)p0 = color; p0+=super->pitch; \
        *(Uint16*)p1 = color; p1+=super->pitch; \
    }while( (i-=4) > 0 );                       \
  }


#elif SDL_DRAW_BPP == 3
#define SDL_DRAW_PUTPIXEL_BPP_3_AUX \
    if (SDL_BYTEORDER == SDL_BIG_ENDIAN) { \
      p0[0] = colorbyte2;                  \
      p0[1] = colorbyte1;                  \
      p0[2] = colorbyte0;                  \
      p1[0] = colorbyte2;                  \
      p1[1] = colorbyte1;                  \
      p1[2] = colorbyte0;                  \
    } else {                               \
      p0[0] = colorbyte0;                  \
      p0[1] = colorbyte1;                  \
      p0[2] = colorbyte2;                  \
      p1[0] = colorbyte0;                  \
      p1[1] = colorbyte1;                  \
      p1[2] = colorbyte2;                  \
    }

#define SDL_DRAW_PUTPIXEL \
  i = w;                                    \
  switch( i % 4 ) {                         \
    do{                                     \
      case 0:                               \
        SDL_DRAW_PUTPIXEL_BPP_3_AUX         \
        p0+=3; p1+=3;                       \
      case 3:                               \
        SDL_DRAW_PUTPIXEL_BPP_3_AUX         \
        p0+=3; p1+=3;                       \
      case 2:                               \
        SDL_DRAW_PUTPIXEL_BPP_3_AUX         \
        p0+=3; p1+=3;                       \
      case 1:                               \
        SDL_DRAW_PUTPIXEL_BPP_3_AUX         \
        p0+=3; p1+=3;                       \
    }while( (i-=4) > 0 );                   \
  }                                         \
  if (h<3)  return;                         \
  p0 = (Uint8*)super->pixels + (y+1)*super->pitch + x*3;       \
  p1 = (Uint8*)super->pixels + (y+1)*super->pitch + (x+w-1)*3; \
  i = h-2;                                  \
  switch( i % 4 ) {                         \
    do{                                     \
      case 0:                               \
        SDL_DRAW_PUTPIXEL_BPP_3_AUX         \
        p0+=super->pitch; p1+=super->pitch; \
      case 3:                               \
        SDL_DRAW_PUTPIXEL_BPP_3_AUX         \
        p0+=super->pitch; p1+=super->pitch; \
      case 2:                               \
        SDL_DRAW_PUTPIXEL_BPP_3_AUX         \
        p0+=super->pitch; p1+=super->pitch; \
      case 1:                               \
        SDL_DRAW_PUTPIXEL_BPP_3_AUX         \
        p0+=super->pitch; p1+=super->pitch; \
    }while( (i-=4) > 0 );                   \
  }


#elif SDL_DRAW_BPP == 4

#ifdef __linux__
#define SDL_DRAW_WMEMSET_START \
if (sizeof(wchar_t) == sizeof(Uint32)) { \
  wmemset((wchar_t*)p0, color, w); \
  wmemset((wchar_t*)p1, color, w); \
} else {
#define SDL_DRAW_WMEMSET_END }
#else
#define SDL_DRAW_WMEMSET_START
#define SDL_DRAW_WMEMSET_END
#endif

#define SDL_DRAW_PUTPIXEL \
SDL_DRAW_WMEMSET_START                          \
  i = w;                                        \
  switch( i % 4 ) {                             \
    do{                                         \
      case 0:                                   \
        *(Uint32*)p0 = color; p0+=4;            \
        *(Uint32*)p1 = color; p1+=4;            \
      case 3:                                   \
        *(Uint32*)p0 = color; p0+=4;            \
        *(Uint32*)p1 = color; p1+=4;            \
      case 2:                                   \
        *(Uint32*)p0 = color; p0+=4;            \
        *(Uint32*)p1 = color; p1+=4;            \
      case 1:                                   \
        *(Uint32*)p0 = color; p0+=4;            \
        *(Uint32*)p1 = color; p1+=4;            \
    }while( (i-=4) > 0 );                       \
  }                                             \
SDL_DRAW_WMEMSET_END                            \
  if (h<3)  return;                             \
  p0 = (Uint8*)super->pixels + (y+1)*super->pitch + x*4;       \
  p1 = (Uint8*)super->pixels + (y+1)*super->pitch + (x+w-1)*4; \
  i = h-2;                                      \
  switch( i % 4 ) {                             \
    do{                                         \
      case 0:                                   \
        *(Uint32*)p0 = color; p0+=super->pitch; \
        *(Uint32*)p1 = color; p1+=super->pitch; \
      case 3:                                   \
        *(Uint32*)p0 = color; p0+=super->pitch; \
        *(Uint32*)p1 = color; p1+=super->pitch; \
      case 2:                                   \
        *(Uint32*)p0 = color; p0+=super->pitch; \
        *(Uint32*)p1 = color; p1+=super->pitch; \
      case 1:                                   \
        *(Uint32*)p0 = color; p0+=super->pitch; \
        *(Uint32*)p1 = color; p1+=super->pitch; \
    }while( (i-=4) > 0 );                       \
  }

#endif /*SDL_DRAW_BPP*/


void SDL_DRAWFUNCTION(SDL_Surface *super,
                  Sint16 x, Sint16 y, Uint16 w, Uint16 h,
                  Uint32 color)
{
#if SDL_DRAW_BPP == 3
  Uint8 colorbyte0 = (Uint8) (color & 0xff);
  Uint8 colorbyte1 = (Uint8) ((color >> 8)  & 0xff);
  Uint8 colorbyte2 = (Uint8) ((color >> 16) & 0xff);
#endif

  register Uint8 *p0;
  register Uint8 *p1;
  register Sint16 i;

  if (w==0 || h==0)  return;

  p0 = (Uint8*)super->pixels +    y    * super->pitch + x * SDL_DRAW_BPP;
  p1 = (Uint8*)super->pixels + (y+h-1) * super->pitch + x * SDL_DRAW_BPP;

  SDL_DRAW_PUTPIXEL

}/*Draw_Rect*/


#undef SDL_DRAW_PUTPIXEL
#undef SDL_DRAW_PUTPIXEL_BPP_3_AUX

#undef SDL_DRAW_WMEMSET_START
#undef SDL_DRAW_WMEMSET_END

