/*
 * Copyright © 2011 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *   Li Xiaowei <xiaowei.a.li@intel.com>
 *   Li Zhong <zhong.li@intel.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "intel_batchbuffer.h"
#include "intel_driver.h"
#include "i965_defines.h"
#include "i965_structs.h"
#include "gen75_vpp_vebox.h"
#include "intel_media.h"

#define PI  3.1415926

extern VAStatus
i965_MapBuffer(VADriverContextP ctx, VABufferID buf_id, void **);

extern VAStatus
i965_UnmapBuffer(VADriverContextP ctx, VABufferID buf_id);

extern VAStatus
i965_DeriveImage(VADriverContextP ctx, VABufferID surface, VAImage *out_image);

extern VAStatus
i965_DestroyImage(VADriverContextP ctx, VAImageID image);

VAStatus
vpp_surface_convert(VADriverContextP ctx, struct object_surface *src_obj_surf,
    struct object_surface *dst_obj_surf)
{
    VAStatus va_status = VA_STATUS_SUCCESS;

    assert(src_obj_surf->orig_width  == dst_obj_surf->orig_width);
    assert(src_obj_surf->orig_height == dst_obj_surf->orig_height);

    VARectangle src_rect, dst_rect;
    src_rect.x = dst_rect.x = 0;
    src_rect.y = dst_rect.y = 0; 
    src_rect.width  = dst_rect.width  = src_obj_surf->orig_width; 
    src_rect.height = dst_rect.height = dst_obj_surf->orig_height;

    struct i965_surface src_surface, dst_surface;
    src_surface.base  = (struct object_base *)src_obj_surf;
    src_surface.type  = I965_SURFACE_TYPE_SURFACE;
    src_surface.flags = I965_SURFACE_FLAG_FRAME;

    dst_surface.base  = (struct object_base *)dst_obj_surf;
    dst_surface.type  = I965_SURFACE_TYPE_SURFACE;
    dst_surface.flags = I965_SURFACE_FLAG_FRAME;

    va_status = i965_image_processing(ctx,
                                     &src_surface,
                                     &src_rect,
                                     &dst_surface,
                                     &dst_rect);
    return va_status;
}

static VAStatus
vpp_surface_scaling(VADriverContextP ctx, struct object_surface *src_obj_surf,
    struct object_surface *dst_obj_surf, uint32_t flags)
{
    VAStatus va_status = VA_STATUS_SUCCESS;

    assert(src_obj_surf->fourcc == VA_FOURCC_NV12);
    assert(dst_obj_surf->fourcc == VA_FOURCC_NV12);

    VARectangle src_rect, dst_rect;
    src_rect.x = 0;
    src_rect.y = 0; 
    src_rect.width  = src_obj_surf->orig_width; 
    src_rect.height = src_obj_surf->orig_height;

    dst_rect.x = 0;
    dst_rect.y = 0; 
    dst_rect.width  = dst_obj_surf->orig_width; 
    dst_rect.height = dst_obj_surf->orig_height;

    va_status = i965_scaling_processing(ctx,
                                       src_obj_surf,
                                       &src_rect,
                                       dst_obj_surf,
                                       &dst_rect,
                                       flags);
     
    return va_status;
}

void hsw_veb_dndi_table(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    unsigned int* p_table ;
    unsigned int progressive_dn = 1;
    unsigned int dndi_top_first = 0;
    unsigned int is_mcdi_enabled = 0;

    if (proc_ctx->is_di_enabled) {
        const VAProcFilterParameterBufferDeinterlacing * const deint_params =
            proc_ctx->filter_di;

        progressive_dn = 0;

        /* If we are in "First Frame" mode, i.e. past frames are not
           available for motion measure, then don't use the TFF flag */
        dndi_top_first = !(deint_params->flags & (proc_ctx->is_first_frame ?
                VA_DEINTERLACING_BOTTOM_FIELD :
                VA_DEINTERLACING_BOTTOM_FIELD_FIRST));

        is_mcdi_enabled =
            (deint_params->algorithm == VAProcDeinterlacingMotionCompensated);
    }

    /*
    VAProcFilterParameterBufferDeinterlacing *di_param =
            (VAProcFilterParameterBufferDeinterlacing *) proc_ctx->filter_di;

    VAProcFilterParameterBuffer * dn_param =
            (VAProcFilterParameterBuffer *) proc_ctx->filter_dn;
    */
    p_table = (unsigned int *)proc_ctx->dndi_state_table.ptr;

     if (IS_HASWELL(i965->intel.device_info))
         *p_table ++ = 0;               // reserved  . w0

    *p_table ++ = ( 140 << 24 |    // denoise STAD threshold . w1
                    192 << 16 |    // dnmh_history_max
                    0   << 12 |    // reserved
                    7   << 8  |    // dnmh_delta[3:0]
                    38 );          // denoise ASD threshold

    *p_table ++ = ( 0  << 30 |    // reserved . w2
                    0  << 24 |    // temporal diff th
                    0  << 22 |    // reserved.
                    0  << 16 |    // low temporal diff th
                    2  << 13 |    // STMM C2
                    1  << 8  |    // denoise moving pixel th
                    38 );         // denoise th for sum of complexity measure

    *p_table ++ = ( 0 << 30  |   // reserved . w3
                    12<< 24  |   // good neighbor th[5:0]
                    9 << 20  |   // CAT slope minus 1
                    5 << 16  |   // SAD Tight in
                    0 << 14  |   // smooth mv th
                    0 << 12  |   // reserved
                    1 << 8   |   // bne_edge_th[3:0]
                    20 );        // block noise estimate noise th

    *p_table ++ = ( 0  << 31  |  // STMM blending constant select. w4
                    64 << 24  |  // STMM trc1
                    125<< 16  |  // STMM trc2
                    0  << 14  |  // reserved
                    30 << 8   |  // VECM_mul
                    150 );       // maximum STMM

    *p_table ++ = ( 118<< 24  |  // minumum STMM  . W5
                    0  << 22  |  // STMM shift down
                    1  << 20  |  // STMM shift up
                    5  << 16  |  // STMM output shift
                    100 << 8  |  // SDI threshold
                    5 );         // SDI delta

    *p_table ++ = ( 50  << 24 |  // SDI fallback mode 1 T1 constant . W6
                    100 << 16 |  // SDI fallback mode 1 T2 constant
                    37  << 8  |  // SDI fallback mode 2 constant(angle2x1)
                    175 );       // FMD temporal difference threshold

    *p_table ++ = ( 16 << 24  |  // FMD #1 vertical difference th . w7
                    100<< 16  |  // FMD #2 vertical difference th
                    0  << 14  |  // CAT th1
                    2  << 8   |  // FMD tear threshold
                    is_mcdi_enabled  << 7  |  // MCDI Enable, use motion compensated deinterlace algorithm
                    progressive_dn  << 6   |  // progressive DN
                    0  << 4   |  // reserved
                    dndi_top_first  << 3   |  // DN/DI Top First
                    0 );         // reserved

    *p_table ++ = ( 0  << 29  |  // reserved . W8
                    32 << 23  |  // dnmh_history_init[5:0]
                    10 << 19  |  // neighborPixel th
                    0  << 18  |  // reserved
                    0  << 16  |  // FMD for 2nd field of previous frame
                    25 << 10  |  // MC pixel consistency th
                    0  << 8   |  // FMD for 1st field for current frame
                    10 << 4   |  // SAD THB
                    5 );         // SAD THA

    *p_table ++ = ( 0  << 24  |  // reserved
                    140<< 16  |  // chr_dnmh_stad_th
                    0  << 13  |  // reserved
                    1  << 12  |  // chrome denoise enable
                    13 << 6   |  // chr temp diff th
                    7 );         // chr temp diff low

    if (IS_GEN8(i965->intel.device_info) ||
        IS_GEN9(i965->intel.device_info))
        *p_table ++ = 0;         // parameters for hot pixel, 
}

//Set default values for STDE
void set_std_table_default(struct intel_vebox_context *proc_ctx, unsigned int *p_table) {

        //DWord 15
        *p_table ++ = ( 0 << 31     |    // Reserved
                        0x3F8 << 21 |    // SATB1 (10 bits, default 8, optimized value -8)
                        31 << 14    |    // SATP3
                        6 << 7      |    // SATP2
                        0x7A );          // SATP1 (7 bits, default 6, optimized value -6)

        //DWord 16
        *p_table ++ = ( 0 << 31   |    // Reserved
                        297 << 20 |    // SATS0
                        124 << 10 |    // SATB3
                        8 );           // SATB2

        //DWord 17
        *p_table ++ = ( 0 << 22   |    // Reserved
                        297 << 11 |    // SATS2
                        85 );          // SATS1

        //DWord 18
        *p_table ++ = ( 14 << 25    |    // HUEP3
                        6 << 18     |    // HUEP2
                        0x7A << 11  |    // HUEP1 (7 bits, default value -6 = 7Ah)
                        256 );           // SATS3

        //DWord 19
        *p_table ++ = ( 0 << 30   |    // Reserved
                        256 << 20 |    // HUEB3
                        8 << 10   |    // HUEB2
                        0x3F8 );       // HUEB1 (10 bits, default value 8, optimized value -8)

        //DWord 20
        *p_table ++ = ( 0 << 22   |    // Reserved
                        85 << 11  |    // HUES1
                        384 );         // HUES0

        //DWord 21
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // HUES3
                        384 );         // HUES2

        //DWord 22
        *p_table ++ = ( 0 << 31   |    // Reserved
                        0 << 21   |    // SATB1_DARK
                        31 << 14  |    // SATP3_DARK
                        31 << 7   |    // SATP2_DARK
                        0x7B );        // SATP1_DARK (7 bits, default value -11 = FF5h, optimized value -5)

        //DWord 23
        *p_table ++ = ( 0 << 31   |    // Reserved
                        305 << 20 |    // SATS0_DARK
                        124 << 10 |    // SATB3_DARK
                        124 );         // SATB2_DARK

        //DWord 24
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // SATS2_DARK
                        220 );         // SATS1_DARK

        //DWord 25
        *p_table ++ = ( 14 << 25  |    // HUEP3_DARK
                        14 << 18  |    // HUEP2_DARK
                        14 << 11  |    // HUEP1_DARK
                        256 );         // SATS3_DARK

        //DWord 26
        *p_table ++ = ( 0 << 30   |    // Reserved
                        56 << 20  |    // HUEB3_DARK
                        56 << 10  |    // HUEB2_DARK
                        56 );          // HUEB1_DARK

        //DWord 27
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // HUES1_DARK
                        256 );         // HUES0_DARK

        //DWord 28
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // HUES3_DARK
                        256 );         // HUES2_DARK
}

//Set values for STDE factor 3
void set_std_table_3(struct intel_vebox_context *proc_ctx, unsigned int *p_table) {

        //DWord 15
        *p_table ++ = ( 0 << 31     |    // Reserved
                        1016 << 21  |    // SATB1 (10 bits, default 8, optimized value 1016)
                        31 << 14    |    // SATP3
                        6 << 7      |    // SATP2
                        122 );           // SATP1 (7 bits, default 6, optimized value 122)

        //DWord 16
        *p_table ++ = ( 0 << 31   |    // Reserved
                        297 << 20 |    // SATS0
                        124 << 10 |    // SATB3
                        8 );           // SATB2

        //DWord 17
        *p_table ++ = ( 0 << 22   |    // Reserved
                        297 << 11 |    // SATS2
                        85 );          // SATS1

        //DWord 18
        *p_table ++ = ( 14 << 25    |    // HUEP3
                        6 << 18     |    // HUEP2
                        122 << 11   |    // HUEP1 (7 bits, default value -6 = 7Ah, optimized 122)
                        256 );           // SATS3

        //DWord 19
        *p_table ++ = ( 0 << 30   |    // Reserved
                        56 << 20  |    // HUEB3 (default 256, optimized 56)
                        8 << 10   |    // HUEB2
                        1016 );        // HUEB1 (10 bits, default value 8, optimized value 1016)

        //DWord 20
        *p_table ++ = ( 0 << 22   |    // Reserved
                        85 << 11  |    // HUES1
                        384 );         // HUES0

        //DWord 21
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // HUES3
                        384 );         // HUES2

        //DWord 22
        *p_table ++ = ( 0 << 31   |    // Reserved
                        0 << 21   |    // SATB1_DARK
                        31 << 14  |    // SATP3_DARK
                        31 << 7   |    // SATP2_DARK
                        123 );         // SATP1_DARK (7 bits, default value -11 = FF5h, optimized value 123)

        //DWord 23
        *p_table ++ = ( 0 << 31   |    // Reserved
                        305 << 20 |    // SATS0_DARK
                        124 << 10 |    // SATB3_DARK
                        124 );         // SATB2_DARK

        //DWord 24
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // SATS2_DARK
                        220 );         // SATS1_DARK

        //DWord 25
        *p_table ++ = ( 14 << 25  |    // HUEP3_DARK
                        14 << 18  |    // HUEP2_DARK
                        14 << 11  |    // HUEP1_DARK
                        256 );         // SATS3_DARK

        //DWord 26
        *p_table ++ = ( 0 << 30   |    // Reserved
                        56 << 20  |    // HUEB3_DARK
                        56 << 10  |    // HUEB2_DARK
                        56 );          // HUEB1_DARK

        //DWord 27
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // HUES1_DARK
                        256 );         // HUES0_DARK

        //DWord 28
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // HUES3_DARK
                        256 );         // HUES2_DARK
}

