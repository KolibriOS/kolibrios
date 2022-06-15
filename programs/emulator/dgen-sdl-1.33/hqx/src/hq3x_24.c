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

#define PIXEL00_1M  Interp1_24(dp, w[5], w[1]);
#define PIXEL00_1U  Interp1_24(dp, w[5], w[2]);
#define PIXEL00_1L  Interp1_24(dp, w[5], w[4]);
#define PIXEL00_2   Interp2_24(dp, w[5], w[4], w[2]);
#define PIXEL00_4   Interp4_24(dp, w[5], w[4], w[2]);
#define PIXEL00_5   Interp5_24(dp, w[4], w[2]);
#define PIXEL00_C   u24cpy(dp, w[5]);

#define PIXEL01_1   Interp1_24((dp+1), w[5], w[2]);
#define PIXEL01_3   Interp3_24((dp+1), w[5], w[2]);
#define PIXEL01_6   Interp1_24((dp+1), w[2], w[5]);
#define PIXEL01_C   u24cpy((dp+1), w[5]);

#define PIXEL02_1M  Interp1_24((dp+2), w[5], w[3]);
#define PIXEL02_1U  Interp1_24((dp+2), w[5], w[2]);
#define PIXEL02_1R  Interp1_24((dp+2), w[5], w[6]);
#define PIXEL02_2   Interp2_24((dp+2), w[5], w[2], w[6]);
#define PIXEL02_4   Interp4_24((dp+2), w[5], w[2], w[6]);
#define PIXEL02_5   Interp5_24((dp+2), w[2], w[6]);
#define PIXEL02_C   u24cpy((dp+2), w[5]);

#define PIXEL10_1   Interp1_24((dp+dpL), w[5], w[4]);
#define PIXEL10_3   Interp3_24((dp+dpL), w[5], w[4]);
#define PIXEL10_6   Interp1_24((dp+dpL), w[4], w[5]);
#define PIXEL10_C   u24cpy((dp+dpL), w[5]);

#define PIXEL11     u24cpy((dp+dpL+1), w[5]);

#define PIXEL12_1   Interp1_24((dp+dpL+2), w[5], w[6]);
#define PIXEL12_3   Interp3_24((dp+dpL+2), w[5], w[6]);
#define PIXEL12_6   Interp1_24((dp+dpL+2), w[6], w[5]);
#define PIXEL12_C   u24cpy((dp+dpL+2), w[5]);

#define PIXEL20_1M  Interp1_24((dp+dpL+dpL), w[5], w[7]);
#define PIXEL20_1D  Interp1_24((dp+dpL+dpL), w[5], w[8]);
#define PIXEL20_1L  Interp1_24((dp+dpL+dpL), w[5], w[4]);
#define PIXEL20_2   Interp2_24((dp+dpL+dpL), w[5], w[8], w[4]);
#define PIXEL20_4   Interp4_24((dp+dpL+dpL), w[5], w[8], w[4]);
#define PIXEL20_5   Interp5_24((dp+dpL+dpL), w[8], w[4]);
#define PIXEL20_C   u24cpy((dp+dpL+dpL), w[5]);

#define PIXEL21_1   Interp1_24((dp+dpL+dpL+1), w[5], w[8]);
#define PIXEL21_3   Interp3_24((dp+dpL+dpL+1), w[5], w[8]);
#define PIXEL21_6   Interp1_24((dp+dpL+dpL+1), w[8], w[5]);
#define PIXEL21_C   u24cpy((dp+dpL+dpL+1), w[5]);

#define PIXEL22_1M  Interp1_24((dp+dpL+dpL+2), w[5], w[9]);
#define PIXEL22_1D  Interp1_24((dp+dpL+dpL+2), w[5], w[8]);
#define PIXEL22_1R  Interp1_24((dp+dpL+dpL+2), w[5], w[6]);
#define PIXEL22_2   Interp2_24((dp+dpL+dpL+2), w[5], w[6], w[8]);
#define PIXEL22_4   Interp4_24((dp+dpL+dpL+2), w[5], w[6], w[8]);
#define PIXEL22_5   Interp5_24((dp+dpL+dpL+2), w[6], w[8]);
#define PIXEL22_C   u24cpy((dp+dpL+dpL+2), w[5]);

#define HQ3X_BITS 24
#define HQ3X_BYTES 3
#define HQ3X_TYPE uint24_t
#define HQ3X_CPY(to, from) u24cpy(&(to), (from))

#define HQ3X_FUNC hq3x_24
#define HQ3X_RB_FUNC hq3x_24_rb

#define RGB_TO_YUV_FUNC rgb24_to_yuv
#define DIFF_FUNC Diff24

#include "hq3x-int.h"
