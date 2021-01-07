/*
 * filter.c
 * Copyright (C) 2003 Florian Schulze <crow@icculus.org>
 *
 * This file is part of Jump'n'Bump.
 *
 * Jump'n'Bump is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Jump'n'Bump is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 The following scaling filter is called advancedmame2x.
 The implementation found here was possible because of the great ideas of
 Lucas Pope.
 */

typedef unsigned char byte;
static int scale2x_inited = 0;
static byte lookup_map[4*16];

void init_scale2x(void)
{
	int i;

	if (scale2x_inited)
		return;

	//-------------------------------------------------------------------------  
	// scale2x takes the following source:
	// A B C
	// D E F
	// G H I
	//
	// and doubles the size of E to produce:
	// E0 E1
	// E2 E3
	//
	//  E0 = D == B && B != F && D != H ? D : E;
	//  E1 = B == F && B != D && F != H ? F : E;
	//  E2 = D == H && D != B && H != F ? D : E;
	//  E3 = H == F && D != H && B != F ? F : E;
	//
	// to make this comparison regimen faster, we encode source color
	// equivalency into a single byte with the getCode() macro
	//
	// #define getCode(b,f,h,d) ( (b == f)<<0 | (f == h)<<1 | (h == d)<<2 | (d == b)<<3 )
  
	// encode the scale2x conditionals into a lookup code
	for (i=0; i<16; i++) {
		//  E0 = D == B && B != F && D != H ? D : E; // 10-0 => 1000 or 1010 => 8 or A
		lookup_map[0*16+i] = (i == 0x8 || i == 0xA) ? 0 : 1;
		//  E1 = B == F && B != D && F != H ? F : E; // 0-01 => 0101 or 0001 => 5 or 1
		lookup_map[1*16+i] = (i == 0x5 || i == 0x1) ? 2 : 1;
		//  E2 = D == H && D != B && H != F ? D : E; // 010- => 0101 or 0100 => 5 or 4
		lookup_map[2*16+i] = (i == 0x4 || i == 0x5) ? 0 : 1;
		//  E3 = H == F && D != H && B != F ? F : E; // -010 => 1010 or 0010 => A or 2
		lookup_map[3*16+i] = (i == 0xA || i == 0x2) ? 2 : 1;
	}
}

