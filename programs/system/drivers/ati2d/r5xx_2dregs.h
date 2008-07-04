/*
 * Copyright 2000 ATI Technologies Inc., Markham, Ontario, and
 *                VA Linux Systems Inc., Fremont, California.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL ATI, VA LINUX SYSTEMS AND/OR
 * THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/* WARNING: the above is not a standard MIT license. */
/*
 * Authors:
 *   Kevin E. Martin <martin@xfree86.org>
 *   Rickard E. Faith <faith@valinux.com>
 *   Alan Hourihane <alanh@fairlite.demon.co.uk>
 */

#ifndef _R5XX_2DREGS_H
# define _R5XX_2DREGS_H

#define R5XX_DATATYPE_VQ	0
#define R5XX_DATATYPE_CI4	1
#define R5XX_DATATYPE_CI8	2
#define R5XX_DATATYPE_ARGB1555	3
#define R5XX_DATATYPE_RGB565	4
#define R5XX_DATATYPE_RGB888	5
#define R5XX_DATATYPE_ARGB8888	6
#define R5XX_DATATYPE_RGB332	7
#define R5XX_DATATYPE_Y8	8
#define R5XX_DATATYPE_RGB8	9
#define R5XX_DATATYPE_CI16	10
#define R5XX_DATATYPE_VYUY_422	11
#define R5XX_DATATYPE_YVYU_422	12
#define R5XX_DATATYPE_AYUV_444	14
#define R5XX_DATATYPE_ARGB4444	15

#define R5XX_RBBM_SOFT_RESET              0x00f0
#       define R5XX_SOFT_RESET_CP         (1 <<  0)
#       define R5XX_SOFT_RESET_HI         (1 <<  1)
#       define R5XX_SOFT_RESET_SE         (1 <<  2)
#       define R5XX_SOFT_RESET_RE         (1 <<  3)
#       define R5XX_SOFT_RESET_PP         (1 <<  4)
#       define R5XX_SOFT_RESET_E2         (1 <<  5)
#       define R5XX_SOFT_RESET_RB         (1 <<  6)
#       define R5XX_SOFT_RESET_HDP        (1 <<  7)

#define R5XX_HOST_PATH_CNTL               0x0130
#       define R5XX_HDP_SOFT_RESET        (1 << 26)
#       define R5XX_HDP_APER_CNTL         (1 << 23)

#define R5XX_SURFACE_CNTL                 0x0b00
#       define R5XX_SURF_TRANSLATION_DIS  (1 << 8)
#       define R5XX_NONSURF_AP0_SWP_16BPP (1 << 20)
#       define R5XX_NONSURF_AP0_SWP_32BPP (1 << 21)
#       define R5XX_NONSURF_AP1_SWP_16BPP (1 << 22)
#       define R5XX_NONSURF_AP1_SWP_32BPP (1 << 23)

#define R5XX_SURFACE0_INFO                0x0b0c
#       define R5XX_SURF_TILE_COLOR_MACRO (0 << 16)
#       define R5XX_SURF_TILE_COLOR_BOTH  (1 << 16)
#       define R5XX_SURF_TILE_DEPTH_32BPP (2 << 16)
#       define R5XX_SURF_TILE_DEPTH_16BPP (3 << 16)
#       define R5XX_SURF_AP0_SWP_16BPP    (1 << 20)
#       define R5XX_SURF_AP0_SWP_32BPP    (1 << 21)
#       define R5XX_SURF_AP1_SWP_16BPP    (1 << 22)
#       define R5XX_SURF_AP1_SWP_32BPP    (1 << 23)
#define R5XX_SURFACE0_LOWER_BOUND         0x0b04
#define R5XX_SURFACE0_UPPER_BOUND         0x0b08

#define R5XX_RBBM_STATUS                  0x0e40
#       define R5XX_RBBM_FIFOCNT_MASK     0x007f
#       define R5XX_RBBM_ACTIVE           (1 << 31)

#define R5XX_SRC_PITCH_OFFSET             0x1428
#define R5XX_DST_PITCH_OFFSET             0x142c

#define R5XX_SRC_Y_X                      0x1434
#define R5XX_DST_Y_X                      0x1438
#define R5XX_DST_HEIGHT_WIDTH             0x143c