//Set values for STDE factor 6
void set_std_table_6(struct intel_vebox_context *proc_ctx, unsigned int *p_table) {

        //DWord 15
        *p_table ++ = ( 0 << 31     |    // Reserved
                        0 << 21     |    // SATB1 (10 bits, default 8, optimized value 0)
                        31 << 14    |    // SATP3
                        31 << 7     |    // SATP2 (default 6, optimized 31)
                        114 );           // SATP1 (7 bits, default 6, optimized value 114)

        //DWord 16
        *p_table ++ = ( 0 << 31   |    // Reserved
                        467 << 20 |    // SATS0 (default 297, optimized 467)
                        124 << 10 |    // SATB3
                        124 );         // SATB2

        //DWord 17
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // SATS2 (default 297, optimized 256)
                        176 );         // SATS1

        //DWord 18
        *p_table ++ = ( 14 << 25    |    // HUEP3
                        14 << 18    |    // HUEP2
                        14 << 11    |    // HUEP1 (7 bits, default value -6 = 7Ah, optimized value 14)
                        256 );           // SATS3

        //DWord 19
        *p_table ++ = ( 0 << 30   |    // Reserved
                        56 << 20  |    // HUEB3
                        56 << 10  |    // HUEB2
                        56 );          // HUEB1 (10 bits, default value 8, optimized value 56)

        //DWord 20
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // HUES1
                        256 );         // HUES0

        //DWord 21
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // HUES3
                        256 );         // HUES2

        //DWord 22
        *p_table ++ = ( 0 << 31   |    // Reserved
                        0 << 21   |    // SATB1_DARK
                        31 << 14  |    // SATP3_DARK
                        31 << 7   |    // SATP2_DARK
                        123 );         // SATP1_DARK (7 bits, default value -11 = FF5h, optimized value 123)

        //DWord 23
        *p_table ++ = ( 0 << 31   |    // Reserved
                        305 << 20 |    // SATS0_DARK
                        124 << 10 |    // SATB3_DARK
                        124 );         // SATB2_DARK

        //DWord 24
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // SATS2_DARK
                        220 );         // SATS1_DARK

        //DWord 25
        *p_table ++ = ( 14 << 25  |    // HUEP3_DARK
                        14 << 18  |    // HUEP2_DARK
                        14 << 11  |    // HUEP1_DARK
                        256 );         // SATS3_DARK

        //DWord 26
        *p_table ++ = ( 0 << 30   |    // Reserved
                        56 << 20  |    // HUEB3_DARK
                        56 << 10  |    // HUEB2_DARK
                        56 );          // HUEB1_DARK

        //DWord 27
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // HUES1_DARK
                        256 );         // HUES0_DARK

        //DWord 28
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // HUES3_DARK
                        256 );         // HUES2_DARK
}

//Set values for STDE factor 9
void set_std_table_9(struct intel_vebox_context *proc_ctx, unsigned int *p_table) {

        //DWord 15
        *p_table ++ = ( 0 << 31     |    // Reserved
                        0 << 21     |    // SATB1 (10 bits, default 8, optimized value 0)
                        31 << 14    |    // SATP3
                        31 << 7     |    // SATP2 (default 6, optimized 31)
                        108 );           // SATP1 (7 bits, default 6, optimized value 108)

        //DWord 16
        *p_table ++ = ( 0 << 31   |    // Reserved
                        721 << 20 |    // SATS0 (default 297, optimized 721)
                        124 << 10 |    // SATB3
                        124 );         // SATB2

        //DWord 17
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // SATS2 (default 297, optimized 256)
                        156 );         // SATS1 (default 176, optimized 156)

        //DWord 18
        *p_table ++ = ( 14 << 25    |    // HUEP3
                        14 << 18    |    // HUEP2
                        14 << 11    |    // HUEP1 (7 bits, default value -6 = 7Ah, optimized value 14)
                        256 );           // SATS3

        //DWord 19
        *p_table ++ = ( 0 << 30   |    // Reserved
                        56 << 20  |    // HUEB3
                        56 << 10  |    // HUEB2
                        56 );          // HUEB1 (10 bits, default value 8, optimized value 56)

        //DWord 20
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // HUES1
                        256 );         // HUES0

        //DWord 21
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // HUES3
                        256 );         // HUES2

        //DWord 22
        *p_table ++ = ( 0 << 31   |    // Reserved
                        0 << 21   |    // SATB1_DARK
                        31 << 14  |    // SATP3_DARK
                        31 << 7   |    // SATP2_DARK
                        123 );         // SATP1_DARK (7 bits, default value -11 = FF5h, optimized value 123)

        //DWord 23
        *p_table ++ = ( 0 << 31   |    // Reserved
                        305 << 20 |    // SATS0_DARK
                        124 << 10 |    // SATB3_DARK
                        124 );         // SATB2_DARK

        //DWord 24
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // SATS2_DARK
                        220 );         // SATS1_DARK

        //DWord 25
        *p_table ++ = ( 14 << 25  |    // HUEP3_DARK
                        14 << 18  |    // HUEP2_DARK
                        14 << 11  |    // HUEP1_DARK
                        256 );         // SATS3_DARK

        //DWord 26
        *p_table ++ = ( 0 << 30   |    // Reserved
                        56 << 20  |    // HUEB3_DARK
                        56 << 10  |    // HUEB2_DARK
                        56 );          // HUEB1_DARK

        //DWord 27
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // HUES1_DARK
                        256 );         // HUES0_DARK

        //DWord 28
        *p_table ++ = ( 0 << 22   |    // Reserved
                        256 << 11 |    // HUES3_DARK
                        256 );         // HUES2_DARK
}


void hsw_veb_iecp_std_table(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    unsigned int *p_table = proc_ctx->iecp_state_table.ptr + 0 ;

    if(!(proc_ctx->filters_mask & VPP_IECP_STD_STE)){ 
        memset(p_table, 0, 29 * 4);
    }else{
        int stde_factor = 0; //default value
        VAProcFilterParameterBuffer * std_param = (VAProcFilterParameterBuffer *) proc_ctx->filter_iecp_std;
        stde_factor = std_param->value;

        //DWord 0
        *p_table ++ = ( 154 << 24 |   // V_Mid
                        110 << 16 |   // U_Mid
                        14 << 10  |   // Hue_Max
                        31 << 4   |   // Sat_Max
                        0 << 3    |   // Reserved
                        0 << 2    |   // Output Control is set to output the 1=STD score /0=Output Pixels
                        1 << 1    |   // Set STE Enable
                        1 );          // Set STD Enable

        //DWord 1
        *p_table ++ = ( 0 << 31   |   // Reserved
                        4 << 28   |   // Diamond Margin
                        0 << 21   |   // Diamond_du
                        3 << 18   |   // HS_Margin
                        79 << 10  |   // Cos(alpha)
                        0 << 8    |   // Reserved
                        101 );        // Sin(alpha)

        //DWord 2
        *p_table ++ = ( 0 << 21   |   // Reserved
                        100 << 13 |   // Diamond_alpha
                        35 << 7   |   // Diamond_Th
                        0 );

        //DWord 3
        *p_table ++ = ( 254 << 24 |   // Y_point_3
                        47 << 16  |   // Y_point_2
                        46 << 8   |   // Y_point_1
                        1 << 7    |   // VY_STD_Enable
                        0 );          // Reserved

        //DWord 4
        *p_table ++ = ( 0 << 18   |   // Reserved
                        31 << 13  |   // Y_slope_2
                        31 << 8   |   // Y_slope_1
                        255 );        // Y_point_4

        //DWord 5
        *p_table ++ = ( 400 << 16 |   // INV_Skin_types_margin = 20* Skin_Type_margin => 20*20
                        3300 );       // INV_Margin_VYL => 1/Margin_VYL

        //DWord 6
        *p_table ++ = ( 216 << 24 |   // P1L
                        46 << 16  |   // P0L
                        1600 );       // INV_Margin_VYU

        //DWord 7
        *p_table ++ = ( 130 << 24 |   // B1L
                        133 << 16 |   // B0L
                        236 << 8  |   // P3L
                        236 );        // P2L

        //DWord 8
        *p_table ++ = ( 0 << 27      |   // Reserved
                        0x7FB << 16  |   // S0L (11 bits, Default value: -5 = FBh, pad it with 1s to make it 11bits)
                        130 << 8     |   // B3L
                        130 );

        //DWord 9
        *p_table ++ = ( 0 << 22   |    // Reserved
                        0 << 11   |    // S2L
                        0);            // S1L

        //DWord 10
        *p_table ++ = ( 0 << 27   |    // Reserved
                        66 << 19  |    // P1U
                        46 << 11  |    // P0U
                        0 );           // S3

        //DWord 11
        *p_table ++ = ( 163 << 24 |    // B1U
                        143 << 16 |    // B0U
                        236 << 8  |    // P3U
                        150 );         // P2U

        //DWord 12
        *p_table ++ = ( 0 << 27   |    // Reserved
                        256 << 16 |    // S0U
                        200 << 8  |    // B3U
                        200 );         // B2U

        //DWord 13
        *p_table ++ = ( 0 << 22     |    // Reserved
                        0x74D << 11 |    // S2U (11 bits, Default value -179 = F4Dh)
                        113 );           // S1U

        //DWoord 14
        *p_table ++ = ( 0 << 28   |    // Reserved
                        20 << 20  |    // Skin_types_margin
                        120 << 12 |    // Skin_types_thresh
                        1 << 11   |    // Skin_Types_Enable
                        0 );           // S3U

        //Set DWord 15 through DWord 28 in their respective methods.
        switch(stde_factor) {
            case 3:
                set_std_table_3(proc_ctx, p_table);
                break;

            case 6:
                set_std_table_6(proc_ctx, p_table);
                break;

            case 9:
                set_std_table_9(proc_ctx, p_table);
                break;

            default:
                set_std_table_default(proc_ctx, p_table);
                break;
        }
    }//end of else
}

void hsw_veb_iecp_ace_table(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
   unsigned int *p_table = (unsigned int*)(proc_ctx->iecp_state_table.ptr + 116);

    if(!(proc_ctx->filters_mask & VPP_IECP_ACE)){ 
        memset(p_table, 0, 13 * 4);
    }else{
        *p_table ++ = 0x00000068;
        *p_table ++ = 0x4c382410;
        *p_table ++ = 0x9c887460;
        *p_table ++ = 0xebd8c4b0;
        *p_table ++ = 0x604c3824;

        *p_table ++ = 0xb09c8874;
        *p_table ++ = 0x0000d8c4;
        *p_table ++ = 0x00000000;
        *p_table ++ = 0x00000000;
        *p_table ++ = 0x00000000;

        *p_table ++ = 0x00000000;
        *p_table ++ = 0x00000000;
        *p_table ++ = 0x00000000;
   }
}

