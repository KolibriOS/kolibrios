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

#define PIXEL00_0     *dp = w[5];
#define PIXEL00_10    *dp = Interp1_16(w[5], w[1]);
#define PIXEL00_11    *dp = Interp1_16(w[5], w[4]);
#define PIXEL00_12    *dp = Interp1_16(w[5], w[2]);
#define PIXEL00_20    *dp = Interp2_16(w[5], w[4], w[2]);
#define PIXEL00_21    *dp = Interp2_16(w[5], w[1], w[2]);
#define PIXEL00_22    *dp = Interp2_16(w[5], w[1], w[4]);
#define PIXEL00_60    *dp = Interp6_16(w[5], w[2], w[4]);
#define PIXEL00_61    *dp = Interp6_16(w[5], w[4], w[2]);
#define PIXEL00_70    *dp = Interp7_16(w[5], w[4], w[2]);
#define PIXEL00_90    *dp = Interp9_16(w[5], w[4], w[2]);
#define PIXEL00_100   *dp = Interp10_16(w[5], w[4], w[2]);
#define PIXEL01_0     *(dp+1) = w[5];
#define PIXEL01_10    *(dp+1) = Interp1_16(w[5], w[3]);
#define PIXEL01_11    *(dp+1) = Interp1_16(w[5], w[2]);
#define PIXEL01_12    *(dp+1) = Interp1_16(w[5], w[6]);
#define PIXEL01_20    *(dp+1) = Interp2_16(w[5], w[2], w[6]);
#define PIXEL01_21    *(dp+1) = Interp2_16(w[5], w[3], w[6]);
#define PIXEL01_22    *(dp+1) = Interp2_16(w[5], w[3], w[2]);
#define PIXEL01_60    *(dp+1) = Interp6_16(w[5], w[6], w[2]);
#define PIXEL01_61    *(dp+1) = Interp6_16(w[5], w[2], w[6]);
#define PIXEL01_70    *(dp+1) = Interp7_16(w[5], w[2], w[6]);
#define PIXEL01_90    *(dp+1) = Interp9_16(w[5], w[2], w[6]);
#define PIXEL01_100   *(dp+1) = Interp10_16(w[5], w[2], w[6]);
#define PIXEL10_0     *(dp+dpL) = w[5];
#define PIXEL10_10    *(dp+dpL) = Interp1_16(w[5], w[7]);
#define PIXEL10_11    *(dp+dpL) = Interp1_16(w[5], w[8]);
#define PIXEL10_12    *(dp+dpL) = Interp1_16(w[5], w[4]);
#define PIXEL10_20    *(dp+dpL) = Interp2_16(w[5], w[8], w[4]);
#define PIXEL10_21    *(dp+dpL) = Interp2_16(w[5], w[7], w[4]);
#define PIXEL10_22    *(dp+dpL) = Interp2_16(w[5], w[7], w[8]);
#define PIXEL10_60    *(dp+dpL) = Interp6_16(w[5], w[4], w[8]);
#define PIXEL10_61    *(dp+dpL) = Interp6_16(w[5], w[8], w[4]);
#define PIXEL10_70    *(dp+dpL) = Interp7_16(w[5], w[8], w[4]);
#define PIXEL10_90    *(dp+dpL) = Interp9_16(w[5], w[8], w[4]);
#define PIXEL10_100   *(dp+dpL) = Interp10_16(w[5], w[8], w[4]);
#define PIXEL11_0     *(dp+dpL+1) = w[5];
#define PIXEL11_10    *(dp+dpL+1) = Interp1_16(w[5], w[9]);
#define PIXEL11_11    *(dp+dpL+1) = Interp1_16(w[5], w[6]);
#define PIXEL11_12    *(dp+dpL+1) = Interp1_16(w[5], w[8]);
#define PIXEL11_20    *(dp+dpL+1) = Interp2_16(w[5], w[6], w[8]);
#define PIXEL11_21    *(dp+dpL+1) = Interp2_16(w[5], w[9], w[8]);
#define PIXEL11_22    *(dp+dpL+1) = Interp2_16(w[5], w[9], w[6]);
#define PIXEL11_60    *(dp+dpL+1) = Interp6_16(w[5], w[8], w[6]);
#define PIXEL11_61    *(dp+dpL+1) = Interp6_16(w[5], w[6], w[8]);
#define PIXEL11_70    *(dp+dpL+1) = Interp7_16(w[5], w[6], w[8]);
#define PIXEL11_90    *(dp+dpL+1) = Interp9_16(w[5], w[6], w[8]);
#define PIXEL11_100   *(dp+dpL+1) = Interp10_16(w[5], w[6], w[8]);

#define HQ2X_BITS 16
#define HQ2X_BYTES 2
#define HQ2X_TYPE uint16_t
#define HQ2X_CPY(to, from) (to) = (from)

#define HQ2X_FUNC hq2x_16
#define HQ2X_RB_FUNC hq2x_16_rb

#define RGB_TO_YUV_FUNC rgb16_to_yuv
#define DIFF_FUNC Diff16

#include "hq2x-int.h"
