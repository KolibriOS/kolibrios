/*
 * Copyright (C) 2003 Maxim Stepin ( maxst@hiend3d.com )
 *
 * Copyright (C) 2010 Cameron Zemek ( grom@zeminvaders.net)
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

#include <stdint.h>
#include "hqx.h"
#include "common.h"

#define PIXEL00_1M  *dp = Interp1_16(w[5], w[1]);
#define PIXEL00_1U  *dp = Interp1_16(w[5], w[2]);
#define PIXEL00_1L  *dp = Interp1_16(w[5], w[4]);
#define PIXEL00_2   *dp = Interp2_16(w[5], w[4], w[2]);
#define PIXEL00_4   *dp = Interp4_16(w[5], w[4], w[2]);
#define PIXEL00_5   *dp = Interp5_16(w[4], w[2]);
#define PIXEL00_C   *dp   = w[5];

#define PIXEL01_1   *(dp+1) = Interp1_16(w[5], w[2]);
#define PIXEL01_3   *(dp+1) = Interp3_16(w[5], w[2]);
#define PIXEL01_6   *(dp+1) = Interp1_16(w[2], w[5]);
#define PIXEL01_C   *(dp+1) = w[5];

#define PIXEL02_1M  *(dp+2) = Interp1_16(w[5], w[3]);
#define PIXEL02_1U  *(dp+2) = Interp1_16(w[5], w[2]);
#define PIXEL02_1R  *(dp+2) = Interp1_16(w[5], w[6]);
#define PIXEL02_2   *(dp+2) = Interp2_16(w[5], w[2], w[6]);
#define PIXEL02_4   *(dp+2) = Interp4_16(w[5], w[2], w[6]);
#define PIXEL02_5   *(dp+2) = Interp5_16(w[2], w[6]);
#define PIXEL02_C   *(dp+2) = w[5];

#define PIXEL10_1   *(dp+dpL) = Interp1_16(w[5], w[4]);
#define PIXEL10_3   *(dp+dpL) = Interp3_16(w[5], w[4]);
#define PIXEL10_6   *(dp+dpL) = Interp1_16(w[4], w[5]);
#define PIXEL10_C   *(dp+dpL) = w[5];

#define PIXEL11     *(dp+dpL+1) = w[5];

#define PIXEL12_1   *(dp+dpL+2) = Interp1_16(w[5], w[6]);
#define PIXEL12_3   *(dp+dpL+2) = Interp3_16(w[5], w[6]);
#define PIXEL12_6   *(dp+dpL+2) = Interp1_16(w[6], w[5]);
#define PIXEL12_C   *(dp+dpL+2) = w[5];

#define PIXEL20_1M  *(dp+dpL+dpL) = Interp1_16(w[5], w[7]);
#define PIXEL20_1D  *(dp+dpL+dpL) = Interp1_16(w[5], w[8]);
#define PIXEL20_1L  *(dp+dpL+dpL) = Interp1_16(w[5], w[4]);
#define PIXEL20_2   *(dp+dpL+dpL) = Interp2_16(w[5], w[8], w[4]);
#define PIXEL20_4   *(dp+dpL+dpL) = Interp4_16(w[5], w[8], w[4]);
#define PIXEL20_5   *(dp+dpL+dpL) = Interp5_16(w[8], w[4]);
#define PIXEL20_C   *(dp+dpL+dpL) = w[5];

#define PIXEL21_1   *(dp+dpL+dpL+1) = Interp1_16(w[5], w[8]);
#define PIXEL21_3   *(dp+dpL+dpL+1) = Interp3_16(w[5], w[8]);
#define PIXEL21_6   *(dp+dpL+dpL+1) = Interp1_16(w[8], w[5]);
#define PIXEL21_C   *(dp+dpL+dpL+1) = w[5];

#define PIXEL22_1M  *(dp+dpL+dpL+2) = Interp1_16(w[5], w[9]);
#define PIXEL22_1D  *(dp+dpL+dpL+2) = Interp1_16(w[5], w[8]);
#define PIXEL22_1R  *(dp+dpL+dpL+2) = Interp1_16(w[5], w[6]);
#define PIXEL22_2   *(dp+dpL+dpL+2) = Interp2_16(w[5], w[6], w[8]);
#define PIXEL22_4   *(dp+dpL+dpL+2) = Interp4_16(w[5], w[6], w[8]);
#define PIXEL22_5   *(dp+dpL+dpL+2) = Interp5_16(w[6], w[8]);
#define PIXEL22_C   *(dp+dpL+dpL+2) = w[5];

#define HQ3X_BITS 16
#define HQ3X_BYTES 2
#define HQ3X_TYPE uint16_t
#define HQ3X_CPY(to, from) (to) = (from)

#define HQ3X_FUNC hq3x_16
#define HQ3X_RB_FUNC hq3x_16_rb

#define RGB_TO_YUV_FUNC rgb16_to_yuv
#define DIFF_FUNC Diff16

#include "hq3x-int.h"