void hsw_veb_iecp_tcc_table(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    unsigned int *p_table = (unsigned int*)(proc_ctx->iecp_state_table.ptr + 168);
//    VAProcFilterParameterBuffer * tcc_param =
//            (VAProcFilterParameterBuffer *) proc_ctx->filter_iecp_tcc;

   if(!(proc_ctx->filters_mask & VPP_IECP_TCC)){ 
        memset(p_table, 0, 11 * 4);
    }else{
        *p_table ++ = 0x00000000;
        *p_table ++ = 0x00000000;
        *p_table ++ = 0x1e34cc91;
        *p_table ++ = 0x3e3cce91;
        *p_table ++ = 0x02e80195;

        *p_table ++ = 0x0197046b;
        *p_table ++ = 0x01790174;
        *p_table ++ = 0x00000000;
        *p_table ++ = 0x00000000;
        *p_table ++ = 0x03030000;

        *p_table ++ = 0x009201c0;
   }
}

void hsw_veb_iecp_pro_amp_table(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    unsigned int contrast = 0x80;  //default 
    int brightness = 0x00;         //default
    int cos_c_s    = 256 ;         //default
    int sin_c_s    = 0;            //default 
    unsigned int *p_table = (unsigned int*)(proc_ctx->iecp_state_table.ptr + 212);

    if(!(proc_ctx->filters_mask & VPP_IECP_PRO_AMP)){
        memset(p_table, 0, 2 * 4);
    }else {
        float  src_saturation = 1.0;
        float  src_hue = 0.0;
        float  src_contrast = 1.0;
        float  src_brightness = 0.0;
        float  tmp_value = 0.0;
        unsigned int i = 0;

        VAProcFilterParameterBufferColorBalance * amp_params =
            (VAProcFilterParameterBufferColorBalance *) proc_ctx->filter_iecp_amp;
 
        for (i = 0; i < proc_ctx->filter_iecp_amp_num_elements; i++){
            VAProcColorBalanceType attrib = amp_params[i].attrib;

            if(attrib == VAProcColorBalanceHue) {
               src_hue = amp_params[i].value;         //(-180.0, 180.0)
            }else if(attrib == VAProcColorBalanceSaturation) {
               src_saturation = amp_params[i].value; //(0.0, 10.0)
            }else if(attrib == VAProcColorBalanceBrightness) {
               src_brightness = amp_params[i].value; // (-100.0, 100.0)
               brightness = intel_format_convert(src_brightness, 7, 4, 1);
            }else if(attrib == VAProcColorBalanceContrast) {
               src_contrast = amp_params[i].value;  //  (0.0, 10.0)
               contrast = intel_format_convert(src_contrast, 4, 7, 0);
            }
        }

        tmp_value = cos(src_hue/180*PI) * src_contrast * src_saturation;
        cos_c_s = intel_format_convert(tmp_value, 7, 8, 1);
        
        tmp_value = sin(src_hue/180*PI) * src_contrast * src_saturation;
        sin_c_s = intel_format_convert(tmp_value, 7, 8, 1);
     
        *p_table ++ = ( 0 << 28 |         //reserved
                        contrast << 17 |  //contrast value (U4.7 format)
                        0 << 13 |         //reserved
                        brightness << 1|  // S7.4 format
                        1);

        *p_table ++ = ( cos_c_s << 16 |  // cos(h) * contrast * saturation
                        sin_c_s);        // sin(h) * contrast * saturation
                 
    }
}


void hsw_veb_iecp_csc_table(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    unsigned int *p_table = (unsigned int*)(proc_ctx->iecp_state_table.ptr + 220);
    float tran_coef[9] = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    float v_coef[3]    = {0.0, 0.0, 0.0};
    float u_coef[3]    = {0.0, 0.0, 0.0};
    int   is_transform_enabled = 0;

    if(!(proc_ctx->filters_mask & VPP_IECP_CSC)){
        memset(p_table, 0, 8 * 4);
        return;
    }

    if(proc_ctx->fourcc_input == VA_FOURCC_RGBA &&
       (proc_ctx->fourcc_output == VA_FOURCC_NV12 ||
        proc_ctx->fourcc_output == VA_FOURCC_YV12 ||
        proc_ctx->fourcc_output == VA_FOURCC_YVY2 ||
        proc_ctx->fourcc_output == VA_FOURCC_AYUV)) {

         tran_coef[0] = 0.257;
         tran_coef[1] = 0.504;
         tran_coef[2] = 0.098;
         tran_coef[3] = -0.148;
         tran_coef[4] = -0.291;
         tran_coef[5] = 0.439;
         tran_coef[6] = 0.439;
         tran_coef[7] = -0.368;
         tran_coef[8] = -0.071; 

         u_coef[0] = 16 * 4;
         u_coef[1] = 128 * 4;
         u_coef[2] = 128 * 4;
 
         is_transform_enabled = 1; 
    }else if((proc_ctx->fourcc_input  == VA_FOURCC_NV12 ||
              proc_ctx->fourcc_input  == VA_FOURCC_YV12 ||
              proc_ctx->fourcc_input  == VA_FOURCC_YUY2 ||
              proc_ctx->fourcc_input  == VA_FOURCC_AYUV) &&
              proc_ctx->fourcc_output == VA_FOURCC_RGBA) {
         tran_coef[0] = 1.164;
         tran_coef[1] = 0.000;
         tran_coef[2] = 1.569;
         tran_coef[3] = 1.164;
         tran_coef[4] = -0.813;
         tran_coef[5] = -0.392;
         tran_coef[6] = 1.164;
         tran_coef[7] = 2.017;
         tran_coef[8] = 0.000; 

         v_coef[0] = -16 * 4;
         v_coef[1] = -128 * 4;
         v_coef[2] = -128 * 4;

        is_transform_enabled = 1; 
    }else if(proc_ctx->fourcc_input != proc_ctx->fourcc_output){
         //enable when input and output format are different.
         is_transform_enabled = 1;
    }

    if(is_transform_enabled == 0){
        memset(p_table, 0, 8 * 4);
    }else{
        *p_table ++ = ( 0 << 29 | //reserved
                        intel_format_convert(tran_coef[1], 2, 10, 1) << 16 | //c1, s2.10 format
                        intel_format_convert(tran_coef[0], 2, 10, 1) << 3 |  //c0, s2.10 format
                        0 << 2 | //reserved
                        0 << 1 | // yuv_channel swap
                        is_transform_enabled);                

        *p_table ++ = ( 0 << 26 | //reserved
                        intel_format_convert(tran_coef[3], 2, 10, 1) << 13 | 
                        intel_format_convert(tran_coef[2], 2, 10, 1));
    
        *p_table ++ = ( 0 << 26 | //reserved
                        intel_format_convert(tran_coef[5], 2, 10, 1) << 13 | 
                        intel_format_convert(tran_coef[4], 2, 10, 1));

        *p_table ++ = ( 0 << 26 | //reserved
                        intel_format_convert(tran_coef[7], 2, 10, 1) << 13 | 
                        intel_format_convert(tran_coef[6], 2, 10, 1));

        *p_table ++ = ( 0 << 13 | //reserved
                        intel_format_convert(tran_coef[8], 2, 10, 1));

        *p_table ++ = ( 0 << 22 | //reserved
                        intel_format_convert(u_coef[0], 10, 0, 1) << 11 | 
                        intel_format_convert(v_coef[0], 10, 0, 1));

        *p_table ++ = ( 0 << 22 | //reserved
                        intel_format_convert(u_coef[1], 10, 0, 1) << 11 | 
                        intel_format_convert(v_coef[1], 10, 0, 1));

        *p_table ++ = ( 0 << 22 | //reserved
                        intel_format_convert(u_coef[2], 10, 0, 1) << 11 | 
                        intel_format_convert(v_coef[2], 10, 0, 1));
    }
}

void hsw_veb_iecp_aoi_table(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    unsigned int *p_table = (unsigned int*)(proc_ctx->iecp_state_table.ptr + 252);
   // VAProcFilterParameterBuffer * tcc_param =
   //         (VAProcFilterParameterBuffer *) proc_ctx->filter_iecp_tcc;

    if(!(proc_ctx->filters_mask & VPP_IECP_AOI)){ 
        memset(p_table, 0, 3 * 4);
    }else{
        *p_table ++ = 0x00000000;
        *p_table ++ = 0x00030000;
        *p_table ++ = 0x00030000;
   }
}

void hsw_veb_state_table_setup(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    if(proc_ctx->filters_mask & VPP_DNDI_MASK) {
        dri_bo *dndi_bo = proc_ctx->dndi_state_table.bo;
        dri_bo_map(dndi_bo, 1);
        proc_ctx->dndi_state_table.ptr = dndi_bo->virtual;

        hsw_veb_dndi_table(ctx, proc_ctx);

        dri_bo_unmap(dndi_bo);
    }

    if(proc_ctx->filters_mask & VPP_IECP_MASK) {
        dri_bo *iecp_bo = proc_ctx->iecp_state_table.bo;
        dri_bo_map(iecp_bo, 1);
        proc_ctx->iecp_state_table.ptr = iecp_bo->virtual;

        hsw_veb_iecp_std_table(ctx, proc_ctx);
        hsw_veb_iecp_ace_table(ctx, proc_ctx);
        hsw_veb_iecp_tcc_table(ctx, proc_ctx);
        hsw_veb_iecp_pro_amp_table(ctx, proc_ctx);
        hsw_veb_iecp_csc_table(ctx, proc_ctx);
        hsw_veb_iecp_aoi_table(ctx, proc_ctx);
   
        dri_bo_unmap(iecp_bo);
    }
}

void hsw_veb_state_command(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    struct intel_batchbuffer *batch = proc_ctx->batch;

    BEGIN_VEB_BATCH(batch, 6);
    OUT_VEB_BATCH(batch, VEB_STATE | (6 - 2));
    OUT_VEB_BATCH(batch,
                  0 << 26 |       // state surface control bits
                  0 << 11 |       // reserved.
                  0 << 10 |       // pipe sync disable
                  proc_ctx->current_output_type << 8  | // DI output frame
                  1 << 7  |       // 444->422 downsample method
                  1 << 6  |       // 422->420 downsample method
                  proc_ctx->is_first_frame  << 5  |   // DN/DI first frame
                  proc_ctx->is_di_enabled   << 4  |   // DI enable
                  proc_ctx->is_dn_enabled   << 3  |   // DN enable
                  proc_ctx->is_iecp_enabled << 2  |   // global IECP enabled
                  0 << 1  |       // ColorGamutCompressionEnable
                  0 ) ;           // ColorGamutExpansionEnable.

    OUT_RELOC(batch, 
              proc_ctx->dndi_state_table.bo,
              I915_GEM_DOMAIN_INSTRUCTION, 0, 0);

    OUT_RELOC(batch,
              proc_ctx->iecp_state_table.bo, 
              I915_GEM_DOMAIN_INSTRUCTION, 0, 0);

    OUT_RELOC(batch,
              proc_ctx->gamut_state_table.bo, 
              I915_GEM_DOMAIN_INSTRUCTION, 0, 0);

    OUT_RELOC(batch,
              proc_ctx->vertex_state_table.bo, 
              I915_GEM_DOMAIN_INSTRUCTION, 0, 0);

    ADVANCE_VEB_BATCH(batch);
}