#define R5XX_DP_GUI_MASTER_CNTL           0x146c
#       define R5XX_GMC_SRC_PITCH_OFFSET_CNTL   (1    <<  0)
#       define R5XX_GMC_DST_PITCH_OFFSET_CNTL   (1    <<  1)
#       define R5XX_GMC_SRC_CLIPPING            (1    <<  2)
#       define R5XX_GMC_DST_CLIPPING            (1    <<  3)
#       define R5XX_GMC_BRUSH_DATATYPE_MASK     (0x0f <<  4)
#       define R5XX_GMC_BRUSH_8X8_MONO_FG_BG    (0    <<  4)
#       define R5XX_GMC_BRUSH_8X8_MONO_FG_LA    (1    <<  4)
#       define R5XX_GMC_BRUSH_1X8_MONO_FG_BG    (4    <<  4)
#       define R5XX_GMC_BRUSH_1X8_MONO_FG_LA    (5    <<  4)
#       define R5XX_GMC_BRUSH_32x1_MONO_FG_BG   (6    <<  4)
#       define R5XX_GMC_BRUSH_32x1_MONO_FG_LA   (7    <<  4)
#       define R5XX_GMC_BRUSH_32x32_MONO_FG_BG  (8    <<  4)
#       define R5XX_GMC_BRUSH_32x32_MONO_FG_LA  (9    <<  4)
#       define R5XX_GMC_BRUSH_8x8_COLOR         (10   <<  4)
#       define R5XX_GMC_BRUSH_1X8_COLOR         (12   <<  4)
#       define R5XX_GMC_BRUSH_SOLID_COLOR       (13   <<  4)
#       define R5XX_GMC_BRUSH_NONE              (15   <<  4)
#       define R5XX_GMC_DST_8BPP_CI             (2    <<  8)
#       define R5XX_GMC_DST_15BPP               (3    <<  8)
#       define R5XX_GMC_DST_16BPP               (4    <<  8)
#       define R5XX_GMC_DST_24BPP               (5    <<  8)
#       define R5XX_GMC_DST_32BPP               (6    <<  8)
#       define R5XX_GMC_DST_8BPP_RGB            (7    <<  8)
#       define R5XX_GMC_DST_Y8                  (8    <<  8)
#       define R5XX_GMC_DST_RGB8                (9    <<  8)
#       define R5XX_GMC_DST_VYUY                (11   <<  8)
#       define R5XX_GMC_DST_YVYU                (12   <<  8)
#       define R5XX_GMC_DST_AYUV444             (14   <<  8)
#       define R5XX_GMC_DST_ARGB4444            (15   <<  8)
#       define R5XX_GMC_DST_DATATYPE_MASK       (0x0f <<  8)
#       define R5XX_GMC_DST_DATATYPE_SHIFT      8
#       define R5XX_GMC_SRC_DATATYPE_MASK       (3    << 12)
#       define R5XX_GMC_SRC_DATATYPE_MONO_FG_BG (0    << 12)
#       define R5XX_GMC_SRC_DATATYPE_MONO_FG_LA (1    << 12)
#       define R5XX_GMC_SRC_DATATYPE_COLOR      (3    << 12)
#       define R5XX_GMC_BYTE_PIX_ORDER          (1    << 14)
#       define R5XX_GMC_BYTE_MSB_TO_LSB         (0    << 14)
#       define R5XX_GMC_BYTE_LSB_TO_MSB         (1    << 14)
#       define R5XX_GMC_CONVERSION_TEMP         (1    << 15)
#       define R5XX_GMC_CONVERSION_TEMP_6500    (0    << 15)
#       define R5XX_GMC_CONVERSION_TEMP_9300    (1    << 15)
#       define R5XX_GMC_ROP3_MASK               (0xff << 16)
#       define R5XX_DP_SRC_SOURCE_MASK          (7    << 24)
#       define R5XX_DP_SRC_SOURCE_MEMORY        (2    << 24)
#       define R5XX_DP_SRC_SOURCE_HOST_DATA     (3    << 24)
#       define R5XX_GMC_3D_FCN_EN               (1    << 27)
#       define R5XX_GMC_CLR_CMP_CNTL_DIS        (1    << 28)
#       define R5XX_GMC_AUX_CLIP_DIS            (1    << 29)
#       define R5XX_GMC_WR_MSK_DIS              (1    << 30)
#       define R5XX_GMC_LD_BRUSH_Y_X            (1    << 31)
#       define R5XX_ROP3_ZERO             0x00000000
#       define R5XX_ROP3_DSa              0x00880000
#       define R5XX_ROP3_SDna             0x00440000
#       define R5XX_ROP3_S                0x00cc0000
#       define R5XX_ROP3_DSna             0x00220000
#       define R5XX_ROP3_D                0x00aa0000
#       define R5XX_ROP3_DSx              0x00660000
#       define R5XX_ROP3_DSo              0x00ee0000
#       define R5XX_ROP3_DSon             0x00110000
#       define R5XX_ROP3_DSxn             0x00990000
#       define R5XX_ROP3_Dn               0x00550000
#       define R5XX_ROP3_SDno             0x00dd0000
#       define R5XX_ROP3_Sn               0x00330000
#       define R5XX_ROP3_DSno             0x00bb0000
#       define R5XX_ROP3_DSan             0x00770000
#       define R5XX_ROP3_ONE              0x00ff0000
#       define R5XX_ROP3_DPa              0x00a00000
#       define R5XX_ROP3_PDna             0x00500000
#       define R5XX_ROP3_P                0x00f00000
#       define R5XX_ROP3_DPna             0x000a0000
#       define R5XX_ROP3_D                0x00aa0000
#       define R5XX_ROP3_DPx              0x005a0000
#       define R5XX_ROP3_DPo              0x00fa0000
#       define R5XX_ROP3_DPon             0x00050000
#       define R5XX_ROP3_PDxn             0x00a50000
#       define R5XX_ROP3_PDno             0x00f50000
#       define R5XX_ROP3_Pn               0x000f0000
#       define R5XX_ROP3_DPno             0x00af0000
#       define R5XX_ROP3_DPan             0x005f0000

