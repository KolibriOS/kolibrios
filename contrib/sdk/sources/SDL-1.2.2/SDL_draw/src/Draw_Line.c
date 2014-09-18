/*!
  \file Draw_Line.c
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
#define SDL_DRAW_PUTPIXEL_BPP(A, B, C)  \
*(##A##(##B##(Uint8*)super->pixels + y*super->pitch + x*SDL_DRAW_BPP))=##C##;

#if SDL_DRAW_BPP == 1
#define SDL_DRAW_PUTPIXEL SDL_DRAW_PUTPIXEL_BPP(0+,0+,color)

#elif SDL_DRAW_BPP == 2
#define SDL_DRAW_PUTPIXEL SDL_DRAW_PUTPIXEL_BPP((Uint16*),0+,color)

#elif SDL_DRAW_BPP == 3
#define SDL_DRAW_PUTPIXEL \
  SDL_DRAW_PUTPIXEL_BPP(0+,1+,(Uint8)colorbyte1)   \
  if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {         \
    SDL_DRAW_PUTPIXEL_BPP(0+,0+,(Uint8)colorbyte2)   \
    SDL_DRAW_PUTPIXEL_BPP(0+,2+,(Uint8)colorbyte0) \
  }else{                                         \
    SDL_DRAW_PUTPIXEL_BPP(0+,0+,(Uint8)colorbyte0)   \
    SDL_DRAW_PUTPIXEL_BPP(0+,2+,(Uint8)colorbyte2) \
  }

#elif SDL_DRAW_BPP == 4
#define SDL_DRAW_PUTPIXEL SDL_DRAW_PUTPIXEL_BPP((Uint32*),0+,color)

#endif /*SDL_DRAW_BPP*/


void SDL_DRAWFUNCTION(SDL_Surface *super,
                      Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,
                      Uint32 color)
{
#if SDL_DRAW_BPP == 3
  Uint8 colorbyte0 = (Uint8) (color & 0xff);
  Uint8 colorbyte1 = (Uint8) ((color >> 8) & 0xff);
  Uint8 colorbyte2 = (Uint8) ((color >> 16) & 0xff);
#endif

  Sint16 x = x1;
  Sint16 y = y1;
  Sint16 dy = y2 - y1;
  Sint16 dx = x2 - x1;

  Sint16 G, DeltaG1, DeltaG2, minG, maxG;
  Sint16 swap;
  Sint16 inc = 1;

  SDL_DRAW_PUTPIXEL

  if (abs(dy) < abs(dx)) { /* -1 < ramp < 1 */
      if (dx < 0) {
        dx = -dx;
        dy = -dy;

        swap = y2;
        y2 = y1;
        y1 = swap;

        swap = x2;
        x2 = x1;
        x1 = swap;
      }
      if (dy < 0) {
        dy = -dy;
        inc = -1;
      }

      G = 2 * dy - dx;
      DeltaG1 = 2 * (dy - dx);
      DeltaG2 = 2 * dy;

      while (x++ < x2) {
        if (G > 0) { G += DeltaG1; y += inc; }
        else  G += DeltaG2;

        SDL_DRAW_PUTPIXEL
      }/*while*/

  } else { /* ramp < -1 or ramp > 1 */
      if (dy < 0) {
        dx = -dx;
        dy = -dy;

        swap = y2;
        y2 = y1;
        y1 = swap;

        swap = x2;
        x2 = x1;
        x1 = swap;
      }
      if (dx < 0) {
        dx = -dx;
        inc = -1;
      }

      G = 2 * dx - dy;
      minG = maxG = G;
      DeltaG1 = 2 * (dx - dy);
      DeltaG2 = 2 * dx;

      while (y++ < y2) {
        if (G > 0) { G += DeltaG1; x += inc; }
        else  G += DeltaG2;

        SDL_DRAW_PUTPIXEL
      }/*while*/

  }/*if*/

}/*Draw_Line*/


#undef SDL_DRAW_PUTPIXEL
#undef SDL_DRAW_PUTPIXEL_BPP