void hsw_veb_surface_state(VADriverContextP ctx, struct intel_vebox_context *proc_ctx, unsigned int is_output)
{
    struct intel_batchbuffer *batch = proc_ctx->batch;
    unsigned int u_offset_y = 0, v_offset_y = 0;
    unsigned int is_uv_interleaved = 0, tiling = 0, swizzle = 0;
    unsigned int surface_format = PLANAR_420_8;
    struct object_surface* obj_surf = NULL;
    unsigned int surface_pitch = 0;
    unsigned int half_pitch_chroma = 0;

    if(is_output){   
        obj_surf = proc_ctx->frame_store[FRAME_OUT_CURRENT].obj_surface;
    }else {
        obj_surf = proc_ctx->frame_store[FRAME_IN_CURRENT].obj_surface;
    }

    assert(obj_surf->fourcc == VA_FOURCC_NV12 ||
           obj_surf->fourcc == VA_FOURCC_YUY2 ||
           obj_surf->fourcc == VA_FOURCC_AYUV ||
           obj_surf->fourcc == VA_FOURCC_RGBA);

    if (obj_surf->fourcc == VA_FOURCC_NV12) {
        surface_format = PLANAR_420_8;
        surface_pitch = obj_surf->width; 
        is_uv_interleaved = 1;
        half_pitch_chroma = 0;
    } else if (obj_surf->fourcc == VA_FOURCC_YUY2) {
        surface_format = YCRCB_NORMAL;
        surface_pitch = obj_surf->width * 2; 
        is_uv_interleaved = 0;
        half_pitch_chroma = 0;
    } else if (obj_surf->fourcc == VA_FOURCC_AYUV) {
        surface_format = PACKED_444A_8;
        surface_pitch = obj_surf->width * 4; 
        is_uv_interleaved = 0;
        half_pitch_chroma = 0;
    } else if (obj_surf->fourcc == VA_FOURCC_RGBA) {
        surface_format = R8G8B8A8_UNORM_SRGB;
        surface_pitch = obj_surf->width * 4; 
        is_uv_interleaved = 0;
        half_pitch_chroma = 0;
    }

    u_offset_y = obj_surf->y_cb_offset;
    v_offset_y = obj_surf->y_cr_offset;
     
    dri_bo_get_tiling(obj_surf->bo, &tiling, &swizzle);

    BEGIN_VEB_BATCH(batch, 6);
    OUT_VEB_BATCH(batch, VEB_SURFACE_STATE | (6 - 2));
    OUT_VEB_BATCH(batch,
                  0 << 1 |         // reserved
                  is_output);      // surface indentification.

    OUT_VEB_BATCH(batch,
                  (obj_surf->height - 1) << 18 |  // height . w3
                  (obj_surf->width -1 )  << 4  |  // width
                  0);                             // reserve

    OUT_VEB_BATCH(batch,
                  surface_format      << 28  |  // surface format, YCbCr420. w4
                  is_uv_interleaved   << 27  |  // interleave chrome , two seperate palar
                  0                   << 20  |  // reserved
                  (surface_pitch - 1) << 3   |  // surface pitch, 64 align
                  half_pitch_chroma   << 2   |  // half pitch for chrome
                  !!tiling            << 1   |  // tiled surface, linear surface used
                  (tiling == I915_TILING_Y));   // tiled walk, ignored when liner surface

    OUT_VEB_BATCH(batch,
                  0 << 29  |     // reserved . w5
                  0 << 16  |     // X offset for V(Cb)
                  0 << 15  |     // reserved
                  u_offset_y);   // Y offset for V(Cb)

    OUT_VEB_BATCH(batch,
                  0 << 29  |     // reserved . w6
                  0 << 16  |     // X offset for V(Cr)
                  0 << 15  |     // reserved
                  v_offset_y );  // Y offset for V(Cr)

    ADVANCE_VEB_BATCH(batch);
}

void hsw_veb_dndi_iecp_command(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    struct intel_batchbuffer *batch = proc_ctx->batch;
    unsigned char frame_ctrl_bits = 0;
    const unsigned int width64 = ALIGN(proc_ctx->width_input, 64);

    /* s1:update the previous and current input */
/*    tempFrame = proc_ctx->frame_store[FRAME_IN_PREVIOUS];
    proc_ctx->frame_store[FRAME_IN_PREVIOUS] = proc_ctx->frame_store[FRAME_IN_CURRENT]; ;
    proc_ctx->frame_store[FRAME_IN_CURRENT] = tempFrame;

    if(proc_ctx->surface_input_vebox != -1){
        vpp_surface_copy(ctx, proc_ctx->frame_store[FRAME_IN_CURRENT].surface_id,
                     proc_ctx->surface_input_vebox);
    } else {
        vpp_surface_copy(ctx, proc_ctx->frame_store[FRAME_IN_CURRENT].surface_id,
                     proc_ctx->surface_input);
    }
*/
    /*s2: update the STMM input and output */
/*    tempFrame = proc_ctx->frame_store[FRAME_IN_STMM];
    proc_ctx->frame_store[FRAME_IN_STMM] = proc_ctx->frame_store[FRAME_OUT_STMM]; ;
    proc_ctx->frame_store[FRAME_OUT_STMM] = tempFrame;
*/	
    /*s3:set reloc buffer address */
    BEGIN_VEB_BATCH(batch, 10);
    OUT_VEB_BATCH(batch, VEB_DNDI_IECP_STATE | (10 - 2));
    OUT_VEB_BATCH(batch, (width64 - 1));
    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_IN_CURRENT].obj_surface->bo,
              I915_GEM_DOMAIN_RENDER, 0, frame_ctrl_bits);
    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_IN_PREVIOUS].obj_surface->bo,
              I915_GEM_DOMAIN_RENDER, 0, frame_ctrl_bits);
    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_IN_STMM].obj_surface->bo,
              I915_GEM_DOMAIN_RENDER, 0, frame_ctrl_bits);
    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_OUT_STMM].obj_surface->bo,
              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER, frame_ctrl_bits);
    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_OUT_CURRENT_DN].obj_surface->bo,
              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER, frame_ctrl_bits);
    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_OUT_CURRENT].obj_surface->bo,
              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER, frame_ctrl_bits);
    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_OUT_PREVIOUS].obj_surface->bo,
              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER, frame_ctrl_bits);
    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_OUT_STATISTIC].obj_surface->bo,
              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER, frame_ctrl_bits);

    ADVANCE_VEB_BATCH(batch);
}

static void
frame_store_reset(VEBFrameStore *fs)
{
    fs->obj_surface = NULL;
    fs->surface_id = VA_INVALID_ID;
    fs->is_internal_surface = 0;
    fs->is_scratch_surface = 0;
}

static void
frame_store_clear(VEBFrameStore *fs, VADriverContextP ctx)
{
    if (fs->obj_surface && fs->is_scratch_surface) {
        VASurfaceID surface_id = fs->obj_surface->base.id;
        i965_DestroySurfaces(ctx, &surface_id, 1);
    }
    frame_store_reset(fs);
}

static VAStatus
gen75_vebox_ensure_surfaces_storage(VADriverContextP ctx,
    struct intel_vebox_context *proc_ctx)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);
    struct object_surface *input_obj_surface, *output_obj_surface;
    unsigned int input_fourcc, output_fourcc;
    unsigned int input_sampling, output_sampling;
    unsigned int input_tiling, output_tiling;
    unsigned int i, swizzle;
    drm_intel_bo *bo;
    VAStatus status;

    /* Determine input surface info. Use native VEBOX format whenever
       possible. i.e. when the input surface format is not supported
       by the VEBOX engine, then allocate a temporary surface (live
       during the whole VPP pipeline lifetime)

       XXX: derive an actual surface format compatible with the input
       surface chroma format */
    input_obj_surface = proc_ctx->surface_input_vebox_object ?
        proc_ctx->surface_input_vebox_object : proc_ctx->surface_input_object;
    if (input_obj_surface->bo) {
        input_fourcc = input_obj_surface->fourcc;
        input_sampling = input_obj_surface->subsampling;
        dri_bo_get_tiling(input_obj_surface->bo, &input_tiling, &swizzle);
        input_tiling = !!input_tiling;
    }
    else {
        input_fourcc = VA_FOURCC_NV12;
        input_sampling = SUBSAMPLE_YUV420;
        input_tiling = 1;
        status = i965_check_alloc_surface_bo(ctx, input_obj_surface,
            input_tiling, input_fourcc, input_sampling);
        if (status != VA_STATUS_SUCCESS)
            return status;
    }

    /* Determine output surface info.

       XXX: derive an actual surface format compatible with the input
       surface chroma format */
    output_obj_surface = proc_ctx->surface_output_vebox_object ?
        proc_ctx->surface_output_vebox_object : proc_ctx->surface_output_object;
    if (output_obj_surface->bo) {
        output_fourcc   = output_obj_surface->fourcc;
        output_sampling = output_obj_surface->subsampling;
        dri_bo_get_tiling(output_obj_surface->bo, &output_tiling, &swizzle);
        output_tiling = !!output_tiling;
    }
    else {
        output_fourcc = VA_FOURCC_NV12;
        output_sampling = SUBSAMPLE_YUV420;
        output_tiling = 1;
        status = i965_check_alloc_surface_bo(ctx, output_obj_surface,
            output_tiling, output_fourcc, output_sampling);
        if (status != VA_STATUS_SUCCESS)
            return status;
    }

    /* Update VEBOX pipeline formats */
    proc_ctx->fourcc_input = input_fourcc;
    proc_ctx->fourcc_output = output_fourcc;
    if (input_fourcc != output_fourcc)
        proc_ctx->is_iecp_enabled = 1; // IECP needed for format conversion
   
    /* Create pipeline surfaces */
    for (i = 0; i < ARRAY_ELEMS(proc_ctx->frame_store); i ++) {
        struct object_surface *obj_surface;
        VASurfaceID new_surface;

        if (proc_ctx->frame_store[i].obj_surface)
            continue; // user allocated surface, not VEBOX internal

        status = i965_CreateSurfaces(ctx, proc_ctx->width_input,
            proc_ctx->height_input, VA_RT_FORMAT_YUV420, 1, &new_surface);
        if (status != VA_STATUS_SUCCESS)
            return status;

        obj_surface = SURFACE(new_surface);
        assert(obj_surface != NULL);

        if (i <= FRAME_IN_PREVIOUS || i == FRAME_OUT_CURRENT_DN) {
            status = i965_check_alloc_surface_bo(ctx, obj_surface,
                input_tiling, input_fourcc, input_sampling);
        }
        else if (i == FRAME_IN_STMM || i == FRAME_OUT_STMM) {
            status = i965_check_alloc_surface_bo(ctx, obj_surface,
                1, input_fourcc, input_sampling);
        }
        else if (i >= FRAME_OUT_CURRENT) {
            status = i965_check_alloc_surface_bo(ctx, obj_surface,
                output_tiling, output_fourcc, output_sampling);
        }
        if (status != VA_STATUS_SUCCESS)
            return status;

        proc_ctx->frame_store[i].obj_surface = obj_surface;
        proc_ctx->frame_store[i].is_internal_surface = 1;
        proc_ctx->frame_store[i].is_scratch_surface = 1;
    }

    /* Allocate DNDI state table  */
    drm_intel_bo_unreference(proc_ctx->dndi_state_table.bo);
    bo = drm_intel_bo_alloc(i965->intel.bufmgr, "vebox: dndi state Buffer",
        0x1000, 0x1000);
    proc_ctx->dndi_state_table.bo = bo;
    if (!bo)
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
 
    /* Allocate IECP state table  */
    drm_intel_bo_unreference(proc_ctx->iecp_state_table.bo);
    bo = drm_intel_bo_alloc(i965->intel.bufmgr, "vebox: iecp state Buffer",
        0x1000, 0x1000);
    proc_ctx->iecp_state_table.bo = bo;
    if (!bo)
        return VA_STATUS_ERROR_ALLOCATION_FAILED;

    /* Allocate Gamut state table  */
    drm_intel_bo_unreference(proc_ctx->gamut_state_table.bo);
    bo = drm_intel_bo_alloc(i965->intel.bufmgr, "vebox: gamut state Buffer",
        0x1000, 0x1000);
    proc_ctx->gamut_state_table.bo = bo;
    if (!bo)
        return VA_STATUS_ERROR_ALLOCATION_FAILED;

    /* Allocate vertex state table  */
    drm_intel_bo_unreference(proc_ctx->vertex_state_table.bo);
    bo = drm_intel_bo_alloc(i965->intel.bufmgr, "vebox: vertex state Buffer",
        0x1000, 0x1000);
    proc_ctx->vertex_state_table.bo = bo;
    if (!bo)
        return VA_STATUS_ERROR_ALLOCATION_FAILED;

    return VA_STATUS_SUCCESS;
}

