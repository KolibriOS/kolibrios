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
#define PIXEL00_11    *dp = Interp1_16(w[5], w[4]);
#define PIXEL00_12    *dp = Interp1_16(w[5], w[2]);
#define PIXEL00_20    *dp = Interp2_16(w[5], w[2], w[4]);
#define PIXEL00_50    *dp = Interp5_16(w[2], w[4]);
#define PIXEL00_80    *dp = Interp8_16(w[5], w[1]);
#define PIXEL00_81    *dp = Interp8_16(w[5], w[4]);
#define PIXEL00_82    *dp = Interp8_16(w[5], w[2]);
#define PIXEL01_0     *(dp+1) = w[5];
#define PIXEL01_10    *(dp+1) = Interp1_16(w[5], w[1]);
#define PIXEL01_12    *(dp+1) = Interp1_16(w[5], w[2]);
#define PIXEL01_14    *(dp+1) = Interp1_16(w[2], w[5]);
#define PIXEL01_21    *(dp+1) = Interp2_16(w[2], w[5], w[4]);
#define PIXEL01_31    *(dp+1) = Interp3_16(w[5], w[4]);
#define PIXEL01_50    *(dp+1) = Interp5_16(w[2], w[5]);
#define PIXEL01_60    *(dp+1) = Interp6_16(w[5], w[2], w[4]);
#define PIXEL01_61    *(dp+1) = Interp6_16(w[5], w[2], w[1]);
#define PIXEL01_82    *(dp+1) = Interp8_16(w[5], w[2]);
#define PIXEL01_83    *(dp+1) = Interp8_16(w[2], w[4]);
#define PIXEL02_0     *(dp+2) = w[5];
#define PIXEL02_10    *(dp+2) = Interp1_16(w[5], w[3]);
#define PIXEL02_11    *(dp+2) = Interp1_16(w[5], w[2]);
#define PIXEL02_13    *(dp+2) = Interp1_16(w[2], w[5]);
#define PIXEL02_21    *(dp+2) = Interp2_16(w[2], w[5], w[6]);
#define PIXEL02_32    *(dp+2) = Interp3_16(w[5], w[6]);
#define PIXEL02_50    *(dp+2) = Interp5_16(w[2], w[5]);
#define PIXEL02_60    *(dp+2) = Interp6_16(w[5], w[2], w[6]);
#define PIXEL02_61    *(dp+2) = Interp6_16(w[5], w[2], w[3]);
#define PIXEL02_81    *(dp+2) = Interp8_16(w[5], w[2]);
#define PIXEL02_83    *(dp+2) = Interp8_16(w[2], w[6]);
#define PIXEL03_0     *(dp+3) = w[5];
#define PIXEL03_11    *(dp+3) = Interp1_16(w[5], w[2]);
#define PIXEL03_12    *(dp+3) = Interp1_16(w[5], w[6]);
#define PIXEL03_20    *(dp+3) = Interp2_16(w[5], w[2], w[6]);
#define PIXEL03_50    *(dp+3) = Interp5_16(w[2], w[6]);
#define PIXEL03_80    *(dp+3) = Interp8_16(w[5], w[3]);
#define PIXEL03_81    *(dp+3) = Interp8_16(w[5], w[2]);
#define PIXEL03_82    *(dp+3) = Interp8_16(w[5], w[6]);
#define PIXEL10_0     *(dp+dpL) = w[5];
#define PIXEL10_10    *(dp+dpL) = Interp1_16(w[5], w[1]);
#define PIXEL10_11    *(dp+dpL) = Interp1_16(w[5], w[4]);
#define PIXEL10_13    *(dp+dpL) = Interp1_16(w[4], w[5]);
#define PIXEL10_21    *(dp+dpL) = Interp2_16(w[4], w[5], w[2]);
#define PIXEL10_32    *(dp+dpL) = Interp3_16(w[5], w[2]);
#define PIXEL10_50    *(dp+dpL) = Interp5_16(w[4], w[5]);
#define PIXEL10_60    *(dp+dpL) = Interp6_16(w[5], w[4], w[2]);
#define PIXEL10_61    *(dp+dpL) = Interp6_16(w[5], w[4], w[1]);
#define PIXEL10_81    *(dp+dpL) = Interp8_16(w[5], w[4]);
#define PIXEL10_83    *(dp+dpL) = Interp8_16(w[4], w[2]);
#define PIXEL11_0     *(dp+dpL+1) = w[5];
#define PIXEL11_30    *(dp+dpL+1) = Interp3_16(w[5], w[1]);
#define PIXEL11_31    *(dp+dpL+1) = Interp3_16(w[5], w[4]);
#define PIXEL11_32    *(dp+dpL+1) = Interp3_16(w[5], w[2]);
#define PIXEL11_70    *(dp+dpL+1) = Interp7_16(w[5], w[4], w[2]);
#define PIXEL12_0     *(dp+dpL+2) = w[5];
#define PIXEL12_30    *(dp+dpL+2) = Interp3_16(w[5], w[3]);
#define PIXEL12_31    *(dp+dpL+2) = Interp3_16(w[5], w[2]);
#define PIXEL12_32    *(dp+dpL+2) = Interp3_16(w[5], w[6]);
#define PIXEL12_70    *(dp+dpL+2) = Interp7_16(w[5], w[6], w[2]);
#define PIXEL13_0     *(dp+dpL+3) = w[5];
#define PIXEL13_10    *(dp+dpL+3) = Interp1_16(w[5], w[3]);
#define PIXEL13_12    *(dp+dpL+3) = Interp1_16(w[5], w[6]);
#define PIXEL13_14    *(dp+dpL+3) = Interp1_16(w[6], w[5]);
#define PIXEL13_21    *(dp+dpL+3) = Interp2_16(w[6], w[5], w[2]);
#define PIXEL13_31    *(dp+dpL+3) = Interp3_16(w[5], w[2]);
#define PIXEL13_50    *(dp+dpL+3) = Interp5_16(w[6], w[5]);
#define PIXEL13_60    *(dp+dpL+3) = Interp6_16(w[5], w[6], w[2]);
#define PIXEL13_61    *(dp+dpL+3) = Interp6_16(w[5], w[6], w[3]);
#define PIXEL13_82    *(dp+dpL+3) = Interp8_16(w[5], w[6]);
#define PIXEL13_83    *(dp+dpL+3) = Interp8_16(w[6], w[2]);
#define PIXEL20_0     *(dp+dpL+dpL) = w[5];
#define PIXEL20_10    *(dp+dpL+dpL) = Interp1_16(w[5], w[7]);
#define PIXEL20_12    *(dp+dpL+dpL) = Interp1_16(w[5], w[4]);
#define PIXEL20_14    *(dp+dpL+dpL) = Interp1_16(w[4], w[5]);
#define PIXEL20_21    *(dp+dpL+dpL) = Interp2_16(w[4], w[5], w[8]);
#define PIXEL20_31    *(dp+dpL+dpL) = Interp3_16(w[5], w[8]);
#define PIXEL20_50    *(dp+dpL+dpL) = Interp5_16(w[4], w[5]);
#define PIXEL20_60    *(dp+dpL+dpL) = Interp6_16(w[5], w[4], w[8]);
#define PIXEL20_61    *(dp+dpL+dpL) = Interp6_16(w[5], w[4], w[7]);
#define PIXEL20_82    *(dp+dpL+dpL) = Interp8_16(w[5], w[4]);
#define PIXEL20_83    *(dp+dpL+dpL) = Interp8_16(w[4], w[8]);
#define PIXEL21_0     *(dp+dpL+dpL+1) = w[5];
#define PIXEL21_30    *(dp+dpL+dpL+1) = Interp3_16(w[5], w[7]);
#define PIXEL21_31    *(dp+dpL+dpL+1) = Interp3_16(w[5], w[8]);
#define PIXEL21_32    *(dp+dpL+dpL+1) = Interp3_16(w[5], w[4]);
#define PIXEL21_70    *(dp+dpL+dpL+1) = Interp7_16(w[5], w[4], w[8]);
#define PIXEL22_0     *(dp+dpL+dpL+2) = w[5];
#define PIXEL22_30    *(dp+dpL+dpL+2) = Interp3_16(w[5], w[9]);
#define PIXEL22_31    *(dp+dpL+dpL+2) = Interp3_16(w[5], w[6]);
#define PIXEL22_32    *(dp+dpL+dpL+2) = Interp3_16(w[5], w[8]);
#define PIXEL22_70    *(dp+dpL+dpL+2) = Interp7_16(w[5], w[6], w[8]);
#define PIXEL23_0     *(dp+dpL+dpL+3) = w[5];
#define PIXEL23_10    *(dp+dpL+dpL+3) = Interp1_16(w[5], w[9]);
#define PIXEL23_11    *(dp+dpL+dpL+3) = Interp1_16(w[5], w[6]);
#define PIXEL23_13    *(dp+dpL+dpL+3) = Interp1_16(w[6], w[5]);
#define PIXEL23_21    *(dp+dpL+dpL+3) = Interp2_16(w[6], w[5], w[8]);
#define PIXEL23_32    *(dp+dpL+dpL+3) = Interp3_16(w[5], w[8]);
#define PIXEL23_50    *(dp+dpL+dpL+3) = Interp5_16(w[6], w[5]);
#define PIXEL23_60    *(dp+dpL+dpL+3) = Interp6_16(w[5], w[6], w[8]);
#define PIXEL23_61    *(dp+dpL+dpL+3) = Interp6_16(w[5], w[6], w[9]);
#define PIXEL23_81    *(dp+dpL+dpL+3) = Interp8_16(w[5], w[6]);
#define PIXEL23_83    *(dp+dpL+dpL+3) = Interp8_16(w[6], w[8]);
#define PIXEL30_0     *(dp+dpL+dpL+dpL) = w[5];
#define PIXEL30_11    *(dp+dpL+dpL+dpL) = Interp1_16(w[5], w[8]);
#define PIXEL30_12    *(dp+dpL+dpL+dpL) = Interp1_16(w[5], w[4]);
#define PIXEL30_20    *(dp+dpL+dpL+dpL) = Interp2_16(w[5], w[8], w[4]);
#define PIXEL30_50    *(dp+dpL+dpL+dpL) = Interp5_16(w[8], w[4]);
#define PIXEL30_80    *(dp+dpL+dpL+dpL) = Interp8_16(w[5], w[7]);
#define PIXEL30_81    *(dp+dpL+dpL+dpL) = Interp8_16(w[5], w[8]);
#define PIXEL30_82    *(dp+dpL+dpL+dpL) = Interp8_16(w[5], w[4]);
#define PIXEL31_0     *(dp+dpL+dpL+dpL+1) = w[5];
#define PIXEL31_10    *(dp+dpL+dpL+dpL+1) = Interp1_16(w[5], w[7]);
#define PIXEL31_11    *(dp+dpL+dpL+dpL+1) = Interp1_16(w[5], w[8]);
#define PIXEL31_13    *(dp+dpL+dpL+dpL+1) = Interp1_16(w[8], w[5]);
#define PIXEL31_21    *(dp+dpL+dpL+dpL+1) = Interp2_16(w[8], w[5], w[4]);
#define PIXEL31_32    *(dp+dpL+dpL+dpL+1) = Interp3_16(w[5], w[4]);
#define PIXEL31_50    *(dp+dpL+dpL+dpL+1) = Interp5_16(w[8], w[5]);
#define PIXEL31_60    *(dp+dpL+dpL+dpL+1) = Interp6_16(w[5], w[8], w[4]);
#define PIXEL31_61    *(dp+dpL+dpL+dpL+1) = Interp6_16(w[5], w[8], w[7]);
#define PIXEL31_81    *(dp+dpL+dpL+dpL+1) = Interp8_16(w[5], w[8]);
#define PIXEL31_83    *(dp+dpL+dpL+dpL+1) = Interp8_16(w[8], w[4]);
#define PIXEL32_0     *(dp+dpL+dpL+dpL+2) = w[5];
#define PIXEL32_10    *(dp+dpL+dpL+dpL+2) = Interp1_16(w[5], w[9]);
#define PIXEL32_12    *(dp+dpL+dpL+dpL+2) = Interp1_16(w[5], w[8]);
#define PIXEL32_14    *(dp+dpL+dpL+dpL+2) = Interp1_16(w[8], w[5]);
#define PIXEL32_21    *(dp+dpL+dpL+dpL+2) = Interp2_16(w[8], w[5], w[6]);
#define PIXEL32_31    *(dp+dpL+dpL+dpL+2) = Interp3_16(w[5], w[6]);
#define PIXEL32_50    *(dp+dpL+dpL+dpL+2) = Interp5_16(w[8], w[5]);
#define PIXEL32_60    *(dp+dpL+dpL+dpL+2) = Interp6_16(w[5], w[8], w[6]);
#define PIXEL32_61    *(dp+dpL+dpL+dpL+2) = Interp6_16(w[5], w[8], w[9]);
#define PIXEL32_82    *(dp+dpL+dpL+dpL+2) = Interp8_16(w[5], w[8]);
#define PIXEL32_83    *(dp+dpL+dpL+dpL+2) = Interp8_16(w[8], w[6]);
#define PIXEL33_0     *(dp+dpL+dpL+dpL+3) = w[5];
#define PIXEL33_11    *(dp+dpL+dpL+dpL+3) = Interp1_16(w[5], w[6]);
#define PIXEL33_12    *(dp+dpL+dpL+dpL+3) = Interp1_16(w[5], w[8]);
#define PIXEL33_20    *(dp+dpL+dpL+dpL+3) = Interp2_16(w[5], w[8], w[6]);
#define PIXEL33_50    *(dp+dpL+dpL+dpL+3) = Interp5_16(w[8], w[6]);
#define PIXEL33_80    *(dp+dpL+dpL+dpL+3) = Interp8_16(w[5], w[9]);
#define PIXEL33_81    *(dp+dpL+dpL+dpL+3) = Interp8_16(w[5], w[6]);
#define PIXEL33_82    *(dp+dpL+dpL+dpL+3) = Interp8_16(w[5], w[8]);

#define HQ4X_BITS 16
#define HQ4X_BYTES 2
#define HQ4X_TYPE uint16_t
#define HQ4X_CPY(to, from) (to) = (from)

#define HQ4X_FUNC hq4x_16
#define HQ4X_RB_FUNC hq4x_16_rb

#define RGB_TO_YUV_FUNC rgb16_to_yuv
#define DIFF_FUNC Diff16

#include "hq4x-int.h"
