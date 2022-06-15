/*
 * Copyright (C) 2003 Maxim Stepin ( maxst@hiend3d.com )
 *
 * Copyright (C) 2010 Cameron Zemek ( grom@zeminvaders.net)
 * Copyright (C) 2011 Francois Gannaz <mytskine@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __HQX_COMMON_H_
#define __HQX_COMMON_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MASK_2     0x0000FF00
#define MASK_13    0x00FF00FF
#define MASK_RGB   0x00FFFFFF
#define MASK_ALPHA 0xFF000000

#define MASK16_2     0x07E0
#define MASK16_13    0xF81F
#define MASK16_RGB   0xFFFF

#define Ymask 0x00FF0000
#define Umask 0x0000FF00
#define Vmask 0x000000FF
#define trY   0x00300000
#define trU   0x00000700
#define trV   0x00000006

/* RGB to YUV lookup table */
extern uint32_t RGBtoYUV[16777216];

static inline uint32_t rgb32_to_yuv(uint32_t c)
{
    // Mask against MASK_RGB to discard the alpha channel
    return RGBtoYUV[MASK_RGB & c];
}

static inline uint32_t rgb16_to_yuv(uint16_t c)
{
    return RGBtoYUV[(((c & 0xF800) << 8) |
                     ((c & 0x07E0) << 5) |
                     ((c & 0x001F) << 3))];
}

static inline uint24_t *u24cpy(uint24_t *dst, const uint24_t src)
{
	/* memcpy() is sometimes faster. */
#ifdef HQX_U24CPY_MEMCPY
	memcpy(*dst, src, sizeof(*dst));
#else
	(*dst)[0] = src[0];
	(*dst)[1] = src[1];
	(*dst)[2] = src[2];
#endif
	return dst;
}

static inline uint32_t rgb24_to_yuv(uint24_t c)
{
    return RGBtoYUV[((c[0] << 16) | (c[1] << 8) | c[2])];
}

/* Test if there is difference in color */
static inline int yuv_diff(uint32_t yuv1, uint32_t yuv2) {
    return (( abs((yuv1 & Ymask) - (yuv2 & Ymask)) > trY ) ||
            ( abs((yuv1 & Umask) - (yuv2 & Umask)) > trU ) ||
            ( abs((yuv1 & Vmask) - (yuv2 & Vmask)) > trV ) );
}

static inline int Diff32(uint32_t c1, uint32_t c2)
{
    return yuv_diff(rgb32_to_yuv(c1), rgb32_to_yuv(c2));
}

static inline int Diff16(uint16_t c1, uint16_t c2)
{
    return yuv_diff(rgb16_to_yuv(c1), rgb16_to_yuv(c2));
}

static inline int Diff24(uint24_t c1, uint24_t c2)
{
    return yuv_diff(rgb24_to_yuv(c1), rgb24_to_yuv(c2));
}

/* Interpolate functions */
static inline uint32_t Interpolate_2_32(uint32_t c1, int w1, uint32_t c2, int w2, int s)
{
    if (c1 == c2) {
        return c1;
    }
    return
        (((((c1 & MASK_ALPHA) >> 24) * w1 + ((c2 & MASK_ALPHA) >> 24) * w2) << (24-s)) & MASK_ALPHA) +
        ((((c1 & MASK_2) * w1 + (c2 & MASK_2) * w2) >> s) & MASK_2)	+
        ((((c1 & MASK_13) * w1 + (c2 & MASK_13) * w2) >> s) & MASK_13);
}

static inline uint32_t Interpolate_3_32(uint32_t c1, int w1, uint32_t c2, int w2, uint32_t c3, int w3, int s)
{
    return
        (((((c1 & MASK_ALPHA) >> 24) * w1 + ((c2 & MASK_ALPHA) >> 24) * w2 + ((c3 & MASK_ALPHA) >> 24) * w3) << (24-s)) & MASK_ALPHA) +
        ((((c1 & MASK_2) * w1 + (c2 & MASK_2) * w2 + (c3 & MASK_2) * w3) >> s) & MASK_2) +
        ((((c1 & MASK_13) * w1 + (c2 & MASK_13) * w2 + (c3 & MASK_13) * w3) >> s) & MASK_13);
}

static inline uint32_t Interp1_32(uint32_t c1, uint32_t c2)
{
    //(c1*3+c2) >> 2;
    return Interpolate_2_32(c1, 3, c2, 1, 2);
}