static VAStatus
gen75_vebox_ensure_surfaces(VADriverContextP ctx,
    struct intel_vebox_context *proc_ctx)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);
    struct object_surface *obj_surface;
    VEBFrameStore *ifs, *ofs;
    bool is_new_frame = 0;
    int i;

    /* Update the previous input surface */
    obj_surface = proc_ctx->surface_input_object;

    is_new_frame = proc_ctx->frame_store[FRAME_IN_CURRENT].surface_id !=
        obj_surface->base.id;
    if (is_new_frame) {
        ifs = &proc_ctx->frame_store[FRAME_IN_PREVIOUS];
        ofs = &proc_ctx->frame_store[proc_ctx->is_dn_enabled ?
            FRAME_OUT_CURRENT_DN : FRAME_IN_CURRENT];
        do {
            const VAProcPipelineParameterBuffer * const pipe =
                proc_ctx->pipeline_param;

            if (pipe->num_forward_references < 1)
                break;
            if (pipe->forward_references[0] == VA_INVALID_ID)
                break;

            obj_surface = SURFACE(pipe->forward_references[0]);
            if (!obj_surface || obj_surface->base.id == ifs->surface_id)
                break;

            frame_store_clear(ifs, ctx);
            if (obj_surface->base.id == ofs->surface_id) {
                *ifs = *ofs;
                frame_store_reset(ofs);
            }
            else {
                ifs->obj_surface = obj_surface;
                ifs->surface_id = obj_surface->base.id;
                ifs->is_internal_surface = 0;
                ifs->is_scratch_surface = 0;
            }
        } while (0);
    }

    /* Update the input surface */
    obj_surface = proc_ctx->surface_input_vebox_object ?
        proc_ctx->surface_input_vebox_object : proc_ctx->surface_input_object;

    ifs = &proc_ctx->frame_store[FRAME_IN_CURRENT];
    frame_store_clear(ifs, ctx);
    ifs->obj_surface = obj_surface;
    ifs->surface_id = proc_ctx->surface_input_object->base.id;
    ifs->is_internal_surface = proc_ctx->surface_input_vebox_object != NULL;
    ifs->is_scratch_surface = 0;

    /* Update the Spatial Temporal Motion Measure (STMM) surfaces */
    if (is_new_frame) {
        const VEBFrameStore tmpfs = proc_ctx->frame_store[FRAME_IN_STMM];
        proc_ctx->frame_store[FRAME_IN_STMM] =
            proc_ctx->frame_store[FRAME_OUT_STMM];
        proc_ctx->frame_store[FRAME_OUT_STMM] = tmpfs;
    }

    /* Reset the output surfaces to defaults. i.e. clean from user surfaces */
    for (i = FRAME_OUT_CURRENT_DN; i <= FRAME_OUT_PREVIOUS; i++) {
        ofs = &proc_ctx->frame_store[i];
        if (!ofs->is_scratch_surface)
            ofs->obj_surface = NULL;
        ofs->surface_id = proc_ctx->surface_input_object->base.id;
    }

    /* Update the output surfaces */
    obj_surface = proc_ctx->surface_output_vebox_object ?
        proc_ctx->surface_output_vebox_object : proc_ctx->surface_output_object;

    proc_ctx->current_output_type = 2;
    if (proc_ctx->filters_mask == VPP_DNDI_DN && !proc_ctx->is_iecp_enabled)
        proc_ctx->current_output = FRAME_OUT_CURRENT_DN;
    else if (proc_ctx->is_di_adv_enabled && !proc_ctx->is_first_frame) {
        proc_ctx->current_output_type = 0;
        proc_ctx->current_output = proc_ctx->is_second_field ?
            FRAME_OUT_CURRENT : FRAME_OUT_PREVIOUS;
    }
    else
        proc_ctx->current_output = FRAME_OUT_CURRENT;
    ofs = &proc_ctx->frame_store[proc_ctx->current_output];
    frame_store_clear(ofs, ctx);
    ofs->obj_surface = obj_surface;
    ofs->surface_id = proc_ctx->surface_input_object->base.id;
    ofs->is_internal_surface = proc_ctx->surface_output_vebox_object != NULL;
    ofs->is_scratch_surface = 0;

    return VA_STATUS_SUCCESS;
}

int hsw_veb_pre_format_convert(VADriverContextP ctx,
                           struct intel_vebox_context *proc_ctx)
{
    VAStatus va_status;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct object_surface* obj_surf_input = proc_ctx->surface_input_object;
    struct object_surface* obj_surf_output = proc_ctx->surface_output_object;
    struct object_surface* obj_surf_input_vebox;
    struct object_surface* obj_surf_output_vebox;

    proc_ctx->format_convert_flags = 0;

    proc_ctx->width_input   = obj_surf_input->orig_width;
    proc_ctx->height_input  = obj_surf_input->orig_height;
    proc_ctx->width_output  = obj_surf_output->orig_width;
    proc_ctx->height_output = obj_surf_output->orig_height;
   
    /* only partial frame is not supported to be processed */
    /*
    assert(proc_ctx->width_input   == proc_ctx->pipeline_param->surface_region->width);
    assert(proc_ctx->height_input  == proc_ctx->pipeline_param->surface_region->height);
    assert(proc_ctx->width_output  == proc_ctx->pipeline_param->output_region->width);
    assert(proc_ctx->height_output == proc_ctx->pipeline_param->output_region->height);
    */

    if(proc_ctx->width_output  != proc_ctx->width_input ||
       proc_ctx->height_output != proc_ctx->height_input){
        proc_ctx->format_convert_flags |= POST_SCALING_CONVERT;
    }

     /* convert the following format to NV12 format */
     if(obj_surf_input->fourcc ==  VA_FOURCC_YV12 ||
        obj_surf_input->fourcc ==  VA_FOURCC_I420 ||
        obj_surf_input->fourcc ==  VA_FOURCC_IMC1 ||
        obj_surf_input->fourcc ==  VA_FOURCC_IMC3 ||
        obj_surf_input->fourcc ==  VA_FOURCC_RGBA ||
        obj_surf_input->fourcc ==  VA_FOURCC_BGRA){

         proc_ctx->format_convert_flags |= PRE_FORMAT_CONVERT;

      } else if(obj_surf_input->fourcc ==  VA_FOURCC_AYUV ||
                obj_surf_input->fourcc ==  VA_FOURCC_YUY2 ||
                obj_surf_input->fourcc ==  VA_FOURCC_NV12){
                // nothing to do here
     } else {
           /* not support other format as input */ 
           assert(0);
     }
    
     if (proc_ctx->format_convert_flags & PRE_FORMAT_CONVERT) {
         if(proc_ctx->surface_input_vebox_object == NULL){
             va_status = i965_CreateSurfaces(ctx,
                                            proc_ctx->width_input,
                                            proc_ctx->height_input,
                                            VA_RT_FORMAT_YUV420,
                                            1,
                                            &(proc_ctx->surface_input_vebox));
             assert(va_status == VA_STATUS_SUCCESS);
             obj_surf_input_vebox = SURFACE(proc_ctx->surface_input_vebox);
             assert(obj_surf_input_vebox);

             if (obj_surf_input_vebox) {
                 proc_ctx->surface_input_vebox_object = obj_surf_input_vebox;
                 i965_check_alloc_surface_bo(ctx, obj_surf_input_vebox, 1, VA_FOURCC_NV12, SUBSAMPLE_YUV420);
             }
         }
       
         vpp_surface_convert(ctx, proc_ctx->surface_input_object, proc_ctx->surface_input_vebox_object);
      }

      /* create one temporary NV12 surfaces for conversion*/
     if(obj_surf_output->fourcc ==  VA_FOURCC_YV12 ||
        obj_surf_output->fourcc ==  VA_FOURCC_I420 ||
        obj_surf_output->fourcc ==  VA_FOURCC_IMC1 ||
        obj_surf_output->fourcc ==  VA_FOURCC_IMC3 ||
        obj_surf_output->fourcc ==  VA_FOURCC_RGBA ||
        obj_surf_output->fourcc ==  VA_FOURCC_BGRA) {

        proc_ctx->format_convert_flags |= POST_FORMAT_CONVERT;
    } else if(obj_surf_output->fourcc ==  VA_FOURCC_AYUV ||
              obj_surf_output->fourcc ==  VA_FOURCC_YUY2 ||
              obj_surf_output->fourcc ==  VA_FOURCC_NV12){
              /* Nothing to do here */
     } else {
           /* not support other format as input */ 
           assert(0);
     }
  
     if(proc_ctx->format_convert_flags & POST_FORMAT_CONVERT ||
        proc_ctx->format_convert_flags & POST_SCALING_CONVERT){
       if(proc_ctx->surface_output_vebox_object == NULL){
             va_status = i965_CreateSurfaces(ctx,
                                            proc_ctx->width_input,
                                            proc_ctx->height_input,
                                            VA_RT_FORMAT_YUV420,
                                            1,
                                            &(proc_ctx->surface_output_vebox));
             assert(va_status == VA_STATUS_SUCCESS);
             obj_surf_output_vebox = SURFACE(proc_ctx->surface_output_vebox);
             assert(obj_surf_output_vebox);

             if (obj_surf_output_vebox) {
                 proc_ctx->surface_output_vebox_object = obj_surf_output_vebox;
                 i965_check_alloc_surface_bo(ctx, obj_surf_output_vebox, 1, VA_FOURCC_NV12, SUBSAMPLE_YUV420);
             }
       }
     }   

     if(proc_ctx->format_convert_flags & POST_SCALING_CONVERT){
       if(proc_ctx->surface_output_scaled_object == NULL){
             va_status = i965_CreateSurfaces(ctx,
                                            proc_ctx->width_output,
                                            proc_ctx->height_output,
                                            VA_RT_FORMAT_YUV420,
                                            1,
                                            &(proc_ctx->surface_output_scaled));
             assert(va_status == VA_STATUS_SUCCESS);
             obj_surf_output_vebox = SURFACE(proc_ctx->surface_output_scaled);
             assert(obj_surf_output_vebox);

             if (obj_surf_output_vebox) {
                 proc_ctx->surface_output_scaled_object = obj_surf_output_vebox;
                 i965_check_alloc_surface_bo(ctx, obj_surf_output_vebox, 1, VA_FOURCC_NV12, SUBSAMPLE_YUV420);
             }
       }
     } 
    
     return 0;
}

int hsw_veb_post_format_convert(VADriverContextP ctx,
                           struct intel_vebox_context *proc_ctx)
{
    struct object_surface *obj_surface = NULL;
    
    obj_surface = proc_ctx->frame_store[proc_ctx->current_output].obj_surface;

    if (proc_ctx->format_convert_flags & POST_COPY_CONVERT) {
        /* copy the saved frame in the second call */
        vpp_surface_convert(ctx, obj_surface, proc_ctx->surface_output_object);
    } else if(!(proc_ctx->format_convert_flags & POST_FORMAT_CONVERT) &&
       !(proc_ctx->format_convert_flags & POST_SCALING_CONVERT)){
        /* Output surface format is covered by vebox pipeline and 
         * processed picture is already store in output surface 
         * so nothing will be done here */
    } else if ((proc_ctx->format_convert_flags & POST_FORMAT_CONVERT) &&
               !(proc_ctx->format_convert_flags & POST_SCALING_CONVERT)){
       /* convert and copy NV12 to YV12/IMC3/IMC2/RGBA output*/
        vpp_surface_convert(ctx, obj_surface, proc_ctx->surface_output_object);

    } else if(proc_ctx->format_convert_flags & POST_SCALING_CONVERT) {
        VAProcPipelineParameterBuffer * const pipe = proc_ctx->pipeline_param;
       /* scaling, convert and copy NV12 to YV12/IMC3/IMC2/RGBA output*/
        assert(obj_surface->fourcc == VA_FOURCC_NV12);
     
        /* first step :surface scaling */
        vpp_surface_scaling(ctx, obj_surface,
            proc_ctx->surface_output_scaled_object, pipe->filter_flags);

        /* second step: color format convert and copy to output */
        obj_surface = proc_ctx->surface_output_object;

        if(obj_surface->fourcc ==  VA_FOURCC_NV12 ||
           obj_surface->fourcc ==  VA_FOURCC_YV12 ||
           obj_surface->fourcc ==  VA_FOURCC_I420 ||
           obj_surface->fourcc ==  VA_FOURCC_YUY2 ||
           obj_surface->fourcc ==  VA_FOURCC_IMC1 ||
           obj_surface->fourcc ==  VA_FOURCC_IMC3 ||
           obj_surface->fourcc ==  VA_FOURCC_RGBA) {
            vpp_surface_convert(ctx, proc_ctx->surface_output_scaled_object, obj_surface);
       }else {
           assert(0); 
       }
   }

