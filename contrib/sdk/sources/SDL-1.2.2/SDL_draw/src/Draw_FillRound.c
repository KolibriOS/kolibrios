/*!
  \file Draw_FillRound.c
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
  memset(p0, color, X2center - Xcenter + 2*corner+1);                      \
  memset(p1, color, X2center - Xcenter + 2*corner+1);                      \
  p0 = ((Uint8*)super->pixels+(Y2center+corner)*super->pitch+(Xcenter-x)); \
  p1 = ((Uint8*)super->pixels+(Ycenter-corner)*super->pitch +(Xcenter-x)); \
  memset(p0, color, X2center - Xcenter + 2*x+1);                           \
  memset(p1, color, X2center - Xcenter + 2*x+1);

#elif SDL_DRAW_BPP == 2
#define SDL_DRAW_PUTPIXEL \
  i = X2center - Xcenter + 2*corner+1;                    \
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
  p0 = ((Uint8*)super->pixels+(Y2center+corner)*super->pitch+(Xcenter-x)*2); \
  p1 = ((Uint8*)super->pixels+(Ycenter-corner)*super->pitch +(Xcenter-x)*2); \
  i = X2center - Xcenter + 2*x+1;                         \
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
  }

#elif SDL_DRAW_BPP == 3
#define SDL_DRAW_PUTPIXEL_BPP_3_AUX \
    if (SDL_BYTEORDER == SDL_BIG_ENDIAN) { \
      p0[0] = colorbyte2;                   \
      p0[1] = colorbyte1;                   \
      p0[2] = colorbyte0;                   \
      p1[0] = colorbyte2;                   \
      p1[1] = colorbyte1;                   \
      p1[2] = colorbyte0;                   \
    } else {                                \
      p0[0] = colorbyte0;                   \
      p0[1] = colorbyte1;                   \
      p0[2] = colorbyte2;                   \
      p1[0] = colorbyte0;                   \
      p1[1] = colorbyte1;                   \
      p1[2] = colorbyte2;                   \
    }

#define SDL_DRAW_PUTPIXEL \
  i = X2center - Xcenter + 2*corner+1;    \
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
  p0 = ((Uint8*)super->pixels+(Y2center+corner)*super->pitch+(Xcenter-x)*3); \
  p1 = ((Uint8*)super->pixels+(Ycenter-corner)*super->pitch +(Xcenter-x)*3); \
  i = X2center - Xcenter + 2*x+1;         \
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
  }


#elif SDL_DRAW_BPP == 4

#ifdef __linux__
#define SDL_DRAW_WMEMSET_START \
if (sizeof(wchar_t) == sizeof(Uint32)) { \
  wmemset((wchar_t*)p0, color, X2center - Xcenter + 2*corner+1); \
  wmemset((wchar_t*)p1, color, X2center - Xcenter + 2*corner+1); \
  p0 = ((Uint8*)super->pixels+(Y2center+corner)*super->pitch+(Xcenter-x)*4); \
  p1 = ((Uint8*)super->pixels+(Ycenter-corner)*super->pitch +(Xcenter-x)*4); \
  wmemset((wchar_t*)p0, color, X2center - Xcenter + 2*x+1); \
  wmemset((wchar_t*)p1, color, X2center - Xcenter + 2*x+1); \
} else {
#define SDL_DRAW_WMEMSET_END }
#else
#define SDL_DRAW_WMEMSET_START
#define SDL_DRAW_WMEMSET_END
#endif

#define SDL_DRAW_PUTPIXEL \
SDL_DRAW_WMEMSET_START                                    \
  i = X2center - Xcenter + 2*corner+1;                    \
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
  p0 = ((Uint8*)super->pixels+(Y2center+corner)*super->pitch+(Xcenter-x)*4); \
  p1 = ((Uint8*)super->pixels+(Ycenter-corner)*super->pitch +(Xcenter-x)*4); \
  i = X2center - Xcenter + 2*x+1;                         \
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
                      Sint16 x0,Sint16 y0, Uint16 w,Uint16 h,
                      Uint16 corner, Uint32 color)
{
#if SDL_DRAW_BPP == 3
  Uint8 colorbyte0 = (Uint8) (color & 0xff);
  Uint8 colorbyte1 = (Uint8) ((color >> 8)  & 0xff);
  Uint8 colorbyte2 = (Uint8) ((color >> 16) & 0xff);
#endif

  register Uint8 *p0;
  register Uint8 *p1;
#if SDL_DRAW_BPP != 1
  register Sint16 i;
#endif
  Sint16 dx, dy;

  Sint16 Xcenter, Ycenter, X2center, Y2center;

  Sint16 x = 0;
  Sint16 rightInc = 6;
  Sint16 d, diagonalInc;

  SDL_Rect r;


  if (w==0 || h==0)  return;

  /*TODO: We can do better :-)*/
  if (corner!=0) {
    d = w<h ? w : h;
    --corner;
    if (corner!=0 && corner+2 >= d ) {
      if (corner+2 == d)  --corner;
      else corner = 0;
    }
  }

  d = 3 - (corner<<1);
  diagonalInc = 10 - (corner<<2);

  /*Rectangles*/
  dx = w - (corner<<1);
  Xcenter = x0+corner;
  dy = h - (corner<<1);
  Ycenter = y0+corner;

  /*Centers*/
  X2center=Xcenter+dx-1;
  Y2center=Ycenter+dy-1;

  r.x = x0; r.y = Ycenter;
  r.w = w; r.h = dy;
  SDL_FillRect(super, &r, color);

  while (x < corner) {

    p0 = ((Uint8*)super->pixels+(Ycenter-x)*super->pitch +
                                       (Xcenter-corner)*SDL_DRAW_BPP);
    p1 = ((Uint8*)super->pixels+(Y2center+x)*super->pitch +
                                        (Xcenter-corner)*SDL_DRAW_BPP);

    SDL_DRAW_PUTPIXEL

    if (d >= 0) {
      d += diagonalInc;
      diagonalInc += 8;
      --corner;
    } else {
      d += rightInc;
      diagonalInc += 4;
    }
    rightInc += 4;
    ++x;
  }/*while*/

}/*Draw_FillRound*/


#undef SDL_DRAW_PUTPIXEL

#undef SDL_DRAW_WMEMSET_START
#undef SDL_DRAW_WMEMSET_END