#define R5XX_BRUSH_Y_X                    0x1474
#define R5XX_DP_BRUSH_BKGD_CLR            0x1478
#define R5XX_DP_BRUSH_FRGD_CLR            0x147c
#define R5XX_BRUSH_DATA0                  0x1480
#define R5XX_BRUSH_DATA1                  0x1484

#define R5XX_DST_WIDTH_HEIGHT             0x1598

#define R5XX_CLR_CMP_CNTL                 0x15c0
#       define R5XX_SRC_CMP_EQ_COLOR      (4 <<  0)
#       define R5XX_SRC_CMP_NEQ_COLOR     (5 <<  0)
#       define R5XX_CLR_CMP_SRC_SOURCE    (1 << 24)

#define R5XX_CLR_CMP_CLR_SRC              0x15c4

#define R5XX_CLR_CMP_MASK                 0x15cc
#       define R5XX_CLR_CMP_MSK           0xffffffff

#define R5XX_DP_SRC_BKGD_CLR              0x15dc
#define R5XX_DP_SRC_FRGD_CLR              0x15d8

#define R5XX_DST_LINE_START               0x1600
#define R5XX_DST_LINE_END                 0x1604
#define R5XX_DST_LINE_PATCOUNT            0x1608
#       define R5XX_BRES_CNTL_SHIFT       8

#define R5XX_DP_CNTL                      0x16c0
#       define R5XX_DST_X_LEFT_TO_RIGHT   (1 <<  0)
#       define R5XX_DST_Y_TOP_TO_BOTTOM   (1 <<  1)
#       define R5XX_DP_DST_TILE_LINEAR    (0 <<  3)
#       define R5XX_DP_DST_TILE_MACRO     (1 <<  3)
#       define R5XX_DP_DST_TILE_MICRO     (2 <<  3)
#       define R5XX_DP_DST_TILE_BOTH      (3 <<  3)

#define R5XX_DP_DATATYPE                  0x16c4
#       define R5XX_HOST_BIG_ENDIAN_EN    (1 << 29)

#define R5XX_DP_WRITE_MASK                0x16cc

#define R5XX_DEFAULT_SC_BOTTOM_RIGHT      0x16e8
#       define R5XX_DEFAULT_SC_RIGHT_MAX  (0x1fff <<  0)
#       define R5XX_DEFAULT_SC_BOTTOM_MAX (0x1fff << 16)

#define R5XX_SC_TOP_LEFT                  0x16ec
#define R5XX_SC_BOTTOM_RIGHT              0x16f0
#       define R5XX_SC_SIGN_MASK_LO       0x8000
#       define R5XX_SC_SIGN_MASK_HI       0x80000000

