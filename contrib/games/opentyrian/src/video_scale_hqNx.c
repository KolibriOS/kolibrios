/* 
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2010  The OpenTyrian Development Team
 * 
 * hq2x, hq3x, hq4x
 * Copyright (C) 2003 MaxSt ( maxst@hiend3d.com )
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "palette.h"
#include "video.h"

void interp1(Uint32 *pc, Uint32 c1, Uint32 c2);
void interp2(Uint32 *pc, Uint32 c1, Uint32 c2, Uint32 c3);
void interp3(Uint32 *pc, Uint32 c1, Uint32 c2);
void interp4(Uint32 *pc, Uint32 c1, Uint32 c2, Uint32 c3);
void interp5(Uint32 *pc, Uint32 c1, Uint32 c2);
void interp6(Uint32 *pc, Uint32 c1, Uint32 c2, Uint32 c3);
void interp7(Uint32 *pc, Uint32 c1, Uint32 c2, Uint32 c3);
void interp8(Uint32 *pc, Uint32 c1, Uint32 c2);
void interp9(Uint32 *pc, Uint32 c1, Uint32 c2, Uint32 c3);
void interp10(Uint32 *pc, Uint32 c1, Uint32 c2, Uint32 c3);
bool diff(unsigned int w1, unsigned int w2);

void hq2x_32( SDL_Surface *src_surface, SDL_Surface *dst_surface );
void hq3x_32( SDL_Surface *src_surface, SDL_Surface *dst_surface );
void hq4x_32( SDL_Surface *src_surface, SDL_Surface *dst_surface );

static int   YUV1, YUV2;
const  int   Ymask = 0x00FF0000;
const  int   Umask = 0x0000FF00;
const  int   Vmask = 0x000000FF;
const  int   trY   = 0x00300000;
const  int   trU   = 0x00000700;
const  int   trV   = 0x00000006;

inline void interp1(Uint32 *pc, Uint32 c1, Uint32 c2)
{
	*pc = (c1*3+c2) >> 2;
}

inline void interp2(Uint32 *pc, Uint32 c1, Uint32 c2, Uint32 c3)
{
	*pc = (c1*2+c2+c3) >> 2;
}

inline void interp3(Uint32 *pc, Uint32 c1, Uint32 c2)
{
	//*((int*)pc) = (c1*7+c2)/8;
	
	*((int*)pc) = ((((c1 & 0x00FF00)*7 + (c2 & 0x00FF00) ) & 0x0007F800) +
	               (((c1 & 0xFF00FF)*7 + (c2 & 0xFF00FF) ) & 0x07F807F8)) >> 3;
}

inline void interp4(Uint32 *pc, Uint32 c1, Uint32 c2, Uint32 c3)
{
	//*((int*)pc) = (c1*2+(c2+c3)*7)/16;
	
	*((int*)pc) = ((((c1 & 0x00FF00)*2 + ((c2 & 0x00FF00) + (c3 & 0x00FF00))*7 ) & 0x000FF000) +
	               (((c1 & 0xFF00FF)*2 + ((c2 & 0xFF00FF) + (c3 & 0xFF00FF))*7 ) & 0x0FF00FF0)) >> 4;
}

inline void interp5(Uint32 *pc, Uint32 c1, Uint32 c2)
{
	*pc = (c1+c2) >> 1;
}

inline void interp6(Uint32 *pc, Uint32 c1, Uint32 c2, Uint32 c3)
{
	//*pc = (c1*5+c2*2+c3)/8;
	
	*pc = ((((c1 & 0x00FF00)*5 + (c2 & 0x00FF00)*2 + (c3 & 0x00FF00) ) & 0x0007F800) +
	       (((c1 & 0xFF00FF)*5 + (c2 & 0xFF00FF)*2 + (c3 & 0xFF00FF) ) & 0x07F807F8)) >> 3;
}

inline void interp7(Uint32 *pc, Uint32 c1, Uint32 c2, Uint32 c3)
{
	//*pc = (c1*6+c2+c3)/8;
	
	*pc = ((((c1 & 0x00FF00)*6 + (c2 & 0x00FF00) + (c3 & 0x00FF00) ) & 0x0007F800) +
	       (((c1 & 0xFF00FF)*6 + (c2 & 0xFF00FF) + (c3 & 0xFF00FF) ) & 0x07F807F8)) >> 3;
}

inline void interp8(Uint32 *pc, Uint32 c1, Uint32 c2)
{
	//*pc = (c1*5+c2*3)/8;
	
	*pc = ((((c1 & 0x00FF00)*5 + (c2 & 0x00FF00)*3 ) & 0x0007F800) +
	       (((c1 & 0xFF00FF)*5 + (c2 & 0xFF00FF)*3 ) & 0x07F807F8)) >> 3;
}

inline void interp9(Uint32 *pc, Uint32 c1, Uint32 c2, Uint32 c3)
{
	//*pc = (c1*2+(c2+c3)*3)/8;
	
	*pc = ((((c1 & 0x00FF00)*2 + ((c2 & 0x00FF00) + (c3 & 0x00FF00))*3 ) & 0x0007F800) +
	       (((c1 & 0xFF00FF)*2 + ((c2 & 0xFF00FF) + (c3 & 0xFF00FF))*3 ) & 0x07F807F8)) >> 3;
}

inline void interp10(Uint32 *pc, Uint32 c1, Uint32 c2, Uint32 c3)
{
	//*pc = (c1*14+c2+c3)/16;
	
	*pc = ((((c1 & 0x00FF00)*14 + (c2 & 0x00FF00) + (c3 & 0x00FF00) ) & 0x000FF000) +
	       (((c1 & 0xFF00FF)*14 + (c2 & 0xFF00FF) + (c3 & 0xFF00FF) ) & 0x0FF00FF0)) >> 4;
}

inline bool diff(unsigned int w1, unsigned int w2)
{
	Uint32 YUV1 = yuv_palette[w1];
	Uint32 YUV2 = yuv_palette[w2];
	return ( ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
	         ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
	         ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) );
}


#define PIXEL00_0     *(Uint32 *)dst = c[5];
#define PIXEL00_10    interp1((Uint32 *)dst, c[5], c[1]);
#define PIXEL00_11    interp1((Uint32 *)dst, c[5], c[4]);
#define PIXEL00_12    interp1((Uint32 *)dst, c[5], c[2]);
#define PIXEL00_20    interp2((Uint32 *)dst, c[5], c[4], c[2]);
#define PIXEL00_21    interp2((Uint32 *)dst, c[5], c[1], c[2]);
#define PIXEL00_22    interp2((Uint32 *)dst, c[5], c[1], c[4]);
#define PIXEL00_60    interp6((Uint32 *)dst, c[5], c[2], c[4]);
#define PIXEL00_61    interp6((Uint32 *)dst, c[5], c[4], c[2]);
#define PIXEL00_70    interp7((Uint32 *)dst, c[5], c[4], c[2]);
#define PIXEL00_90    interp9((Uint32 *)dst, c[5], c[4], c[2]);
#define PIXEL00_100   interp10((Uint32 *)dst, c[5], c[4], c[2]);
#define PIXEL01_0     *(Uint32 *)(dst + dst_Bpp) = c[5];
#define PIXEL01_10    interp1((Uint32 *)(dst + dst_Bpp), c[5], c[3]);
#define PIXEL01_11    interp1((Uint32 *)(dst + dst_Bpp), c[5], c[2]);
#define PIXEL01_12    interp1((Uint32 *)(dst + dst_Bpp), c[5], c[6]);
#define PIXEL01_20    interp2((Uint32 *)(dst + dst_Bpp), c[5], c[2], c[6]);
#define PIXEL01_21    interp2((Uint32 *)(dst + dst_Bpp), c[5], c[3], c[6]);
#define PIXEL01_22    interp2((Uint32 *)(dst + dst_Bpp), c[5], c[3], c[2]);
#define PIXEL01_60    interp6((Uint32 *)(dst + dst_Bpp), c[5], c[6], c[2]);
#define PIXEL01_61    interp6((Uint32 *)(dst + dst_Bpp), c[5], c[2], c[6]);
#define PIXEL01_70    interp7((Uint32 *)(dst + dst_Bpp), c[5], c[2], c[6]);
#define PIXEL01_90    interp9((Uint32 *)(dst + dst_Bpp), c[5], c[2], c[6]);
#define PIXEL01_100   interp10((Uint32 *)(dst + dst_Bpp), c[5], c[2], c[6]);
#define PIXEL10_0     *(Uint32 *)(dst + dst_pitch) = c[5];
#define PIXEL10_10    interp1((Uint32 *)(dst + dst_pitch), c[5], c[7]);
#define PIXEL10_11    interp1((Uint32 *)(dst + dst_pitch), c[5], c[8]);
#define PIXEL10_12    interp1((Uint32 *)(dst + dst_pitch), c[5], c[4]);
#define PIXEL10_20    interp2((Uint32 *)(dst + dst_pitch), c[5], c[8], c[4]);
#define PIXEL10_21    interp2((Uint32 *)(dst + dst_pitch), c[5], c[7], c[4]);
#define PIXEL10_22    interp2((Uint32 *)(dst + dst_pitch), c[5], c[7], c[8]);
#define PIXEL10_60    interp6((Uint32 *)(dst + dst_pitch), c[5], c[4], c[8]);
#define PIXEL10_61    interp6((Uint32 *)(dst + dst_pitch), c[5], c[8], c[4]);
#define PIXEL10_70    interp7((Uint32 *)(dst + dst_pitch), c[5], c[8], c[4]);
#define PIXEL10_90    interp9((Uint32 *)(dst + dst_pitch), c[5], c[8], c[4]);
#define PIXEL10_100   interp10((Uint32 *)(dst + dst_pitch), c[5], c[8], c[4]);
#define PIXEL11_0     *(Uint32 *)(dst + dst_pitch + dst_Bpp) = c[5];
#define PIXEL11_10    interp1((Uint32 *)(dst + dst_pitch + dst_Bpp), c[5], c[9]);
#define PIXEL11_11    interp1((Uint32 *)(dst + dst_pitch + dst_Bpp), c[5], c[6]);
#define PIXEL11_12    interp1((Uint32 *)(dst + dst_pitch + dst_Bpp), c[5], c[8]);
#define PIXEL11_20    interp2((Uint32 *)(dst + dst_pitch + dst_Bpp), c[5], c[6], c[8]);
#define PIXEL11_21    interp2((Uint32 *)(dst + dst_pitch + dst_Bpp), c[5], c[9], c[8]);
#define PIXEL11_22    interp2((Uint32 *)(dst + dst_pitch + dst_Bpp), c[5], c[9], c[6]);
#define PIXEL11_60    interp6((Uint32 *)(dst + dst_pitch + dst_Bpp), c[5], c[8], c[6]);
#define PIXEL11_61    interp6((Uint32 *)(dst + dst_pitch + dst_Bpp), c[5], c[6], c[8]);
#define PIXEL11_70    interp7((Uint32 *)(dst + dst_pitch + dst_Bpp), c[5], c[6], c[8]);
#define PIXEL11_90    interp9((Uint32 *)(dst + dst_pitch + dst_Bpp), c[5], c[6], c[8]);
#define PIXEL11_100   interp10((Uint32 *)(dst + dst_pitch + dst_Bpp), c[5], c[6], c[8]);

void hq2x_32( SDL_Surface *src_surface, SDL_Surface *dst_surface )
{
	Uint8 *src = src_surface->pixels, *src_temp,
	      *dst = dst_surface->pixels, *dst_temp;
	int src_pitch = src_surface->pitch,
	    dst_pitch = dst_surface->pitch;
	const int dst_Bpp = 4;         // dst_surface->format->BytesPerPixel
	
	const int height = vga_height, // src_surface->h
	          width = vga_width;   // src_surface->w
	
	int prevline, nextline;
	
	Uint32 w[10];
	Uint32 c[10];
	
	//   +----+----+----+
	//   |    |    |    |
	//   | w1 | w2 | w3 |
	//   +----+----+----+
	//   |    |    |    |
	//   | w4 | w5 | w6 |
	//   +----+----+----+
	//   |    |    |    |
	//   | w7 | w8 | w9 |
	//   +----+----+----+
	
	for (int j = 0; j < height; j++)
	{
		src_temp = src;
		dst_temp = dst;
		
		prevline = (j > 0) ? -width : 0;
		nextline = (j < height - 1) ? width : 0;
		
		for (int i = 0; i < width; i++)
		{
			w[2] = *(src + prevline);
			w[5] = *src;
			w[8] = *(src + nextline);
			
			if (i > 0)
			{
				w[1] = *(src + prevline - 1);
				w[4] = *(src - 1);
				w[7] = *(src + nextline - 1);
			} else {
				w[1] = w[2];
				w[4] = w[5];
				w[7] = w[8];
			}
			
			if (i < width - 1)
			{
				w[3] = *(src + prevline + 1);
				w[6] = *(src + 1);
				w[9] = *(src + nextline + 1);
			} else {
				w[3] = w[2];
				w[6] = w[5];
				w[9] = w[8];
			}
			
			int pattern = 0;
			int flag = 1;
			
			YUV1 = yuv_palette[w[5]];
			
			for (int k=1; k<=9; k++)
			{
				if (k==5) continue;
				
				if ( w[k] != w[5] )
				{
					YUV2 = yuv_palette[w[k]];
					if ( ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
					     ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
					     ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) )
					pattern |= flag;
				}
				flag <<= 1;
			}
			
			for (int k=1; k<=9; k++)
				c[k] = rgb_palette[w[k]] & 0xfcfcfcfc; // hq2x has a nasty inability to accept more than 6 bits for each component
			
			switch (pattern)
			{
				case 0:
				case 1:
				case 4:
				case 32:
				case 128:
				case 5:
				case 132:
				case 160:
				case 33:
				case 129:
				case 36:
				case 133:
				case 164:
				case 161:
				case 37:
				case 165:
				{
					PIXEL00_20
					PIXEL01_20
					PIXEL10_20
					PIXEL11_20
					break;
				}
				case 2:
				case 34:
				case 130:
				case 162:
				{
					PIXEL00_22
					PIXEL01_21
					PIXEL10_20
					PIXEL11_20
					break;
				}
				case 16:
				case 17:
				case 48:
				case 49:
				{
					PIXEL00_20
					PIXEL01_22
					PIXEL10_20
					PIXEL11_21
					break;
				}
				case 64:
				case 65:
				case 68:
				case 69:
				{
					PIXEL00_20
					PIXEL01_20
					PIXEL10_21
					PIXEL11_22
					break;
				}
				case 8:
				case 12:
				case 136:
				case 140:
				{
					PIXEL00_21
					PIXEL01_20
					PIXEL10_22
					PIXEL11_20
					break;
				}
				case 3:
				case 35:
				case 131:
				case 163:
				{
					PIXEL00_11
					PIXEL01_21
					PIXEL10_20
					PIXEL11_20
					break;
				}
				case 6:
				case 38:
				case 134:
				case 166:
				{
					PIXEL00_22
					PIXEL01_12
					PIXEL10_20
					PIXEL11_20
					break;
				}
				case 20:
				case 21:
				case 52:
				case 53:
				{
					PIXEL00_20
					PIXEL01_11
					PIXEL10_20
					PIXEL11_21
					break;
				}
				case 144:
				case 145:
				case 176:
				case 177:
				{
					PIXEL00_20
					PIXEL01_22
					PIXEL10_20
					PIXEL11_12
					break;
				}
				case 192:
				case 193:
				case 196:
				case 197:
				{
					PIXEL00_20
					PIXEL01_20
					PIXEL10_21
					PIXEL11_11
					break;
				}
				case 96:
				case 97:
				case 100:
				case 101:
				{
					PIXEL00_20
					PIXEL01_20
					PIXEL10_12
					PIXEL11_22
					break;
				}
				case 40:
				case 44:
				case 168:
				case 172:
				{
					PIXEL00_21
					PIXEL01_20
					PIXEL10_11
					PIXEL11_20
					break;
				}
				case 9:
				case 13:
				case 137:
				case 141:
				{
					PIXEL00_12
					PIXEL01_20
					PIXEL10_22
					PIXEL11_20
					break;
				}
				case 18:
				case 50:
				{
					PIXEL00_22
					if (diff(w[2], w[6]))
					{
						PIXEL01_10
					}
					else
					{
						PIXEL01_20
					}
					PIXEL10_20
					PIXEL11_21
					break;
				}
				case 80:
				case 81:
				{
					PIXEL00_20
					PIXEL01_22
					PIXEL10_21
					if (diff(w[6], w[8]))
					{
						PIXEL11_10
					}
					else
					{
						PIXEL11_20
					}
					break;
				}
				case 72:
				case 76:
				{
					PIXEL00_21
					PIXEL01_20
					if (diff(w[8], w[4]))
					{
						PIXEL10_10
					}
					else
					{
						PIXEL10_20
					}
					PIXEL11_22
					break;
				}
				case 10:
				case 138:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_10
					}
					else
					{
						PIXEL00_20
					}
					PIXEL01_21
					PIXEL10_22
					PIXEL11_20
					break;
				}
				case 66:
				{
					PIXEL00_22
					PIXEL01_21
					PIXEL10_21
					PIXEL11_22
					break;
				}
				case 24:
				{
					PIXEL00_21
					PIXEL01_22
					PIXEL10_22
					PIXEL11_21
					break;
				}
				case 7:
				case 39:
				case 135:
				{
					PIXEL00_11
					PIXEL01_12
					PIXEL10_20
					PIXEL11_20
					break;
				}
				case 148:
				case 149:
				case 180:
				{
					PIXEL00_20
					PIXEL01_11
					PIXEL10_20
					PIXEL11_12
					break;
				}
				case 224:
				case 228:
				case 225:
				{
					PIXEL00_20
					PIXEL01_20
					PIXEL10_12
					PIXEL11_11
					break;
				}
				case 41:
				case 169:
				case 45:
				{
					PIXEL00_12
					PIXEL01_20
					PIXEL10_11
					PIXEL11_20
					break;
				}
				case 22:
				case 54:
				{
					PIXEL00_22
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_20
					}
					PIXEL10_20
					PIXEL11_21
					break;
				}
				case 208:
				case 209:
				{
					PIXEL00_20
					PIXEL01_22
					PIXEL10_21
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_20
					}
					break;
				}
				case 104:
				case 108:
				{
					PIXEL00_21
					PIXEL01_20
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_20
					}
					PIXEL11_22
					break;
				}
				case 11:
				case 139:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_20
					}
					PIXEL01_21
					PIXEL10_22
					PIXEL11_20
					break;
				}
				case 19:
				case 51:
				{
					if (diff(w[2], w[6]))
					{
						PIXEL00_11
						PIXEL01_10
					}
					else
					{
						PIXEL00_60
						PIXEL01_90
					}
					PIXEL10_20
					PIXEL11_21
					break;
				}
				case 146:
				case 178:
				{
					PIXEL00_22
					if (diff(w[2], w[6]))
					{
						PIXEL01_10
						PIXEL11_12
					}
					else
					{
						PIXEL01_90
						PIXEL11_61
					}
					PIXEL10_20
					break;
				}
				case 84:
				case 85:
				{
					PIXEL00_20
					if (diff(w[6], w[8]))
					{
						PIXEL01_11
						PIXEL11_10
					}
					else
					{
						PIXEL01_60
						PIXEL11_90
					}
					PIXEL10_21
					break;
				}
				case 112:
				case 113:
				{
					PIXEL00_20
					PIXEL01_22
					if (diff(w[6], w[8]))
					{
						PIXEL10_12
						PIXEL11_10
					}
					else
					{
						PIXEL10_61
						PIXEL11_90
					}
					break;
				}
				case 200:
				case 204:
				{
					PIXEL00_21
					PIXEL01_20
					if (diff(w[8], w[4]))
					{
						PIXEL10_10
						PIXEL11_11
					}
					else
					{
						PIXEL10_90
						PIXEL11_60
					}
					break;
				}
				case 73:
				case 77:
				{
					if (diff(w[8], w[4]))
					{
						PIXEL00_12
						PIXEL10_10
					}
					else
					{
						PIXEL00_61
						PIXEL10_90
					}
					PIXEL01_20
					PIXEL11_22
					break;
				}
				case 42:
				case 170:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_10
						PIXEL10_11
					}
					else
					{
						PIXEL00_90
						PIXEL10_60
					}
					PIXEL01_21
					PIXEL11_20
					break;
				}
				case 14:
				case 142:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_10
						PIXEL01_12
					}
					else
					{
						PIXEL00_90
						PIXEL01_61
					}
					PIXEL10_22
					PIXEL11_20
					break;
				}
				case 67:
				{
					PIXEL00_11
					PIXEL01_21
					PIXEL10_21
					PIXEL11_22
					break;
				}
				case 70:
				{
					PIXEL00_22
					PIXEL01_12
					PIXEL10_21
					PIXEL11_22
					break;
				}
				case 28:
				{
					PIXEL00_21
					PIXEL01_11
					PIXEL10_22
					PIXEL11_21
					break;
				}
				case 152:
				{
					PIXEL00_21
					PIXEL01_22
					PIXEL10_22
					PIXEL11_12
					break;
				}
				case 194:
				{
					PIXEL00_22
					PIXEL01_21
					PIXEL10_21
					PIXEL11_11
					break;
				}
				case 98:
				{
					PIXEL00_22
					PIXEL01_21
					PIXEL10_12
					PIXEL11_22
					break;
				}
				case 56:
				{
					PIXEL00_21
					PIXEL01_22
					PIXEL10_11
					PIXEL11_21
					break;
				}
				case 25:
				{
					PIXEL00_12
					PIXEL01_22
					PIXEL10_22
					PIXEL11_21
					break;
				}
				case 26:
				case 31:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_20
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_20
					}
					PIXEL10_22
					PIXEL11_21
					break;
				}
				case 82:
				case 214:
				{
					PIXEL00_22
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_20
					}
					PIXEL10_21
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_20
					}
					break;
				}
				case 88:
				case 248:
				{
					PIXEL00_21
					PIXEL01_22
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_20
					}
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_20
					}
					break;
				}
				case 74:
				case 107:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_20
					}
					PIXEL01_21
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_20
					}
					PIXEL11_22
					break;
				}
				case 27:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_20
					}
					PIXEL01_10
					PIXEL10_22
					PIXEL11_21
					break;
				}
				case 86:
				{
					PIXEL00_22
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_20
					}
					PIXEL10_21
					PIXEL11_10
					break;
				}
				case 216:
				{
					PIXEL00_21
					PIXEL01_22
					PIXEL10_10
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_20
					}
					break;
				}
				case 106:
				{
					PIXEL00_10
					PIXEL01_21
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_20
					}
					PIXEL11_22
					break;
				}
				case 30:
				{
					PIXEL00_10
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_20
					}
					PIXEL10_22
					PIXEL11_21
					break;
				}
				case 210:
				{
					PIXEL00_22
					PIXEL01_10
					PIXEL10_21
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_20
					}
					break;
				}
				case 120:
				{
					PIXEL00_21
					PIXEL01_22
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_20
					}
					PIXEL11_10
					break;
				}
				case 75:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_20
					}
					PIXEL01_21
					PIXEL10_10
					PIXEL11_22
					break;
				}
				case 29:
				{
					PIXEL00_12
					PIXEL01_11
					PIXEL10_22
					PIXEL11_21
					break;
				}
				case 198:
				{
					PIXEL00_22
					PIXEL01_12
					PIXEL10_21
					PIXEL11_11
					break;
				}
				case 184:
				{
					PIXEL00_21
					PIXEL01_22
					PIXEL10_11
					PIXEL11_12
					break;
				}
				case 99:
				{
					PIXEL00_11
					PIXEL01_21
					PIXEL10_12
					PIXEL11_22
					break;
				}
				case 57:
				{
					PIXEL00_12
					PIXEL01_22
					PIXEL10_11
					PIXEL11_21
					break;
				}
				case 71:
				{
					PIXEL00_11
					PIXEL01_12
					PIXEL10_21
					PIXEL11_22
					break;
				}
				case 156:
				{
					PIXEL00_21
					PIXEL01_11
					PIXEL10_22
					PIXEL11_12
					break;
				}
				case 226:
				{
					PIXEL00_22
					PIXEL01_21
					PIXEL10_12
					PIXEL11_11
					break;
				}
				case 60:
				{
					PIXEL00_21
					PIXEL01_11
					PIXEL10_11
					PIXEL11_21
					break;
				}
				case 195:
				{
					PIXEL00_11
					PIXEL01_21
					PIXEL10_21
					PIXEL11_11
					break;
				}
				case 102:
				{
					PIXEL00_22
					PIXEL01_12
					PIXEL10_12
					PIXEL11_22
					break;
				}
				case 153:
				{
					PIXEL00_12
					PIXEL01_22
					PIXEL10_22
					PIXEL11_12
					break;
				}
				case 58:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_10
					}
					else
					{
						PIXEL00_70
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_10
					}
					else
					{
						PIXEL01_70
					}
					PIXEL10_11
					PIXEL11_21
					break;
				}
				case 83:
				{
					PIXEL00_11
					if (diff(w[2], w[6]))
					{
						PIXEL01_10
					}
					else
					{
						PIXEL01_70
					}
					PIXEL10_21
					if (diff(w[6], w[8]))
					{
						PIXEL11_10
					}
					else
					{
						PIXEL11_70
					}
					break;
				}
				case 92:
				{
					PIXEL00_21
					PIXEL01_11
					if (diff(w[8], w[4]))
					{
						PIXEL10_10
					}
					else
					{
						PIXEL10_70
					}
					if (diff(w[6], w[8]))
					{
						PIXEL11_10
					}
					else
					{
						PIXEL11_70
					}
					break;
				}
				case 202:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_10
					}
					else
					{
						PIXEL00_70
					}
					PIXEL01_21
					if (diff(w[8], w[4]))
					{
						PIXEL10_10
					}
					else
					{
						PIXEL10_70
					}
					PIXEL11_11
					break;
				}
				case 78:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_10
					}
					else
					{
						PIXEL00_70
					}
					PIXEL01_12
					if (diff(w[8], w[4]))
					{
						PIXEL10_10
					}
					else
					{
						PIXEL10_70
					}
					PIXEL11_22
					break;
				}
				case 154:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_10
					}
					else
					{
						PIXEL00_70
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_10
					}
					else
					{
						PIXEL01_70
					}
					PIXEL10_22
					PIXEL11_12
					break;
				}
				case 114:
				{
					PIXEL00_22
					if (diff(w[2], w[6]))
					{
						PIXEL01_10
					}
					else
					{
						PIXEL01_70
					}
					PIXEL10_12
					if (diff(w[6], w[8]))
					{
						PIXEL11_10
					}
					else
					{
						PIXEL11_70
					}
					break;
				}
				case 89:
				{
					PIXEL00_12
					PIXEL01_22
					if (diff(w[8], w[4]))
					{
						PIXEL10_10
					}
					else
					{
						PIXEL10_70
					}
					if (diff(w[6], w[8]))
					{
						PIXEL11_10
					}
					else
					{
						PIXEL11_70
					}
					break;
				}
				case 90:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_10
					}
					else
					{
						PIXEL00_70
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_10
					}
					else
					{
						PIXEL01_70
					}
					if (diff(w[8], w[4]))
					{
						PIXEL10_10
					}
					else
					{
						PIXEL10_70
					}
					if (diff(w[6], w[8]))
					{
						PIXEL11_10
					}
					else
					{
						PIXEL11_70
					}
					break;
				}
				case 55:
				case 23:
				{
					if (diff(w[2], w[6]))
					{
						PIXEL00_11
						PIXEL01_0
					}
					else
					{
						PIXEL00_60
						PIXEL01_90
					}
					PIXEL10_20
					PIXEL11_21
					break;
				}
				case 182:
				case 150:
				{
					PIXEL00_22
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
						PIXEL11_12
					}
					else
					{
						PIXEL01_90
						PIXEL11_61
					}
					PIXEL10_20
					break;
				}
				case 213:
				case 212:
				{
					PIXEL00_20
					if (diff(w[6], w[8]))
					{
						PIXEL01_11
						PIXEL11_0
					}
					else
					{
						PIXEL01_60
						PIXEL11_90
					}
					PIXEL10_21
					break;
				}
				case 241:
				case 240:
				{
					PIXEL00_20
					PIXEL01_22
					if (diff(w[6], w[8]))
					{
						PIXEL10_12
						PIXEL11_0
					}
					else
					{
						PIXEL10_61
						PIXEL11_90
					}
					break;
				}
				case 236:
				case 232:
				{
					PIXEL00_21
					PIXEL01_20
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
						PIXEL11_11
					}
					else
					{
						PIXEL10_90
						PIXEL11_60
					}
					break;
				}
				case 109:
				case 105:
				{
					if (diff(w[8], w[4]))
					{
						PIXEL00_12
						PIXEL10_0
					}
					else
					{
						PIXEL00_61
						PIXEL10_90
					}
					PIXEL01_20
					PIXEL11_22
					break;
				}
				case 171:
				case 43:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
						PIXEL10_11
					}
					else
					{
						PIXEL00_90
						PIXEL10_60
					}
					PIXEL01_21
					PIXEL11_20
					break;
				}
				case 143:
				case 15:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
						PIXEL01_12
					}
					else
					{
						PIXEL00_90
						PIXEL01_61
					}
					PIXEL10_22
					PIXEL11_20
					break;
				}
				case 124:
				{
					PIXEL00_21
					PIXEL01_11
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_20
					}
					PIXEL11_10
					break;
				}
				case 203:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_20
					}
					PIXEL01_21
					PIXEL10_10
					PIXEL11_11
					break;
				}
				case 62:
				{
					PIXEL00_10
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_20
					}
					PIXEL10_11
					PIXEL11_21
					break;
				}
				case 211:
				{
					PIXEL00_11
					PIXEL01_10
					PIXEL10_21
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_20
					}
					break;
				}
				case 118:
				{
					PIXEL00_22
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_20
					}
					PIXEL10_12
					PIXEL11_10
					break;
				}
				case 217:
				{
					PIXEL00_12
					PIXEL01_22
					PIXEL10_10
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_20
					}
					break;
				}
				case 110:
				{
					PIXEL00_10
					PIXEL01_12
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_20
					}
					PIXEL11_22
					break;
				}
				case 155:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_20
					}
					PIXEL01_10
					PIXEL10_22
					PIXEL11_12
					break;
				}
				case 188:
				{
					PIXEL00_21
					PIXEL01_11
					PIXEL10_11
					PIXEL11_12
					break;
				}
				case 185:
				{
					PIXEL00_12
					PIXEL01_22
					PIXEL10_11
					PIXEL11_12
					break;
				}
				case 61:
				{
					PIXEL00_12
					PIXEL01_11
					PIXEL10_11
					PIXEL11_21
					break;
				}
				case 157:
				{
					PIXEL00_12
					PIXEL01_11
					PIXEL10_22
					PIXEL11_12
					break;
				}
				case 103:
				{
					PIXEL00_11
					PIXEL01_12
					PIXEL10_12
					PIXEL11_22
					break;
				}
				case 227:
				{
					PIXEL00_11
					PIXEL01_21
					PIXEL10_12
					PIXEL11_11
					break;
				}
				case 230:
				{
					PIXEL00_22
					PIXEL01_12
					PIXEL10_12
					PIXEL11_11
					break;
				}
				case 199:
				{
					PIXEL00_11
					PIXEL01_12
					PIXEL10_21
					PIXEL11_11
					break;
				}
				case 220:
				{
					PIXEL00_21
					PIXEL01_11
					if (diff(w[8], w[4]))
					{
						PIXEL10_10
					}
					else
					{
						PIXEL10_70
					}
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_20
					}
					break;
				}
				case 158:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_10
					}
					else
					{
						PIXEL00_70
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_20
					}
					PIXEL10_22
					PIXEL11_12
					break;
				}
				case 234:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_10
					}
					else
					{
						PIXEL00_70
					}
					PIXEL01_21
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_20
					}
					PIXEL11_11
					break;
				}
				case 242:
				{
					PIXEL00_22
					if (diff(w[2], w[6]))
					{
						PIXEL01_10
					}
					else
					{
						PIXEL01_70
					}
					PIXEL10_12
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_20
					}
					break;
				}
				case 59:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_20
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_10
					}
					else
					{
						PIXEL01_70
					}
					PIXEL10_11
					PIXEL11_21
					break;
				}
				case 121:
				{
					PIXEL00_12
					PIXEL01_22
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_20
					}
					if (diff(w[6], w[8]))
					{
						PIXEL11_10
					}
					else
					{
						PIXEL11_70
					}
					break;
				}
				case 87:
				{
					PIXEL00_11
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_20
					}
					PIXEL10_21
					if (diff(w[6], w[8]))
					{
						PIXEL11_10
					}
					else
					{
						PIXEL11_70
					}
					break;
				}
				case 79:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_20
					}
					PIXEL01_12
					if (diff(w[8], w[4]))
					{
						PIXEL10_10
					}
					else
					{
						PIXEL10_70
					}
					PIXEL11_22
					break;
				}
				case 122:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_10
					}
					else
					{
						PIXEL00_70
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_10
					}
					else
					{
						PIXEL01_70
					}
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_20
					}
					if (diff(w[6], w[8]))
					{
						PIXEL11_10
					}
					else
					{
						PIXEL11_70
					}
					break;
				}
				case 94:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_10
					}
					else
					{
						PIXEL00_70
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_20
					}
					if (diff(w[8], w[4]))
					{
						PIXEL10_10
					}
					else
					{
						PIXEL10_70
					}
					if (diff(w[6], w[8]))
					{
						PIXEL11_10
					}
					else
					{
						PIXEL11_70
					}
					break;
				}
				case 218:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_10
					}
					else
					{
						PIXEL00_70
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_10
					}
					else
					{
						PIXEL01_70
					}
					if (diff(w[8], w[4]))
					{
						PIXEL10_10
					}
					else
					{
						PIXEL10_70
					}
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_20
					}
					break;
				}
				case 91:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_20
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_10
					}
					else
					{
						PIXEL01_70
					}
					if (diff(w[8], w[4]))
					{
						PIXEL10_10
					}
					else
					{
						PIXEL10_70
					}
					if (diff(w[6], w[8]))
					{
						PIXEL11_10
					}
					else
					{
						PIXEL11_70
					}
					break;
				}
				case 229:
				{
					PIXEL00_20
					PIXEL01_20
					PIXEL10_12
					PIXEL11_11
					break;
				}
				case 167:
				{
					PIXEL00_11
					PIXEL01_12
					PIXEL10_20
					PIXEL11_20
					break;
				}
				case 173:
				{
					PIXEL00_12
					PIXEL01_20
					PIXEL10_11
					PIXEL11_20
					break;
				}
				case 181:
				{
					PIXEL00_20
					PIXEL01_11
					PIXEL10_20
					PIXEL11_12
					break;
				}
				case 186:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_10
					}
					else
					{
						PIXEL00_70
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_10
					}
					else
					{
						PIXEL01_70
					}
					PIXEL10_11
					PIXEL11_12
					break;
				}
				case 115:
				{
					PIXEL00_11
					if (diff(w[2], w[6]))
					{
						PIXEL01_10
					}
					else
					{
						PIXEL01_70
					}
					PIXEL10_12
					if (diff(w[6], w[8]))
					{
						PIXEL11_10
					}
					else
					{
						PIXEL11_70
					}
					break;
				}
				case 93:
				{
					PIXEL00_12
					PIXEL01_11
					if (diff(w[8], w[4]))
					{
						PIXEL10_10
					}
					else
					{
						PIXEL10_70
					}
					if (diff(w[6], w[8]))
					{
						PIXEL11_10
					}
					else
					{
						PIXEL11_70
					}
					break;
				}
				case 206:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_10
					}
					else
					{
						PIXEL00_70
					}
					PIXEL01_12
					if (diff(w[8], w[4]))
					{
						PIXEL10_10
					}
					else
					{
						PIXEL10_70
					}
					PIXEL11_11
					break;
				}
				case 205:
				case 201:
				{
					PIXEL00_12
					PIXEL01_20
					if (diff(w[8], w[4]))
					{
						PIXEL10_10
					}
					else
					{
						PIXEL10_70
					}
					PIXEL11_11
					break;
				}
				case 174:
				case 46:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_10
					}
					else
					{
						PIXEL00_70
					}
					PIXEL01_12
					PIXEL10_11
					PIXEL11_20
					break;
				}
				case 179:
				case 147:
				{
					PIXEL00_11
					if (diff(w[2], w[6]))
					{
						PIXEL01_10
					}
					else
					{
						PIXEL01_70
					}
					PIXEL10_20
					PIXEL11_12
					break;
				}
				case 117:
				case 116:
				{
					PIXEL00_20
					PIXEL01_11
					PIXEL10_12
					if (diff(w[6], w[8]))
					{
						PIXEL11_10
					}
					else
					{
						PIXEL11_70
					}
					break;
				}
				case 189:
				{
					PIXEL00_12
					PIXEL01_11
					PIXEL10_11
					PIXEL11_12
					break;
				}
				case 231:
				{
					PIXEL00_11
					PIXEL01_12
					PIXEL10_12
					PIXEL11_11
					break;
				}
				case 126:
				{
					PIXEL00_10
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_20
					}
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_20
					}
					PIXEL11_10
					break;
				}
				case 219:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_20
					}
					PIXEL01_10
					PIXEL10_10
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_20
					}
					break;
				}
				case 125:
				{
					if (diff(w[8], w[4]))
					{
						PIXEL00_12
						PIXEL10_0
					}
					else
					{
						PIXEL00_61
						PIXEL10_90
					}
					PIXEL01_11
					PIXEL11_10
					break;
				}
				case 221:
				{
					PIXEL00_12
					if (diff(w[6], w[8]))
					{
						PIXEL01_11
						PIXEL11_0
					}
					else
					{
						PIXEL01_60
						PIXEL11_90
					}
					PIXEL10_10
					break;
				}
				case 207:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
						PIXEL01_12
					}
					else
					{
						PIXEL00_90
						PIXEL01_61
					}
					PIXEL10_10
					PIXEL11_11
					break;
				}
				case 238:
				{
					PIXEL00_10
					PIXEL01_12
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
						PIXEL11_11
					}
					else
					{
						PIXEL10_90
						PIXEL11_60
					}
					break;
				}
				case 190:
				{
					PIXEL00_10
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
						PIXEL11_12
					}
					else
					{
						PIXEL01_90
						PIXEL11_61
					}
					PIXEL10_11
					break;
				}
				case 187:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
						PIXEL10_11
					}
					else
					{
						PIXEL00_90
						PIXEL10_60
					}
					PIXEL01_10
					PIXEL11_12
					break;
				}
				case 243:
				{
					PIXEL00_11
					PIXEL01_10
					if (diff(w[6], w[8]))
					{
						PIXEL10_12
						PIXEL11_0
					}
					else
					{
						PIXEL10_61
						PIXEL11_90
					}
					break;
				}
				case 119:
				{
					if (diff(w[2], w[6]))
					{
						PIXEL00_11
						PIXEL01_0
					}
					else
					{
						PIXEL00_60
						PIXEL01_90
					}
					PIXEL10_12
					PIXEL11_10
					break;
				}
				case 237:
				case 233:
				{
					PIXEL00_12
					PIXEL01_20
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_100
					}
					PIXEL11_11
					break;
				}
				case 175:
				case 47:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_100
					}
					PIXEL01_12
					PIXEL10_11
					PIXEL11_20
					break;
				}
				case 183:
				case 151:
				{
					PIXEL00_11
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_100
					}
					PIXEL10_20
					PIXEL11_12
					break;
				}
				case 245:
				case 244:
				{
					PIXEL00_20
					PIXEL01_11
					PIXEL10_12
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_100
					}
					break;
				}
				case 250:
				{
					PIXEL00_10
					PIXEL01_10
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_20
					}
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_20
					}
					break;
				}
				case 123:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_20
					}
					PIXEL01_10
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_20
					}
					PIXEL11_10
					break;
				}
				case 95:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_20
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_20
					}
					PIXEL10_10
					PIXEL11_10
					break;
				}
				case 222:
				{
					PIXEL00_10
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_20
					}
					PIXEL10_10
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_20
					}
					break;
				}
				case 252:
				{
					PIXEL00_21
					PIXEL01_11
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_20
					}
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_100
					}
					break;
				}
				case 249:
				{
					PIXEL00_12
					PIXEL01_22
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_100
					}
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_20
					}
					break;
				}
				case 235:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_20
					}
					PIXEL01_21
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_100
					}
					PIXEL11_11
					break;
				}
				case 111:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_100
					}
					PIXEL01_12
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_20
					}
					PIXEL11_22
					break;
				}
				case 63:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_100
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_20
					}
					PIXEL10_11
					PIXEL11_21
					break;
				}
				case 159:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_20
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_100
					}
					PIXEL10_22
					PIXEL11_12
					break;
				}
				case 215:
				{
					PIXEL00_11
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_100
					}
					PIXEL10_21
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_20
					}
					break;
				}
				case 246:
				{
					PIXEL00_22
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_20
					}
					PIXEL10_12
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_100
					}
					break;
				}
				case 254:
				{
					PIXEL00_10
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_20
					}
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_20
					}
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_100
					}
					break;
				}
				case 253:
				{
					PIXEL00_12
					PIXEL01_11
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_100
					}
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_100
					}
					break;
				}
				case 251:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_20
					}
					PIXEL01_10
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_100
					}
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_20
					}
					break;
				}
				case 239:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_100
					}
					PIXEL01_12
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_100
					}
					PIXEL11_11
					break;
				}
				case 127:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_100
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_20
					}
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_20
					}
					PIXEL11_10
					break;
				}
				case 191:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_100
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_100
					}
					PIXEL10_11
					PIXEL11_12
					break;
				}
				case 223:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_20
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_100
					}
					PIXEL10_10
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_20
					}
					break;
				}
				case 247:
				{
					PIXEL00_11
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_100
					}
					PIXEL10_12
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_100
					}
					break;
				}
				case 255:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_0
					}
					else
					{
						PIXEL00_100
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_0
					}
					else
					{
						PIXEL01_100
					}
					if (diff(w[8], w[4]))
					{
						PIXEL10_0
					}
					else
					{
						PIXEL10_100
					}
					if (diff(w[6], w[8]))
					{
						PIXEL11_0
					}
					else
					{
						PIXEL11_100
					}
					break;
				}
			}
			
			src++;
			dst += 2 * dst_Bpp;
		}
		
		src = src_temp + src_pitch;
		dst = dst_temp + 2 * dst_pitch;
	}
}


#define PIXEL00_1M  interp1((Uint32 *)dst, c[5], c[1]);
#define PIXEL00_1U  interp1((Uint32 *)dst, c[5], c[2]);
#define PIXEL00_1L  interp1((Uint32 *)dst, c[5], c[4]);
#define PIXEL00_2   interp2((Uint32 *)dst, c[5], c[4], c[2]);
#define PIXEL00_4   interp4((Uint32 *)dst, c[5], c[4], c[2]);
#define PIXEL00_5   interp5((Uint32 *)dst, c[4], c[2]);
#define PIXEL00_C   *(Uint32 *)dst   = c[5];

#define PIXEL01_1   interp1((Uint32 *)(dst + dst_Bpp), c[5], c[2]);
#define PIXEL01_3   interp3((Uint32 *)(dst + dst_Bpp), c[5], c[2]);
#define PIXEL01_6   interp1((Uint32 *)(dst + dst_Bpp), c[2], c[5]);
#define PIXEL01_C   *(Uint32 *)(dst + dst_Bpp) = c[5];

#define PIXEL02_1M  interp1((Uint32 *)(dst + 2 * dst_Bpp), c[5], c[3]);
#define PIXEL02_1U  interp1((Uint32 *)(dst + 2 * dst_Bpp), c[5], c[2]);
#define PIXEL02_1R  interp1((Uint32 *)(dst + 2 * dst_Bpp), c[5], c[6]);
#define PIXEL02_2   interp2((Uint32 *)(dst + 2 * dst_Bpp), c[5], c[2], c[6]);
#define PIXEL02_4   interp4((Uint32 *)(dst + 2 * dst_Bpp), c[5], c[2], c[6]);
#define PIXEL02_5   interp5((Uint32 *)(dst + 2 * dst_Bpp), c[2], c[6]);
#define PIXEL02_C   *(Uint32 *)(dst + 2 * dst_Bpp) = c[5];

#define PIXEL10_1   interp1((Uint32 *)(dst + dst_pitch), c[5], c[4]);
#define PIXEL10_3   interp3((Uint32 *)(dst + dst_pitch), c[5], c[4]);
#define PIXEL10_6   interp1((Uint32 *)(dst + dst_pitch), c[4], c[5]);
#define PIXEL10_C   *(Uint32 *)(dst + dst_pitch) = c[5];

#define PIXEL11     *(Uint32 *)(dst + dst_pitch + dst_Bpp) = c[5];

#define PIXEL12_1   interp1((Uint32 *)(dst + dst_pitch + 2 * dst_Bpp), c[5], c[6]);
#define PIXEL12_3   interp3((Uint32 *)(dst + dst_pitch + 2 * dst_Bpp), c[5], c[6]);
#define PIXEL12_6   interp1((Uint32 *)(dst + dst_pitch + 2 * dst_Bpp), c[6], c[5]);
#define PIXEL12_C   *(Uint32 *)(dst + dst_pitch + 2 * dst_Bpp) = c[5];

#define PIXEL20_1M  interp1((Uint32 *)(dst + 2 * dst_pitch), c[5], c[7]);
#define PIXEL20_1D  interp1((Uint32 *)(dst + 2 * dst_pitch), c[5], c[8]);
#define PIXEL20_1L  interp1((Uint32 *)(dst + 2 * dst_pitch), c[5], c[4]);
#define PIXEL20_2   interp2((Uint32 *)(dst + 2 * dst_pitch), c[5], c[8], c[4]);
#define PIXEL20_4   interp4((Uint32 *)(dst + 2 * dst_pitch), c[5], c[8], c[4]);
#define PIXEL20_5   interp5((Uint32 *)(dst + 2 * dst_pitch), c[8], c[4]);
#define PIXEL20_C   *(Uint32 *)(dst + 2 * dst_pitch) = c[5];

#define PIXEL21_1   interp1((Uint32 *)(dst + 2 * dst_pitch + dst_Bpp), c[5], c[8]);
#define PIXEL21_3   interp3((Uint32 *)(dst + 2 * dst_pitch + dst_Bpp), c[5], c[8]);
#define PIXEL21_6   interp1((Uint32 *)(dst + 2 * dst_pitch + dst_Bpp), c[8], c[5]);
#define PIXEL21_C   *(Uint32 *)(dst + 2 * dst_pitch + dst_Bpp) = c[5];

#define PIXEL22_1M  interp1((Uint32 *)(dst + 2 * dst_pitch + 2 * dst_Bpp), c[5], c[9]);
#define PIXEL22_1D  interp1((Uint32 *)(dst + 2 * dst_pitch + 2 * dst_Bpp), c[5], c[8]);
#define PIXEL22_1R  interp1((Uint32 *)(dst + 2 * dst_pitch + 2 * dst_Bpp), c[5], c[6]);
#define PIXEL22_2   interp2((Uint32 *)(dst + 2 * dst_pitch + 2 * dst_Bpp), c[5], c[6], c[8]);
#define PIXEL22_4   interp4((Uint32 *)(dst + 2 * dst_pitch + 2 * dst_Bpp), c[5], c[6], c[8]);
#define PIXEL22_5   interp5((Uint32 *)(dst + 2 * dst_pitch + 2 * dst_Bpp), c[6], c[8]);
#define PIXEL22_C   *(Uint32 *)(dst + 2 * dst_pitch + 2 * dst_Bpp) = c[5];

void hq3x_32( SDL_Surface *src_surface, SDL_Surface *dst_surface )
{
	Uint8 *src = src_surface->pixels, *src_temp,
	      *dst = dst_surface->pixels, *dst_temp;
	int src_pitch = src_surface->pitch,
	    dst_pitch = dst_surface->pitch;
	const int dst_Bpp = 4;         // dst_surface->format->BytesPerPixel
	
	const int height = vga_height, // src_surface->h
	          width = vga_width;   // src_surface->w
	
	int prevline, nextline;
	
	Uint32 w[10];
	Uint32 c[10];
	
	//   +----+----+----+
	//   |    |    |    |
	//   | w1 | w2 | w3 |
	//   +----+----+----+
	//   |    |    |    |
	//   | w4 | w5 | w6 |
	//   +----+----+----+
	//   |    |    |    |
	//   | w7 | w8 | w9 |
	//   +----+----+----+
	
	for (int j = 0; j < height; j++)
	{
		src_temp = src;
		dst_temp = dst;
		
		prevline = (j > 0) ? -width : 0;
		nextline = (j < height - 1) ? width : 0;
		
		for (int i = 0; i < width; i++)
		{
			w[2] = *(src + prevline);
			w[5] = *src;
			w[8] = *(src + nextline);
			
			if (i>0)
			{
				w[1] = *(src + prevline - 1);
				w[4] = *(src - 1);
				w[7] = *(src + nextline - 1);
			} else {
				w[1] = w[2];
				w[4] = w[5];
				w[7] = w[8];
			}
			
			if (i < width - 1)
			{
				w[3] = *(src + prevline + 1);
				w[6] = *(src + 1);
				w[9] = *(src + nextline + 1);
			} else {
				w[3] = w[2];
				w[6] = w[5];
				w[9] = w[8];
			}
			
			int pattern = 0;
			int flag = 1;
			
			YUV1 = yuv_palette[w[5]];
			
			for (int k=1; k<=9; k++)
			{
				if (k==5) continue;
				
				if ( w[k] != w[5] )
				{
					YUV2 = yuv_palette[w[k]];
					if ( ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
					     ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
					     ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) )
					pattern |= flag;
				}
				flag <<= 1;
			}
			
			for (int k=1; k<=9; k++)
				c[k] = rgb_palette[w[k]] & 0xfcfcfcfc; // hq3x has a nasty inability to accept more than 6 bits for each component
			
			switch (pattern)
			{
				case 0:
				case 1:
				case 4:
				case 32:
				case 128:
				case 5:
				case 132:
				case 160:
				case 33:
				case 129:
				case 36:
				case 133:
				case 164:
				case 161:
				case 37:
				case 165:
				{
					PIXEL00_2
					PIXEL01_1
					PIXEL02_2
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_2
					PIXEL21_1
					PIXEL22_2
					break;
				}
				case 2:
				case 34:
				case 130:
				case 162:
				{
					PIXEL00_1M
					PIXEL01_C
					PIXEL02_1M
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_2
					PIXEL21_1
					PIXEL22_2
					break;
				}
				case 16:
				case 17:
				case 48:
				case 49:
				{
					PIXEL00_2
					PIXEL01_1
					PIXEL02_1M
					PIXEL10_1
					PIXEL11
					PIXEL12_C
					PIXEL20_2
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 64:
				case 65:
				case 68:
				case 69:
				{
					PIXEL00_2
					PIXEL01_1
					PIXEL02_2
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1M
					PIXEL21_C
					PIXEL22_1M
					break;
				}
				case 8:
				case 12:
				case 136:
				case 140:
				{
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_2
					PIXEL10_C
					PIXEL11
					PIXEL12_1
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_2
					break;
				}
				case 3:
				case 35:
				case 131:
				case 163:
				{
					PIXEL00_1L
					PIXEL01_C
					PIXEL02_1M
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_2
					PIXEL21_1
					PIXEL22_2
					break;
				}
				case 6:
				case 38:
				case 134:
				case 166:
				{
					PIXEL00_1M
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_2
					PIXEL21_1
					PIXEL22_2
					break;
				}
				case 20:
				case 21:
				case 52:
				case 53:
				{
					PIXEL00_2
					PIXEL01_1
					PIXEL02_1U
					PIXEL10_1
					PIXEL11
					PIXEL12_C
					PIXEL20_2
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 144:
				case 145:
				case 176:
				case 177:
				{
					PIXEL00_2
					PIXEL01_1
					PIXEL02_1M
					PIXEL10_1
					PIXEL11
					PIXEL12_C
					PIXEL20_2
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 192:
				case 193:
				case 196:
				case 197:
				{
					PIXEL00_2
					PIXEL01_1
					PIXEL02_2
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1M
					PIXEL21_C
					PIXEL22_1R
					break;
				}
				case 96:
				case 97:
				case 100:
				case 101:
				{
					PIXEL00_2
					PIXEL01_1
					PIXEL02_2
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1L
					PIXEL21_C
					PIXEL22_1M
					break;
				}
				case 40:
				case 44:
				case 168:
				case 172:
				{
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_2
					PIXEL10_C
					PIXEL11
					PIXEL12_1
					PIXEL20_1D
					PIXEL21_1
					PIXEL22_2
					break;
				}
				case 9:
				case 13:
				case 137:
				case 141:
				{
					PIXEL00_1U
					PIXEL01_1
					PIXEL02_2
					PIXEL10_C
					PIXEL11
					PIXEL12_1
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_2
					break;
				}
				case 18:
				case 50:
				{
					PIXEL00_1M
					if (diff(w[2], w[6]))
					{
						PIXEL01_C
						PIXEL02_1M
						PIXEL12_C
					}
					else
					{
						PIXEL01_3
						PIXEL02_4
						PIXEL12_3
					}
					PIXEL10_1
					PIXEL11
					PIXEL20_2
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 80:
				case 81:
				{
					PIXEL00_2
					PIXEL01_1
					PIXEL02_1M
					PIXEL10_1
					PIXEL11
					PIXEL20_1M
					if (diff(w[6], w[8]))
					{
						PIXEL12_C
						PIXEL21_C
						PIXEL22_1M
					}
					else
					{
						PIXEL12_3
						PIXEL21_3
						PIXEL22_4
					}
					break;
				}
				case 72:
				case 76:
				{
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_2
					PIXEL11
					PIXEL12_1
					if (diff(w[8], w[4]))
					{
						PIXEL10_C
						PIXEL20_1M
						PIXEL21_C
					}
					else
					{
						PIXEL10_3
						PIXEL20_4
						PIXEL21_3
					}
					PIXEL22_1M
					break;
				}
				case 10:
				case 138:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_1M
						PIXEL01_C
						PIXEL10_C
					}
					else
					{
						PIXEL00_4
						PIXEL01_3
						PIXEL10_3
					}
					PIXEL02_1M
					PIXEL11
					PIXEL12_1
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_2
					break;
				}
				case 66:
				{
					PIXEL00_1M
					PIXEL01_C
					PIXEL02_1M
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1M
					PIXEL21_C
					PIXEL22_1M
					break;
				}
				case 24:
				{
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_1M
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 7:
				case 39:
				case 135:
				{
					PIXEL00_1L
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_2
					PIXEL21_1
					PIXEL22_2
					break;
				}
				case 148:
				case 149:
				case 180:
				{
					PIXEL00_2
					PIXEL01_1
					PIXEL02_1U
					PIXEL10_1
					PIXEL11
					PIXEL12_C
					PIXEL20_2
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 224:
				case 228:
				case 225:
				{
					PIXEL00_2
					PIXEL01_1
					PIXEL02_2
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1L
					PIXEL21_C
					PIXEL22_1R
					break;
				}
				case 41:
				case 169:
				case 45:
				{
					PIXEL00_1U
					PIXEL01_1
					PIXEL02_2
					PIXEL10_C
					PIXEL11
					PIXEL12_1
					PIXEL20_1D
					PIXEL21_1
					PIXEL22_2
					break;
				}
				case 22:
				case 54:
				{
					PIXEL00_1M
					if (diff(w[2], w[6]))
					{
						PIXEL01_C
						PIXEL02_C
						PIXEL12_C
					}
					else
					{
						PIXEL01_3
						PIXEL02_4
						PIXEL12_3
					}
					PIXEL10_1
					PIXEL11
					PIXEL20_2
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 208:
				case 209:
				{
					PIXEL00_2
					PIXEL01_1
					PIXEL02_1M
					PIXEL10_1
					PIXEL11
					PIXEL20_1M
					if (diff(w[6], w[8]))
					{
						PIXEL12_C
						PIXEL21_C
						PIXEL22_C
					}
					else
					{
						PIXEL12_3
						PIXEL21_3
						PIXEL22_4
					}
					break;
				}
				case 104:
				case 108:
				{
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_2
					PIXEL11
					PIXEL12_1
					if (diff(w[8], w[4]))
					{
						PIXEL10_C
						PIXEL20_C
						PIXEL21_C
					}
					else
					{
						PIXEL10_3
						PIXEL20_4
						PIXEL21_3
					}
					PIXEL22_1M
					break;
				}
				case 11:
				case 139:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL01_C
						PIXEL10_C
					}
					else
					{
						PIXEL00_4
						PIXEL01_3
						PIXEL10_3
					}
					PIXEL02_1M
					PIXEL11
					PIXEL12_1
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_2
					break;
				}
				case 19:
				case 51:
				{
					if (diff(w[2], w[6]))
					{
						PIXEL00_1L
						PIXEL01_C
						PIXEL02_1M
						PIXEL12_C
					}
					else
					{
						PIXEL00_2
						PIXEL01_6
						PIXEL02_5
						PIXEL12_1
					}
					PIXEL10_1
					PIXEL11
					PIXEL20_2
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 146:
				case 178:
				{
					if (diff(w[2], w[6]))
					{
						PIXEL01_C
						PIXEL02_1M
						PIXEL12_C
						PIXEL22_1D
					}
					else
					{
						PIXEL01_1
						PIXEL02_5
						PIXEL12_6
						PIXEL22_2
					}
					PIXEL00_1M
					PIXEL10_1
					PIXEL11
					PIXEL20_2
					PIXEL21_1
					break;
				}
				case 84:
				case 85:
				{
					if (diff(w[6], w[8]))
					{
						PIXEL02_1U
						PIXEL12_C
						PIXEL21_C
						PIXEL22_1M
					}
					else
					{
						PIXEL02_2
						PIXEL12_6
						PIXEL21_1
						PIXEL22_5
					}
					PIXEL00_2
					PIXEL01_1
					PIXEL10_1
					PIXEL11
					PIXEL20_1M
					break;
				}
				case 112:
				case 113:
				{
					if (diff(w[6], w[8]))
					{
						PIXEL12_C
						PIXEL20_1L
						PIXEL21_C
						PIXEL22_1M
					}
					else
					{
						PIXEL12_1
						PIXEL20_2
						PIXEL21_6
						PIXEL22_5
					}
					PIXEL00_2
					PIXEL01_1
					PIXEL02_1M
					PIXEL10_1
					PIXEL11
					break;
				}
				case 200:
				case 204:
				{
					if (diff(w[8], w[4]))
					{
						PIXEL10_C
						PIXEL20_1M
						PIXEL21_C
						PIXEL22_1R
					}
					else
					{
						PIXEL10_1
						PIXEL20_5
						PIXEL21_6
						PIXEL22_2
					}
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_2
					PIXEL11
					PIXEL12_1
					break;
				}
				case 73:
				case 77:
				{
					if (diff(w[8], w[4]))
					{
						PIXEL00_1U
						PIXEL10_C
						PIXEL20_1M
						PIXEL21_C
					}
					else
					{
						PIXEL00_2
						PIXEL10_6
						PIXEL20_5
						PIXEL21_1
					}
					PIXEL01_1
					PIXEL02_2
					PIXEL11
					PIXEL12_1
					PIXEL22_1M
					break;
				}
				case 42:
				case 170:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_1M
						PIXEL01_C
						PIXEL10_C
						PIXEL20_1D
					}
					else
					{
						PIXEL00_5
						PIXEL01_1
						PIXEL10_6
						PIXEL20_2
					}
					PIXEL02_1M
					PIXEL11
					PIXEL12_1
					PIXEL21_1
					PIXEL22_2
					break;
				}
				case 14:
				case 142:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_1M
						PIXEL01_C
						PIXEL02_1R
						PIXEL10_C
					}
					else
					{
						PIXEL00_5
						PIXEL01_6
						PIXEL02_2
						PIXEL10_1
					}
					PIXEL11
					PIXEL12_1
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_2
					break;
				}
				case 67:
				{
					PIXEL00_1L
					PIXEL01_C
					PIXEL02_1M
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1M
					PIXEL21_C
					PIXEL22_1M
					break;
				}
				case 70:
				{
					PIXEL00_1M
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1M
					PIXEL21_C
					PIXEL22_1M
					break;
				}
				case 28:
				{
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_1U
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 152:
				{
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_1M
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 194:
				{
					PIXEL00_1M
					PIXEL01_C
					PIXEL02_1M
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1M
					PIXEL21_C
					PIXEL22_1R
					break;
				}
				case 98:
				{
					PIXEL00_1M
					PIXEL01_C
					PIXEL02_1M
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1L
					PIXEL21_C
					PIXEL22_1M
					break;
				}
				case 56:
				{
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_1M
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1D
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 25:
				{
					PIXEL00_1U
					PIXEL01_1
					PIXEL02_1M
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 26:
				case 31:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL10_C
					}
					else
					{
						PIXEL00_4
						PIXEL10_3
					}
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_C
						PIXEL12_C
					}
					else
					{
						PIXEL02_4
						PIXEL12_3
					}
					PIXEL11
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 82:
				case 214:
				{
					PIXEL00_1M
					if (diff(w[2], w[6]))
					{
						PIXEL01_C
						PIXEL02_C
					}
					else
					{
						PIXEL01_3
						PIXEL02_4
					}
					PIXEL10_1
					PIXEL11
					PIXEL12_C
					PIXEL20_1M
					if (diff(w[6], w[8]))
					{
						PIXEL21_C
						PIXEL22_C
					}
					else
					{
						PIXEL21_3
						PIXEL22_4
					}
					break;
				}
				case 88:
				case 248:
				{
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_1M
					PIXEL11
					if (diff(w[8], w[4]))
					{
						PIXEL10_C
						PIXEL20_C
					}
					else
					{
						PIXEL10_3
						PIXEL20_4
					}
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL12_C
						PIXEL22_C
					}
					else
					{
						PIXEL12_3
						PIXEL22_4
					}
					break;
				}
				case 74:
				case 107:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL01_C
					}
					else
					{
						PIXEL00_4
						PIXEL01_3
					}
					PIXEL02_1M
					PIXEL10_C
					PIXEL11
					PIXEL12_1
					if (diff(w[8], w[4]))
					{
						PIXEL20_C
						PIXEL21_C
					}
					else
					{
						PIXEL20_4
						PIXEL21_3
					}
					PIXEL22_1M
					break;
				}
				case 27:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL01_C
						PIXEL10_C
					}
					else
					{
						PIXEL00_4
						PIXEL01_3
						PIXEL10_3
					}
					PIXEL02_1M
					PIXEL11
					PIXEL12_C
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 86:
				{
					PIXEL00_1M
					if (diff(w[2], w[6]))
					{
						PIXEL01_C
						PIXEL02_C
						PIXEL12_C
					}
					else
					{
						PIXEL01_3
						PIXEL02_4
						PIXEL12_3
					}
					PIXEL10_1
					PIXEL11
					PIXEL20_1M
					PIXEL21_C
					PIXEL22_1M
					break;
				}
				case 216:
				{
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_1M
					PIXEL10_C
					PIXEL11
					PIXEL20_1M
					if (diff(w[6], w[8]))
					{
						PIXEL12_C
						PIXEL21_C
						PIXEL22_C
					}
					else
					{
						PIXEL12_3
						PIXEL21_3
						PIXEL22_4
					}
					break;
				}
				case 106:
				{
					PIXEL00_1M
					PIXEL01_C
					PIXEL02_1M
					PIXEL11
					PIXEL12_1
					if (diff(w[8], w[4]))
					{
						PIXEL10_C
						PIXEL20_C
						PIXEL21_C
					}
					else
					{
						PIXEL10_3
						PIXEL20_4
						PIXEL21_3
					}
					PIXEL22_1M
					break;
				}
				case 30:
				{
					PIXEL00_1M
					if (diff(w[2], w[6]))
					{
						PIXEL01_C
						PIXEL02_C
						PIXEL12_C
					}
					else
					{
						PIXEL01_3
						PIXEL02_4
						PIXEL12_3
					}
					PIXEL10_C
					PIXEL11
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 210:
				{
					PIXEL00_1M
					PIXEL01_C
					PIXEL02_1M
					PIXEL10_1
					PIXEL11
					PIXEL20_1M
					if (diff(w[6], w[8]))
					{
						PIXEL12_C
						PIXEL21_C
						PIXEL22_C
					}
					else
					{
						PIXEL12_3
						PIXEL21_3
						PIXEL22_4
					}
					break;
				}
				case 120:
				{
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_1M
					PIXEL11
					PIXEL12_C
					if (diff(w[8], w[4]))
					{
						PIXEL10_C
						PIXEL20_C
						PIXEL21_C
					}
					else
					{
						PIXEL10_3
						PIXEL20_4
						PIXEL21_3
					}
					PIXEL22_1M
					break;
				}
				case 75:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL01_C
						PIXEL10_C
					}
					else
					{
						PIXEL00_4
						PIXEL01_3
						PIXEL10_3
					}
					PIXEL02_1M
					PIXEL11
					PIXEL12_1
					PIXEL20_1M
					PIXEL21_C
					PIXEL22_1M
					break;
				}
				case 29:
				{
					PIXEL00_1U
					PIXEL01_1
					PIXEL02_1U
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 198:
				{
					PIXEL00_1M
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1M
					PIXEL21_C
					PIXEL22_1R
					break;
				}
				case 184:
				{
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_1M
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1D
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 99:
				{
					PIXEL00_1L
					PIXEL01_C
					PIXEL02_1M
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1L
					PIXEL21_C
					PIXEL22_1M
					break;
				}
				case 57:
				{
					PIXEL00_1U
					PIXEL01_1
					PIXEL02_1M
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1D
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 71:
				{
					PIXEL00_1L
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1M
					PIXEL21_C
					PIXEL22_1M
					break;
				}
				case 156:
				{
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_1U
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 226:
				{
					PIXEL00_1M
					PIXEL01_C
					PIXEL02_1M
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1L
					PIXEL21_C
					PIXEL22_1R
					break;
				}
				case 60:
				{
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_1U
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1D
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 195:
				{
					PIXEL00_1L
					PIXEL01_C
					PIXEL02_1M
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1M
					PIXEL21_C
					PIXEL22_1R
					break;
				}
				case 102:
				{
					PIXEL00_1M
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1L
					PIXEL21_C
					PIXEL22_1M
					break;
				}
				case 153:
				{
					PIXEL00_1U
					PIXEL01_1
					PIXEL02_1M
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 58:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_1M
					}
					else
					{
						PIXEL00_2
					}
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_1M
					}
					else
					{
						PIXEL02_2
					}
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1D
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 83:
				{
					PIXEL00_1L
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_1M
					}
					else
					{
						PIXEL02_2
					}
					PIXEL10_1
					PIXEL11
					PIXEL12_C
					PIXEL20_1M
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL22_1M
					}
					else
					{
						PIXEL22_2
					}
					break;
				}
				case 92:
				{
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_1U
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					if (diff(w[8], w[4]))
					{
						PIXEL20_1M
					}
					else
					{
						PIXEL20_2
					}
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL22_1M
					}
					else
					{
						PIXEL22_2
					}
					break;
				}
				case 202:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_1M
					}
					else
					{
						PIXEL00_2
					}
					PIXEL01_C
					PIXEL02_1M
					PIXEL10_C
					PIXEL11
					PIXEL12_1
					if (diff(w[8], w[4]))
					{
						PIXEL20_1M
					}
					else
					{
						PIXEL20_2
					}
					PIXEL21_C
					PIXEL22_1R
					break;
				}
				case 78:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_1M
					}
					else
					{
						PIXEL00_2
					}
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_C
					PIXEL11
					PIXEL12_1
					if (diff(w[8], w[4]))
					{
						PIXEL20_1M
					}
					else
					{
						PIXEL20_2
					}
					PIXEL21_C
					PIXEL22_1M
					break;
				}
				case 154:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_1M
					}
					else
					{
						PIXEL00_2
					}
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_1M
					}
					else
					{
						PIXEL02_2
					}
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 114:
				{
					PIXEL00_1M
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_1M
					}
					else
					{
						PIXEL02_2
					}
					PIXEL10_1
					PIXEL11
					PIXEL12_C
					PIXEL20_1L
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL22_1M
					}
					else
					{
						PIXEL22_2
					}
					break;
				}
				case 89:
				{
					PIXEL00_1U
					PIXEL01_1
					PIXEL02_1M
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					if (diff(w[8], w[4]))
					{
						PIXEL20_1M
					}
					else
					{
						PIXEL20_2
					}
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL22_1M
					}
					else
					{
						PIXEL22_2
					}
					break;
				}
				case 90:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_1M
					}
					else
					{
						PIXEL00_2
					}
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_1M
					}
					else
					{
						PIXEL02_2
					}
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					if (diff(w[8], w[4]))
					{
						PIXEL20_1M
					}
					else
					{
						PIXEL20_2
					}
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL22_1M
					}
					else
					{
						PIXEL22_2
					}
					break;
				}
				case 55:
				case 23:
				{
					if (diff(w[2], w[6]))
					{
						PIXEL00_1L
						PIXEL01_C
						PIXEL02_C
						PIXEL12_C
					}
					else
					{
						PIXEL00_2
						PIXEL01_6
						PIXEL02_5
						PIXEL12_1
					}
					PIXEL10_1
					PIXEL11
					PIXEL20_2
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 182:
				case 150:
				{
					if (diff(w[2], w[6]))
					{
						PIXEL01_C
						PIXEL02_C
						PIXEL12_C
						PIXEL22_1D
					}
					else
					{
						PIXEL01_1
						PIXEL02_5
						PIXEL12_6
						PIXEL22_2
					}
					PIXEL00_1M
					PIXEL10_1
					PIXEL11
					PIXEL20_2
					PIXEL21_1
					break;
				}
				case 213:
				case 212:
				{
					if (diff(w[6], w[8]))
					{
						PIXEL02_1U
						PIXEL12_C
						PIXEL21_C
						PIXEL22_C
					}
					else
					{
						PIXEL02_2
						PIXEL12_6
						PIXEL21_1
						PIXEL22_5
					}
					PIXEL00_2
					PIXEL01_1
					PIXEL10_1
					PIXEL11
					PIXEL20_1M
					break;
				}
				case 241:
				case 240:
				{
					if (diff(w[6], w[8]))
					{
						PIXEL12_C
						PIXEL20_1L
						PIXEL21_C
						PIXEL22_C
					}
					else
					{
						PIXEL12_1
						PIXEL20_2
						PIXEL21_6
						PIXEL22_5
					}
					PIXEL00_2
					PIXEL01_1
					PIXEL02_1M
					PIXEL10_1
					PIXEL11
					break;
				}
				case 236:
				case 232:
				{
					if (diff(w[8], w[4]))
					{
						PIXEL10_C
						PIXEL20_C
						PIXEL21_C
						PIXEL22_1R
					}
					else
					{
						PIXEL10_1
						PIXEL20_5
						PIXEL21_6
						PIXEL22_2
					}
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_2
					PIXEL11
					PIXEL12_1
					break;
				}
				case 109:
				case 105:
				{
					if (diff(w[8], w[4]))
					{
						PIXEL00_1U
						PIXEL10_C
						PIXEL20_C
						PIXEL21_C
					}
					else
					{
						PIXEL00_2
						PIXEL10_6
						PIXEL20_5
						PIXEL21_1
					}
					PIXEL01_1
					PIXEL02_2
					PIXEL11
					PIXEL12_1
					PIXEL22_1M
					break;
				}
				case 171:
				case 43:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL01_C
						PIXEL10_C
						PIXEL20_1D
					}
					else
					{
						PIXEL00_5
						PIXEL01_1
						PIXEL10_6
						PIXEL20_2
					}
					PIXEL02_1M
					PIXEL11
					PIXEL12_1
					PIXEL21_1
					PIXEL22_2
					break;
				}
				case 143:
				case 15:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL01_C
						PIXEL02_1R
						PIXEL10_C
					}
					else
					{
						PIXEL00_5
						PIXEL01_6
						PIXEL02_2
						PIXEL10_1
					}
					PIXEL11
					PIXEL12_1
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_2
					break;
				}
				case 124:
				{
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_1U
					PIXEL11
					PIXEL12_C
					if (diff(w[8], w[4]))
					{
						PIXEL10_C
						PIXEL20_C
						PIXEL21_C
					}
					else
					{
						PIXEL10_3
						PIXEL20_4
						PIXEL21_3
					}
					PIXEL22_1M
					break;
				}
				case 203:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL01_C
						PIXEL10_C
					}
					else
					{
						PIXEL00_4
						PIXEL01_3
						PIXEL10_3
					}
					PIXEL02_1M
					PIXEL11
					PIXEL12_1
					PIXEL20_1M
					PIXEL21_C
					PIXEL22_1R
					break;
				}
				case 62:
				{
					PIXEL00_1M
					if (diff(w[2], w[6]))
					{
						PIXEL01_C
						PIXEL02_C
						PIXEL12_C
					}
					else
					{
						PIXEL01_3
						PIXEL02_4
						PIXEL12_3
					}
					PIXEL10_C
					PIXEL11
					PIXEL20_1D
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 211:
				{
					PIXEL00_1L
					PIXEL01_C
					PIXEL02_1M
					PIXEL10_1
					PIXEL11
					PIXEL20_1M
					if (diff(w[6], w[8]))
					{
						PIXEL12_C
						PIXEL21_C
						PIXEL22_C
					}
					else
					{
						PIXEL12_3
						PIXEL21_3
						PIXEL22_4
					}
					break;
				}
				case 118:
				{
					PIXEL00_1M
					if (diff(w[2], w[6]))
					{
						PIXEL01_C
						PIXEL02_C
						PIXEL12_C
					}
					else
					{
						PIXEL01_3
						PIXEL02_4
						PIXEL12_3
					}
					PIXEL10_1
					PIXEL11
					PIXEL20_1L
					PIXEL21_C
					PIXEL22_1M
					break;
				}
				case 217:
				{
					PIXEL00_1U
					PIXEL01_1
					PIXEL02_1M
					PIXEL10_C
					PIXEL11
					PIXEL20_1M
					if (diff(w[6], w[8]))
					{
						PIXEL12_C
						PIXEL21_C
						PIXEL22_C
					}
					else
					{
						PIXEL12_3
						PIXEL21_3
						PIXEL22_4
					}
					break;
				}
				case 110:
				{
					PIXEL00_1M
					PIXEL01_C
					PIXEL02_1R
					PIXEL11
					PIXEL12_1
					if (diff(w[8], w[4]))
					{
						PIXEL10_C
						PIXEL20_C
						PIXEL21_C
					}
					else
					{
						PIXEL10_3
						PIXEL20_4
						PIXEL21_3
					}
					PIXEL22_1M
					break;
				}
				case 155:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL01_C
						PIXEL10_C
					}
					else
					{
						PIXEL00_4
						PIXEL01_3
						PIXEL10_3
					}
					PIXEL02_1M
					PIXEL11
					PIXEL12_C
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 188:
				{
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_1U
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1D
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 185:
				{
					PIXEL00_1U
					PIXEL01_1
					PIXEL02_1M
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1D
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 61:
				{
					PIXEL00_1U
					PIXEL01_1
					PIXEL02_1U
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1D
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 157:
				{
					PIXEL00_1U
					PIXEL01_1
					PIXEL02_1U
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 103:
				{
					PIXEL00_1L
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1L
					PIXEL21_C
					PIXEL22_1M
					break;
				}
				case 227:
				{
					PIXEL00_1L
					PIXEL01_C
					PIXEL02_1M
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1L
					PIXEL21_C
					PIXEL22_1R
					break;
				}
				case 230:
				{
					PIXEL00_1M
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1L
					PIXEL21_C
					PIXEL22_1R
					break;
				}
				case 199:
				{
					PIXEL00_1L
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1M
					PIXEL21_C
					PIXEL22_1R
					break;
				}
				case 220:
				{
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_1U
					PIXEL10_C
					PIXEL11
					if (diff(w[8], w[4]))
					{
						PIXEL20_1M
					}
					else
					{
						PIXEL20_2
					}
					if (diff(w[6], w[8]))
					{
						PIXEL12_C
						PIXEL21_C
						PIXEL22_C
					}
					else
					{
						PIXEL12_3
						PIXEL21_3
						PIXEL22_4
					}
					break;
				}
				case 158:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_1M
					}
					else
					{
						PIXEL00_2
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_C
						PIXEL02_C
						PIXEL12_C
					}
					else
					{
						PIXEL01_3
						PIXEL02_4
						PIXEL12_3
					}
					PIXEL10_C
					PIXEL11
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 234:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_1M
					}
					else
					{
						PIXEL00_2
					}
					PIXEL01_C
					PIXEL02_1M
					PIXEL11
					PIXEL12_1
					if (diff(w[8], w[4]))
					{
						PIXEL10_C
						PIXEL20_C
						PIXEL21_C
					}
					else
					{
						PIXEL10_3
						PIXEL20_4
						PIXEL21_3
					}
					PIXEL22_1R
					break;
				}
				case 242:
				{
					PIXEL00_1M
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_1M
					}
					else
					{
						PIXEL02_2
					}
					PIXEL10_1
					PIXEL11
					PIXEL20_1L
					if (diff(w[6], w[8]))
					{
						PIXEL12_C
						PIXEL21_C
						PIXEL22_C
					}
					else
					{
						PIXEL12_3
						PIXEL21_3
						PIXEL22_4
					}
					break;
				}
				case 59:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL01_C
						PIXEL10_C
					}
					else
					{
						PIXEL00_4
						PIXEL01_3
						PIXEL10_3
					}
					if (diff(w[2], w[6]))
					{
						PIXEL02_1M
					}
					else
					{
						PIXEL02_2
					}
					PIXEL11
					PIXEL12_C
					PIXEL20_1D
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 121:
				{
					PIXEL00_1U
					PIXEL01_1
					PIXEL02_1M
					PIXEL11
					PIXEL12_C
					if (diff(w[8], w[4]))
					{
						PIXEL10_C
						PIXEL20_C
						PIXEL21_C
					}
					else
					{
						PIXEL10_3
						PIXEL20_4
						PIXEL21_3
					}
					if (diff(w[6], w[8]))
					{
						PIXEL22_1M
					}
					else
					{
						PIXEL22_2
					}
					break;
				}
				case 87:
				{
					PIXEL00_1L
					if (diff(w[2], w[6]))
					{
						PIXEL01_C
						PIXEL02_C
						PIXEL12_C
					}
					else
					{
						PIXEL01_3
						PIXEL02_4
						PIXEL12_3
					}
					PIXEL10_1
					PIXEL11
					PIXEL20_1M
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL22_1M
					}
					else
					{
						PIXEL22_2
					}
					break;
				}
				case 79:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL01_C
						PIXEL10_C
					}
					else
					{
						PIXEL00_4
						PIXEL01_3
						PIXEL10_3
					}
					PIXEL02_1R
					PIXEL11
					PIXEL12_1
					if (diff(w[8], w[4]))
					{
						PIXEL20_1M
					}
					else
					{
						PIXEL20_2
					}
					PIXEL21_C
					PIXEL22_1M
					break;
				}
				case 122:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_1M
					}
					else
					{
						PIXEL00_2
					}
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_1M
					}
					else
					{
						PIXEL02_2
					}
					PIXEL11
					PIXEL12_C
					if (diff(w[8], w[4]))
					{
						PIXEL10_C
						PIXEL20_C
						PIXEL21_C
					}
					else
					{
						PIXEL10_3
						PIXEL20_4
						PIXEL21_3
					}
					if (diff(w[6], w[8]))
					{
						PIXEL22_1M
					}
					else
					{
						PIXEL22_2
					}
					break;
				}
				case 94:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_1M
					}
					else
					{
						PIXEL00_2
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_C
						PIXEL02_C
						PIXEL12_C
					}
					else
					{
						PIXEL01_3
						PIXEL02_4
						PIXEL12_3
					}
					PIXEL10_C
					PIXEL11
					if (diff(w[8], w[4]))
					{
						PIXEL20_1M
					}
					else
					{
						PIXEL20_2
					}
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL22_1M
					}
					else
					{
						PIXEL22_2
					}
					break;
				}
				case 218:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_1M
					}
					else
					{
						PIXEL00_2
					}
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_1M
					}
					else
					{
						PIXEL02_2
					}
					PIXEL10_C
					PIXEL11
					if (diff(w[8], w[4]))
					{
						PIXEL20_1M
					}
					else
					{
						PIXEL20_2
					}
					if (diff(w[6], w[8]))
					{
						PIXEL12_C
						PIXEL21_C
						PIXEL22_C
					}
					else
					{
						PIXEL12_3
						PIXEL21_3
						PIXEL22_4
					}
					break;
				}
				case 91:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL01_C
						PIXEL10_C
					}
					else
					{
						PIXEL00_4
						PIXEL01_3
						PIXEL10_3
					}
					if (diff(w[2], w[6]))
					{
						PIXEL02_1M
					}
					else
					{
						PIXEL02_2
					}
					PIXEL11
					PIXEL12_C
					if (diff(w[8], w[4]))
					{
						PIXEL20_1M
					}
					else
					{
						PIXEL20_2
					}
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL22_1M
					}
					else
					{
						PIXEL22_2
					}
					break;
				}
				case 229:
				{
					PIXEL00_2
					PIXEL01_1
					PIXEL02_2
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1L
					PIXEL21_C
					PIXEL22_1R
					break;
				}
				case 167:
				{
					PIXEL00_1L
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_2
					PIXEL21_1
					PIXEL22_2
					break;
				}
				case 173:
				{
					PIXEL00_1U
					PIXEL01_1
					PIXEL02_2
					PIXEL10_C
					PIXEL11
					PIXEL12_1
					PIXEL20_1D
					PIXEL21_1
					PIXEL22_2
					break;
				}
				case 181:
				{
					PIXEL00_2
					PIXEL01_1
					PIXEL02_1U
					PIXEL10_1
					PIXEL11
					PIXEL12_C
					PIXEL20_2
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 186:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_1M
					}
					else
					{
						PIXEL00_2
					}
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_1M
					}
					else
					{
						PIXEL02_2
					}
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1D
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 115:
				{
					PIXEL00_1L
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_1M
					}
					else
					{
						PIXEL02_2
					}
					PIXEL10_1
					PIXEL11
					PIXEL12_C
					PIXEL20_1L
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL22_1M
					}
					else
					{
						PIXEL22_2
					}
					break;
				}
				case 93:
				{
					PIXEL00_1U
					PIXEL01_1
					PIXEL02_1U
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					if (diff(w[8], w[4]))
					{
						PIXEL20_1M
					}
					else
					{
						PIXEL20_2
					}
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL22_1M
					}
					else
					{
						PIXEL22_2
					}
					break;
				}
				case 206:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_1M
					}
					else
					{
						PIXEL00_2
					}
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_C
					PIXEL11
					PIXEL12_1
					if (diff(w[8], w[4]))
					{
						PIXEL20_1M
					}
					else
					{
						PIXEL20_2
					}
					PIXEL21_C
					PIXEL22_1R
					break;
				}
				case 205:
				case 201:
				{
					PIXEL00_1U
					PIXEL01_1
					PIXEL02_2
					PIXEL10_C
					PIXEL11
					PIXEL12_1
					if (diff(w[8], w[4]))
					{
						PIXEL20_1M
					}
					else
					{
						PIXEL20_2
					}
					PIXEL21_C
					PIXEL22_1R
					break;
				}
				case 174:
				case 46:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_1M
					}
					else
					{
						PIXEL00_2
					}
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_C
					PIXEL11
					PIXEL12_1
					PIXEL20_1D
					PIXEL21_1
					PIXEL22_2
					break;
				}
				case 179:
				case 147:
				{
					PIXEL00_1L
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_1M
					}
					else
					{
						PIXEL02_2
					}
					PIXEL10_1
					PIXEL11
					PIXEL12_C
					PIXEL20_2
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 117:
				case 116:
				{
					PIXEL00_2
					PIXEL01_1
					PIXEL02_1U
					PIXEL10_1
					PIXEL11
					PIXEL12_C
					PIXEL20_1L
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL22_1M
					}
					else
					{
						PIXEL22_2
					}
					break;
				}
				case 189:
				{
					PIXEL00_1U
					PIXEL01_1
					PIXEL02_1U
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1D
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 231:
				{
					PIXEL00_1L
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_1
					PIXEL11
					PIXEL12_1
					PIXEL20_1L
					PIXEL21_C
					PIXEL22_1R
					break;
				}
				case 126:
				{
					PIXEL00_1M
					if (diff(w[2], w[6]))
					{
						PIXEL01_C
						PIXEL02_C
						PIXEL12_C
					}
					else
					{
						PIXEL01_3
						PIXEL02_4
						PIXEL12_3
					}
					PIXEL11
					if (diff(w[8], w[4]))
					{
						PIXEL10_C
						PIXEL20_C
						PIXEL21_C
					}
					else
					{
						PIXEL10_3
						PIXEL20_4
						PIXEL21_3
					}
					PIXEL22_1M
					break;
				}
				case 219:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL01_C
						PIXEL10_C
					}
					else
					{
						PIXEL00_4
						PIXEL01_3
						PIXEL10_3
					}
					PIXEL02_1M
					PIXEL11
					PIXEL20_1M
					if (diff(w[6], w[8]))
					{
						PIXEL12_C
						PIXEL21_C
						PIXEL22_C
					}
					else
					{
						PIXEL12_3
						PIXEL21_3
						PIXEL22_4
					}
					break;
				}
				case 125:
				{
					if (diff(w[8], w[4]))
					{
						PIXEL00_1U
						PIXEL10_C
						PIXEL20_C
						PIXEL21_C
					}
					else
					{
						PIXEL00_2
						PIXEL10_6
						PIXEL20_5
						PIXEL21_1
					}
					PIXEL01_1
					PIXEL02_1U
					PIXEL11
					PIXEL12_C
					PIXEL22_1M
					break;
				}
				case 221:
				{
					if (diff(w[6], w[8]))
					{
						PIXEL02_1U
						PIXEL12_C
						PIXEL21_C
						PIXEL22_C
					}
					else
					{
						PIXEL02_2
						PIXEL12_6
						PIXEL21_1
						PIXEL22_5
					}
					PIXEL00_1U
					PIXEL01_1
					PIXEL10_C
					PIXEL11
					PIXEL20_1M
					break;
				}
				case 207:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL01_C
						PIXEL02_1R
						PIXEL10_C
					}
					else
					{
						PIXEL00_5
						PIXEL01_6
						PIXEL02_2
						PIXEL10_1
					}
					PIXEL11
					PIXEL12_1
					PIXEL20_1M
					PIXEL21_C
					PIXEL22_1R
					break;
				}
				case 238:
				{
					if (diff(w[8], w[4]))
					{
						PIXEL10_C
						PIXEL20_C
						PIXEL21_C
						PIXEL22_1R
					}
					else
					{
						PIXEL10_1
						PIXEL20_5
						PIXEL21_6
						PIXEL22_2
					}
					PIXEL00_1M
					PIXEL01_C
					PIXEL02_1R
					PIXEL11
					PIXEL12_1
					break;
				}
				case 190:
				{
					if (diff(w[2], w[6]))
					{
						PIXEL01_C
						PIXEL02_C
						PIXEL12_C
						PIXEL22_1D
					}
					else
					{
						PIXEL01_1
						PIXEL02_5
						PIXEL12_6
						PIXEL22_2
					}
					PIXEL00_1M
					PIXEL10_C
					PIXEL11
					PIXEL20_1D
					PIXEL21_1
					break;
				}
				case 187:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL01_C
						PIXEL10_C
						PIXEL20_1D
					}
					else
					{
						PIXEL00_5
						PIXEL01_1
						PIXEL10_6
						PIXEL20_2
					}
					PIXEL02_1M
					PIXEL11
					PIXEL12_C
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 243:
				{
					if (diff(w[6], w[8]))
					{
						PIXEL12_C
						PIXEL20_1L
						PIXEL21_C
						PIXEL22_C
					}
					else
					{
						PIXEL12_1
						PIXEL20_2
						PIXEL21_6
						PIXEL22_5
					}
					PIXEL00_1L
					PIXEL01_C
					PIXEL02_1M
					PIXEL10_1
					PIXEL11
					break;
				}
				case 119:
				{
					if (diff(w[2], w[6]))
					{
						PIXEL00_1L
						PIXEL01_C
						PIXEL02_C
						PIXEL12_C
					}
					else
					{
						PIXEL00_2
						PIXEL01_6
						PIXEL02_5
						PIXEL12_1
					}
					PIXEL10_1
					PIXEL11
					PIXEL20_1L
					PIXEL21_C
					PIXEL22_1M
					break;
				}
				case 237:
				case 233:
				{
					PIXEL00_1U
					PIXEL01_1
					PIXEL02_2
					PIXEL10_C
					PIXEL11
					PIXEL12_1
					if (diff(w[8], w[4]))
					{
						PIXEL20_C
					}
					else
					{
						PIXEL20_2
					}
					PIXEL21_C
					PIXEL22_1R
					break;
				}
				case 175:
				case 47:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
					}
					else
					{
						PIXEL00_2
					}
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_C
					PIXEL11
					PIXEL12_1
					PIXEL20_1D
					PIXEL21_1
					PIXEL22_2
					break;
				}
				case 183:
				case 151:
				{
					PIXEL00_1L
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_C
					}
					else
					{
						PIXEL02_2
					}
					PIXEL10_1
					PIXEL11
					PIXEL12_C
					PIXEL20_2
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 245:
				case 244:
				{
					PIXEL00_2
					PIXEL01_1
					PIXEL02_1U
					PIXEL10_1
					PIXEL11
					PIXEL12_C
					PIXEL20_1L
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL22_C
					}
					else
					{
						PIXEL22_2
					}
					break;
				}
				case 250:
				{
					PIXEL00_1M
					PIXEL01_C
					PIXEL02_1M
					PIXEL11
					if (diff(w[8], w[4]))
					{
						PIXEL10_C
						PIXEL20_C
					}
					else
					{
						PIXEL10_3
						PIXEL20_4
					}
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL12_C
						PIXEL22_C
					}
					else
					{
						PIXEL12_3
						PIXEL22_4
					}
					break;
				}
				case 123:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL01_C
					}
					else
					{
						PIXEL00_4
						PIXEL01_3
					}
					PIXEL02_1M
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					if (diff(w[8], w[4]))
					{
						PIXEL20_C
						PIXEL21_C
					}
					else
					{
						PIXEL20_4
						PIXEL21_3
					}
					PIXEL22_1M
					break;
				}
				case 95:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL10_C
					}
					else
					{
						PIXEL00_4
						PIXEL10_3
					}
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_C
						PIXEL12_C
					}
					else
					{
						PIXEL02_4
						PIXEL12_3
					}
					PIXEL11
					PIXEL20_1M
					PIXEL21_C
					PIXEL22_1M
					break;
				}
				case 222:
				{
					PIXEL00_1M
					if (diff(w[2], w[6]))
					{
						PIXEL01_C
						PIXEL02_C
					}
					else
					{
						PIXEL01_3
						PIXEL02_4
					}
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1M
					if (diff(w[6], w[8]))
					{
						PIXEL21_C
						PIXEL22_C
					}
					else
					{
						PIXEL21_3
						PIXEL22_4
					}
					break;
				}
				case 252:
				{
					PIXEL00_1M
					PIXEL01_1
					PIXEL02_1U
					PIXEL11
					PIXEL12_C
					if (diff(w[8], w[4]))
					{
						PIXEL10_C
						PIXEL20_C
					}
					else
					{
						PIXEL10_3
						PIXEL20_4
					}
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL22_C
					}
					else
					{
						PIXEL22_2
					}
					break;
				}
				case 249:
				{
					PIXEL00_1U
					PIXEL01_1
					PIXEL02_1M
					PIXEL10_C
					PIXEL11
					if (diff(w[8], w[4]))
					{
						PIXEL20_C
					}
					else
					{
						PIXEL20_2
					}
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL12_C
						PIXEL22_C
					}
					else
					{
						PIXEL12_3
						PIXEL22_4
					}
					break;
				}
				case 235:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL01_C
					}
					else
					{
						PIXEL00_4
						PIXEL01_3
					}
					PIXEL02_1M
					PIXEL10_C
					PIXEL11
					PIXEL12_1
					if (diff(w[8], w[4]))
					{
						PIXEL20_C
					}
					else
					{
						PIXEL20_2
					}
					PIXEL21_C
					PIXEL22_1R
					break;
				}
				case 111:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
					}
					else
					{
						PIXEL00_2
					}
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_C
					PIXEL11
					PIXEL12_1
					if (diff(w[8], w[4]))
					{
						PIXEL20_C
						PIXEL21_C
					}
					else
					{
						PIXEL20_4
						PIXEL21_3
					}
					PIXEL22_1M
					break;
				}
				case 63:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
					}
					else
					{
						PIXEL00_2
					}
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_C
						PIXEL12_C
					}
					else
					{
						PIXEL02_4
						PIXEL12_3
					}
					PIXEL10_C
					PIXEL11
					PIXEL20_1D
					PIXEL21_1
					PIXEL22_1M
					break;
				}
				case 159:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL10_C
					}
					else
					{
						PIXEL00_4
						PIXEL10_3
					}
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_C
					}
					else
					{
						PIXEL02_2
					}
					PIXEL11
					PIXEL12_C
					PIXEL20_1M
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 215:
				{
					PIXEL00_1L
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_C
					}
					else
					{
						PIXEL02_2
					}
					PIXEL10_1
					PIXEL11
					PIXEL12_C
					PIXEL20_1M
					if (diff(w[6], w[8]))
					{
						PIXEL21_C
						PIXEL22_C
					}
					else
					{
						PIXEL21_3
						PIXEL22_4
					}
					break;
				}
				case 246:
				{
					PIXEL00_1M
					if (diff(w[2], w[6]))
					{
						PIXEL01_C
						PIXEL02_C
					}
					else
					{
						PIXEL01_3
						PIXEL02_4
					}
					PIXEL10_1
					PIXEL11
					PIXEL12_C
					PIXEL20_1L
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL22_C
					}
					else
					{
						PIXEL22_2
					}
					break;
				}
				case 254:
				{
					PIXEL00_1M
					if (diff(w[2], w[6]))
					{
						PIXEL01_C
						PIXEL02_C
					}
					else
					{
						PIXEL01_3
						PIXEL02_4
					}
					PIXEL11
					if (diff(w[8], w[4]))
					{
						PIXEL10_C
						PIXEL20_C
					}
					else
					{
						PIXEL10_3
						PIXEL20_4
					}
					if (diff(w[6], w[8]))
					{
						PIXEL12_C
						PIXEL21_C
						PIXEL22_C
					}
					else
					{
						PIXEL12_3
						PIXEL21_3
						PIXEL22_2
					}
					break;
				}
				case 253:
				{
					PIXEL00_1U
					PIXEL01_1
					PIXEL02_1U
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					if (diff(w[8], w[4]))
					{
						PIXEL20_C
					}
					else
					{
						PIXEL20_2
					}
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL22_C
					}
					else
					{
						PIXEL22_2
					}
					break;
				}
				case 251:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL01_C
					}
					else
					{
						PIXEL00_4
						PIXEL01_3
					}
					PIXEL02_1M
					PIXEL11
					if (diff(w[8], w[4]))
					{
						PIXEL10_C
						PIXEL20_C
						PIXEL21_C
					}
					else
					{
						PIXEL10_3
						PIXEL20_2
						PIXEL21_3
					}
					if (diff(w[6], w[8]))
					{
						PIXEL12_C
						PIXEL22_C
					}
					else
					{
						PIXEL12_3
						PIXEL22_4
					}
					break;
				}
				case 239:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
					}
					else
					{
						PIXEL00_2
					}
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_C
					PIXEL11
					PIXEL12_1
					if (diff(w[8], w[4]))
					{
						PIXEL20_C
					}
					else
					{
						PIXEL20_2
					}
					PIXEL21_C
					PIXEL22_1R
					break;
				}
				case 127:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL01_C
						PIXEL10_C
					}
					else
					{
						PIXEL00_2
						PIXEL01_3
						PIXEL10_3
					}
					if (diff(w[2], w[6]))
					{
						PIXEL02_C
						PIXEL12_C
					}
					else
					{
						PIXEL02_4
						PIXEL12_3
					}
					PIXEL11
					if (diff(w[8], w[4]))
					{
						PIXEL20_C
						PIXEL21_C
					}
					else
					{
						PIXEL20_4
						PIXEL21_3
					}
					PIXEL22_1M
					break;
				}
				case 191:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
					}
					else
					{
						PIXEL00_2
					}
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_C
					}
					else
					{
						PIXEL02_2
					}
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					PIXEL20_1D
					PIXEL21_1
					PIXEL22_1D
					break;
				}
				case 223:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
						PIXEL10_C
					}
					else
					{
						PIXEL00_4
						PIXEL10_3
					}
					if (diff(w[2], w[6]))
					{
						PIXEL01_C
						PIXEL02_C
						PIXEL12_C
					}
					else
					{
						PIXEL01_3
						PIXEL02_2
						PIXEL12_3
					}
					PIXEL11
					PIXEL20_1M
					if (diff(w[6], w[8]))
					{
						PIXEL21_C
						PIXEL22_C
					}
					else
					{
						PIXEL21_3
						PIXEL22_4
					}
					break;
				}
				case 247:
				{
					PIXEL00_1L
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_C
					}
					else
					{
						PIXEL02_2
					}
					PIXEL10_1
					PIXEL11
					PIXEL12_C
					PIXEL20_1L
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL22_C
					}
					else
					{
						PIXEL22_2
					}
					break;
				}
				case 255:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL00_C
					}
					else
					{
						PIXEL00_2
					}
					PIXEL01_C
					if (diff(w[2], w[6]))
					{
						PIXEL02_C
					}
					else
					{
						PIXEL02_2
					}
					PIXEL10_C
					PIXEL11
					PIXEL12_C
					if (diff(w[8], w[4]))
					{
						PIXEL20_C
					}
					else
					{
						PIXEL20_2
					}
					PIXEL21_C
					if (diff(w[6], w[8]))
					{
						PIXEL22_C
					}
					else
					{
						PIXEL22_2
					}
					break;
				}
			}
			
			src++;
			dst += 3 * dst_Bpp;
		}
		
		src = src_temp + src_pitch;
		dst = dst_temp + 3 * dst_pitch;
	}
}


#define PIXEL4_00_0     *(Uint32 *)(dst) = c[5];
#define PIXEL4_00_11    interp1((Uint32 *)(dst), c[5], c[4]);
#define PIXEL4_00_12    interp1((Uint32 *)(dst), c[5], c[2]);
#define PIXEL4_00_20    interp2((Uint32 *)(dst), c[5], c[2], c[4]);
#define PIXEL4_00_50    interp5((Uint32 *)(dst), c[2], c[4]);
#define PIXEL4_00_80    interp8((Uint32 *)(dst), c[5], c[1]);
#define PIXEL4_00_81    interp8((Uint32 *)(dst), c[5], c[4]);
#define PIXEL4_00_82    interp8((Uint32 *)(dst), c[5], c[2]);
#define PIXEL4_01_0     *(Uint32 *)(dst + dst_Bpp) = c[5];
#define PIXEL4_01_10    interp1((Uint32 *)(dst + dst_Bpp), c[5], c[1]);
#define PIXEL4_01_12    interp1((Uint32 *)(dst + dst_Bpp), c[5], c[2]);
#define PIXEL4_01_14    interp1((Uint32 *)(dst + dst_Bpp), c[2], c[5]);
#define PIXEL4_01_21    interp2((Uint32 *)(dst + dst_Bpp), c[2], c[5], c[4]);
#define PIXEL4_01_31    interp3((Uint32 *)(dst + dst_Bpp), c[5], c[4]);
#define PIXEL4_01_50    interp5((Uint32 *)(dst + dst_Bpp), c[2], c[5]);
#define PIXEL4_01_60    interp6((Uint32 *)(dst + dst_Bpp), c[5], c[2], c[4]);
#define PIXEL4_01_61    interp6((Uint32 *)(dst + dst_Bpp), c[5], c[2], c[1]);
#define PIXEL4_01_82    interp8((Uint32 *)(dst + dst_Bpp), c[5], c[2]);
#define PIXEL4_01_83    interp8((Uint32 *)(dst + dst_Bpp), c[2], c[4]);
#define PIXEL4_02_0     *(Uint32 *)(dst + 2 * dst_Bpp) = c[5];
#define PIXEL4_02_10    interp1((Uint32 *)(dst + 2 * dst_Bpp), c[5], c[3]);
#define PIXEL4_02_11    interp1((Uint32 *)(dst + 2 * dst_Bpp), c[5], c[2]);
#define PIXEL4_02_13    interp1((Uint32 *)(dst + 2 * dst_Bpp), c[2], c[5]);
#define PIXEL4_02_21    interp2((Uint32 *)(dst + 2 * dst_Bpp), c[2], c[5], c[6]);
#define PIXEL4_02_32    interp3((Uint32 *)(dst + 2 * dst_Bpp), c[5], c[6]);
#define PIXEL4_02_50    interp5((Uint32 *)(dst + 2 * dst_Bpp), c[2], c[5]);
#define PIXEL4_02_60    interp6((Uint32 *)(dst + 2 * dst_Bpp), c[5], c[2], c[6]);
#define PIXEL4_02_61    interp6((Uint32 *)(dst + 2 * dst_Bpp), c[5], c[2], c[3]);
#define PIXEL4_02_81    interp8((Uint32 *)(dst + 2 * dst_Bpp), c[5], c[2]);
#define PIXEL4_02_83    interp8((Uint32 *)(dst + 2 * dst_Bpp), c[2], c[6]);
#define PIXEL4_03_0     *(Uint32 *)(dst + 3 * dst_Bpp) = c[5];
#define PIXEL4_03_11    interp1((Uint32 *)(dst + 3 * dst_Bpp), c[5], c[2]);
#define PIXEL4_03_12    interp1((Uint32 *)(dst + 3 * dst_Bpp), c[5], c[6]);
#define PIXEL4_03_20    interp2((Uint32 *)(dst + 3 * dst_Bpp), c[5], c[2], c[6]);
#define PIXEL4_03_50    interp5((Uint32 *)(dst + 3 * dst_Bpp), c[2], c[6]);
#define PIXEL4_03_80    interp8((Uint32 *)(dst + 3 * dst_Bpp), c[5], c[3]);
#define PIXEL4_03_81    interp8((Uint32 *)(dst + 3 * dst_Bpp), c[5], c[2]);
#define PIXEL4_03_82    interp8((Uint32 *)(dst + 3 * dst_Bpp), c[5], c[6]);
#define PIXEL4_10_0     *(Uint32 *)(dst + dst_pitch) = c[5];
#define PIXEL4_10_10    interp1((Uint32 *)(dst + dst_pitch ), c[5], c[1]);
#define PIXEL4_10_11    interp1((Uint32 *)(dst + dst_pitch ), c[5], c[4]);
#define PIXEL4_10_13    interp1((Uint32 *)(dst + dst_pitch ), c[4], c[5]);
#define PIXEL4_10_21    interp2((Uint32 *)(dst + dst_pitch ), c[4], c[5], c[2]);
#define PIXEL4_10_32    interp3((Uint32 *)(dst + dst_pitch ), c[5], c[2]);
#define PIXEL4_10_50    interp5((Uint32 *)(dst + dst_pitch ), c[4], c[5]);
#define PIXEL4_10_60    interp6((Uint32 *)(dst + dst_pitch ), c[5], c[4], c[2]);
#define PIXEL4_10_61    interp6((Uint32 *)(dst + dst_pitch ), c[5], c[4], c[1]);
#define PIXEL4_10_81    interp8((Uint32 *)(dst + dst_pitch ), c[5], c[4]);
#define PIXEL4_10_83    interp8((Uint32 *)(dst + dst_pitch ), c[4], c[2]);
#define PIXEL4_11_0     *(Uint32 *)(dst + dst_pitch + dst_Bpp) = c[5];
#define PIXEL4_11_30    interp3((Uint32 *)(dst + dst_pitch + dst_Bpp), c[5], c[1]);
#define PIXEL4_11_31    interp3((Uint32 *)(dst + dst_pitch + dst_Bpp), c[5], c[4]);
#define PIXEL4_11_32    interp3((Uint32 *)(dst + dst_pitch + dst_Bpp), c[5], c[2]);
#define PIXEL4_11_70    interp7((Uint32 *)(dst + dst_pitch + dst_Bpp), c[5], c[4], c[2]);
#define PIXEL4_12_0     *(Uint32 *)(dst + dst_pitch + 2 * dst_Bpp) = c[5];
#define PIXEL4_12_30    interp3((Uint32 *)(dst + dst_pitch + 2 * dst_Bpp), c[5], c[3]);
#define PIXEL4_12_31    interp3((Uint32 *)(dst + dst_pitch + 2 * dst_Bpp), c[5], c[2]);
#define PIXEL4_12_32    interp3((Uint32 *)(dst + dst_pitch + 2 * dst_Bpp), c[5], c[6]);
#define PIXEL4_12_70    interp7((Uint32 *)(dst + dst_pitch + 2 * dst_Bpp), c[5], c[6], c[2]);
#define PIXEL4_13_0     *(Uint32 *)(dst + dst_pitch + 3 * dst_Bpp) = c[5];
#define PIXEL4_13_10    interp1((Uint32 *)(dst + dst_pitch + 3 * dst_Bpp), c[5], c[3]);
#define PIXEL4_13_12    interp1((Uint32 *)(dst + dst_pitch + 3 * dst_Bpp), c[5], c[6]);
#define PIXEL4_13_14    interp1((Uint32 *)(dst + dst_pitch + 3 * dst_Bpp), c[6], c[5]);
#define PIXEL4_13_21    interp2((Uint32 *)(dst + dst_pitch + 3 * dst_Bpp), c[6], c[5], c[2]);
#define PIXEL4_13_31    interp3((Uint32 *)(dst + dst_pitch + 3 * dst_Bpp), c[5], c[2]);
#define PIXEL4_13_50    interp5((Uint32 *)(dst + dst_pitch + 3 * dst_Bpp), c[6], c[5]);
#define PIXEL4_13_60    interp6((Uint32 *)(dst + dst_pitch + 3 * dst_Bpp), c[5], c[6], c[2]);
#define PIXEL4_13_61    interp6((Uint32 *)(dst + dst_pitch + 3 * dst_Bpp), c[5], c[6], c[3]);
#define PIXEL4_13_82    interp8((Uint32 *)(dst + dst_pitch + 3 * dst_Bpp), c[5], c[6]);
#define PIXEL4_13_83    interp8((Uint32 *)(dst + dst_pitch + 3 * dst_Bpp), c[6], c[2]);
#define PIXEL4_20_0     *(Uint32 *)(dst + 2 * dst_pitch) = c[5];
#define PIXEL4_20_10    interp1((Uint32 *)(dst + 2 * dst_pitch ), c[5], c[7]);
#define PIXEL4_20_12    interp1((Uint32 *)(dst + 2 * dst_pitch ), c[5], c[4]);
#define PIXEL4_20_14    interp1((Uint32 *)(dst + 2 * dst_pitch ), c[4], c[5]);
#define PIXEL4_20_21    interp2((Uint32 *)(dst + 2 * dst_pitch ), c[4], c[5], c[8]);
#define PIXEL4_20_31    interp3((Uint32 *)(dst + 2 * dst_pitch ), c[5], c[8]);
#define PIXEL4_20_50    interp5((Uint32 *)(dst + 2 * dst_pitch ), c[4], c[5]);
#define PIXEL4_20_60    interp6((Uint32 *)(dst + 2 * dst_pitch ), c[5], c[4], c[8]);
#define PIXEL4_20_61    interp6((Uint32 *)(dst + 2 * dst_pitch ), c[5], c[4], c[7]);
#define PIXEL4_20_82    interp8((Uint32 *)(dst + 2 * dst_pitch ), c[5], c[4]);
#define PIXEL4_20_83    interp8((Uint32 *)(dst + 2 * dst_pitch ), c[4], c[8]);
#define PIXEL4_21_0     *(Uint32 *)(dst + 2 * dst_pitch + dst_Bpp) = c[5];
#define PIXEL4_21_30    interp3((Uint32 *)(dst + 2 * dst_pitch + dst_Bpp), c[5], c[7]);
#define PIXEL4_21_31    interp3((Uint32 *)(dst + 2 * dst_pitch + dst_Bpp), c[5], c[8]);
#define PIXEL4_21_32    interp3((Uint32 *)(dst + 2 * dst_pitch + dst_Bpp), c[5], c[4]);
#define PIXEL4_21_70    interp7((Uint32 *)(dst + 2 * dst_pitch + dst_Bpp), c[5], c[4], c[8]);
#define PIXEL4_22_0     *(Uint32 *)(dst + 2 * dst_pitch + 2 * dst_Bpp) = c[5];
#define PIXEL4_22_30    interp3((Uint32 *)(dst + 2 * dst_pitch + 2 * dst_Bpp), c[5], c[9]);
#define PIXEL4_22_31    interp3((Uint32 *)(dst + 2 * dst_pitch + 2 * dst_Bpp), c[5], c[6]);
#define PIXEL4_22_32    interp3((Uint32 *)(dst + 2 * dst_pitch + 2 * dst_Bpp), c[5], c[8]);
#define PIXEL4_22_70    interp7((Uint32 *)(dst + 2 * dst_pitch + 2 * dst_Bpp), c[5], c[6], c[8]);
#define PIXEL4_23_0     *(Uint32 *)(dst + 2 * dst_pitch + 3 * dst_Bpp) = c[5];
#define PIXEL4_23_10    interp1((Uint32 *)(dst + 2 * dst_pitch + 3 * dst_Bpp), c[5], c[9]);
#define PIXEL4_23_11    interp1((Uint32 *)(dst + 2 * dst_pitch + 3 * dst_Bpp), c[5], c[6]);
#define PIXEL4_23_13    interp1((Uint32 *)(dst + 2 * dst_pitch + 3 * dst_Bpp), c[6], c[5]);
#define PIXEL4_23_21    interp2((Uint32 *)(dst + 2 * dst_pitch + 3 * dst_Bpp), c[6], c[5], c[8]);
#define PIXEL4_23_32    interp3((Uint32 *)(dst + 2 * dst_pitch + 3 * dst_Bpp), c[5], c[8]);
#define PIXEL4_23_50    interp5((Uint32 *)(dst + 2 * dst_pitch + 3 * dst_Bpp), c[6], c[5]);
#define PIXEL4_23_60    interp6((Uint32 *)(dst + 2 * dst_pitch + 3 * dst_Bpp), c[5], c[6], c[8]);
#define PIXEL4_23_61    interp6((Uint32 *)(dst + 2 * dst_pitch + 3 * dst_Bpp), c[5], c[6], c[9]);
#define PIXEL4_23_81    interp8((Uint32 *)(dst + 2 * dst_pitch + 3 * dst_Bpp), c[5], c[6]);
#define PIXEL4_23_83    interp8((Uint32 *)(dst + 2 * dst_pitch + 3 * dst_Bpp), c[6], c[8]);
#define PIXEL4_30_0     *(Uint32 *)(dst + 3 * dst_pitch) = c[5];
#define PIXEL4_30_11    interp1((Uint32 *)(dst + 3 * dst_pitch ), c[5], c[8]);
#define PIXEL4_30_12    interp1((Uint32 *)(dst + 3 * dst_pitch ), c[5], c[4]);
#define PIXEL4_30_20    interp2((Uint32 *)(dst + 3 * dst_pitch ), c[5], c[8], c[4]);
#define PIXEL4_30_50    interp5((Uint32 *)(dst + 3 * dst_pitch ), c[8], c[4]);
#define PIXEL4_30_80    interp8((Uint32 *)(dst + 3 * dst_pitch ), c[5], c[7]);
#define PIXEL4_30_81    interp8((Uint32 *)(dst + 3 * dst_pitch ), c[5], c[8]);
#define PIXEL4_30_82    interp8((Uint32 *)(dst + 3 * dst_pitch ), c[5], c[4]);
#define PIXEL4_31_0     *(Uint32 *)(dst + 3 * dst_pitch + dst_Bpp) = c[5];
#define PIXEL4_31_10    interp1((Uint32 *)(dst + 3 * dst_pitch + dst_Bpp), c[5], c[7]);
#define PIXEL4_31_11    interp1((Uint32 *)(dst + 3 * dst_pitch + dst_Bpp), c[5], c[8]);
#define PIXEL4_31_13    interp1((Uint32 *)(dst + 3 * dst_pitch + dst_Bpp), c[8], c[5]);
#define PIXEL4_31_21    interp2((Uint32 *)(dst + 3 * dst_pitch + dst_Bpp), c[8], c[5], c[4]);
#define PIXEL4_31_32    interp3((Uint32 *)(dst + 3 * dst_pitch + dst_Bpp), c[5], c[4]);
#define PIXEL4_31_50    interp5((Uint32 *)(dst + 3 * dst_pitch + dst_Bpp), c[8], c[5]);
#define PIXEL4_31_60    interp6((Uint32 *)(dst + 3 * dst_pitch + dst_Bpp), c[5], c[8], c[4]);
#define PIXEL4_31_61    interp6((Uint32 *)(dst + 3 * dst_pitch + dst_Bpp), c[5], c[8], c[7]);
#define PIXEL4_31_81    interp8((Uint32 *)(dst + 3 * dst_pitch + dst_Bpp), c[5], c[8]);
#define PIXEL4_31_83    interp8((Uint32 *)(dst + 3 * dst_pitch + dst_Bpp), c[8], c[4]);
#define PIXEL4_32_0     *(Uint32 *)(dst + 3 * dst_pitch + 2 * dst_Bpp) = c[5];
#define PIXEL4_32_10    interp1((Uint32 *)(dst + 3 * dst_pitch + 2 * dst_Bpp), c[5], c[9]);
#define PIXEL4_32_12    interp1((Uint32 *)(dst + 3 * dst_pitch + 2 * dst_Bpp), c[5], c[8]);
#define PIXEL4_32_14    interp1((Uint32 *)(dst + 3 * dst_pitch + 2 * dst_Bpp), c[8], c[5]);
#define PIXEL4_32_21    interp2((Uint32 *)(dst + 3 * dst_pitch + 2 * dst_Bpp), c[8], c[5], c[6]);
#define PIXEL4_32_31    interp3((Uint32 *)(dst + 3 * dst_pitch + 2 * dst_Bpp), c[5], c[6]);
#define PIXEL4_32_50    interp5((Uint32 *)(dst + 3 * dst_pitch + 2 * dst_Bpp), c[8], c[5]);
#define PIXEL4_32_60    interp6((Uint32 *)(dst + 3 * dst_pitch + 2 * dst_Bpp), c[5], c[8], c[6]);
#define PIXEL4_32_61    interp6((Uint32 *)(dst + 3 * dst_pitch + 2 * dst_Bpp), c[5], c[8], c[9]);
#define PIXEL4_32_82    interp8((Uint32 *)(dst + 3 * dst_pitch + 2 * dst_Bpp), c[5], c[8]);
#define PIXEL4_32_83    interp8((Uint32 *)(dst + 3 * dst_pitch + 2 * dst_Bpp), c[8], c[6]);
#define PIXEL4_33_0     *(Uint32 *)(dst + 3 * dst_pitch + 3 * dst_Bpp) = c[5];
#define PIXEL4_33_11    interp1((Uint32 *)(dst + 3 * dst_pitch + 3 * dst_Bpp), c[5], c[6]);
#define PIXEL4_33_12    interp1((Uint32 *)(dst + 3 * dst_pitch + 3 * dst_Bpp), c[5], c[8]);
#define PIXEL4_33_20    interp2((Uint32 *)(dst + 3 * dst_pitch + 3 * dst_Bpp), c[5], c[8], c[6]);
#define PIXEL4_33_50    interp5((Uint32 *)(dst + 3 * dst_pitch + 3 * dst_Bpp), c[8], c[6]);
#define PIXEL4_33_80    interp8((Uint32 *)(dst + 3 * dst_pitch + 3 * dst_Bpp), c[5], c[9]);
#define PIXEL4_33_81    interp8((Uint32 *)(dst + 3 * dst_pitch + 3 * dst_Bpp), c[5], c[6]);
#define PIXEL4_33_82    interp8((Uint32 *)(dst + 3 * dst_pitch + 3 * dst_Bpp), c[5], c[8]);

void hq4x_32( SDL_Surface *src_surface, SDL_Surface *dst_surface )
{
	Uint8 *src = src_surface->pixels, *src_temp,
	      *dst = dst_surface->pixels, *dst_temp;
	int src_pitch = src_surface->pitch,
	    dst_pitch = dst_surface->pitch;
	const int dst_Bpp = 4;         // dst_surface->format->BytesPerPixel
	
	const int height = vga_height, // src_surface->h
	          width = vga_width;   // src_surface->w
	
	int prevline, nextline;
	
	Uint32 w[10];
	Uint32 c[10];
	
	//   +----+----+----+
	//   |    |    |    |
	//   | w1 | w2 | w3 |
	//   +----+----+----+
	//   |    |    |    |
	//   | w4 | w5 | w6 |
	//   +----+----+----+
	//   |    |    |    |
	//   | w7 | w8 | w9 |
	//   +----+----+----+
	
	for (int j = 0; j < height; j++)
	{
		src_temp = src;
		dst_temp = dst;
		
		prevline = (j > 0) ? -width : 0;
		nextline = (j < height - 1) ? width : 0;
		
		for (int i = 0; i < width; i++)
		{
			w[2] = *(src + prevline);
			w[5] = *src;
			w[8] = *(src + nextline);
			
			if (i>0)
			{
				w[1] = *(src + prevline - 1);
				w[4] = *(src - 1);
				w[7] = *(src + nextline - 1);
			} else {
				w[1] = w[2];
				w[4] = w[5];
				w[7] = w[8];
			}
			
			if (i < width - 1)
			{
				w[3] = *(src + prevline + 1);
				w[6] = *(src + 1);
				w[9] = *(src + nextline + 1);
			} else {
				w[3] = w[2];
				w[6] = w[5];
				w[9] = w[8];
			}
			
			int pattern = 0;
			int flag = 1;
			
			YUV1 = yuv_palette[w[5]];
			
			for (int k=1; k<=9; k++)
			{
				if (k==5) continue;
				
				if ( w[k] != w[5] )
				{
					YUV2 = yuv_palette[w[k]];
					if ( ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
					     ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
					     ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) )
					pattern |= flag;
				}
				flag <<= 1;
			}
			
			for (int k=1; k<=9; k++)
				c[k] = rgb_palette[w[k]] & 0xfcfcfcfc; // hq4x has a nasty inability to accept more than 6 bits for each component
			
			switch (pattern)
			{
				case 0:
				case 1:
				case 4:
				case 32:
				case 128:
				case 5:
				case 132:
				case 160:
				case 33:
				case 129:
				case 36:
				case 133:
				case 164:
				case 161:
				case 37:
				case 165:
				{
					PIXEL4_00_20
					PIXEL4_01_60
					PIXEL4_02_60
					PIXEL4_03_20
					PIXEL4_10_60
					PIXEL4_11_70
					PIXEL4_12_70
					PIXEL4_13_60
					PIXEL4_20_60
					PIXEL4_21_70
					PIXEL4_22_70
					PIXEL4_23_60
					PIXEL4_30_20
					PIXEL4_31_60
					PIXEL4_32_60
					PIXEL4_33_20
					break;
				}
				case 2:
				case 34:
				case 130:
				case 162:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_12_30
					PIXEL4_13_61
					PIXEL4_20_60
					PIXEL4_21_70
					PIXEL4_22_70
					PIXEL4_23_60
					PIXEL4_30_20
					PIXEL4_31_60
					PIXEL4_32_60
					PIXEL4_33_20
					break;
				}
				case 16:
				case 17:
				case 48:
				case 49:
				{
					PIXEL4_00_20
					PIXEL4_01_60
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_60
					PIXEL4_11_70
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_60
					PIXEL4_21_70
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_20
					PIXEL4_31_60
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 64:
				case 65:
				case 68:
				case 69:
				{
					PIXEL4_00_20
					PIXEL4_01_60
					PIXEL4_02_60
					PIXEL4_03_20
					PIXEL4_10_60
					PIXEL4_11_70
					PIXEL4_12_70
					PIXEL4_13_60
					PIXEL4_20_61
					PIXEL4_21_30
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_30_80
					PIXEL4_31_10
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 8:
				case 12:
				case 136:
				case 140:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_60
					PIXEL4_03_20
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_70
					PIXEL4_13_60
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_70
					PIXEL4_23_60
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_60
					PIXEL4_33_20
					break;
				}
				case 3:
				case 35:
				case 131:
				case 163:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_12_30
					PIXEL4_13_61
					PIXEL4_20_60
					PIXEL4_21_70
					PIXEL4_22_70
					PIXEL4_23_60
					PIXEL4_30_20
					PIXEL4_31_60
					PIXEL4_32_60
					PIXEL4_33_20
					break;
				}
				case 6:
				case 38:
				case 134:
				case 166:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_12_32
					PIXEL4_13_82
					PIXEL4_20_60
					PIXEL4_21_70
					PIXEL4_22_70
					PIXEL4_23_60
					PIXEL4_30_20
					PIXEL4_31_60
					PIXEL4_32_60
					PIXEL4_33_20
					break;
				}
				case 20:
				case 21:
				case 52:
				case 53:
				{
					PIXEL4_00_20
					PIXEL4_01_60
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_10_60
					PIXEL4_11_70
					PIXEL4_12_31
					PIXEL4_13_31
					PIXEL4_20_60
					PIXEL4_21_70
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_20
					PIXEL4_31_60
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 144:
				case 145:
				case 176:
				case 177:
				{
					PIXEL4_00_20
					PIXEL4_01_60
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_60
					PIXEL4_11_70
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_60
					PIXEL4_21_70
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_30_20
					PIXEL4_31_60
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 192:
				case 193:
				case 196:
				case 197:
				{
					PIXEL4_00_20
					PIXEL4_01_60
					PIXEL4_02_60
					PIXEL4_03_20
					PIXEL4_10_60
					PIXEL4_11_70
					PIXEL4_12_70
					PIXEL4_13_60
					PIXEL4_20_61
					PIXEL4_21_30
					PIXEL4_22_31
					PIXEL4_23_81
					PIXEL4_30_80
					PIXEL4_31_10
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 96:
				case 97:
				case 100:
				case 101:
				{
					PIXEL4_00_20
					PIXEL4_01_60
					PIXEL4_02_60
					PIXEL4_03_20
					PIXEL4_10_60
					PIXEL4_11_70
					PIXEL4_12_70
					PIXEL4_13_60
					PIXEL4_20_82
					PIXEL4_21_32
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_30_82
					PIXEL4_31_32
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 40:
				case 44:
				case 168:
				case 172:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_60
					PIXEL4_03_20
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_70
					PIXEL4_13_60
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_70
					PIXEL4_23_60
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_60
					PIXEL4_33_20
					break;
				}
				case 9:
				case 13:
				case 137:
				case 141:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_60
					PIXEL4_03_20
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_70
					PIXEL4_13_60
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_70
					PIXEL4_23_60
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_60
					PIXEL4_33_20
					break;
				}
				case 18:
				case 50:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_10
						PIXEL4_03_80
						PIXEL4_12_30
						PIXEL4_13_10
					}
					else
					{
						PIXEL4_02_50
						PIXEL4_03_50
						PIXEL4_12_0
						PIXEL4_13_50
					}
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_20_60
					PIXEL4_21_70
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_20
					PIXEL4_31_60
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 80:
				case 81:
				{
					PIXEL4_00_20
					PIXEL4_01_60
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_60
					PIXEL4_11_70
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_61
					PIXEL4_21_30
					if (diff(w[6], w[8]))
					{
						PIXEL4_22_30
						PIXEL4_23_10
						PIXEL4_32_10
						PIXEL4_33_80
					}
					else
					{
						PIXEL4_22_0
						PIXEL4_23_50
						PIXEL4_32_50
						PIXEL4_33_50
					}
					PIXEL4_30_80
					PIXEL4_31_10
					break;
				}
				case 72:
				case 76:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_60
					PIXEL4_03_20
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_70
					PIXEL4_13_60
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_10
						PIXEL4_21_30
						PIXEL4_30_80
						PIXEL4_31_10
					}
					else
					{
						PIXEL4_20_50
						PIXEL4_21_0
						PIXEL4_30_50
						PIXEL4_31_50
					}
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 10:
				case 138:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_80
						PIXEL4_01_10
						PIXEL4_10_10
						PIXEL4_11_30
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_50
						PIXEL4_10_50
						PIXEL4_11_0
					}
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_12_30
					PIXEL4_13_61
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_70
					PIXEL4_23_60
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_60
					PIXEL4_33_20
					break;
				}
				case 66:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_12_30
					PIXEL4_13_61
					PIXEL4_20_61
					PIXEL4_21_30
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_30_80
					PIXEL4_31_10
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 24:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 7:
				case 39:
				case 135:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_12_32
					PIXEL4_13_82
					PIXEL4_20_60
					PIXEL4_21_70
					PIXEL4_22_70
					PIXEL4_23_60
					PIXEL4_30_20
					PIXEL4_31_60
					PIXEL4_32_60
					PIXEL4_33_20
					break;
				}
				case 148:
				case 149:
				case 180:
				{
					PIXEL4_00_20
					PIXEL4_01_60
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_10_60
					PIXEL4_11_70
					PIXEL4_12_31
					PIXEL4_13_31
					PIXEL4_20_60
					PIXEL4_21_70
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_30_20
					PIXEL4_31_60
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 224:
				case 228:
				case 225:
				{
					PIXEL4_00_20
					PIXEL4_01_60
					PIXEL4_02_60
					PIXEL4_03_20
					PIXEL4_10_60
					PIXEL4_11_70
					PIXEL4_12_70
					PIXEL4_13_60
					PIXEL4_20_82
					PIXEL4_21_32
					PIXEL4_22_31
					PIXEL4_23_81
					PIXEL4_30_82
					PIXEL4_31_32
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 41:
				case 169:
				case 45:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_60
					PIXEL4_03_20
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_70
					PIXEL4_13_60
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_70
					PIXEL4_23_60
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_60
					PIXEL4_33_20
					break;
				}
				case 22:
				case 54:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_13_0
					}
					else
					{
						PIXEL4_02_50
						PIXEL4_03_50
						PIXEL4_13_50
					}
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_12_0
					PIXEL4_20_60
					PIXEL4_21_70
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_20
					PIXEL4_31_60
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 208:
				case 209:
				{
					PIXEL4_00_20
					PIXEL4_01_60
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_60
					PIXEL4_11_70
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_61
					PIXEL4_21_30
					PIXEL4_22_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_23_0
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_23_50
						PIXEL4_32_50
						PIXEL4_33_50
					}
					PIXEL4_30_80
					PIXEL4_31_10
					break;
				}
				case 104:
				case 108:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_60
					PIXEL4_03_20
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_70
					PIXEL4_13_60
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_0
						PIXEL4_30_0
						PIXEL4_31_0
					}
					else
					{
						PIXEL4_20_50
						PIXEL4_30_50
						PIXEL4_31_50
					}
					PIXEL4_21_0
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 11:
				case 139:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_10_0
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_50
						PIXEL4_10_50
					}
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_11_0
					PIXEL4_12_30
					PIXEL4_13_61
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_70
					PIXEL4_23_60
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_60
					PIXEL4_33_20
					break;
				}
				case 19:
				case 51:
				{
					if (diff(w[2], w[6]))
					{
						PIXEL4_00_81
						PIXEL4_01_31
						PIXEL4_02_10
						PIXEL4_03_80
						PIXEL4_12_30
						PIXEL4_13_10
					}
					else
					{
						PIXEL4_00_12
						PIXEL4_01_14
						PIXEL4_02_83
						PIXEL4_03_50
						PIXEL4_12_70
						PIXEL4_13_21
					}
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_20_60
					PIXEL4_21_70
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_20
					PIXEL4_31_60
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 146:
				case 178:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_10
						PIXEL4_03_80
						PIXEL4_12_30
						PIXEL4_13_10
						PIXEL4_23_32
						PIXEL4_33_82
					}
					else
					{
						PIXEL4_02_21
						PIXEL4_03_50
						PIXEL4_12_70
						PIXEL4_13_83
						PIXEL4_23_13
						PIXEL4_33_11
					}
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_20_60
					PIXEL4_21_70
					PIXEL4_22_32
					PIXEL4_30_20
					PIXEL4_31_60
					PIXEL4_32_82
					break;
				}
				case 84:
				case 85:
				{
					PIXEL4_00_20
					PIXEL4_01_60
					PIXEL4_02_81
					if (diff(w[6], w[8]))
					{
						PIXEL4_03_81
						PIXEL4_13_31
						PIXEL4_22_30
						PIXEL4_23_10
						PIXEL4_32_10
						PIXEL4_33_80
					}
					else
					{
						PIXEL4_03_12
						PIXEL4_13_14
						PIXEL4_22_70
						PIXEL4_23_83
						PIXEL4_32_21
						PIXEL4_33_50
					}
					PIXEL4_10_60
					PIXEL4_11_70
					PIXEL4_12_31
					PIXEL4_20_61
					PIXEL4_21_30
					PIXEL4_30_80
					PIXEL4_31_10
					break;
				}
				case 112:
				case 113:
				{
					PIXEL4_00_20
					PIXEL4_01_60
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_60
					PIXEL4_11_70
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_82
					PIXEL4_21_32
					if (diff(w[6], w[8]))
					{
						PIXEL4_22_30
						PIXEL4_23_10
						PIXEL4_30_82
						PIXEL4_31_32
						PIXEL4_32_10
						PIXEL4_33_80
					}
					else
					{
						PIXEL4_22_70
						PIXEL4_23_21
						PIXEL4_30_11
						PIXEL4_31_13
						PIXEL4_32_83
						PIXEL4_33_50
					}
					break;
				}
				case 200:
				case 204:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_60
					PIXEL4_03_20
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_70
					PIXEL4_13_60
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_10
						PIXEL4_21_30
						PIXEL4_30_80
						PIXEL4_31_10
						PIXEL4_32_31
						PIXEL4_33_81
					}
					else
					{
						PIXEL4_20_21
						PIXEL4_21_70
						PIXEL4_30_50
						PIXEL4_31_83
						PIXEL4_32_14
						PIXEL4_33_12
					}
					PIXEL4_22_31
					PIXEL4_23_81
					break;
				}
				case 73:
				case 77:
				{
					if (diff(w[8], w[4]))
					{
						PIXEL4_00_82
						PIXEL4_10_32
						PIXEL4_20_10
						PIXEL4_21_30
						PIXEL4_30_80
						PIXEL4_31_10
					}
					else
					{
						PIXEL4_00_11
						PIXEL4_10_13
						PIXEL4_20_83
						PIXEL4_21_70
						PIXEL4_30_50
						PIXEL4_31_21
					}
					PIXEL4_01_82
					PIXEL4_02_60
					PIXEL4_03_20
					PIXEL4_11_32
					PIXEL4_12_70
					PIXEL4_13_60
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 42:
				case 170:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_80
						PIXEL4_01_10
						PIXEL4_10_10
						PIXEL4_11_30
						PIXEL4_20_31
						PIXEL4_30_81
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_21
						PIXEL4_10_83
						PIXEL4_11_70
						PIXEL4_20_14
						PIXEL4_30_12
					}
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_12_30
					PIXEL4_13_61
					PIXEL4_21_31
					PIXEL4_22_70
					PIXEL4_23_60
					PIXEL4_31_81
					PIXEL4_32_60
					PIXEL4_33_20
					break;
				}
				case 14:
				case 142:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_80
						PIXEL4_01_10
						PIXEL4_02_32
						PIXEL4_03_82
						PIXEL4_10_10
						PIXEL4_11_30
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_83
						PIXEL4_02_13
						PIXEL4_03_11
						PIXEL4_10_21
						PIXEL4_11_70
					}
					PIXEL4_12_32
					PIXEL4_13_82
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_70
					PIXEL4_23_60
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_60
					PIXEL4_33_20
					break;
				}
				case 67:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_12_30
					PIXEL4_13_61
					PIXEL4_20_61
					PIXEL4_21_30
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_30_80
					PIXEL4_31_10
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 70:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_12_32
					PIXEL4_13_82
					PIXEL4_20_61
					PIXEL4_21_30
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_30_80
					PIXEL4_31_10
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 28:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_31
					PIXEL4_13_31
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 152:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 194:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_12_30
					PIXEL4_13_61
					PIXEL4_20_61
					PIXEL4_21_30
					PIXEL4_22_31
					PIXEL4_23_81
					PIXEL4_30_80
					PIXEL4_31_10
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 98:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_12_30
					PIXEL4_13_61
					PIXEL4_20_82
					PIXEL4_21_32
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_30_82
					PIXEL4_31_32
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 56:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 25:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 26:
				case 31:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_10_0
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_50
						PIXEL4_10_50
					}
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_13_0
					}
					else
					{
						PIXEL4_02_50
						PIXEL4_03_50
						PIXEL4_13_50
					}
					PIXEL4_11_0
					PIXEL4_12_0
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 82:
				case 214:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_13_0
					}
					else
					{
						PIXEL4_02_50
						PIXEL4_03_50
						PIXEL4_13_50
					}
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_12_0
					PIXEL4_20_61
					PIXEL4_21_30
					PIXEL4_22_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_23_0
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_23_50
						PIXEL4_32_50
						PIXEL4_33_50
					}
					PIXEL4_30_80
					PIXEL4_31_10
					break;
				}
				case 88:
				case 248:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_30
					PIXEL4_13_10
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_0
						PIXEL4_30_0
						PIXEL4_31_0
					}
					else
					{
						PIXEL4_20_50
						PIXEL4_30_50
						PIXEL4_31_50
					}
					PIXEL4_21_0
					PIXEL4_22_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_23_0
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_23_50
						PIXEL4_32_50
						PIXEL4_33_50
					}
					break;
				}
				case 74:
				case 107:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_10_0
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_50
						PIXEL4_10_50
					}
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_11_0
					PIXEL4_12_30
					PIXEL4_13_61
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_0
						PIXEL4_30_0
						PIXEL4_31_0
					}
					else
					{
						PIXEL4_20_50
						PIXEL4_30_50
						PIXEL4_31_50
					}
					PIXEL4_21_0
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 27:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_10_0
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_50
						PIXEL4_10_50
					}
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_11_0
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 86:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_13_0
					}
					else
					{
						PIXEL4_02_50
						PIXEL4_03_50
						PIXEL4_13_50
					}
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_12_0
					PIXEL4_20_61
					PIXEL4_21_30
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_80
					PIXEL4_31_10
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 216:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_23_0
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_23_50
						PIXEL4_32_50
						PIXEL4_33_50
					}
					PIXEL4_30_80
					PIXEL4_31_10
					break;
				}
				case 106:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_30
					PIXEL4_13_61
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_0
						PIXEL4_30_0
						PIXEL4_31_0
					}
					else
					{
						PIXEL4_20_50
						PIXEL4_30_50
						PIXEL4_31_50
					}
					PIXEL4_21_0
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 30:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_13_0
					}
					else
					{
						PIXEL4_02_50
						PIXEL4_03_50
						PIXEL4_13_50
					}
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_0
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 210:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_61
					PIXEL4_21_30
					PIXEL4_22_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_23_0
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_23_50
						PIXEL4_32_50
						PIXEL4_33_50
					}
					PIXEL4_30_80
					PIXEL4_31_10
					break;
				}
				case 120:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_30
					PIXEL4_13_10
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_0
						PIXEL4_30_0
						PIXEL4_31_0
					}
					else
					{
						PIXEL4_20_50
						PIXEL4_30_50
						PIXEL4_31_50
					}
					PIXEL4_21_0
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 75:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_10_0
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_50
						PIXEL4_10_50
					}
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_11_0
					PIXEL4_12_30
					PIXEL4_13_61
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_30_80
					PIXEL4_31_10
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 29:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_31
					PIXEL4_13_31
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 198:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_12_32
					PIXEL4_13_82
					PIXEL4_20_61
					PIXEL4_21_30
					PIXEL4_22_31
					PIXEL4_23_81
					PIXEL4_30_80
					PIXEL4_31_10
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 184:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 99:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_12_30
					PIXEL4_13_61
					PIXEL4_20_82
					PIXEL4_21_32
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_30_82
					PIXEL4_31_32
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 57:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 71:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_12_32
					PIXEL4_13_82
					PIXEL4_20_61
					PIXEL4_21_30
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_30_80
					PIXEL4_31_10
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 156:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_31
					PIXEL4_13_31
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 226:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_12_30
					PIXEL4_13_61
					PIXEL4_20_82
					PIXEL4_21_32
					PIXEL4_22_31
					PIXEL4_23_81
					PIXEL4_30_82
					PIXEL4_31_32
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 60:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_31
					PIXEL4_13_31
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 195:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_12_30
					PIXEL4_13_61
					PIXEL4_20_61
					PIXEL4_21_30
					PIXEL4_22_31
					PIXEL4_23_81
					PIXEL4_30_80
					PIXEL4_31_10
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 102:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_12_32
					PIXEL4_13_82
					PIXEL4_20_82
					PIXEL4_21_32
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_30_82
					PIXEL4_31_32
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 153:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 58:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_80
						PIXEL4_01_10
						PIXEL4_10_10
						PIXEL4_11_30
					}
					else
					{
						PIXEL4_00_20
						PIXEL4_01_12
						PIXEL4_10_11
						PIXEL4_11_0
					}
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_10
						PIXEL4_03_80
						PIXEL4_12_30
						PIXEL4_13_10
					}
					else
					{
						PIXEL4_02_11
						PIXEL4_03_20
						PIXEL4_12_0
						PIXEL4_13_12
					}
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 83:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_10
						PIXEL4_03_80
						PIXEL4_12_30
						PIXEL4_13_10
					}
					else
					{
						PIXEL4_02_11
						PIXEL4_03_20
						PIXEL4_12_0
						PIXEL4_13_12
					}
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_20_61
					PIXEL4_21_30
					if (diff(w[6], w[8]))
					{
						PIXEL4_22_30
						PIXEL4_23_10
						PIXEL4_32_10
						PIXEL4_33_80
					}
					else
					{
						PIXEL4_22_0
						PIXEL4_23_11
						PIXEL4_32_12
						PIXEL4_33_20
					}
					PIXEL4_30_80
					PIXEL4_31_10
					break;
				}
				case 92:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_31
					PIXEL4_13_31
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_10
						PIXEL4_21_30
						PIXEL4_30_80
						PIXEL4_31_10
					}
					else
					{
						PIXEL4_20_12
						PIXEL4_21_0
						PIXEL4_30_20
						PIXEL4_31_11
					}
					if (diff(w[6], w[8]))
					{
						PIXEL4_22_30
						PIXEL4_23_10
						PIXEL4_32_10
						PIXEL4_33_80
					}
					else
					{
						PIXEL4_22_0
						PIXEL4_23_11
						PIXEL4_32_12
						PIXEL4_33_20
					}
					break;
				}
				case 202:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_80
						PIXEL4_01_10
						PIXEL4_10_10
						PIXEL4_11_30
					}
					else
					{
						PIXEL4_00_20
						PIXEL4_01_12
						PIXEL4_10_11
						PIXEL4_11_0
					}
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_12_30
					PIXEL4_13_61
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_10
						PIXEL4_21_30
						PIXEL4_30_80
						PIXEL4_31_10
					}
					else
					{
						PIXEL4_20_12
						PIXEL4_21_0
						PIXEL4_30_20
						PIXEL4_31_11
					}
					PIXEL4_22_31
					PIXEL4_23_81
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 78:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_80
						PIXEL4_01_10
						PIXEL4_10_10
						PIXEL4_11_30
					}
					else
					{
						PIXEL4_00_20
						PIXEL4_01_12
						PIXEL4_10_11
						PIXEL4_11_0
					}
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_12_32
					PIXEL4_13_82
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_10
						PIXEL4_21_30
						PIXEL4_30_80
						PIXEL4_31_10
					}
					else
					{
						PIXEL4_20_12
						PIXEL4_21_0
						PIXEL4_30_20
						PIXEL4_31_11
					}
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 154:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_80
						PIXEL4_01_10
						PIXEL4_10_10
						PIXEL4_11_30
					}
					else
					{
						PIXEL4_00_20
						PIXEL4_01_12
						PIXEL4_10_11
						PIXEL4_11_0
					}
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_10
						PIXEL4_03_80
						PIXEL4_12_30
						PIXEL4_13_10
					}
					else
					{
						PIXEL4_02_11
						PIXEL4_03_20
						PIXEL4_12_0
						PIXEL4_13_12
					}
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 114:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_10
						PIXEL4_03_80
						PIXEL4_12_30
						PIXEL4_13_10
					}
					else
					{
						PIXEL4_02_11
						PIXEL4_03_20
						PIXEL4_12_0
						PIXEL4_13_12
					}
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_20_82
					PIXEL4_21_32
					if (diff(w[6], w[8]))
					{
						PIXEL4_22_30
						PIXEL4_23_10
						PIXEL4_32_10
						PIXEL4_33_80
					}
					else
					{
						PIXEL4_22_0
						PIXEL4_23_11
						PIXEL4_32_12
						PIXEL4_33_20
					}
					PIXEL4_30_82
					PIXEL4_31_32
					break;
				}
				case 89:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_30
					PIXEL4_13_10
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_10
						PIXEL4_21_30
						PIXEL4_30_80
						PIXEL4_31_10
					}
					else
					{
						PIXEL4_20_12
						PIXEL4_21_0
						PIXEL4_30_20
						PIXEL4_31_11
					}
					if (diff(w[6], w[8]))
					{
						PIXEL4_22_30
						PIXEL4_23_10
						PIXEL4_32_10
						PIXEL4_33_80
					}
					else
					{
						PIXEL4_22_0
						PIXEL4_23_11
						PIXEL4_32_12
						PIXEL4_33_20
					}
					break;
				}
				case 90:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_80
						PIXEL4_01_10
						PIXEL4_10_10
						PIXEL4_11_30
					}
					else
					{
						PIXEL4_00_20
						PIXEL4_01_12
						PIXEL4_10_11
						PIXEL4_11_0
					}
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_10
						PIXEL4_03_80
						PIXEL4_12_30
						PIXEL4_13_10
					}
					else
					{
						PIXEL4_02_11
						PIXEL4_03_20
						PIXEL4_12_0
						PIXEL4_13_12
					}
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_10
						PIXEL4_21_30
						PIXEL4_30_80
						PIXEL4_31_10
					}
					else
					{
						PIXEL4_20_12
						PIXEL4_21_0
						PIXEL4_30_20
						PIXEL4_31_11
					}
					if (diff(w[6], w[8]))
					{
						PIXEL4_22_30
						PIXEL4_23_10
						PIXEL4_32_10
						PIXEL4_33_80
					}
					else
					{
						PIXEL4_22_0
						PIXEL4_23_11
						PIXEL4_32_12
						PIXEL4_33_20
					}
					break;
				}
				case 55:
				case 23:
				{
					if (diff(w[2], w[6]))
					{
						PIXEL4_00_81
						PIXEL4_01_31
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_12_0
						PIXEL4_13_0
					}
					else
					{
						PIXEL4_00_12
						PIXEL4_01_14
						PIXEL4_02_83
						PIXEL4_03_50
						PIXEL4_12_70
						PIXEL4_13_21
					}
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_20_60
					PIXEL4_21_70
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_20
					PIXEL4_31_60
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 182:
				case 150:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_12_0
						PIXEL4_13_0
						PIXEL4_23_32
						PIXEL4_33_82
					}
					else
					{
						PIXEL4_02_21
						PIXEL4_03_50
						PIXEL4_12_70
						PIXEL4_13_83
						PIXEL4_23_13
						PIXEL4_33_11
					}
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_20_60
					PIXEL4_21_70
					PIXEL4_22_32
					PIXEL4_30_20
					PIXEL4_31_60
					PIXEL4_32_82
					break;
				}
				case 213:
				case 212:
				{
					PIXEL4_00_20
					PIXEL4_01_60
					PIXEL4_02_81
					if (diff(w[6], w[8]))
					{
						PIXEL4_03_81
						PIXEL4_13_31
						PIXEL4_22_0
						PIXEL4_23_0
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_03_12
						PIXEL4_13_14
						PIXEL4_22_70
						PIXEL4_23_83
						PIXEL4_32_21
						PIXEL4_33_50
					}
					PIXEL4_10_60
					PIXEL4_11_70
					PIXEL4_12_31
					PIXEL4_20_61
					PIXEL4_21_30
					PIXEL4_30_80
					PIXEL4_31_10
					break;
				}
				case 241:
				case 240:
				{
					PIXEL4_00_20
					PIXEL4_01_60
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_60
					PIXEL4_11_70
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_82
					PIXEL4_21_32
					if (diff(w[6], w[8]))
					{
						PIXEL4_22_0
						PIXEL4_23_0
						PIXEL4_30_82
						PIXEL4_31_32
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_22_70
						PIXEL4_23_21
						PIXEL4_30_11
						PIXEL4_31_13
						PIXEL4_32_83
						PIXEL4_33_50
					}
					break;
				}
				case 236:
				case 232:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_60
					PIXEL4_03_20
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_70
					PIXEL4_13_60
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_0
						PIXEL4_21_0
						PIXEL4_30_0
						PIXEL4_31_0
						PIXEL4_32_31
						PIXEL4_33_81
					}
					else
					{
						PIXEL4_20_21
						PIXEL4_21_70
						PIXEL4_30_50
						PIXEL4_31_83
						PIXEL4_32_14
						PIXEL4_33_12
					}
					PIXEL4_22_31
					PIXEL4_23_81
					break;
				}
				case 109:
				case 105:
				{
					if (diff(w[8], w[4]))
					{
						PIXEL4_00_82
						PIXEL4_10_32
						PIXEL4_20_0
						PIXEL4_21_0
						PIXEL4_30_0
						PIXEL4_31_0
					}
					else
					{
						PIXEL4_00_11
						PIXEL4_10_13
						PIXEL4_20_83
						PIXEL4_21_70
						PIXEL4_30_50
						PIXEL4_31_21
					}
					PIXEL4_01_82
					PIXEL4_02_60
					PIXEL4_03_20
					PIXEL4_11_32
					PIXEL4_12_70
					PIXEL4_13_60
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 171:
				case 43:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_10_0
						PIXEL4_11_0
						PIXEL4_20_31
						PIXEL4_30_81
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_21
						PIXEL4_10_83
						PIXEL4_11_70
						PIXEL4_20_14
						PIXEL4_30_12
					}
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_12_30
					PIXEL4_13_61
					PIXEL4_21_31
					PIXEL4_22_70
					PIXEL4_23_60
					PIXEL4_31_81
					PIXEL4_32_60
					PIXEL4_33_20
					break;
				}
				case 143:
				case 15:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_02_32
						PIXEL4_03_82
						PIXEL4_10_0
						PIXEL4_11_0
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_83
						PIXEL4_02_13
						PIXEL4_03_11
						PIXEL4_10_21
						PIXEL4_11_70
					}
					PIXEL4_12_32
					PIXEL4_13_82
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_70
					PIXEL4_23_60
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_60
					PIXEL4_33_20
					break;
				}
				case 124:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_31
					PIXEL4_13_31
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_0
						PIXEL4_30_0
						PIXEL4_31_0
					}
					else
					{
						PIXEL4_20_50
						PIXEL4_30_50
						PIXEL4_31_50
					}
					PIXEL4_21_0
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 203:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_10_0
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_50
						PIXEL4_10_50
					}
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_11_0
					PIXEL4_12_30
					PIXEL4_13_61
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_31
					PIXEL4_23_81
					PIXEL4_30_80
					PIXEL4_31_10
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 62:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_13_0
					}
					else
					{
						PIXEL4_02_50
						PIXEL4_03_50
						PIXEL4_13_50
					}
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_0
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 211:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_61
					PIXEL4_21_30
					PIXEL4_22_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_23_0
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_23_50
						PIXEL4_32_50
						PIXEL4_33_50
					}
					PIXEL4_30_80
					PIXEL4_31_10
					break;
				}
				case 118:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_13_0
					}
					else
					{
						PIXEL4_02_50
						PIXEL4_03_50
						PIXEL4_13_50
					}
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_12_0
					PIXEL4_20_82
					PIXEL4_21_32
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_82
					PIXEL4_31_32
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 217:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_23_0
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_23_50
						PIXEL4_32_50
						PIXEL4_33_50
					}
					PIXEL4_30_80
					PIXEL4_31_10
					break;
				}
				case 110:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_32
					PIXEL4_13_82
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_0
						PIXEL4_30_0
						PIXEL4_31_0
					}
					else
					{
						PIXEL4_20_50
						PIXEL4_30_50
						PIXEL4_31_50
					}
					PIXEL4_21_0
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 155:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_10_0
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_50
						PIXEL4_10_50
					}
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_11_0
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 188:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_31
					PIXEL4_13_31
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 185:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 61:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_31
					PIXEL4_13_31
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 157:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_31
					PIXEL4_13_31
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 103:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_12_32
					PIXEL4_13_82
					PIXEL4_20_82
					PIXEL4_21_32
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_30_82
					PIXEL4_31_32
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 227:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_12_30
					PIXEL4_13_61
					PIXEL4_20_82
					PIXEL4_21_32
					PIXEL4_22_31
					PIXEL4_23_81
					PIXEL4_30_82
					PIXEL4_31_32
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 230:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_12_32
					PIXEL4_13_82
					PIXEL4_20_82
					PIXEL4_21_32
					PIXEL4_22_31
					PIXEL4_23_81
					PIXEL4_30_82
					PIXEL4_31_32
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 199:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_12_32
					PIXEL4_13_82
					PIXEL4_20_61
					PIXEL4_21_30
					PIXEL4_22_31
					PIXEL4_23_81
					PIXEL4_30_80
					PIXEL4_31_10
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 220:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_31
					PIXEL4_13_31
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_10
						PIXEL4_21_30
						PIXEL4_30_80
						PIXEL4_31_10
					}
					else
					{
						PIXEL4_20_12
						PIXEL4_21_0
						PIXEL4_30_20
						PIXEL4_31_11
					}
					PIXEL4_22_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_23_0
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_23_50
						PIXEL4_32_50
						PIXEL4_33_50
					}
					break;
				}
				case 158:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_80
						PIXEL4_01_10
						PIXEL4_10_10
						PIXEL4_11_30
					}
					else
					{
						PIXEL4_00_20
						PIXEL4_01_12
						PIXEL4_10_11
						PIXEL4_11_0
					}
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_13_0
					}
					else
					{
						PIXEL4_02_50
						PIXEL4_03_50
						PIXEL4_13_50
					}
					PIXEL4_12_0
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 234:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_80
						PIXEL4_01_10
						PIXEL4_10_10
						PIXEL4_11_30
					}
					else
					{
						PIXEL4_00_20
						PIXEL4_01_12
						PIXEL4_10_11
						PIXEL4_11_0
					}
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_12_30
					PIXEL4_13_61
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_0
						PIXEL4_30_0
						PIXEL4_31_0
					}
					else
					{
						PIXEL4_20_50
						PIXEL4_30_50
						PIXEL4_31_50
					}
					PIXEL4_21_0
					PIXEL4_22_31
					PIXEL4_23_81
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 242:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_10
						PIXEL4_03_80
						PIXEL4_12_30
						PIXEL4_13_10
					}
					else
					{
						PIXEL4_02_11
						PIXEL4_03_20
						PIXEL4_12_0
						PIXEL4_13_12
					}
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_20_82
					PIXEL4_21_32
					PIXEL4_22_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_23_0
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_23_50
						PIXEL4_32_50
						PIXEL4_33_50
					}
					PIXEL4_30_82
					PIXEL4_31_32
					break;
				}
				case 59:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_10_0
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_50
						PIXEL4_10_50
					}
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_10
						PIXEL4_03_80
						PIXEL4_12_30
						PIXEL4_13_10
					}
					else
					{
						PIXEL4_02_11
						PIXEL4_03_20
						PIXEL4_12_0
						PIXEL4_13_12
					}
					PIXEL4_11_0
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 121:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_30
					PIXEL4_13_10
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_0
						PIXEL4_30_0
						PIXEL4_31_0
					}
					else
					{
						PIXEL4_20_50
						PIXEL4_30_50
						PIXEL4_31_50
					}
					PIXEL4_21_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_22_30
						PIXEL4_23_10
						PIXEL4_32_10
						PIXEL4_33_80
					}
					else
					{
						PIXEL4_22_0
						PIXEL4_23_11
						PIXEL4_32_12
						PIXEL4_33_20
					}
					break;
				}
				case 87:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_13_0
					}
					else
					{
						PIXEL4_02_50
						PIXEL4_03_50
						PIXEL4_13_50
					}
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_12_0
					PIXEL4_20_61
					PIXEL4_21_30
					if (diff(w[6], w[8]))
					{
						PIXEL4_22_30
						PIXEL4_23_10
						PIXEL4_32_10
						PIXEL4_33_80
					}
					else
					{
						PIXEL4_22_0
						PIXEL4_23_11
						PIXEL4_32_12
						PIXEL4_33_20
					}
					PIXEL4_30_80
					PIXEL4_31_10
					break;
				}
				case 79:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_10_0
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_50
						PIXEL4_10_50
					}
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_11_0
					PIXEL4_12_32
					PIXEL4_13_82
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_10
						PIXEL4_21_30
						PIXEL4_30_80
						PIXEL4_31_10
					}
					else
					{
						PIXEL4_20_12
						PIXEL4_21_0
						PIXEL4_30_20
						PIXEL4_31_11
					}
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 122:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_80
						PIXEL4_01_10
						PIXEL4_10_10
						PIXEL4_11_30
					}
					else
					{
						PIXEL4_00_20
						PIXEL4_01_12
						PIXEL4_10_11
						PIXEL4_11_0
					}
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_10
						PIXEL4_03_80
						PIXEL4_12_30
						PIXEL4_13_10
					}
					else
					{
						PIXEL4_02_11
						PIXEL4_03_20
						PIXEL4_12_0
						PIXEL4_13_12
					}
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_0
						PIXEL4_30_0
						PIXEL4_31_0
					}
					else
					{
						PIXEL4_20_50
						PIXEL4_30_50
						PIXEL4_31_50
					}
					PIXEL4_21_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_22_30
						PIXEL4_23_10
						PIXEL4_32_10
						PIXEL4_33_80
					}
					else
					{
						PIXEL4_22_0
						PIXEL4_23_11
						PIXEL4_32_12
						PIXEL4_33_20
					}
					break;
				}
				case 94:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_80
						PIXEL4_01_10
						PIXEL4_10_10
						PIXEL4_11_30
					}
					else
					{
						PIXEL4_00_20
						PIXEL4_01_12
						PIXEL4_10_11
						PIXEL4_11_0
					}
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_13_0
					}
					else
					{
						PIXEL4_02_50
						PIXEL4_03_50
						PIXEL4_13_50
					}
					PIXEL4_12_0
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_10
						PIXEL4_21_30
						PIXEL4_30_80
						PIXEL4_31_10
					}
					else
					{
						PIXEL4_20_12
						PIXEL4_21_0
						PIXEL4_30_20
						PIXEL4_31_11
					}
					if (diff(w[6], w[8]))
					{
						PIXEL4_22_30
						PIXEL4_23_10
						PIXEL4_32_10
						PIXEL4_33_80
					}
					else
					{
						PIXEL4_22_0
						PIXEL4_23_11
						PIXEL4_32_12
						PIXEL4_33_20
					}
					break;
				}
				case 218:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_80
						PIXEL4_01_10
						PIXEL4_10_10
						PIXEL4_11_30
					}
					else
					{
						PIXEL4_00_20
						PIXEL4_01_12
						PIXEL4_10_11
						PIXEL4_11_0
					}
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_10
						PIXEL4_03_80
						PIXEL4_12_30
						PIXEL4_13_10
					}
					else
					{
						PIXEL4_02_11
						PIXEL4_03_20
						PIXEL4_12_0
						PIXEL4_13_12
					}
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_10
						PIXEL4_21_30
						PIXEL4_30_80
						PIXEL4_31_10
					}
					else
					{
						PIXEL4_20_12
						PIXEL4_21_0
						PIXEL4_30_20
						PIXEL4_31_11
					}
					PIXEL4_22_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_23_0
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_23_50
						PIXEL4_32_50
						PIXEL4_33_50
					}
					break;
				}
				case 91:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_10_0
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_50
						PIXEL4_10_50
					}
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_10
						PIXEL4_03_80
						PIXEL4_12_30
						PIXEL4_13_10
					}
					else
					{
						PIXEL4_02_11
						PIXEL4_03_20
						PIXEL4_12_0
						PIXEL4_13_12
					}
					PIXEL4_11_0
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_10
						PIXEL4_21_30
						PIXEL4_30_80
						PIXEL4_31_10
					}
					else
					{
						PIXEL4_20_12
						PIXEL4_21_0
						PIXEL4_30_20
						PIXEL4_31_11
					}
					if (diff(w[6], w[8]))
					{
						PIXEL4_22_30
						PIXEL4_23_10
						PIXEL4_32_10
						PIXEL4_33_80
					}
					else
					{
						PIXEL4_22_0
						PIXEL4_23_11
						PIXEL4_32_12
						PIXEL4_33_20
					}
					break;
				}
				case 229:
				{
					PIXEL4_00_20
					PIXEL4_01_60
					PIXEL4_02_60
					PIXEL4_03_20
					PIXEL4_10_60
					PIXEL4_11_70
					PIXEL4_12_70
					PIXEL4_13_60
					PIXEL4_20_82
					PIXEL4_21_32
					PIXEL4_22_31
					PIXEL4_23_81
					PIXEL4_30_82
					PIXEL4_31_32
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 167:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_12_32
					PIXEL4_13_82
					PIXEL4_20_60
					PIXEL4_21_70
					PIXEL4_22_70
					PIXEL4_23_60
					PIXEL4_30_20
					PIXEL4_31_60
					PIXEL4_32_60
					PIXEL4_33_20
					break;
				}
				case 173:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_60
					PIXEL4_03_20
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_70
					PIXEL4_13_60
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_70
					PIXEL4_23_60
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_60
					PIXEL4_33_20
					break;
				}
				case 181:
				{
					PIXEL4_00_20
					PIXEL4_01_60
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_10_60
					PIXEL4_11_70
					PIXEL4_12_31
					PIXEL4_13_31
					PIXEL4_20_60
					PIXEL4_21_70
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_30_20
					PIXEL4_31_60
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 186:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_80
						PIXEL4_01_10
						PIXEL4_10_10
						PIXEL4_11_30
					}
					else
					{
						PIXEL4_00_20
						PIXEL4_01_12
						PIXEL4_10_11
						PIXEL4_11_0
					}
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_10
						PIXEL4_03_80
						PIXEL4_12_30
						PIXEL4_13_10
					}
					else
					{
						PIXEL4_02_11
						PIXEL4_03_20
						PIXEL4_12_0
						PIXEL4_13_12
					}
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 115:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_10
						PIXEL4_03_80
						PIXEL4_12_30
						PIXEL4_13_10
					}
					else
					{
						PIXEL4_02_11
						PIXEL4_03_20
						PIXEL4_12_0
						PIXEL4_13_12
					}
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_20_82
					PIXEL4_21_32
					if (diff(w[6], w[8]))
					{
						PIXEL4_22_30
						PIXEL4_23_10
						PIXEL4_32_10
						PIXEL4_33_80
					}
					else
					{
						PIXEL4_22_0
						PIXEL4_23_11
						PIXEL4_32_12
						PIXEL4_33_20
					}
					PIXEL4_30_82
					PIXEL4_31_32
					break;
				}
				case 93:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_31
					PIXEL4_13_31
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_10
						PIXEL4_21_30
						PIXEL4_30_80
						PIXEL4_31_10
					}
					else
					{
						PIXEL4_20_12
						PIXEL4_21_0
						PIXEL4_30_20
						PIXEL4_31_11
					}
					if (diff(w[6], w[8]))
					{
						PIXEL4_22_30
						PIXEL4_23_10
						PIXEL4_32_10
						PIXEL4_33_80
					}
					else
					{
						PIXEL4_22_0
						PIXEL4_23_11
						PIXEL4_32_12
						PIXEL4_33_20
					}
					break;
				}
				case 206:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_80
						PIXEL4_01_10
						PIXEL4_10_10
						PIXEL4_11_30
					}
					else
					{
						PIXEL4_00_20
						PIXEL4_01_12
						PIXEL4_10_11
						PIXEL4_11_0
					}
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_12_32
					PIXEL4_13_82
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_10
						PIXEL4_21_30
						PIXEL4_30_80
						PIXEL4_31_10
					}
					else
					{
						PIXEL4_20_12
						PIXEL4_21_0
						PIXEL4_30_20
						PIXEL4_31_11
					}
					PIXEL4_22_31
					PIXEL4_23_81
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 205:
				case 201:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_60
					PIXEL4_03_20
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_70
					PIXEL4_13_60
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_10
						PIXEL4_21_30
						PIXEL4_30_80
						PIXEL4_31_10
					}
					else
					{
						PIXEL4_20_12
						PIXEL4_21_0
						PIXEL4_30_20
						PIXEL4_31_11
					}
					PIXEL4_22_31
					PIXEL4_23_81
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 174:
				case 46:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_80
						PIXEL4_01_10
						PIXEL4_10_10
						PIXEL4_11_30
					}
					else
					{
						PIXEL4_00_20
						PIXEL4_01_12
						PIXEL4_10_11
						PIXEL4_11_0
					}
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_12_32
					PIXEL4_13_82
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_70
					PIXEL4_23_60
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_60
					PIXEL4_33_20
					break;
				}
				case 179:
				case 147:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_10
						PIXEL4_03_80
						PIXEL4_12_30
						PIXEL4_13_10
					}
					else
					{
						PIXEL4_02_11
						PIXEL4_03_20
						PIXEL4_12_0
						PIXEL4_13_12
					}
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_20_60
					PIXEL4_21_70
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_30_20
					PIXEL4_31_60
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 117:
				case 116:
				{
					PIXEL4_00_20
					PIXEL4_01_60
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_10_60
					PIXEL4_11_70
					PIXEL4_12_31
					PIXEL4_13_31
					PIXEL4_20_82
					PIXEL4_21_32
					if (diff(w[6], w[8]))
					{
						PIXEL4_22_30
						PIXEL4_23_10
						PIXEL4_32_10
						PIXEL4_33_80
					}
					else
					{
						PIXEL4_22_0
						PIXEL4_23_11
						PIXEL4_32_12
						PIXEL4_33_20
					}
					PIXEL4_30_82
					PIXEL4_31_32
					break;
				}
				case 189:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_31
					PIXEL4_13_31
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 231:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_12_32
					PIXEL4_13_82
					PIXEL4_20_82
					PIXEL4_21_32
					PIXEL4_22_31
					PIXEL4_23_81
					PIXEL4_30_82
					PIXEL4_31_32
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 126:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_13_0
					}
					else
					{
						PIXEL4_02_50
						PIXEL4_03_50
						PIXEL4_13_50
					}
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_0
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_0
						PIXEL4_30_0
						PIXEL4_31_0
					}
					else
					{
						PIXEL4_20_50
						PIXEL4_30_50
						PIXEL4_31_50
					}
					PIXEL4_21_0
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 219:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_10_0
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_50
						PIXEL4_10_50
					}
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_11_0
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_23_0
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_23_50
						PIXEL4_32_50
						PIXEL4_33_50
					}
					PIXEL4_30_80
					PIXEL4_31_10
					break;
				}
				case 125:
				{
					if (diff(w[8], w[4]))
					{
						PIXEL4_00_82
						PIXEL4_10_32
						PIXEL4_20_0
						PIXEL4_21_0
						PIXEL4_30_0
						PIXEL4_31_0
					}
					else
					{
						PIXEL4_00_11
						PIXEL4_10_13
						PIXEL4_20_83
						PIXEL4_21_70
						PIXEL4_30_50
						PIXEL4_31_21
					}
					PIXEL4_01_82
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_11_32
					PIXEL4_12_31
					PIXEL4_13_31
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 221:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_81
					if (diff(w[6], w[8]))
					{
						PIXEL4_03_81
						PIXEL4_13_31
						PIXEL4_22_0
						PIXEL4_23_0
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_03_12
						PIXEL4_13_14
						PIXEL4_22_70
						PIXEL4_23_83
						PIXEL4_32_21
						PIXEL4_33_50
					}
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_31
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_30_80
					PIXEL4_31_10
					break;
				}
				case 207:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_02_32
						PIXEL4_03_82
						PIXEL4_10_0
						PIXEL4_11_0
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_83
						PIXEL4_02_13
						PIXEL4_03_11
						PIXEL4_10_21
						PIXEL4_11_70
					}
					PIXEL4_12_32
					PIXEL4_13_82
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_31
					PIXEL4_23_81
					PIXEL4_30_80
					PIXEL4_31_10
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 238:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_32
					PIXEL4_13_82
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_0
						PIXEL4_21_0
						PIXEL4_30_0
						PIXEL4_31_0
						PIXEL4_32_31
						PIXEL4_33_81
					}
					else
					{
						PIXEL4_20_21
						PIXEL4_21_70
						PIXEL4_30_50
						PIXEL4_31_83
						PIXEL4_32_14
						PIXEL4_33_12
					}
					PIXEL4_22_31
					PIXEL4_23_81
					break;
				}
				case 190:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_12_0
						PIXEL4_13_0
						PIXEL4_23_32
						PIXEL4_33_82
					}
					else
					{
						PIXEL4_02_21
						PIXEL4_03_50
						PIXEL4_12_70
						PIXEL4_13_83
						PIXEL4_23_13
						PIXEL4_33_11
					}
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_32
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_82
					break;
				}
				case 187:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_10_0
						PIXEL4_11_0
						PIXEL4_20_31
						PIXEL4_30_81
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_21
						PIXEL4_10_83
						PIXEL4_11_70
						PIXEL4_20_14
						PIXEL4_30_12
					}
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_21_31
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_31_81
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 243:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_82
					PIXEL4_21_32
					if (diff(w[6], w[8]))
					{
						PIXEL4_22_0
						PIXEL4_23_0
						PIXEL4_30_82
						PIXEL4_31_32
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_22_70
						PIXEL4_23_21
						PIXEL4_30_11
						PIXEL4_31_13
						PIXEL4_32_83
						PIXEL4_33_50
					}
					break;
				}
				case 119:
				{
					if (diff(w[2], w[6]))
					{
						PIXEL4_00_81
						PIXEL4_01_31
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_12_0
						PIXEL4_13_0
					}
					else
					{
						PIXEL4_00_12
						PIXEL4_01_14
						PIXEL4_02_83
						PIXEL4_03_50
						PIXEL4_12_70
						PIXEL4_13_21
					}
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_20_82
					PIXEL4_21_32
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_82
					PIXEL4_31_32
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 237:
				case 233:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_60
					PIXEL4_03_20
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_70
					PIXEL4_13_60
					PIXEL4_20_0
					PIXEL4_21_0
					PIXEL4_22_31
					PIXEL4_23_81
					if (diff(w[8], w[4]))
					{
						PIXEL4_30_0
					}
					else
					{
						PIXEL4_30_20
					}
					PIXEL4_31_0
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 175:
				case 47:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
					}
					else
					{
						PIXEL4_00_20
					}
					PIXEL4_01_0
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_10_0
					PIXEL4_11_0
					PIXEL4_12_32
					PIXEL4_13_82
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_70
					PIXEL4_23_60
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_60
					PIXEL4_33_20
					break;
				}
				case 183:
				case 151:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					PIXEL4_02_0
					if (diff(w[2], w[6]))
					{
						PIXEL4_03_0
					}
					else
					{
						PIXEL4_03_20
					}
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_12_0
					PIXEL4_13_0
					PIXEL4_20_60
					PIXEL4_21_70
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_30_20
					PIXEL4_31_60
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 245:
				case 244:
				{
					PIXEL4_00_20
					PIXEL4_01_60
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_10_60
					PIXEL4_11_70
					PIXEL4_12_31
					PIXEL4_13_31
					PIXEL4_20_82
					PIXEL4_21_32
					PIXEL4_22_0
					PIXEL4_23_0
					PIXEL4_30_82
					PIXEL4_31_32
					PIXEL4_32_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_33_20
					}
					break;
				}
				case 250:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_30
					PIXEL4_13_10
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_0
						PIXEL4_30_0
						PIXEL4_31_0
					}
					else
					{
						PIXEL4_20_50
						PIXEL4_30_50
						PIXEL4_31_50
					}
					PIXEL4_21_0
					PIXEL4_22_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_23_0
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_23_50
						PIXEL4_32_50
						PIXEL4_33_50
					}
					break;
				}
				case 123:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_10_0
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_50
						PIXEL4_10_50
					}
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_11_0
					PIXEL4_12_30
					PIXEL4_13_10
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_0
						PIXEL4_30_0
						PIXEL4_31_0
					}
					else
					{
						PIXEL4_20_50
						PIXEL4_30_50
						PIXEL4_31_50
					}
					PIXEL4_21_0
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 95:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_10_0
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_50
						PIXEL4_10_50
					}
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_13_0
					}
					else
					{
						PIXEL4_02_50
						PIXEL4_03_50
						PIXEL4_13_50
					}
					PIXEL4_11_0
					PIXEL4_12_0
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_80
					PIXEL4_31_10
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 222:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_13_0
					}
					else
					{
						PIXEL4_02_50
						PIXEL4_03_50
						PIXEL4_13_50
					}
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_0
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_23_0
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_23_50
						PIXEL4_32_50
						PIXEL4_33_50
					}
					PIXEL4_30_80
					PIXEL4_31_10
					break;
				}
				case 252:
				{
					PIXEL4_00_80
					PIXEL4_01_61
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_31
					PIXEL4_13_31
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_0
						PIXEL4_30_0
						PIXEL4_31_0
					}
					else
					{
						PIXEL4_20_50
						PIXEL4_30_50
						PIXEL4_31_50
					}
					PIXEL4_21_0
					PIXEL4_22_0
					PIXEL4_23_0
					PIXEL4_32_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_33_20
					}
					break;
				}
				case 249:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_61
					PIXEL4_03_80
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_0
					PIXEL4_21_0
					PIXEL4_22_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_23_0
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_23_50
						PIXEL4_32_50
						PIXEL4_33_50
					}
					if (diff(w[8], w[4]))
					{
						PIXEL4_30_0
					}
					else
					{
						PIXEL4_30_20
					}
					PIXEL4_31_0
					break;
				}
				case 235:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_10_0
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_50
						PIXEL4_10_50
					}
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_11_0
					PIXEL4_12_30
					PIXEL4_13_61
					PIXEL4_20_0
					PIXEL4_21_0
					PIXEL4_22_31
					PIXEL4_23_81
					if (diff(w[8], w[4]))
					{
						PIXEL4_30_0
					}
					else
					{
						PIXEL4_30_20
					}
					PIXEL4_31_0
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 111:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
					}
					else
					{
						PIXEL4_00_20
					}
					PIXEL4_01_0
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_10_0
					PIXEL4_11_0
					PIXEL4_12_32
					PIXEL4_13_82
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_0
						PIXEL4_30_0
						PIXEL4_31_0
					}
					else
					{
						PIXEL4_20_50
						PIXEL4_30_50
						PIXEL4_31_50
					}
					PIXEL4_21_0
					PIXEL4_22_30
					PIXEL4_23_61
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 63:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
					}
					else
					{
						PIXEL4_00_20
					}
					PIXEL4_01_0
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_13_0
					}
					else
					{
						PIXEL4_02_50
						PIXEL4_03_50
						PIXEL4_13_50
					}
					PIXEL4_10_0
					PIXEL4_11_0
					PIXEL4_12_0
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_61
					PIXEL4_33_80
					break;
				}
				case 159:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_10_0
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_50
						PIXEL4_10_50
					}
					PIXEL4_02_0
					if (diff(w[2], w[6]))
					{
						PIXEL4_03_0
					}
					else
					{
						PIXEL4_03_20
					}
					PIXEL4_11_0
					PIXEL4_12_0
					PIXEL4_13_0
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_30_80
					PIXEL4_31_61
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 215:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					PIXEL4_02_0
					if (diff(w[2], w[6]))
					{
						PIXEL4_03_0
					}
					else
					{
						PIXEL4_03_20
					}
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_12_0
					PIXEL4_13_0
					PIXEL4_20_61
					PIXEL4_21_30
					PIXEL4_22_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_23_0
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_23_50
						PIXEL4_32_50
						PIXEL4_33_50
					}
					PIXEL4_30_80
					PIXEL4_31_10
					break;
				}
				case 246:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_13_0
					}
					else
					{
						PIXEL4_02_50
						PIXEL4_03_50
						PIXEL4_13_50
					}
					PIXEL4_10_61
					PIXEL4_11_30
					PIXEL4_12_0
					PIXEL4_20_82
					PIXEL4_21_32
					PIXEL4_22_0
					PIXEL4_23_0
					PIXEL4_30_82
					PIXEL4_31_32
					PIXEL4_32_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_33_20
					}
					break;
				}
				case 254:
				{
					PIXEL4_00_80
					PIXEL4_01_10
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_13_0
					}
					else
					{
						PIXEL4_02_50
						PIXEL4_03_50
						PIXEL4_13_50
					}
					PIXEL4_10_10
					PIXEL4_11_30
					PIXEL4_12_0
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_0
						PIXEL4_30_0
						PIXEL4_31_0
					}
					else
					{
						PIXEL4_20_50
						PIXEL4_30_50
						PIXEL4_31_50
					}
					PIXEL4_21_0
					PIXEL4_22_0
					PIXEL4_23_0
					PIXEL4_32_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_33_20
					}
					break;
				}
				case 253:
				{
					PIXEL4_00_82
					PIXEL4_01_82
					PIXEL4_02_81
					PIXEL4_03_81
					PIXEL4_10_32
					PIXEL4_11_32
					PIXEL4_12_31
					PIXEL4_13_31
					PIXEL4_20_0
					PIXEL4_21_0
					PIXEL4_22_0
					PIXEL4_23_0
					if (diff(w[8], w[4]))
					{
						PIXEL4_30_0
					}
					else
					{
						PIXEL4_30_20
					}
					PIXEL4_31_0
					PIXEL4_32_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_33_20
					}
					break;
				}
				case 251:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_10_0
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_50
						PIXEL4_10_50
					}
					PIXEL4_02_10
					PIXEL4_03_80
					PIXEL4_11_0
					PIXEL4_12_30
					PIXEL4_13_10
					PIXEL4_20_0
					PIXEL4_21_0
					PIXEL4_22_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_23_0
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_23_50
						PIXEL4_32_50
						PIXEL4_33_50
					}
					if (diff(w[8], w[4]))
					{
						PIXEL4_30_0
					}
					else
					{
						PIXEL4_30_20
					}
					PIXEL4_31_0
					break;
				}
				case 239:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
					}
					else
					{
						PIXEL4_00_20
					}
					PIXEL4_01_0
					PIXEL4_02_32
					PIXEL4_03_82
					PIXEL4_10_0
					PIXEL4_11_0
					PIXEL4_12_32
					PIXEL4_13_82
					PIXEL4_20_0
					PIXEL4_21_0
					PIXEL4_22_31
					PIXEL4_23_81
					if (diff(w[8], w[4]))
					{
						PIXEL4_30_0
					}
					else
					{
						PIXEL4_30_20
					}
					PIXEL4_31_0
					PIXEL4_32_31
					PIXEL4_33_81
					break;
				}
				case 127:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
					}
					else
					{
						PIXEL4_00_20
					}
					PIXEL4_01_0
					if (diff(w[2], w[6]))
					{
						PIXEL4_02_0
						PIXEL4_03_0
						PIXEL4_13_0
					}
					else
					{
						PIXEL4_02_50
						PIXEL4_03_50
						PIXEL4_13_50
					}
					PIXEL4_10_0
					PIXEL4_11_0
					PIXEL4_12_0
					if (diff(w[8], w[4]))
					{
						PIXEL4_20_0
						PIXEL4_30_0
						PIXEL4_31_0
					}
					else
					{
						PIXEL4_20_50
						PIXEL4_30_50
						PIXEL4_31_50
					}
					PIXEL4_21_0
					PIXEL4_22_30
					PIXEL4_23_10
					PIXEL4_32_10
					PIXEL4_33_80
					break;
				}
				case 191:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
					}
					else
					{
						PIXEL4_00_20
					}
					PIXEL4_01_0
					PIXEL4_02_0
					if (diff(w[2], w[6]))
					{
						PIXEL4_03_0
					}
					else
					{
						PIXEL4_03_20
					}
					PIXEL4_10_0
					PIXEL4_11_0
					PIXEL4_12_0
					PIXEL4_13_0
					PIXEL4_20_31
					PIXEL4_21_31
					PIXEL4_22_32
					PIXEL4_23_32
					PIXEL4_30_81
					PIXEL4_31_81
					PIXEL4_32_82
					PIXEL4_33_82
					break;
				}
				case 223:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
						PIXEL4_01_0
						PIXEL4_10_0
					}
					else
					{
						PIXEL4_00_50
						PIXEL4_01_50
						PIXEL4_10_50
					}
					PIXEL4_02_0
					if (diff(w[2], w[6]))
					{
						PIXEL4_03_0
					}
					else
					{
						PIXEL4_03_20
					}
					PIXEL4_11_0
					PIXEL4_12_0
					PIXEL4_13_0
					PIXEL4_20_10
					PIXEL4_21_30
					PIXEL4_22_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_23_0
						PIXEL4_32_0
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_23_50
						PIXEL4_32_50
						PIXEL4_33_50
					}
					PIXEL4_30_80
					PIXEL4_31_10
					break;
				}
				case 247:
				{
					PIXEL4_00_81
					PIXEL4_01_31
					PIXEL4_02_0
					if (diff(w[2], w[6]))
					{
						PIXEL4_03_0
					}
					else
					{
						PIXEL4_03_20
					}
					PIXEL4_10_81
					PIXEL4_11_31
					PIXEL4_12_0
					PIXEL4_13_0
					PIXEL4_20_82
					PIXEL4_21_32
					PIXEL4_22_0
					PIXEL4_23_0
					PIXEL4_30_82
					PIXEL4_31_32
					PIXEL4_32_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_33_20
					}
					break;
				}
				case 255:
				{
					if (diff(w[4], w[2]))
					{
						PIXEL4_00_0
					}
					else
					{
						PIXEL4_00_20
					}
					PIXEL4_01_0
					PIXEL4_02_0
					if (diff(w[2], w[6]))
					{
						PIXEL4_03_0
					}
					else
					{
						PIXEL4_03_20
					}
					PIXEL4_10_0
					PIXEL4_11_0
					PIXEL4_12_0
					PIXEL4_13_0
					PIXEL4_20_0
					PIXEL4_21_0
					PIXEL4_22_0
					PIXEL4_23_0
					if (diff(w[8], w[4]))
					{
						PIXEL4_30_0
					}
					else
					{
						PIXEL4_30_20
					}
					PIXEL4_31_0
					PIXEL4_32_0
					if (diff(w[6], w[8]))
					{
						PIXEL4_33_0
					}
					else
					{
						PIXEL4_33_20
					}
					break;
				}
			}
			
			src++;
			dst += 4 * dst_Bpp;
		}
		
		src = src_temp + src_pitch;
		dst = dst_temp + 4 * dst_pitch;
	}
}

// kate: tab-width 4; vim: set noet:
