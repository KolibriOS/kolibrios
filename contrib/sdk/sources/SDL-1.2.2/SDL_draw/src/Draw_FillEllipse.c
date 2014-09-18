/*!
  \file Draw_FillEllipse.c
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
  memset(((Uint8*)super->pixels+ (y0+y)*super->pitch+ (x0-x)), \
         color, 2*x+1); \
  memset(((Uint8*)super->pixels+ (y0-y)*super->pitch+ (x0-x)), \
         color, 2*x+1);


#elif SDL_DRAW_BPP == 2
#define SDL_DRAW_PUTPIXEL \
{                                                         \
  p0 = ((Uint8*)super->pixels+ (y0+y)*super->pitch+ (x0-x)*2); \
  p1 = ((Uint8*)super->pixels+ (y0-y)*super->pitch+ (x0-x)*2); \
  i = 2*x+1;                                              \
  switch( i % 4 ) {                                       \
    do{                                                   \
      case 0: *(Uint16*)p0 = color; *(Uint16*)p1 = color; \
              p0+=2; p1+=2;                               \
      case 3: *(Uint16*)p0 = color; *(Uint16*)p1 = color; \
              p0+=2; p1+=2;                               \
      case 2: *(Uint16*)p0 = color; *(Uint16*)p1 = color; \
              p0+=2; p1+=2;                               \
      case 1: *(Uint16*)p0 = color; *(Uint16*)p1 = color; \
              p0+=2; p1+=2;                               \
    }while( (i-=4) > 0 );                                 \
  }                                                       \
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
{                                         \
  p0 = ((Uint8*)super->pixels+ (y0+y)*super->pitch+ (x0-x)*3); \
  p1 = ((Uint8*)super->pixels+ (y0-y)*super->pitch+ (x0-x)*3); \
  i = 2*x+1;                              \
  switch( i % 4 ) {                       \
    do{                                   \
      case 0: SDL_DRAW_PUTPIXEL_BPP_3_AUX \
              p0+=3; p1+=3;               \
      case 3: SDL_DRAW_PUTPIXEL_BPP_3_AUX \
              p0+=3; p1+=3;               \
      case 2: SDL_DRAW_PUTPIXEL_BPP_3_AUX \
              p0+=3; p1+=3;               \
      case 1: SDL_DRAW_PUTPIXEL_BPP_3_AUX \
              p0+=3; p1+=3;               \
    }while( (i-=4) > 0 );                 \
  }                                       \
}


#elif SDL_DRAW_BPP == 4

#ifdef __linux__
#define SDL_DRAW_WMEMSET_START \
if (sizeof(wchar_t) == sizeof(Uint32)) {                                     \
  wmemset( (wchar_t*)((Uint8*)super->pixels+ (y0+y)*super->pitch+ (x0-x)*4), \
         color, 2*x+1);                                                      \
  wmemset( (wchar_t*)((Uint8*)super->pixels+ (y0-y)*super->pitch+ (x0-x)*4), \
         color, 2*x+1);                                                      \
} else {
#define SDL_DRAW_WMEMSET_END }
#else
#define SDL_DRAW_WMEMSET_START
#define SDL_DRAW_WMEMSET_END
#endif

#define SDL_DRAW_PUTPIXEL \
SDL_DRAW_WMEMSET_START                                    \
  p0 = ((Uint8*)super->pixels+ (y0+y)*super->pitch+ (x0-x)*4); \
  p1 = ((Uint8*)super->pixels+ (y0-y)*super->pitch+ (x0-x)*4); \
  i = 2*x+1;                                              \
  switch( i % 4 ) {                                       \
    do{                                                   \
      case 0: *(Uint32*)p0 = color; *(Uint32*)p1 = color; \
              p0+=4; p1+=4;                               \
      case 3: *(Uint32*)p0 = color; *(Uint32*)p1 = color; \
              p0+=4; p1+=4;                               \
      case 2: *(Uint32*)p0 = color; *(Uint32*)p1 = color; \
              p0+=4; p1+=4;                               \
      case 1: *(Uint32*)p0 = color; *(Uint32*)p1 = color; \
              p0+=4; p1+=4;                               \
    }while( (i-=4) > 0 );                                 \
  }                                                       \
SDL_DRAW_WMEMSET_END

#endif /*SDL_DRAW_BPP*/



void SDL_DRAWFUNCTION(SDL_Surface *super,
                      Sint16 x0, Sint16 y0,
                      Uint16 Xradius, Uint16 Yradius,
                      Uint32 color)
{
#if SDL_DRAW_BPP == 3
  Uint8 colorbyte0 = (Uint8) (color & 0xff);
  Uint8 colorbyte1 = (Uint8) ((color >> 8) & 0xff);
  Uint8 colorbyte2 = (Uint8) ((color >> 16) & 0xff);
#endif

#if SDL_DRAW_BPP != 1
  register Uint8 *p0;
  register Uint8 *p1;
  register Sint16 i;
#endif

  Sint32 x, y;
  Sint32 Xchange, Ychange;
  Sint32 EllipseError;
  Sint32 TwoASquare, TwoBSquare;
  Sint32 StoppingX, StoppingY;

  TwoASquare = 2*Xradius*Xradius;
  TwoBSquare = 2*Yradius*Yradius;

  /*1st set of points*/
  x = Xradius-1;  /*radius zero == draw nothing*/
  y = 0;

  Xchange = Yradius*Yradius*(1-2*Xradius);
  Ychange = Xradius*Xradius;

  EllipseError = 0;

  StoppingX = TwoBSquare*Xradius;
  StoppingY = 0;

  /*Plot 2 ellipse scan lines for iteration*/
  while (StoppingX > StoppingY) {

    SDL_DRAW_PUTPIXEL

    ++y;
    StoppingY    += TwoASquare;
    EllipseError += Ychange;
    Ychange      += TwoASquare;
    if (( 2*EllipseError + Xchange) > 0) {
      --x;
      StoppingX    -= TwoBSquare;
      EllipseError += Xchange;
      Xchange      += TwoBSquare;
    }
  }/*while*/

  /*2nd set of points*/
  x = 0;
  y = Yradius-1;  /*radius zero == draw nothing*/
  Xchange = Yradius*Yradius;
  Ychange = Xradius*Xradius*(1-2*Yradius);
  EllipseError = 0;
  StoppingX = 0;
  StoppingY = TwoASquare*Yradius;

  /*Plot 2 ellipse scan lines for iteration*/
  while (StoppingX < StoppingY) {

    SDL_DRAW_PUTPIXEL

    ++x;
    StoppingX    += TwoBSquare;
    EllipseError += Xchange;
    Xchange      += TwoBSquare;
    if ((2*EllipseError + Ychange) > 0) {
      --y;
      StoppingY    -= TwoASquare;
      EllipseError += Ychange;
      Ychange      += TwoASquare;
    }
  }
}/*Draw_FillEllipse*/


#undef SDL_DRAW_PUTPIXEL
#undef SDL_DRAW_PUTPIXEL_BPP_3_AUX

#undef SDL_DRAW_WMEMSET_START
#undef SDL_DRAW_WMEMSET_END