    return 0;
}

static VAStatus
gen75_vebox_init_pipe_params(VADriverContextP ctx,
    struct intel_vebox_context *proc_ctx)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);
    const VAProcPipelineParameterBuffer * const pipe = proc_ctx->pipeline_param;
    VAProcFilterParameterBuffer *filter;
    unsigned int i;

    proc_ctx->filters_mask = 0;
    for (i = 0; i < pipe->num_filters; i++) {
        struct object_buffer * const obj_buffer = BUFFER(pipe->filters[i]);

        assert(obj_buffer && obj_buffer->buffer_store);
        if (!obj_buffer || !obj_buffer->buffer_store)
            return VA_STATUS_ERROR_INVALID_PARAMETER;

        filter = (VAProcFilterParameterBuffer *)
            obj_buffer->buffer_store->buffer;
        switch (filter->type) {
        case VAProcFilterNoiseReduction:
            proc_ctx->filters_mask |= VPP_DNDI_DN;
            proc_ctx->filter_dn = filter;
            break;
        case VAProcFilterDeinterlacing:
            proc_ctx->filters_mask |= VPP_DNDI_DI;
            proc_ctx->filter_di = filter;
            break;
        case VAProcFilterColorBalance:
            proc_ctx->filters_mask |= VPP_IECP_PRO_AMP;
            proc_ctx->filter_iecp_amp = filter;
            proc_ctx->filter_iecp_amp_num_elements = obj_buffer->num_elements;
            break;
        case VAProcFilterSkinToneEnhancement:
            proc_ctx->filters_mask |= VPP_IECP_STD_STE;
            proc_ctx->filter_iecp_std = filter;
            break;
        default:
            WARN_ONCE("unsupported filter (type: %d)\n", filter->type);
            return VA_STATUS_ERROR_UNSUPPORTED_FILTER;
        }
    }
    return VA_STATUS_SUCCESS;
}

static VAStatus
gen75_vebox_init_filter_params(VADriverContextP ctx,
    struct intel_vebox_context *proc_ctx)
{
    proc_ctx->format_convert_flags = 0; /* initialized in hsw_veb_pre_format_convert() */

    proc_ctx->is_iecp_enabled = (proc_ctx->filters_mask & VPP_IECP_MASK) != 0;
    proc_ctx->is_dn_enabled = (proc_ctx->filters_mask & VPP_DNDI_DN) != 0;
    proc_ctx->is_di_enabled = (proc_ctx->filters_mask & VPP_DNDI_DI) != 0;
    proc_ctx->is_di_adv_enabled = 0;
    proc_ctx->is_first_frame = 0;
    proc_ctx->is_second_field = 0;

    /* Check whether we are deinterlacing the second field */
    if (proc_ctx->is_di_enabled) {
        const VAProcFilterParameterBufferDeinterlacing * const deint_params =
            proc_ctx->filter_di;

        const unsigned int tff =
            !(deint_params->flags & VA_DEINTERLACING_BOTTOM_FIELD_FIRST);
        const unsigned int is_top_field =
            !(deint_params->flags & VA_DEINTERLACING_BOTTOM_FIELD);

        if ((tff ^ is_top_field) != 0) {
            struct object_surface * const obj_surface =
                proc_ctx->surface_input_object;

            if (proc_ctx->frame_store[FRAME_IN_CURRENT].surface_id != obj_surface->base.id) {
                WARN_ONCE("invalid surface provided for second field\n");
                return VA_STATUS_ERROR_INVALID_PARAMETER;
            }
            proc_ctx->is_second_field = 1;
        }
    }

    /* Check whether we are deinterlacing the first frame */
    if (proc_ctx->is_di_enabled) {
        const VAProcFilterParameterBufferDeinterlacing * const deint_params =
            proc_ctx->filter_di;

        switch (deint_params->algorithm) {
        case VAProcDeinterlacingBob:
            proc_ctx->is_first_frame = 1;
            break;
        case VAProcDeinterlacingMotionAdaptive:
        case VAProcDeinterlacingMotionCompensated:
            if (proc_ctx->frame_store[FRAME_IN_CURRENT].surface_id == VA_INVALID_ID)
                proc_ctx->is_first_frame = 1;
            else if (proc_ctx->is_second_field) {
                /* At this stage, we have already deinterlaced the
                   first field successfully. So, the first frame flag
                   is trigerred if the previous field was deinterlaced
                   without reference frame */
                if (proc_ctx->frame_store[FRAME_IN_PREVIOUS].surface_id == VA_INVALID_ID)
                    proc_ctx->is_first_frame = 1;
            }
            else {
                const VAProcPipelineParameterBuffer * const pipe =
                    proc_ctx->pipeline_param;

                if (pipe->num_forward_references < 1 ||
                    pipe->forward_references[0] == VA_INVALID_ID) {
                    WARN_ONCE("A forward temporal reference is needed for Motion adaptive/compensated deinterlacing !!!\n");
                    return VA_STATUS_ERROR_INVALID_PARAMETER;
                }
            }
            proc_ctx->is_di_adv_enabled = 1;
            break;
        default:
            WARN_ONCE("unsupported deinterlacing algorithm (%d)\n",
                      deint_params->algorithm);
            return VA_STATUS_ERROR_UNSUPPORTED_FILTER;
        }
    }
    return VA_STATUS_SUCCESS;
}

VAStatus
gen75_vebox_process_picture(VADriverContextP ctx,
    struct intel_vebox_context *proc_ctx)
{
    VAStatus status;

    status = gen75_vebox_init_pipe_params(ctx, proc_ctx);
    if (status != VA_STATUS_SUCCESS)
        return status;

    status = gen75_vebox_init_filter_params(ctx, proc_ctx);
    if (status != VA_STATUS_SUCCESS)
        return status;

    hsw_veb_pre_format_convert(ctx, proc_ctx);

    status = gen75_vebox_ensure_surfaces(ctx, proc_ctx);
    if (status != VA_STATUS_SUCCESS)
        return status;

    status = gen75_vebox_ensure_surfaces_storage(ctx, proc_ctx);
    if (status != VA_STATUS_SUCCESS)
        return status;

    if (proc_ctx->format_convert_flags & POST_COPY_CONVERT) {
        assert(proc_ctx->is_second_field);
        /* directly copy the saved frame in the second call */
    } else {
        intel_batchbuffer_start_atomic_veb(proc_ctx->batch, 0x1000);
        intel_batchbuffer_emit_mi_flush(proc_ctx->batch);
        hsw_veb_state_table_setup(ctx, proc_ctx);
        hsw_veb_state_command(ctx, proc_ctx);		
        hsw_veb_surface_state(ctx, proc_ctx, INPUT_SURFACE);
        hsw_veb_surface_state(ctx, proc_ctx, OUTPUT_SURFACE);
        hsw_veb_dndi_iecp_command(ctx, proc_ctx);
        intel_batchbuffer_end_atomic(proc_ctx->batch);
        intel_batchbuffer_flush(proc_ctx->batch);
    }

    hsw_veb_post_format_convert(ctx, proc_ctx);
     
    return VA_STATUS_SUCCESS;
}

void gen75_vebox_context_destroy(VADriverContextP ctx, 
                          struct intel_vebox_context *proc_ctx)
{
    int i;

    if(proc_ctx->surface_input_vebox != VA_INVALID_ID){
       i965_DestroySurfaces(ctx, &proc_ctx->surface_input_vebox, 1);
       proc_ctx->surface_input_vebox = VA_INVALID_ID;
       proc_ctx->surface_input_vebox_object = NULL;
     }

    if(proc_ctx->surface_output_vebox != VA_INVALID_ID){
       i965_DestroySurfaces(ctx, &proc_ctx->surface_output_vebox, 1);
       proc_ctx->surface_output_vebox = VA_INVALID_ID;
       proc_ctx->surface_output_vebox_object = NULL;
     }

    if(proc_ctx->surface_output_scaled != VA_INVALID_ID){
       i965_DestroySurfaces(ctx, &proc_ctx->surface_output_scaled, 1);
       proc_ctx->surface_output_scaled = VA_INVALID_ID;
       proc_ctx->surface_output_scaled_object = NULL;
     }

    for (i = 0; i < ARRAY_ELEMS(proc_ctx->frame_store); i++)
        frame_store_clear(&proc_ctx->frame_store[i], ctx);

    /* dndi state table  */
    drm_intel_bo_unreference(proc_ctx->dndi_state_table.bo);
    proc_ctx->dndi_state_table.bo = NULL;

    /* iecp state table  */
    drm_intel_bo_unreference(proc_ctx->iecp_state_table.bo);
    proc_ctx->iecp_state_table.bo = NULL;
 
    /* gamut statu table */
    drm_intel_bo_unreference(proc_ctx->gamut_state_table.bo);
    proc_ctx->gamut_state_table.bo = NULL;

    /* vertex state table  */
    drm_intel_bo_unreference(proc_ctx->vertex_state_table.bo);
    proc_ctx->vertex_state_table.bo = NULL;

    intel_batchbuffer_free(proc_ctx->batch);

    free(proc_ctx);
}

struct intel_vebox_context * gen75_vebox_context_init(VADriverContextP ctx)
{
    struct intel_driver_data *intel = intel_driver_data(ctx);
    struct intel_vebox_context *proc_context = calloc(1, sizeof(struct intel_vebox_context));
    int i;

    assert(proc_context);
    proc_context->batch = intel_batchbuffer_new(intel, I915_EXEC_VEBOX, 0);

    for (i = 0; i < ARRAY_ELEMS(proc_context->frame_store); i++)
        proc_context->frame_store[i].surface_id = VA_INVALID_ID;
  
    proc_context->filters_mask          = 0;
    proc_context->surface_output_object = NULL;
    proc_context->surface_input_object  = NULL;
    proc_context->surface_input_vebox   = VA_INVALID_ID;
    proc_context->surface_input_vebox_object = NULL;
    proc_context->surface_output_vebox  = VA_INVALID_ID;
    proc_context->surface_output_vebox_object = NULL;
    proc_context->surface_output_scaled = VA_INVALID_ID;
    proc_context->surface_output_scaled_object = NULL;
    proc_context->filters_mask          = 0;
    proc_context->format_convert_flags  = 0;

    return proc_context;
}

void bdw_veb_state_command(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    struct intel_batchbuffer *batch = proc_ctx->batch;

    BEGIN_VEB_BATCH(batch, 0xc);
    OUT_VEB_BATCH(batch, VEB_STATE | (0xc - 2));
    OUT_VEB_BATCH(batch,
                  0 << 25 |       // state surface control bits
                  0 << 23 |       // reserved.
                  0 << 22 |       // gamut expansion position
                  0 << 15 |       // reserved.
                  0 << 14 |       // single slice vebox enable
                  0 << 13 |       // hot pixel filter enable
                  0 << 12 |       // alpha plane enable
                  0 << 11 |       // vignette enable
                  0 << 10 |       // demosaic enable
                  proc_ctx->current_output_type << 8  | // DI output frame
                  1 << 7  |       // 444->422 downsample method
                  1 << 6  |       // 422->420 downsample method
                  proc_ctx->is_first_frame  << 5  |   // DN/DI first frame
                  proc_ctx->is_di_enabled   << 4  |   // DI enable
                  proc_ctx->is_dn_enabled   << 3  |   // DN enable
                  proc_ctx->is_iecp_enabled << 2  |   // global IECP enabled
                  0 << 1  |       // ColorGamutCompressionEnable
                  0 ) ;           // ColorGamutExpansionEnable.

    OUT_RELOC(batch,
              proc_ctx->dndi_state_table.bo,
              I915_GEM_DOMAIN_INSTRUCTION, 0, 0);

    OUT_VEB_BATCH(batch, 0);

    OUT_RELOC(batch,
              proc_ctx->iecp_state_table.bo,
              I915_GEM_DOMAIN_INSTRUCTION, 0, 0);

    OUT_VEB_BATCH(batch, 0);

    OUT_RELOC(batch,
              proc_ctx->gamut_state_table.bo,
              I915_GEM_DOMAIN_INSTRUCTION, 0, 0);

    OUT_VEB_BATCH(batch, 0);

    OUT_RELOC(batch,
              proc_ctx->vertex_state_table.bo,
              I915_GEM_DOMAIN_INSTRUCTION, 0, 0);

    OUT_VEB_BATCH(batch, 0);

    OUT_VEB_BATCH(batch, 0);/*caputre pipe state pointer*/
    OUT_VEB_BATCH(batch, 0);

    ADVANCE_VEB_BATCH(batch);
}

