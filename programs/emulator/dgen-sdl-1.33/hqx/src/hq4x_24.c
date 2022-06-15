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
#define PIXEL00_11    Interp1_24(dp, w[5], w[4]);
#define PIXEL00_12    Interp1_24(dp, w[5], w[2]);
#define PIXEL00_20    Interp2_24(dp, w[5], w[2], w[4]);
#define PIXEL00_50    Interp5_24(dp, w[2], w[4]);
#define PIXEL00_80    Interp8_24(dp, w[5], w[1]);
#define PIXEL00_81    Interp8_24(dp, w[5], w[4]);
#define PIXEL00_82    Interp8_24(dp, w[5], w[2]);
#define PIXEL01_0     u24cpy((dp+1), w[5]);
#define PIXEL01_10    Interp1_24((dp+1), w[5], w[1]);
#define PIXEL01_12    Interp1_24((dp+1), w[5], w[2]);
#define PIXEL01_14    Interp1_24((dp+1), w[2], w[5]);
#define PIXEL01_21    Interp2_24((dp+1), w[2], w[5], w[4]);
#define PIXEL01_31    Interp3_24((dp+1), w[5], w[4]);
#define PIXEL01_50    Interp5_24((dp+1), w[2], w[5]);
#define PIXEL01_60    Interp6_24((dp+1), w[5], w[2], w[4]);
#define PIXEL01_61    Interp6_24((dp+1), w[5], w[2], w[1]);
#define PIXEL01_82    Interp8_24((dp+1), w[5], w[2]);
#define PIXEL01_83    Interp8_24((dp+1), w[2], w[4]);
#define PIXEL02_0     u24cpy((dp+2), w[5]);
#define PIXEL02_10    Interp1_24((dp+2), w[5], w[3]);
#define PIXEL02_11    Interp1_24((dp+2), w[5], w[2]);
#define PIXEL02_13    Interp1_24((dp+2), w[2], w[5]);
#define PIXEL02_21    Interp2_24((dp+2), w[2], w[5], w[6]);
#define PIXEL02_32    Interp3_24((dp+2), w[5], w[6]);
#define PIXEL02_50    Interp5_24((dp+2), w[2], w[5]);
#define PIXEL02_60    Interp6_24((dp+2), w[5], w[2], w[6]);
#define PIXEL02_61    Interp6_24((dp+2), w[5], w[2], w[3]);
#define PIXEL02_81    Interp8_24((dp+2), w[5], w[2]);
#define PIXEL02_83    Interp8_24((dp+2), w[2], w[6]);
#define PIXEL03_0     u24cpy((dp+3), w[5]);
#define PIXEL03_11    Interp1_24((dp+3), w[5], w[2]);
#define PIXEL03_12    Interp1_24((dp+3), w[5], w[6]);
#define PIXEL03_20    Interp2_24((dp+3), w[5], w[2], w[6]);
#define PIXEL03_50    Interp5_24((dp+3), w[2], w[6]);
#define PIXEL03_80    Interp8_24((dp+3), w[5], w[3]);
#define PIXEL03_81    Interp8_24((dp+3), w[5], w[2]);
#define PIXEL03_82    Interp8_24((dp+3), w[5], w[6]);
#define PIXEL10_0     u24cpy((dp+dpL), w[5]);
#define PIXEL10_10    Interp1_24((dp+dpL), w[5], w[1]);
#define PIXEL10_11    Interp1_24((dp+dpL), w[5], w[4]);
#define PIXEL10_13    Interp1_24((dp+dpL), w[4], w[5]);
#define PIXEL10_21    Interp2_24((dp+dpL), w[4], w[5], w[2]);
#define PIXEL10_32    Interp3_24((dp+dpL), w[5], w[2]);
#define PIXEL10_50    Interp5_24((dp+dpL), w[4], w[5]);
#define PIXEL10_60    Interp6_24((dp+dpL), w[5], w[4], w[2]);
#define PIXEL10_61    Interp6_24((dp+dpL), w[5], w[4], w[1]);
#define PIXEL10_81    Interp8_24((dp+dpL), w[5], w[4]);
#define PIXEL10_83    Interp8_24((dp+dpL), w[4], w[2]);
#define PIXEL11_0     u24cpy((dp+dpL+1), w[5]);
#define PIXEL11_30    Interp3_24((dp+dpL+1), w[5], w[1]);
#define PIXEL11_31    Interp3_24((dp+dpL+1), w[5], w[4]);
#define PIXEL11_32    Interp3_24((dp+dpL+1), w[5], w[2]);
#define PIXEL11_70    Interp7_24((dp+dpL+1), w[5], w[4], w[2]);
#define PIXEL12_0     u24cpy((dp+dpL+2), w[5]);
#define PIXEL12_30    Interp3_24((dp+dpL+2), w[5], w[3]);
#define PIXEL12_31    Interp3_24((dp+dpL+2), w[5], w[2]);
#define PIXEL12_32    Interp3_24((dp+dpL+2), w[5], w[6]);
#define PIXEL12_70    Interp7_24((dp+dpL+2), w[5], w[6], w[2]);
#define PIXEL13_0     u24cpy((dp+dpL+3), w[5]);
#define PIXEL13_10    Interp1_24((dp+dpL+3), w[5], w[3]);
#define PIXEL13_12    Interp1_24((dp+dpL+3), w[5], w[6]);
#define PIXEL13_14    Interp1_24((dp+dpL+3), w[6], w[5]);
#define PIXEL13_21    Interp2_24((dp+dpL+3), w[6], w[5], w[2]);
#define PIXEL13_31    Interp3_24((dp+dpL+3), w[5], w[2]);
#define PIXEL13_50    Interp5_24((dp+dpL+3), w[6], w[5]);
#define PIXEL13_60    Interp6_24((dp+dpL+3), w[5], w[6], w[2]);
#define PIXEL13_61    Interp6_24((dp+dpL+3), w[5], w[6], w[3]);
#define PIXEL13_82    Interp8_24((dp+dpL+3), w[5], w[6]);
#define PIXEL13_83    Interp8_24((dp+dpL+3), w[6], w[2]);
#define PIXEL20_0     u24cpy((dp+dpL+dpL), w[5]);
#define PIXEL20_10    Interp1_24((dp+dpL+dpL), w[5], w[7]);
#define PIXEL20_12    Interp1_24((dp+dpL+dpL), w[5], w[4]);
#define PIXEL20_14    Interp1_24((dp+dpL+dpL), w[4], w[5]);
#define PIXEL20_21    Interp2_24((dp+dpL+dpL), w[4], w[5], w[8]);
#define PIXEL20_31    Interp3_24((dp+dpL+dpL), w[5], w[8]);
#define PIXEL20_50    Interp5_24((dp+dpL+dpL), w[4], w[5]);
#define PIXEL20_60    Interp6_24((dp+dpL+dpL), w[5], w[4], w[8]);
#define PIXEL20_61    Interp6_24((dp+dpL+dpL), w[5], w[4], w[7]);
#define PIXEL20_82    Interp8_24((dp+dpL+dpL), w[5], w[4]);
#define PIXEL20_83    Interp8_24((dp+dpL+dpL), w[4], w[8]);
#define PIXEL21_0     u24cpy((dp+dpL+dpL+1), w[5]);
#define PIXEL21_30    Interp3_24((dp+dpL+dpL+1), w[5], w[7]);
#define PIXEL21_31    Interp3_24((dp+dpL+dpL+1), w[5], w[8]);
#define PIXEL21_32    Interp3_24((dp+dpL+dpL+1), w[5], w[4]);
#define PIXEL21_70    Interp7_24((dp+dpL+dpL+1), w[5], w[4], w[8]);
#define PIXEL22_0     u24cpy((dp+dpL+dpL+2), w[5]);
#define PIXEL22_30    Interp3_24((dp+dpL+dpL+2), w[5], w[9]);
#define PIXEL22_31    Interp3_24((dp+dpL+dpL+2), w[5], w[6]);
#define PIXEL22_32    Interp3_24((dp+dpL+dpL+2), w[5], w[8]);
#define PIXEL22_70    Interp7_24((dp+dpL+dpL+2), w[5], w[6], w[8]);
#define PIXEL23_0     u24cpy((dp+dpL+dpL+3), w[5]);
#define PIXEL23_10    Interp1_24((dp+dpL+dpL+3), w[5], w[9]);
#define PIXEL23_11    Interp1_24((dp+dpL+dpL+3), w[5], w[6]);
#define PIXEL23_13    Interp1_24((dp+dpL+dpL+3), w[6], w[5]);
#define PIXEL23_21    Interp2_24((dp+dpL+dpL+3), w[6], w[5], w[8]);
#define PIXEL23_32    Interp3_24((dp+dpL+dpL+3), w[5], w[8]);
#define PIXEL23_50    Interp5_24((dp+dpL+dpL+3), w[6], w[5]);
#define PIXEL23_60    Interp6_24((dp+dpL+dpL+3), w[5], w[6], w[8]);
#define PIXEL23_61    Interp6_24((dp+dpL+dpL+3), w[5], w[6], w[9]);
#define PIXEL23_81    Interp8_24((dp+dpL+dpL+3), w[5], w[6]);
#define PIXEL23_83    Interp8_24((dp+dpL+dpL+3), w[6], w[8]);
#define PIXEL30_0     u24cpy((dp+dpL+dpL+dpL), w[5]);
#define PIXEL30_11    Interp1_24((dp+dpL+dpL+dpL), w[5], w[8]);
#define PIXEL30_12    Interp1_24((dp+dpL+dpL+dpL), w[5], w[4]);
#define PIXEL30_20    Interp2_24((dp+dpL+dpL+dpL), w[5], w[8], w[4]);
#define PIXEL30_50    Interp5_24((dp+dpL+dpL+dpL), w[8], w[4]);
#define PIXEL30_80    Interp8_24((dp+dpL+dpL+dpL), w[5], w[7]);
#define PIXEL30_81    Interp8_24((dp+dpL+dpL+dpL), w[5], w[8]);
#define PIXEL30_82    Interp8_24((dp+dpL+dpL+dpL), w[5], w[4]);
#define PIXEL31_0     u24cpy((dp+dpL+dpL+dpL+1), w[5]);
#define PIXEL31_10    Interp1_24((dp+dpL+dpL+dpL+1), w[5], w[7]);
#define PIXEL31_11    Interp1_24((dp+dpL+dpL+dpL+1), w[5], w[8]);
#define PIXEL31_13    Interp1_24((dp+dpL+dpL+dpL+1), w[8], w[5]);
#define PIXEL31_21    Interp2_24((dp+dpL+dpL+dpL+1), w[8], w[5], w[4]);
#define PIXEL31_32    Interp3_24((dp+dpL+dpL+dpL+1), w[5], w[4]);
#define PIXEL31_50    Interp5_24((dp+dpL+dpL+dpL+1), w[8], w[5]);
#define PIXEL31_60    Interp6_24((dp+dpL+dpL+dpL+1), w[5], w[8], w[4]);
#define PIXEL31_61    Interp6_24((dp+dpL+dpL+dpL+1), w[5], w[8], w[7]);
#define PIXEL31_81    Interp8_24((dp+dpL+dpL+dpL+1), w[5], w[8]);
#define PIXEL31_83    Interp8_24((dp+dpL+dpL+dpL+1), w[8], w[4]);
#define PIXEL32_0     u24cpy((dp+dpL+dpL+dpL+2), w[5]);
#define PIXEL32_10    Interp1_24((dp+dpL+dpL+dpL+2), w[5], w[9]);
#define PIXEL32_12    Interp1_24((dp+dpL+dpL+dpL+2), w[5], w[8]);
#define PIXEL32_14    Interp1_24((dp+dpL+dpL+dpL+2), w[8], w[5]);
#define PIXEL32_21    Interp2_24((dp+dpL+dpL+dpL+2), w[8], w[5], w[6]);
#define PIXEL32_31    Interp3_24((dp+dpL+dpL+dpL+2), w[5], w[6]);
#define PIXEL32_50    Interp5_24((dp+dpL+dpL+dpL+2), w[8], w[5]);
#define PIXEL32_60    Interp6_24((dp+dpL+dpL+dpL+2), w[5], w[8], w[6]);
#define PIXEL32_61    Interp6_24((dp+dpL+dpL+dpL+2), w[5], w[8], w[9]);
#define PIXEL32_82    Interp8_24((dp+dpL+dpL+dpL+2), w[5], w[8]);
#define PIXEL32_83    Interp8_24((dp+dpL+dpL+dpL+2), w[8], w[6]);
#define PIXEL33_0     u24cpy((dp+dpL+dpL+dpL+3), w[5]);
#define PIXEL33_11    Interp1_24((dp+dpL+dpL+dpL+3), w[5], w[6]);
#define PIXEL33_12    Interp1_24((dp+dpL+dpL+dpL+3), w[5], w[8]);
#define PIXEL33_20    Interp2_24((dp+dpL+dpL+dpL+3), w[5], w[8], w[6]);
#define PIXEL33_50    Interp5_24((dp+dpL+dpL+dpL+3), w[8], w[6]);
#define PIXEL33_80    Interp8_24((dp+dpL+dpL+dpL+3), w[5], w[9]);
#define PIXEL33_81    Interp8_24((dp+dpL+dpL+dpL+3), w[5], w[6]);
#define PIXEL33_82    Interp8_24((dp+dpL+dpL+dpL+3), w[5], w[8]);

#define HQ4X_BITS 24
#define HQ4X_BYTES 3
#define HQ4X_TYPE uint24_t
#define HQ4X_CPY(to, from) u24cpy(&(to), (from))

#define HQ4X_FUNC hq4x_24
#define HQ4X_RB_FUNC hq4x_24_rb

#define RGB_TO_YUV_FUNC rgb24_to_yuv
#define DIFF_FUNC Diff24

#include "hq4x-int.h"