static inline uint32_t Interp2_32(uint32_t c1, uint32_t c2, uint32_t c3)
{
    //(c1*2+c2+c3) >> 2;
    return Interpolate_3_32(c1, 2, c2, 1, c3, 1, 2);
}

static inline uint32_t Interp3_32(uint32_t c1, uint32_t c2)
{
    //(c1*7+c2)/8;
    return Interpolate_2_32(c1, 7, c2, 1, 3);
}

static inline uint32_t Interp4_32(uint32_t c1, uint32_t c2, uint32_t c3)
{
    //(c1*2+(c2+c3)*7)/16;
    return Interpolate_3_32(c1, 2, c2, 7, c3, 7, 4);
}

static inline uint32_t Interp5_32(uint32_t c1, uint32_t c2)
{
    //(c1+c2) >> 1;
    return Interpolate_2_32(c1, 1, c2, 1, 1);
}

static inline uint32_t Interp6_32(uint32_t c1, uint32_t c2, uint32_t c3)
{
    //(c1*5+c2*2+c3)/8;
    return Interpolate_3_32(c1, 5, c2, 2, c3, 1, 3);
}

static inline uint32_t Interp7_32(uint32_t c1, uint32_t c2, uint32_t c3)
{
    //(c1*6+c2+c3)/8;
    return Interpolate_3_32(c1, 6, c2, 1, c3, 1, 3);
}

static inline uint32_t Interp8_32(uint32_t c1, uint32_t c2)
{
    //(c1*5+c2*3)/8;
    return Interpolate_2_32(c1, 5, c2, 3, 3);
}

static inline uint32_t Interp9_32(uint32_t c1, uint32_t c2, uint32_t c3)
{
    //(c1*2+(c2+c3)*3)/8;
    return Interpolate_3_32(c1, 2, c2, 3, c3, 3, 3);
}

static inline uint32_t Interp10_32(uint32_t c1, uint32_t c2, uint32_t c3)
{
    //(c1*14+c2+c3)/16;
    return Interpolate_3_32(c1, 14, c2, 1, c3, 1, 4);
}

/* Interpolate functions (16 bit, 565) */
static inline uint16_t Interpolate_2_16(uint16_t c1, int w1, uint16_t c2, int w2, int s)
{
    if (c1 == c2) {
        return c1;
    }
    return
        ((((c1 & MASK16_2) * w1 + (c2 & MASK16_2) * w2) >> s) & MASK16_2) +
        ((((c1 & MASK16_13) * w1 + (c2 & MASK16_13) * w2) >> s) & MASK16_13);
}

static inline uint16_t Interpolate_3_16(uint16_t c1, int w1, uint16_t c2, int w2, uint16_t c3, int w3, int s)
{
    return
        ((((c1 & MASK16_2) * w1 + (c2 & MASK16_2) * w2 + (c3 & MASK16_2) * w3) >> s) & MASK16_2) +
        ((((c1 & MASK16_13) * w1 + (c2 & MASK16_13) * w2 + (c3 & MASK16_13) * w3) >> s) & MASK16_13);
}

static inline uint16_t Interp1_16(uint16_t c1, uint16_t c2)
{
    //(c1*3+c2) >> 2;
    return Interpolate_2_16(c1, 3, c2, 1, 2);
}

static inline uint16_t Interp2_16(uint16_t c1, uint16_t c2, uint16_t c3)
{
    //(c1*2+c2+c3) >> 2;
    return Interpolate_3_16(c1, 2, c2, 1, c3, 1, 2);
}

static inline uint16_t Interp3_16(uint16_t c1, uint16_t c2)
{
    //(c1*7+c2)/8;
    return Interpolate_2_16(c1, 7, c2, 1, 3);
}

static inline uint16_t Interp4_16(uint16_t c1, uint16_t c2, uint16_t c3)
{
    //(c1*2+(c2+c3)*7)/16;
    return Interpolate_3_16(c1, 2, c2, 7, c3, 7, 4);
}

static inline uint16_t Interp5_16(uint16_t c1, uint16_t c2)
{
    //(c1+c2) >> 1;
    return Interpolate_2_16(c1, 1, c2, 1, 1);
}

static inline uint16_t Interp6_16(uint16_t c1, uint16_t c2, uint16_t c3)
{
    //(c1*5+c2*2+c3)/8;
    return Interpolate_3_16(c1, 5, c2, 2, c3, 1, 3);
}