void bdw_veb_dndi_iecp_command(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    struct intel_batchbuffer *batch = proc_ctx->batch;
    unsigned char frame_ctrl_bits = 0;
    const unsigned int width64 = ALIGN(proc_ctx->width_input, 64);

    BEGIN_VEB_BATCH(batch, 0x14);
    OUT_VEB_BATCH(batch, VEB_DNDI_IECP_STATE | (0x14 - 2));//DWord 0
    OUT_VEB_BATCH(batch, (width64 - 1));

    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_IN_CURRENT].obj_surface->bo,
              I915_GEM_DOMAIN_RENDER, 0, frame_ctrl_bits);//DWord 2
    OUT_VEB_BATCH(batch,0);//DWord 3

    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_IN_PREVIOUS].obj_surface->bo,
              I915_GEM_DOMAIN_RENDER, 0, frame_ctrl_bits);//DWord 4
    OUT_VEB_BATCH(batch,0);//DWord 5

    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_IN_STMM].obj_surface->bo,
              I915_GEM_DOMAIN_RENDER, 0, frame_ctrl_bits);//DWord 6
    OUT_VEB_BATCH(batch,0);//DWord 7

    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_OUT_STMM].obj_surface->bo,
              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER, frame_ctrl_bits);//DWord 8
    OUT_VEB_BATCH(batch,0);//DWord 9

    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_OUT_CURRENT_DN].obj_surface->bo,
              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER, frame_ctrl_bits);//DWord 10
    OUT_VEB_BATCH(batch,0);//DWord 11

    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_OUT_CURRENT].obj_surface->bo,
              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER, frame_ctrl_bits);//DWord 12
    OUT_VEB_BATCH(batch,0);//DWord 13

    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_OUT_PREVIOUS].obj_surface->bo,
              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER, frame_ctrl_bits);//DWord 14
    OUT_VEB_BATCH(batch,0);//DWord 15

    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_OUT_STATISTIC].obj_surface->bo,
              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER, frame_ctrl_bits);//DWord 16
    OUT_VEB_BATCH(batch,0);//DWord 17

    OUT_VEB_BATCH(batch,0);//DWord 18
    OUT_VEB_BATCH(batch,0);//DWord 19

    ADVANCE_VEB_BATCH(batch);
}

VAStatus
gen8_vebox_process_picture(VADriverContextP ctx,
    struct intel_vebox_context *proc_ctx)
{
    VAStatus status;

    status = gen75_vebox_init_pipe_params(ctx, proc_ctx);
    if (status != VA_STATUS_SUCCESS)
        return status;

    status = gen75_vebox_init_filter_params(ctx, proc_ctx);
    if (status != VA_STATUS_SUCCESS)
        return status;

    hsw_veb_pre_format_convert(ctx, proc_ctx);

    status = gen75_vebox_ensure_surfaces(ctx, proc_ctx);
    if (status != VA_STATUS_SUCCESS)
        return status;

    status = gen75_vebox_ensure_surfaces_storage(ctx, proc_ctx);
    if (status != VA_STATUS_SUCCESS)
        return status;

    if (proc_ctx->format_convert_flags & POST_COPY_CONVERT) {
        assert(proc_ctx->is_second_field);
        /* directly copy the saved frame in the second call */
    } else {
        intel_batchbuffer_start_atomic_veb(proc_ctx->batch, 0x1000);
        intel_batchbuffer_emit_mi_flush(proc_ctx->batch);
        hsw_veb_state_table_setup(ctx, proc_ctx);
        bdw_veb_state_command(ctx, proc_ctx);		
        hsw_veb_surface_state(ctx, proc_ctx, INPUT_SURFACE);
        hsw_veb_surface_state(ctx, proc_ctx, OUTPUT_SURFACE);
        bdw_veb_dndi_iecp_command(ctx, proc_ctx);
        intel_batchbuffer_end_atomic(proc_ctx->batch);
        intel_batchbuffer_flush(proc_ctx->batch);
    }

    hsw_veb_post_format_convert(ctx, proc_ctx);
     
    return VA_STATUS_SUCCESS;
}


void
skl_veb_dndi_table(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    unsigned int* p_table ;
    unsigned int progressive_dn = 1;
    unsigned int dndi_top_first = 0;
    unsigned int is_mcdi_enabled = 0;

    if (proc_ctx->is_di_enabled) {
        const VAProcFilterParameterBufferDeinterlacing * const deint_params =
            proc_ctx->filter_di;

        progressive_dn = 0;

        /* If we are in "First Frame" mode, i.e. past frames are not
           available for motion measure, then don't use the TFF flag */
        dndi_top_first = !(deint_params->flags & (proc_ctx->is_first_frame ?
                VA_DEINTERLACING_BOTTOM_FIELD :
                VA_DEINTERLACING_BOTTOM_FIELD_FIRST));

        is_mcdi_enabled =
            (deint_params->algorithm == VAProcDeinterlacingMotionCompensated);
    }

    /*
    VAProcFilterParameterBufferDeinterlacing *di_param =
            (VAProcFilterParameterBufferDeinterlacing *) proc_ctx->filter_di;

    VAProcFilterParameterBuffer * dn_param =
            (VAProcFilterParameterBuffer *) proc_ctx->filter_dn;
    */
    p_table = (unsigned int *)proc_ctx->dndi_state_table.ptr;

    *p_table ++ = ( 140 << 20 |   // denoise stad threshold . w1
                    192 << 12 |   // dnmh_history_max
                    7   << 8  |   // dnmh_delta[3:0]
                    1 );          // denoise moving pixel threshold

    *p_table ++ = ( 38 << 20 |    // denoise asd threshold
                    0  << 10 |    // temporal diff th
                    0 );          // low temporal diff th

    *p_table ++ = ( progressive_dn << 28  |  // progressive dn
                    38 << 16 |    // denoise th for sum of complexity measure
                    32 << 10 |    // dnmh_history_init[5:0]
                    0 );          // reserved

    *p_table ++ = ( 0 << 28  |    // hot pixel count
                    0 << 20  |    // hot pixel threshold
                    1 << 12  |    // block noise estimate edge threshold
                    20 );         // block noise estimate noise threshold

    *p_table ++ = ( 140<< 16 |    // chroma denoise stad threshold
                    0  << 13 |    // reserved
                    1  << 12 |    // chrome denoise enable
                    13 << 6  |    // chr temp diff th
                    7 );          // chr temp diff low

    *p_table ++ = 0;              // weight

    *p_table ++ = ( 0 << 16  |    // dn_thmax
                    0 );          // dn_thmin

    *p_table ++ = ( 0 << 16  |    // dn_prt5
                    0 );          // dn_dyn_thmin

    *p_table ++ = ( 0 << 16  |    // dn_prt4
                    0 );          // dn_prt3

    *p_table ++ = ( 0 << 16  |    // dn_prt2
                    0 );          // dn_prt1

    *p_table ++ = ( 0 << 16  |    // dn_prt0
                    0 << 10  |    // dn_wd22
                    0 << 5   |    // dh_wd21
                    0 );          // dh_wd20

   *p_table ++ = ( 0 << 25  |    // dn_wd12
                    0 << 20  |    // dn_wd11
                    0 << 15  |    // dn_wd10
                    0 << 10  |    // dn_wd02
                    0 << 5   |    // dn_wd01
                    0 );          // dn_wd00

    *p_table ++ = ( 2 << 10 |     // stmm c2
                    9 << 6  |     // cat slope minus 1
                    5 << 2  |     // sad tight threshold
                    0 );          // smooth mv th

    *p_table ++ = ( 0  << 31 |    // stmm blending constant select
                    64 << 24 |    // stmm trc1
                    125<< 16 |    // stmm trc2
                    0  << 14 |    // reserved
                    30 << 8  |    // multiplier for vecm
                    150 );        // maximum stmm

    *p_table ++ = ( 118<< 24  |   // minumum stmm
                    0  << 22  |   // stmm shift down
                    1  << 20  |   // stmm shift up
                    5  << 16  |   // stmm output shift
                    100 << 8  |   // sdi threshold
                    5 );          // sdi delta

    *p_table ++ = ( 50  << 24 |   // sdi fallback mode 1 t1 constant
                    100 << 16 |   // sdi fallback mode 1 t2 constant
                    37  << 8  |   // sdi fallback mode 2 constant(angle2x1)
                    175 );        // fmd temporal difference threshold

    *p_table ++ = ( 16 << 24  |   // fmd #1 vertical difference th . w7
                    100<< 16  |   // fmd #2 vertical difference th
                    0  << 14  |   // cat threshold
                    2  << 8   |   // fmd tear threshold
                    is_mcdi_enabled  << 7  |  // mcdi enable, use motion compensated deinterlace algorithm
                    dndi_top_first  << 3   |  // dn/di top first
                    0 );          // reserved

    *p_table ++ = ( 10 << 19  |   // neighbor pixel threshold
                    0  << 16  |   // fmd for 2nd field of previous frame
                    25 << 10  |   // mc pixel consistency threshold
                    0  << 8   |   // fmd for 1st field for current frame
                    10 << 4   |   // sad thb
                    5 );          // sad tha
}

void skl_veb_iecp_csc_table(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    unsigned int *p_table = (unsigned int*)(proc_ctx->iecp_state_table.ptr + 220);
    float tran_coef[9] = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    float v_coef[3]    = {0.0, 0.0, 0.0};
    float u_coef[3]    = {0.0, 0.0, 0.0};
    int   is_transform_enabled = 0;

    if(!(proc_ctx->filters_mask & VPP_IECP_CSC)){
        memset(p_table, 0, 12 * 4);
        return;
    }

    if(proc_ctx->fourcc_input == VA_FOURCC_RGBA &&
       (proc_ctx->fourcc_output == VA_FOURCC_NV12 ||
        proc_ctx->fourcc_output == VA_FOURCC_YV12 ||
        proc_ctx->fourcc_output == VA_FOURCC_YVY2 ||
        proc_ctx->fourcc_output == VA_FOURCC_AYUV)) {

        tran_coef[0] = 0.257;
        tran_coef[1] = 0.504;
        tran_coef[2] = 0.098;
        tran_coef[3] = -0.148;
        tran_coef[4] = -0.291;
        tran_coef[5] = 0.439;
        tran_coef[6] = 0.439;
        tran_coef[7] = -0.368;
        tran_coef[8] = -0.071;

        u_coef[0] = 16 * 4;
        u_coef[1] = 128 * 4;
        u_coef[2] = 128 * 4;

        is_transform_enabled = 1;
    }else if((proc_ctx->fourcc_input  == VA_FOURCC_NV12 ||
              proc_ctx->fourcc_input  == VA_FOURCC_YV12 ||
              proc_ctx->fourcc_input  == VA_FOURCC_YUY2 ||
              proc_ctx->fourcc_input  == VA_FOURCC_AYUV) &&
             proc_ctx->fourcc_output == VA_FOURCC_RGBA) {
        tran_coef[0] = 1.164;
        tran_coef[1] = 0.000;
        tran_coef[2] = 1.569;
        tran_coef[3] = 1.164;
        tran_coef[4] = -0.813;
        tran_coef[5] = -0.392;
        tran_coef[6] = 1.164;
        tran_coef[7] = 2.017;
        tran_coef[8] = 0.000;

        v_coef[0] = -16 * 4;
        v_coef[1] = -128 * 4;
        v_coef[2] = -128 * 4;

        is_transform_enabled = 1;
    }else if(proc_ctx->fourcc_input != proc_ctx->fourcc_output){
        //enable when input and output format are different.
        is_transform_enabled = 1;
    }

    if(is_transform_enabled == 0){
        memset(p_table, 0, 12 * 4);
    }else{
        *p_table ++ = ( is_transform_enabled << 31 |
                        0 << 29 | // yuv_channel swap
                        intel_format_convert(tran_coef[0], 2, 16, 1));          //c0, s2.16 format

        *p_table ++ = ( 0 << 19 | //reserved
                        intel_format_convert(tran_coef[1], 2, 16, 1));          //c1, s2.16 format

        *p_table ++ = ( 0 << 19 | //reserved
                        intel_format_convert(tran_coef[2], 2, 16, 1));          //c2, s2.16 format

        *p_table ++ = ( 0 << 19 | //reserved
                        intel_format_convert(tran_coef[3], 2, 16, 1));          //c3, s2.16 format

        *p_table ++ = ( 0 << 19 | //reserved
                        intel_format_convert(tran_coef[4], 2, 16, 1));          //c4, s2.16 format

        *p_table ++ = ( 0 << 19 | //reserved
                        intel_format_convert(tran_coef[5], 2, 16, 1));          //c5, s2.16 format

        *p_table ++ = ( 0 << 19 | //reserved
                        intel_format_convert(tran_coef[6], 2, 16, 1));          //c6, s2.16 format

        *p_table ++ = ( 0 << 19 | //reserved
                        intel_format_convert(tran_coef[7], 2, 16, 1));          //c7, s2.16 format

        *p_table ++ = ( 0 << 19 | //reserved
                        intel_format_convert(tran_coef[8], 2, 16, 1));          //c8, s2.16 format

        *p_table ++ = ( intel_format_convert(u_coef[0], 16, 0, 1) << 16 |
                        intel_format_convert(v_coef[0], 16, 0, 1));

        *p_table ++ = ( intel_format_convert(u_coef[1], 16, 0, 1) << 16 |
                        intel_format_convert(v_coef[1], 16, 0, 1));

        *p_table ++ = ( intel_format_convert(u_coef[2], 16, 0, 1) << 16 |
                        intel_format_convert(v_coef[2], 16, 0, 1));
    }
}