void do_scale2x(unsigned char *src,
		int src_width,
		int src_height,
		unsigned char *dst)
{
	int x;
	int y;
	int dst_width = src_width * 2;
	int dst_height = src_height * 2;
	int code;
	byte rowColors[3];
	byte *e0;
	byte *e1;
	byte *e2;
	byte *e3;

	if (!scale2x_inited)
		init_scale2x();

	// special top case - b is always unknown
	{
		byte *d;
		byte *e;
		byte *f;
		byte *h;

		e0 = &dst[0];
		e1 = &dst[1];
		e2 = &dst[dst_width];
		e3 = &dst[dst_width + 1];
		e = &src[0];
		f = &src[1];
		h = &src[src_width];

		// special left case - d is unknown
		rowColors[0] = *e;
		rowColors[1] = *e;
		rowColors[2] = *f;
		code = ( (*f == *h)<<1 );
		*e0 = rowColors[lookup_map[0*16+code]];
		*e1 = rowColors[lookup_map[1*16+code]];
		*e2 = rowColors[lookup_map[2*16+code]];
		*e3 = rowColors[lookup_map[3*16+code]];
		e++; f++; h++;
		d = &src[src_width]; // (src_width - 1) + 1
		e0+=2; e1+=2; e2+=2; e3+=2;

		// normal case
		for (x=1; x<(src_width-1); x++) {
			rowColors[0] = *d;
			rowColors[1] = *e;
			rowColors[2] = *f;
			code = ( (*f == *h)<<1 | (*h == *d)<<2 );
			*e0 = rowColors[lookup_map[0*16+code]];
			*e1 = rowColors[lookup_map[1*16+code]];
			*e2 = rowColors[lookup_map[2*16+code]];
			*e3 = rowColors[lookup_map[3*16+code]];
			d++; e++; f++; h++;
			e0+=2; e1+=2; e2+=2; e3+=2;
		}

		// special right case - f is unknown
		rowColors[0] = *d;
		rowColors[1] = *e;
		rowColors[2] = *e;
		code = ( (*h == *d)<<2 );
		*e0 = rowColors[lookup_map[0*16+code]];
		*e1 = rowColors[lookup_map[1*16+code]];
		*e2 = rowColors[lookup_map[2*16+code]];
		*e3 = rowColors[lookup_map[3*16+code]];
	}

	// top and bottom always known
	for (y=1; y<(src_height-1); y++) {
		byte *b;
		byte *d;
		byte *e;
		byte *f;
		byte *h;

		e0 = &dst[y*dst_width*2];
		e1 = &dst[y*dst_width*2 + 1];
		e2 = &dst[y*dst_width*2 + dst_width];
		e3 = &dst[y*dst_width*2 + dst_width + 1];
		b = &src[y * src_width - src_width];
		e = &src[y * src_width];
		f = &src[y * src_width + 1];
		h = &src[y * src_width + src_width];

		// special left case - d is unknown
		rowColors[0] = *e;
		rowColors[1] = *e;
		rowColors[2] = *f;
		code = ( (*b == *f)<<0 | (*f == *h)<<1 );
		*e0 = rowColors[lookup_map[0*16+code]];
		*e1 = rowColors[lookup_map[1*16+code]];
		*e2 = rowColors[lookup_map[2*16+code]];
		*e3 = rowColors[lookup_map[3*16+code]];
		b++; e++; f++; h++;
		d = &src[y * src_width]; // (y * src_width - 1) + 1
		e0+=2; e1+=2; e2+=2; e3+=2;

		// normal case
		for (x=1; x<(src_width-1); x++) {
			rowColors[0] = *d;
			rowColors[1] = *e;
			rowColors[2] = *f;
			code = ( (*b == *f)<<0 | (*f == *h)<<1 | (*h == *d)<<2 | (*d == *b)<<3 );
			*e0 = rowColors[lookup_map[0*16+code]];
			*e1 = rowColors[lookup_map[1*16+code]];
			*e2 = rowColors[lookup_map[2*16+code]];
			*e3 = rowColors[lookup_map[3*16+code]];
			b++; d++; e++; f++; h++;
			e0+=2; e1+=2; e2+=2; e3+=2;
		}

		// special right case - f is unknown
		rowColors[0] = *d;
		rowColors[1] = *e;
		rowColors[2] = *e;
		code = ( (*h == *d)<<2 | (*d == *b)<<3 );
		*e0 = rowColors[lookup_map[0*16+code]];
		*e1 = rowColors[lookup_map[1*16+code]];
		*e2 = rowColors[lookup_map[2*16+code]];
		*e3 = rowColors[lookup_map[3*16+code]];
	}

	// special bottom case - h is always unknown
	{
		byte *b;
		byte *d;
		byte *e;
		byte *f;

		e0 = &dst[y*dst_width*2];
		e1 = &dst[y*dst_width*2 + 1];
		e2 = &dst[y*dst_width*2 + dst_width];
		e3 = &dst[y*dst_width*2 + dst_width + 1];
		b = &src[y * src_width - src_width];
		e = &src[y * src_width];
		f = &src[y * src_width + 1];

		// special left case - d is unknown
		rowColors[0] = *e;
		rowColors[1] = *e;
		rowColors[2] = *f;
		code = ( (*b == *f)<<0 );
		*e0 = rowColors[lookup_map[0*16+code]];
		*e1 = rowColors[lookup_map[1*16+code]];
		*e2 = rowColors[lookup_map[2*16+code]];
		*e3 = rowColors[lookup_map[3*16+code]];
		b++; e++; f++;
		d = &src[y * src_width]; // (y * src_width - 1) + 1
		e0+=2; e1+=2; e2+=2; e3+=2;

		// normal case
		for (x=1; x<(src_width-1); x++) {
			rowColors[0] = *d;
			rowColors[1] = *e;
			rowColors[2] = *f;
			code = ( (*b == *f)<<0 | (*d == *b)<<3 );
			*e0 = rowColors[lookup_map[0*16+code]];
			*e1 = rowColors[lookup_map[1*16+code]];
			*e2 = rowColors[lookup_map[2*16+code]];
			*e3 = rowColors[lookup_map[3*16+code]];
			b++; d++; e++; f++;
			e0+=2; e1+=2; e2+=2; e3+=2;
		}

		// special right case - f is unknown
		rowColors[0] = *d;
		rowColors[1] = *e;
		rowColors[2] = *e;
		code = ( (*d == *b)<<3 );
		*e0 = rowColors[lookup_map[0*16+code]];
		*e1 = rowColors[lookup_map[1*16+code]];
		*e2 = rowColors[lookup_map[2*16+code]];
		*e3 = rowColors[lookup_map[3*16+code]];
	}
}
