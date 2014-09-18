/*!
  \file Draw_Round.c
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

/*Circle arcs*/
#define SDL_DRAW_PUTPIXEL_CIRCLE_BPP(A, B, C) \
  *(##A##(##B##(Uint8*)super->pixels + (Ycenter-x)*super->pitch +        \
                                (Xcenter - corner)*SDL_DRAW_BPP)) = C##; \
  *(##A##(##B##(Uint8*)super->pixels + (Ycenter-corner)*super->pitch +   \
                                     (Xcenter - x)*SDL_DRAW_BPP)) = C##; \
  *(##A##(##B##(Uint8*)super->pixels + (Ycenter-corner)*super->pitch +   \
                                    (X2center + x)*SDL_DRAW_BPP)) = C##; \
  *(##A##(##B##(Uint8*)super->pixels + (Ycenter-x)*super->pitch +        \
                               (X2center + corner)*SDL_DRAW_BPP)) = C##; \
  *(##A##(##B##(Uint8*)super->pixels + (Y2center+corner)*super->pitch +  \
                                    (X2center + x)*SDL_DRAW_BPP)) = C##; \
  *(##A##(##B##(Uint8*)super->pixels + (Y2center+x)*super->pitch +       \
                               (X2center + corner)*SDL_DRAW_BPP)) = C##; \
  *(##A##(##B##(Uint8*)super->pixels + (Y2center+corner)*super->pitch +  \
                                     (Xcenter - x)*SDL_DRAW_BPP)) = C##; \
  *(##A##(##B##(Uint8*)super->pixels + (Y2center+x)*super->pitch +       \
                                (Xcenter - corner)*SDL_DRAW_BPP)) = C##;


#if SDL_DRAW_BPP == 1
#define SDL_DRAW_PUTPIXEL_CIRCLE SDL_DRAW_PUTPIXEL_CIRCLE_BPP(0+,0+,color)

#elif SDL_DRAW_BPP == 2
#define SDL_DRAW_PUTPIXEL_CIRCLE SDL_DRAW_PUTPIXEL_CIRCLE_BPP((Uint16*),0+,color)

#elif SDL_DRAW_BPP == 3
#define SDL_DRAW_PUTPIXEL_CIRCLE \
  SDL_DRAW_PUTPIXEL_CIRCLE_BPP(0+,1+,colorbyte1)   \
  if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {         \
    SDL_DRAW_PUTPIXEL_CIRCLE_BPP(0+,0+,colorbyte2)   \
    SDL_DRAW_PUTPIXEL_CIRCLE_BPP(0+,2+,colorbyte0) \
  }else{                                         \
    SDL_DRAW_PUTPIXEL_CIRCLE_BPP(0+,0+,colorbyte0)   \
    SDL_DRAW_PUTPIXEL_CIRCLE_BPP(0+,2+,colorbyte2) \
  }

#elif SDL_DRAW_BPP == 4
#define SDL_DRAW_PUTPIXEL_CIRCLE SDL_DRAW_PUTPIXEL_CIRCLE_BPP((Uint32*),0+,color)

#endif /*SDL_DRAW_BPP*/


/*Rectangles*/
#if SDL_DRAW_BPP == 1
#define SDL_DRAW_PUTPIXEL \
  memset(p0, color, dx);               \
  memset(p1, color, dx);               \
                                       \
  if (h<3)  return;                    \
  p0 = (Uint8*)super->pixels + Ycenter*super->pitch + x0; \
  p1 = (Uint8*)super->pixels + Ycenter*super->pitch + x0+w-1; \
  i=dy;                                \
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
  i=dx;                                         \
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
  p0 = (Uint8*)super->pixels + Ycenter*super->pitch + x0*2;       \
  p1 = (Uint8*)super->pixels + Ycenter*super->pitch + (x0+w-1)*2; \
  i=dy;                                         \
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
  i=dx;                                     \
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
  p0 = (Uint8*)super->pixels + Ycenter*super->pitch + x0*3;       \
  p1 = (Uint8*)super->pixels + Ycenter*super->pitch + (x0+w-1)*3; \
  i=dy;                                     \
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
if (sizeof(wchar_t) == sizeof(Uint32)) {        \
  wmemset((wchar_t*)p0, color, dx);             \
  wmemset((wchar_t*)p1, color, dx);             \
} else {
#define SDL_DRAW_WMEMSET_END }
#else
#define SDL_DRAW_WMEMSET_START
#define SDL_DRAW_WMEMSET_END
#endif

#define SDL_DRAW_PUTPIXEL \
SDL_DRAW_WMEMSET_START                          \
  i=dx;                                         \
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
  p0 = (Uint8*)super->pixels + Ycenter*super->pitch + x0*4;       \
  p1 = (Uint8*)super->pixels + Ycenter*super->pitch + (x0+w-1)*4; \
  i=dy;                                         \
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
  register Sint16 i;
  Sint16 dx, dy;

  Sint16 Xcenter, Ycenter, X2center, Y2center;

  Sint16 x = 0;
  Sint16 rightInc = 6;
  Sint16 d, diagonalInc;

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

  p0 = (Uint8*)super->pixels +    y0    * super->pitch + Xcenter*SDL_DRAW_BPP;
  p1 = (Uint8*)super->pixels + (y0+h-1) * super->pitch + Xcenter*SDL_DRAW_BPP;

  SDL_DRAW_PUTPIXEL

  while (x < corner) {

    SDL_DRAW_PUTPIXEL_CIRCLE

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

}/*Draw_Round*/


#undef SDL_DRAW_PUTPIXEL
#undef SDL_DRAW_PUTPIXEL_CIRCLE
#undef SDL_DRAW_PUTPIXEL_CIRCLE_BPP

#undef SDL_DRAW_WMEMSET_START
#undef SDL_DRAW_WMEMSET_END