#define R5XX_RBBM_GUICNTL                 0x172c
#       define R5XX_HOST_DATA_SWAP_NONE   (0 << 0)
#       define R5XX_HOST_DATA_SWAP_16BIT  (1 << 0)
#       define R5XX_HOST_DATA_SWAP_32BIT  (2 << 0)
#       define R5XX_HOST_DATA_SWAP_HDW    (3 << 0)

#define R5XX_HOST_DATA0                   0x17c0
#define R5XX_HOST_DATA1                   0x17c4
#define R5XX_HOST_DATA2                   0x17c8
#define R5XX_HOST_DATA3                   0x17cc
#define R5XX_HOST_DATA4                   0x17d0
#define R5XX_HOST_DATA5                   0x17d4
#define R5XX_HOST_DATA6                   0x17d8
#define R5XX_HOST_DATA7                   0x17dc
#define R5XX_HOST_DATA_LAST               0x17e0

#define R5XX_RB3D_CNTL                    0x1c3c
#       define R5XX_ALPHA_BLEND_ENABLE       (1  <<  0)
#       define R5XX_PLANE_MASK_ENABLE        (1  <<  1)
#       define R5XX_DITHER_ENABLE            (1  <<  2)
#       define R5XX_ROUND_ENABLE             (1  <<  3)
#       define R5XX_SCALE_DITHER_ENABLE      (1  <<  4)
#       define R5XX_DITHER_INIT              (1  <<  5)
#       define R5XX_ROP_ENABLE               (1  <<  6)
#       define R5XX_STENCIL_ENABLE           (1  <<  7)
#       define R5XX_Z_ENABLE                 (1  <<  8)
#       define R5XX_DEPTH_XZ_OFFEST_ENABLE   (1  <<  9)
#       define R5XX_COLOR_FORMAT_ARGB1555    (3  << 10)
#       define R5XX_COLOR_FORMAT_RGB565      (4  << 10)
#       define R5XX_COLOR_FORMAT_ARGB8888    (6  << 10)
#       define R5XX_COLOR_FORMAT_RGB332      (7  << 10)
#       define R5XX_COLOR_FORMAT_Y8          (8  << 10)
#       define R5XX_COLOR_FORMAT_RGB8        (9  << 10)
#       define R5XX_COLOR_FORMAT_YUV422_VYUY (11 << 10)
#       define R5XX_COLOR_FORMAT_YUV422_YVYU (12 << 10)
#       define R5XX_COLOR_FORMAT_aYUV444     (14 << 10)
#       define R5XX_COLOR_FORMAT_ARGB4444    (15 << 10)
#       define R5XX_CLRCMP_FLIP_ENABLE       (1  << 14)

#define R5XX_RB3D_DSTCACHE_CTLSTAT        0x325C
#       define R5XX_RB3D_DC_FLUSH         (3 << 0)
#       define R5XX_RB3D_DC_FREE          (3 << 2)
#       define R5XX_RB3D_DC_FLUSH_ALL     0xf
#       define R5XX_RB3D_DC_BUSY          (1 << 31)

#define R5XX_RB3D_DSTCACHE_MODE           0x3258
# define R5XX_RB3D_DC_CACHE_ENABLE            (0)
# define R5XX_RB3D_DC_2D_CACHE_DISABLE        (1)
# define R5XX_RB3D_DC_3D_CACHE_DISABLE        (2)
# define R5XX_RB3D_DC_CACHE_DISABLE           (3)
# define R5XX_RB3D_DC_2D_CACHE_LINESIZE_128   (1 << 2)
# define R5XX_RB3D_DC_3D_CACHE_LINESIZE_128   (2 << 2)
# define R5XX_RB3D_DC_2D_CACHE_AUTOFLUSH      (1 << 8)
# define R5XX_RB3D_DC_3D_CACHE_AUTOFLUSH      (2 << 8)
# define R200_RB3D_DC_2D_CACHE_AUTOFREE       (1 << 10)
# define R200_RB3D_DC_3D_CACHE_AUTOFREE       (2 << 10)
# define R5XX_RB3D_DC_FORCE_RMW               (1 << 16)
# define R5XX_RB3D_DC_DISABLE_RI_FILL         (1 << 24)
# define R5XX_RB3D_DC_DISABLE_RI_READ         (1 << 25)

#endif /* _R5XX_2DREGS_H */
