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

#define PIXEL00_0     u24cpy(dp, w[5]);
#define PIXEL00_10    Interp1_24(dp, w[5], w[1]);
#define PIXEL00_11    Interp1_24(dp, w[5], w[4]);
#define PIXEL00_12    Interp1_24(dp, w[5], w[2]);
#define PIXEL00_20    Interp2_24(dp, w[5], w[4], w[2]);
#define PIXEL00_21    Interp2_24(dp, w[5], w[1], w[2]);
#define PIXEL00_22    Interp2_24(dp, w[5], w[1], w[4]);
#define PIXEL00_60    Interp6_24(dp, w[5], w[2], w[4]);
#define PIXEL00_61    Interp6_24(dp, w[5], w[4], w[2]);
#define PIXEL00_70    Interp7_24(dp, w[5], w[4], w[2]);
#define PIXEL00_90    Interp9_24(dp, w[5], w[4], w[2]);
#define PIXEL00_100   Interp10_24(dp, w[5], w[4], w[2]);
#define PIXEL01_0     u24cpy((dp+1), w[5]);
#define PIXEL01_10    Interp1_24((dp+1), w[5], w[3]);
#define PIXEL01_11    Interp1_24((dp+1), w[5], w[2]);
#define PIXEL01_12    Interp1_24((dp+1), w[5], w[6]);
#define PIXEL01_20    Interp2_24((dp+1), w[5], w[2], w[6]);
#define PIXEL01_21    Interp2_24((dp+1), w[5], w[3], w[6]);
#define PIXEL01_22    Interp2_24((dp+1), w[5], w[3], w[2]);
#define PIXEL01_60    Interp6_24((dp+1), w[5], w[6], w[2]);
#define PIXEL01_61    Interp6_24((dp+1), w[5], w[2], w[6]);
#define PIXEL01_70    Interp7_24((dp+1), w[5], w[2], w[6]);
#define PIXEL01_90    Interp9_24((dp+1), w[5], w[2], w[6]);
#define PIXEL01_100   Interp10_24((dp+1), w[5], w[2], w[6]);
#define PIXEL10_0     u24cpy((dp+dpL), w[5]);
#define PIXEL10_10    Interp1_24((dp+dpL), w[5], w[7]);
#define PIXEL10_11    Interp1_24((dp+dpL), w[5], w[8]);
#define PIXEL10_12    Interp1_24((dp+dpL), w[5], w[4]);
#define PIXEL10_20    Interp2_24((dp+dpL), w[5], w[8], w[4]);
#define PIXEL10_21    Interp2_24((dp+dpL), w[5], w[7], w[4]);
#define PIXEL10_22    Interp2_24((dp+dpL), w[5], w[7], w[8]);
#define PIXEL10_60    Interp6_24((dp+dpL), w[5], w[4], w[8]);
#define PIXEL10_61    Interp6_24((dp+dpL), w[5], w[8], w[4]);
#define PIXEL10_70    Interp7_24((dp+dpL), w[5], w[8], w[4]);
#define PIXEL10_90    Interp9_24((dp+dpL), w[5], w[8], w[4]);
#define PIXEL10_100   Interp10_24((dp+dpL), w[5], w[8], w[4]);
#define PIXEL11_0     u24cpy((dp+dpL+1), w[5]);
#define PIXEL11_10    Interp1_24((dp+dpL+1), w[5], w[9]);
#define PIXEL11_11    Interp1_24((dp+dpL+1), w[5], w[6]);
#define PIXEL11_12    Interp1_24((dp+dpL+1), w[5], w[8]);
#define PIXEL11_20    Interp2_24((dp+dpL+1), w[5], w[6], w[8]);
#define PIXEL11_21    Interp2_24((dp+dpL+1), w[5], w[9], w[8]);
#define PIXEL11_22    Interp2_24((dp+dpL+1), w[5], w[9], w[6]);
#define PIXEL11_60    Interp6_24((dp+dpL+1), w[5], w[8], w[6]);
#define PIXEL11_61    Interp6_24((dp+dpL+1), w[5], w[6], w[8]);
#define PIXEL11_70    Interp7_24((dp+dpL+1), w[5], w[6], w[8]);
#define PIXEL11_90    Interp9_24((dp+dpL+1), w[5], w[6], w[8]);
#define PIXEL11_100   Interp10_24((dp+dpL+1), w[5], w[6], w[8]);

#define HQ2X_BITS 24
#define HQ2X_BYTES 3
#define HQ2X_TYPE uint24_t
#define HQ2X_CPY(to, from) u24cpy(&(to), (from))

#define HQ2X_FUNC hq2x_24
#define HQ2X_RB_FUNC hq2x_24_rb

#define RGB_TO_YUV_FUNC rgb24_to_yuv
#define DIFF_FUNC Diff24

#include "hq2x-int.h"