void skl_veb_iecp_aoi_table(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    unsigned int *p_table = (unsigned int*)(proc_ctx->iecp_state_table.ptr + 27 * sizeof(unsigned int));

    if (!(proc_ctx->filters_mask & VPP_IECP_AOI)) {
        memset(p_table, 0, 3 * 4);
    } else {
        *p_table ++ = 0x00000000;
        *p_table ++ = 0x00030000;
        *p_table ++ = 0x00030000;
    }
}

void skl_veb_state_table_setup(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    if(proc_ctx->filters_mask & VPP_DNDI_MASK) {
        dri_bo *dndi_bo = proc_ctx->dndi_state_table.bo;
        dri_bo_map(dndi_bo, 1);
        proc_ctx->dndi_state_table.ptr = dndi_bo->virtual;

        skl_veb_dndi_table(ctx, proc_ctx);

        dri_bo_unmap(dndi_bo);
    }

    if(proc_ctx->filters_mask & VPP_IECP_MASK) {
        dri_bo *iecp_bo = proc_ctx->iecp_state_table.bo;
        dri_bo_map(iecp_bo, 1);
        proc_ctx->iecp_state_table.ptr = iecp_bo->virtual;

        hsw_veb_iecp_std_table(ctx, proc_ctx);
        hsw_veb_iecp_ace_table(ctx, proc_ctx);
        hsw_veb_iecp_tcc_table(ctx, proc_ctx);
        hsw_veb_iecp_pro_amp_table(ctx, proc_ctx);
        skl_veb_iecp_csc_table(ctx, proc_ctx);
        skl_veb_iecp_aoi_table(ctx, proc_ctx);

        dri_bo_unmap(iecp_bo);
    }
}

void
skl_veb_state_command(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    struct intel_batchbuffer *batch = proc_ctx->batch;

    BEGIN_VEB_BATCH(batch, 0x10);
    OUT_VEB_BATCH(batch, VEB_STATE | (0x10 - 2));
    OUT_VEB_BATCH(batch,
                  0 << 25 |       // state surface control bits
                  0 << 23 |       // reserved.
                  0 << 22 |       // gamut expansion position
                  0 << 15 |       // reserved.
                  0 << 14 |       // single slice vebox enable
                  0 << 13 |       // hot pixel filter enable
                  0 << 12 |       // alpha plane enable
                  0 << 11 |       // vignette enable
                  0 << 10 |       // demosaic enable
                  proc_ctx->current_output_type << 8  | // DI output frame
                  1 << 7  |       // 444->422 downsample method
                  1 << 6  |       // 422->420 downsample method
                  proc_ctx->is_first_frame  << 5  |   // DN/DI first frame
                  proc_ctx->is_di_enabled   << 4  |   // DI enable
                  proc_ctx->is_dn_enabled   << 3  |   // DN enable
                  proc_ctx->is_iecp_enabled << 2  |   // global IECP enabled
                  0 << 1  |       // ColorGamutCompressionEnable
                  0 ) ;           // ColorGamutExpansionEnable.

    OUT_RELOC(batch,
              proc_ctx->dndi_state_table.bo,
              I915_GEM_DOMAIN_INSTRUCTION, 0, 0);

    OUT_VEB_BATCH(batch, 0);

    OUT_RELOC(batch,
              proc_ctx->iecp_state_table.bo,
              I915_GEM_DOMAIN_INSTRUCTION, 0, 0);

    OUT_VEB_BATCH(batch, 0);

    OUT_RELOC(batch,
              proc_ctx->gamut_state_table.bo,
              I915_GEM_DOMAIN_INSTRUCTION, 0, 0);

    OUT_VEB_BATCH(batch, 0);

    OUT_RELOC(batch,
              proc_ctx->vertex_state_table.bo,
              I915_GEM_DOMAIN_INSTRUCTION, 0, 0);

    OUT_VEB_BATCH(batch, 0);

    OUT_VEB_BATCH(batch, 0);/*caputre pipe state pointer*/
    OUT_VEB_BATCH(batch, 0);

    OUT_VEB_BATCH(batch, 0);/*lace lut table state pointer*/
    OUT_VEB_BATCH(batch, 0);

    OUT_VEB_BATCH(batch, 0);/*gamma correction values address*/
    OUT_VEB_BATCH(batch, 0);

    ADVANCE_VEB_BATCH(batch);
}

void skl_veb_surface_state(VADriverContextP ctx, struct intel_vebox_context *proc_ctx, unsigned int is_output)
{
    struct intel_batchbuffer *batch = proc_ctx->batch;
    unsigned int u_offset_y = 0, v_offset_y = 0;
    unsigned int is_uv_interleaved = 0, tiling = 0, swizzle = 0;
    unsigned int surface_format = PLANAR_420_8;
    struct object_surface* obj_surf = NULL;
    unsigned int surface_pitch = 0;
    unsigned int half_pitch_chroma = 0;
    unsigned int derived_pitch;

    if (is_output) {
        obj_surf = proc_ctx->frame_store[FRAME_OUT_CURRENT].obj_surface;
    } else {
        obj_surf = proc_ctx->frame_store[FRAME_IN_CURRENT].obj_surface;
    }

    assert(obj_surf->fourcc == VA_FOURCC_NV12 ||
           obj_surf->fourcc == VA_FOURCC_YUY2 ||
           obj_surf->fourcc == VA_FOURCC_AYUV ||
           obj_surf->fourcc == VA_FOURCC_RGBA);

    if (obj_surf->fourcc == VA_FOURCC_NV12) {
        surface_format = PLANAR_420_8;
        surface_pitch = obj_surf->width;
        is_uv_interleaved = 1;
        half_pitch_chroma = 0;
    } else if (obj_surf->fourcc == VA_FOURCC_YUY2) {
        surface_format = YCRCB_NORMAL;
        surface_pitch = obj_surf->width * 2;
        is_uv_interleaved = 0;
        half_pitch_chroma = 0;
    } else if (obj_surf->fourcc == VA_FOURCC_AYUV) {
        surface_format = PACKED_444A_8;
        surface_pitch = obj_surf->width * 4;
        is_uv_interleaved = 0;
        half_pitch_chroma = 0;
    } else if (obj_surf->fourcc == VA_FOURCC_RGBA) {
        surface_format = R8G8B8A8_UNORM_SRGB;
        surface_pitch = obj_surf->width * 4;
        is_uv_interleaved = 0;
        half_pitch_chroma = 0;
    }

    derived_pitch = surface_pitch;

    u_offset_y = obj_surf->y_cb_offset;
    v_offset_y = obj_surf->y_cr_offset;

    dri_bo_get_tiling(obj_surf->bo, &tiling, &swizzle);

    BEGIN_VEB_BATCH(batch, 9);
    OUT_VEB_BATCH(batch, VEB_SURFACE_STATE | (9 - 2));
    OUT_VEB_BATCH(batch,
                  0 << 1 |         // reserved
                  is_output);      // surface indentification.

    OUT_VEB_BATCH(batch,
                  (obj_surf->height - 1) << 18 |  // height . w3
                  (obj_surf->width -1 )  << 4  |  // width
                  0);                             // reserve

    OUT_VEB_BATCH(batch,
                  surface_format      << 28  |  // surface format, YCbCr420. w4
                  is_uv_interleaved   << 27  |  // interleave chrome , two seperate palar
                  0                   << 20  |  // reserved
                  (surface_pitch - 1) << 3   |  // surface pitch, 64 align
                  half_pitch_chroma   << 2   |  // half pitch for chrome
                  !!tiling            << 1   |  // tiled surface, linear surface used
                  (tiling == I915_TILING_Y));   // tiled walk, ignored when liner surface

    OUT_VEB_BATCH(batch,
                  0 << 16  |     // X offset for V(Cb)
                  u_offset_y);   // Y offset for V(Cb)

    OUT_VEB_BATCH(batch,
                  0 << 16  |     // X offset for V(Cr)
                  v_offset_y );  // Y offset for V(Cr)

    OUT_VEB_BATCH(batch, 0);

    OUT_VEB_BATCH(batch, derived_pitch - 1);

    OUT_VEB_BATCH(batch, 0);

    ADVANCE_VEB_BATCH(batch);
}

VAStatus
gen9_vebox_process_picture(VADriverContextP ctx,
    struct intel_vebox_context *proc_ctx)
{
    VAStatus status;

    status = gen75_vebox_init_pipe_params(ctx, proc_ctx);
    if (status != VA_STATUS_SUCCESS)
        return status;

    status = gen75_vebox_init_filter_params(ctx, proc_ctx);
    if (status != VA_STATUS_SUCCESS)
        return status;

    hsw_veb_pre_format_convert(ctx, proc_ctx);

    status = gen75_vebox_ensure_surfaces(ctx, proc_ctx);
    if (status != VA_STATUS_SUCCESS)
        return status;

    status = gen75_vebox_ensure_surfaces_storage(ctx, proc_ctx);
    if (status != VA_STATUS_SUCCESS)
        return status;

    if (proc_ctx->format_convert_flags & POST_COPY_CONVERT) {
        assert(proc_ctx->is_second_field);
        /* directly copy the saved frame in the second call */
    } else {
        intel_batchbuffer_start_atomic_veb(proc_ctx->batch, 0x1000);
        intel_batchbuffer_emit_mi_flush(proc_ctx->batch);
        skl_veb_state_table_setup(ctx, proc_ctx);
        skl_veb_state_command(ctx, proc_ctx);
        skl_veb_surface_state(ctx, proc_ctx, INPUT_SURFACE);
        skl_veb_surface_state(ctx, proc_ctx, OUTPUT_SURFACE);
        bdw_veb_dndi_iecp_command(ctx, proc_ctx);
        intel_batchbuffer_end_atomic(proc_ctx->batch);
        intel_batchbuffer_flush(proc_ctx->batch);
    }

    hsw_veb_post_format_convert(ctx, proc_ctx);

    return VA_STATUS_SUCCESS;
}