static inline uint16_t Interp7_16(uint16_t c1, uint16_t c2, uint16_t c3)
{
    //(c1*6+c2+c3)/8;
    return Interpolate_3_16(c1, 6, c2, 1, c3, 1, 3);
}

static inline uint16_t Interp8_16(uint16_t c1, uint16_t c2)
{
    //(c1*5+c2*3)/8;
    return Interpolate_2_16(c1, 5, c2, 3, 3);
}

static inline uint16_t Interp9_16(uint16_t c1, uint16_t c2, uint16_t c3)
{
    //(c1*2+(c2+c3)*3)/8;
    return Interpolate_3_16(c1, 2, c2, 3, c3, 3, 3);
}

static inline uint16_t Interp10_16(uint16_t c1, uint16_t c2, uint16_t c3)
{
    //(c1*14+c2+c3)/16;
    return Interpolate_3_16(c1, 14, c2, 1, c3, 1, 4);
}

/* Interpolate functions (24 bit, 888) */
static inline void Interpolate_2_24(uint24_t *ret, uint24_t c1, int w1, uint24_t c2, int w2, int s)
{
    if (!memcmp(c1, c2, 3)) {
        u24cpy(ret, c1);
        return;
    }
    (*ret)[0] = (((c1[0] * w1) + (c2[0] * w2)) >> s);
    (*ret)[1] = (((c1[1] * w1) + (c2[1] * w2)) >> s);
    (*ret)[2] = (((c1[2] * w1) + (c2[2] * w2)) >> s);
}

static inline void Interpolate_3_24(uint24_t *ret, uint24_t c1, int w1, uint24_t c2, int w2, uint24_t c3, int w3, int s)
{
    (*ret)[0] = (((c1[0] * w1) + (c2[0] * w2) + (c3[0] * w3)) >> s);
    (*ret)[1] = (((c1[1] * w1) + (c2[1] * w2) + (c3[1] * w3)) >> s);
    (*ret)[2] = (((c1[2] * w1) + (c2[2] * w2) + (c3[2] * w3)) >> s);
}

static inline void Interp1_24(uint24_t *ret, uint24_t c1, uint24_t c2)
{
    //(c1*3+c2) >> 2;
    Interpolate_2_24(ret, c1, 3, c2, 1, 2);
}

static inline void Interp2_24(uint24_t *ret, uint24_t c1, uint24_t c2, uint24_t c3)
{
    //(c1*2+c2+c3) >> 2;
    Interpolate_3_24(ret, c1, 2, c2, 1, c3, 1, 2);
}

static inline void Interp3_24(uint24_t *ret, uint24_t c1, uint24_t c2)
{
    //(c1*7+c2)/8;
    Interpolate_2_24(ret, c1, 7, c2, 1, 3);
}

static inline void Interp4_24(uint24_t *ret, uint24_t c1, uint24_t c2, uint24_t c3)
{
    //(c1*2+(c2+c3)*7)/16;
    Interpolate_3_24(ret, c1, 2, c2, 7, c3, 7, 4);
}

static inline void Interp5_24(uint24_t *ret, uint24_t c1, uint24_t c2)
{
    //(c1+c2) >> 1;
    Interpolate_2_24(ret, c1, 1, c2, 1, 1);
}

static inline void Interp6_24(uint24_t *ret, uint24_t c1, uint24_t c2, uint24_t c3)
{
    //(c1*5+c2*2+c3)/8;
    Interpolate_3_24(ret, c1, 5, c2, 2, c3, 1, 3);
}

static inline void Interp7_24(uint24_t *ret, uint24_t c1, uint24_t c2, uint24_t c3)
{
    //(c1*6+c2+c3)/8;
    Interpolate_3_24(ret, c1, 6, c2, 1, c3, 1, 3);
}

static inline void Interp8_24(uint24_t *ret, uint24_t c1, uint24_t c2)
{
    //(c1*5+c2*3)/8;
    Interpolate_2_24(ret, c1, 5, c2, 3, 3);
}

static inline void Interp9_24(uint24_t *ret, uint24_t c1, uint24_t c2, uint24_t c3)
{
    //(c1*2+(c2+c3)*3)/8;
    Interpolate_3_24(ret, c1, 2, c2, 3, c3, 3, 3);
}

static inline void Interp10_24(uint24_t *ret, uint24_t c1, uint24_t c2, uint24_t c3)
{
    //(c1*14+c2+c3)/16;
    Interpolate_3_24(ret, c1, 14, c2, 1, c3, 1, 4);
}

#endif
