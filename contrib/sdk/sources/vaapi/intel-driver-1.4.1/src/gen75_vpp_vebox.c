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


VAStatus vpp_surface_convert(VADriverContextP ctx,
                             struct object_surface *src_obj_surf,
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

VAStatus vpp_surface_scaling(VADriverContextP ctx,
                             struct object_surface *dst_obj_surf,
                             struct object_surface *src_obj_surf)
{
    VAStatus va_status = VA_STATUS_SUCCESS;
    int flags = I965_PP_FLAG_AVS;

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
    int progressive_dn = 1;
    int dndi_top_first = 0;
    int motion_compensated_enable = 0;

    if (proc_ctx->filters_mask & VPP_DNDI_DI) {
        VAProcFilterParameterBufferDeinterlacing *di_param =
            (VAProcFilterParameterBufferDeinterlacing *)proc_ctx->filter_di;
        assert(di_param);

        progressive_dn = 0;
        dndi_top_first = !(di_param->flags & VA_DEINTERLACING_BOTTOM_FIELD);
        motion_compensated_enable = (di_param->algorithm == VAProcDeinterlacingMotionCompensated);
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
                    motion_compensated_enable  << 7   |  // MCDI Enable, use motion compensated deinterlace algorithm
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

    if (IS_GEN8(i965->intel.device_info))
        *p_table ++ = 0;         // parameters for hot pixel, 
}

void hsw_veb_iecp_std_table(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    unsigned int *p_table = proc_ctx->iecp_state_table.ptr + 0 ;
    //VAProcFilterParameterBuffer * std_param =
    //        (VAProcFilterParameterBuffer *) proc_ctx->filter_std;

    if(!(proc_ctx->filters_mask & VPP_IECP_STD_STE)){ 
        memset(p_table, 0, 29 * 4);
    }else{
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
                        384 );         // HUES

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
    if(proc_ctx->filters_mask & 0x000000ff) {
        dri_bo *dndi_bo = proc_ctx->dndi_state_table.bo;
        dri_bo_map(dndi_bo, 1);
        proc_ctx->dndi_state_table.ptr = dndi_bo->virtual;

        hsw_veb_dndi_table(ctx, proc_ctx);

        dri_bo_unmap(dndi_bo);
    }

    if(proc_ctx->filters_mask & 0x0000ff00) {
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
    unsigned int is_dn_enabled   = (proc_ctx->filters_mask & 0x01)? 1: 0;
    unsigned int is_di_enabled   = (proc_ctx->filters_mask & 0x02)? 1: 0;
    unsigned int is_iecp_enabled = (proc_ctx->filters_mask & 0xff00)?1:0;
    unsigned int is_first_frame  = !!((proc_ctx->frame_order == -1) &&
                                      (is_di_enabled ||
                                       is_dn_enabled));
    unsigned int di_output_frames_flag = 2; /* Output Current Frame Only */

    if(proc_ctx->fourcc_input != proc_ctx->fourcc_output ||
       (is_dn_enabled == 0 && is_di_enabled == 0)){
       is_iecp_enabled = 1;
    }

    if (is_di_enabled) {
        VAProcFilterParameterBufferDeinterlacing *di_param =
            (VAProcFilterParameterBufferDeinterlacing *)proc_ctx->filter_di;

        assert(di_param);
        
        if (di_param->algorithm == VAProcDeinterlacingBob)
            is_first_frame = 1;

        if ((di_param->algorithm == VAProcDeinterlacingMotionAdaptive ||
            di_param->algorithm == VAProcDeinterlacingMotionCompensated) &&
            proc_ctx->frame_order != -1)
            di_output_frames_flag = 0; /* Output both Current Frame and Previous Frame */
    }

    BEGIN_VEB_BATCH(batch, 6);
    OUT_VEB_BATCH(batch, VEB_STATE | (6 - 2));
    OUT_VEB_BATCH(batch,
                  0 << 26 |       // state surface control bits
                  0 << 11 |       // reserved.
                  0 << 10 |       // pipe sync disable
                  di_output_frames_flag << 8  |       // DI output frame
                  1 << 7  |       // 444->422 downsample method
                  1 << 6  |       // 422->420 downsample method
                  is_first_frame  << 5  |   // DN/DI first frame
                  is_di_enabled   << 4  |             // DI enable
                  is_dn_enabled   << 3  |             // DN enable
                  is_iecp_enabled << 2  |             // global IECP enabled
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
    unsigned int startingX = 0;
    unsigned int endingX = (proc_ctx->width_input + 63 ) / 64 * 64;

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
    OUT_VEB_BATCH(batch,
                  startingX << 16 |
                  (endingX-1));
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

void hsw_veb_resource_prepare(VADriverContextP ctx,
                              struct intel_vebox_context *proc_ctx)
{
    VAStatus va_status;
    dri_bo *bo;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    unsigned int input_fourcc, output_fourcc;
    unsigned int input_sampling, output_sampling;
    unsigned int input_tiling, output_tiling;
    unsigned int i, swizzle;
    struct object_surface *obj_surf_out = NULL, *obj_surf_in = NULL;

    if (proc_ctx->surface_input_vebox_object != NULL) {
        obj_surf_in = proc_ctx->surface_input_vebox_object;
    } else {
        obj_surf_in = proc_ctx->surface_input_object;
    } 

    if (proc_ctx->surface_output_vebox_object != NULL) {
        obj_surf_out = proc_ctx->surface_output_vebox_object;
    } else {
        obj_surf_out = proc_ctx->surface_output_object;
    } 

    if(obj_surf_in->bo == NULL){
          input_fourcc = VA_FOURCC_NV12;
          input_sampling = SUBSAMPLE_YUV420;
          input_tiling = 0;
          i965_check_alloc_surface_bo(ctx, obj_surf_in, input_tiling, input_fourcc, input_sampling);
    } else {
        input_fourcc = obj_surf_in->fourcc;
        input_sampling = obj_surf_in->subsampling;
        dri_bo_get_tiling(obj_surf_in->bo, &input_tiling, &swizzle);
        input_tiling = !!input_tiling;
    }

    if(obj_surf_out->bo == NULL){
          output_fourcc = VA_FOURCC_NV12;
          output_sampling = SUBSAMPLE_YUV420;
          output_tiling = 0;
          i965_check_alloc_surface_bo(ctx, obj_surf_out, output_tiling, output_fourcc, output_sampling);
    }else {
        output_fourcc   = obj_surf_out->fourcc;
        output_sampling = obj_surf_out->subsampling;
        dri_bo_get_tiling(obj_surf_out->bo, &output_tiling, &swizzle);
        output_tiling = !!output_tiling;
    }

    /* vebox pipelien input surface format info */
    proc_ctx->fourcc_input = input_fourcc;
    proc_ctx->fourcc_output = output_fourcc;
   
    /* create pipeline surfaces */
    for(i = 0; i < FRAME_STORE_SUM; i ++) {
        if(proc_ctx->frame_store[i].obj_surface){
            continue; //refer external surface for vebox pipeline
        }
    
        VASurfaceID new_surface;
        struct object_surface *obj_surf = NULL;

        va_status =   i965_CreateSurfaces(ctx,
                                          proc_ctx ->width_input,
                                          proc_ctx ->height_input,
                                          VA_RT_FORMAT_YUV420,
                                          1,
                                          &new_surface);
        assert(va_status == VA_STATUS_SUCCESS);

        obj_surf = SURFACE(new_surface);
        assert(obj_surf);

        if( i <= FRAME_IN_PREVIOUS || i == FRAME_OUT_CURRENT_DN) {
            i965_check_alloc_surface_bo(ctx, obj_surf, input_tiling, input_fourcc, input_sampling);
        } else if( i == FRAME_IN_STMM || i == FRAME_OUT_STMM){
            i965_check_alloc_surface_bo(ctx, obj_surf, 1, input_fourcc, input_sampling);
        } else if( i >= FRAME_OUT_CURRENT){
            i965_check_alloc_surface_bo(ctx, obj_surf, output_tiling, output_fourcc, output_sampling);
        }

        proc_ctx->frame_store[i].surface_id = new_surface;
        proc_ctx->frame_store[i].is_internal_surface = 1;
        proc_ctx->frame_store[i].obj_surface = obj_surf;
    }

    /* alloc dndi state table  */
    dri_bo_unreference(proc_ctx->dndi_state_table.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "vebox: dndi state Buffer",
                      0x1000, 0x1000);
    proc_ctx->dndi_state_table.bo = bo;
    dri_bo_reference(proc_ctx->dndi_state_table.bo);
 
    /* alloc iecp state table  */
    dri_bo_unreference(proc_ctx->iecp_state_table.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "vebox: iecp state Buffer",
                      0x1000, 0x1000);
    proc_ctx->iecp_state_table.bo = bo;
    dri_bo_reference(proc_ctx->iecp_state_table.bo);

    /* alloc gamut state table  */
    dri_bo_unreference(proc_ctx->gamut_state_table.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "vebox: gamut state Buffer",
                      0x1000, 0x1000);
    proc_ctx->gamut_state_table.bo = bo;
    dri_bo_reference(proc_ctx->gamut_state_table.bo);

    /* alloc vertex state table  */
    dri_bo_unreference(proc_ctx->vertex_state_table.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "vertex: iecp state Buffer",
                      0x1000, 0x1000);
    proc_ctx->vertex_state_table.bo = bo;
    dri_bo_reference(proc_ctx->vertex_state_table.bo);

}

static VAStatus
hsw_veb_surface_reference(VADriverContextP ctx,
                          struct intel_vebox_context *proc_ctx)
{
    struct object_surface * obj_surf; 
    VEBFrameStore tmp_store;

    if (proc_ctx->surface_input_vebox_object != NULL) {
        obj_surf = proc_ctx->surface_input_vebox_object;
    } else {
        obj_surf = proc_ctx->surface_input_object;
    } 

    /* update the input surface */ 
    proc_ctx->frame_store[FRAME_IN_CURRENT].surface_id = VA_INVALID_ID;
    proc_ctx->frame_store[FRAME_IN_CURRENT].is_internal_surface = 0;
    proc_ctx->frame_store[FRAME_IN_CURRENT].obj_surface = obj_surf;

    /* update the previous input surface */
    if (proc_ctx->frame_order != -1) {
        if (proc_ctx->filters_mask == VPP_DNDI_DN) {
            proc_ctx->frame_store[FRAME_IN_PREVIOUS] = proc_ctx->frame_store[FRAME_OUT_CURRENT_DN];
        } else if (proc_ctx->filters_mask & VPP_DNDI_DI) {
            VAProcFilterParameterBufferDeinterlacing *di_param =
                (VAProcFilterParameterBufferDeinterlacing *)proc_ctx->filter_di;

            if (di_param && 
                (di_param->algorithm == VAProcDeinterlacingMotionAdaptive ||
                di_param->algorithm == VAProcDeinterlacingMotionCompensated)) {
                if ((proc_ctx->filters_mask & VPP_DNDI_DN) &&
                    proc_ctx->frame_order == 0) { /* DNDI */
                    tmp_store = proc_ctx->frame_store[FRAME_OUT_CURRENT_DN];
                    proc_ctx->frame_store[FRAME_OUT_CURRENT_DN] = proc_ctx->frame_store[FRAME_IN_PREVIOUS];
                    proc_ctx->frame_store[FRAME_IN_PREVIOUS] = tmp_store;
                } else { /* DI only */
                    VAProcPipelineParameterBuffer *pipe = proc_ctx->pipeline_param;
                    struct object_surface *obj_surf = NULL;
                    struct i965_driver_data * const i965 = i965_driver_data(ctx);

                    if (!pipe ||
                        !pipe->num_forward_references ||
                        pipe->forward_references[0] == VA_INVALID_ID) {
                        WARN_ONCE("A forward temporal reference is needed for Motion adaptive/compensated deinterlacing !!!\n");

                        return VA_STATUS_ERROR_INVALID_PARAMETER;
                    }

                    obj_surf = SURFACE(pipe->forward_references[0]);
                    assert(obj_surf && obj_surf->bo);
                
                    proc_ctx->frame_store[FRAME_IN_PREVIOUS].surface_id = pipe->forward_references[0];
                    proc_ctx->frame_store[FRAME_IN_PREVIOUS].is_internal_surface = 0;
                    proc_ctx->frame_store[FRAME_IN_PREVIOUS].obj_surface = obj_surf;
                }
            }
        }
    }

    /* update STMM surface */
    if (proc_ctx->frame_order != -1) {
        tmp_store = proc_ctx->frame_store[FRAME_IN_STMM];
        proc_ctx->frame_store[FRAME_IN_STMM] = proc_ctx->frame_store[FRAME_OUT_STMM];
        proc_ctx->frame_store[FRAME_OUT_STMM] = tmp_store;
    }

    /* update the output surface */ 
    if (proc_ctx->surface_output_vebox_object != NULL) {
        obj_surf = proc_ctx->surface_output_vebox_object;
    } else {
        obj_surf = proc_ctx->surface_output_object;
    } 

    if (proc_ctx->filters_mask == VPP_DNDI_DN) {
        proc_ctx->frame_store[FRAME_OUT_CURRENT_DN].surface_id = VA_INVALID_ID;
        proc_ctx->frame_store[FRAME_OUT_CURRENT_DN].is_internal_surface = 0;
        proc_ctx->frame_store[FRAME_OUT_CURRENT_DN].obj_surface = obj_surf;
        proc_ctx->current_output = FRAME_OUT_CURRENT_DN;
    } else if (proc_ctx->filters_mask & VPP_DNDI_DI) {
        VAProcFilterParameterBufferDeinterlacing *di_param =
            (VAProcFilterParameterBufferDeinterlacing *)proc_ctx->filter_di;

        if (di_param && 
            (di_param->algorithm == VAProcDeinterlacingMotionAdaptive ||
            di_param->algorithm == VAProcDeinterlacingMotionCompensated)) {
            if (proc_ctx->frame_order == -1) {
                proc_ctx->frame_store[FRAME_OUT_CURRENT].surface_id = VA_INVALID_ID;
                proc_ctx->frame_store[FRAME_OUT_CURRENT].is_internal_surface = 0;
                proc_ctx->frame_store[FRAME_OUT_CURRENT].obj_surface = obj_surf;
                proc_ctx->current_output = FRAME_OUT_CURRENT;
            } else if (proc_ctx->frame_order == 0) {
                proc_ctx->frame_store[FRAME_OUT_PREVIOUS].surface_id = VA_INVALID_ID;
                proc_ctx->frame_store[FRAME_OUT_PREVIOUS].is_internal_surface = 0;
                proc_ctx->frame_store[FRAME_OUT_PREVIOUS].obj_surface = obj_surf;
                proc_ctx->current_output = FRAME_OUT_PREVIOUS;
            } else {
                proc_ctx->current_output = FRAME_OUT_CURRENT;
                proc_ctx->format_convert_flags |= POST_COPY_CONVERT;
            }
        } else {
            proc_ctx->frame_store[FRAME_OUT_CURRENT].surface_id = VA_INVALID_ID;
            proc_ctx->frame_store[FRAME_OUT_CURRENT].is_internal_surface = 0;
            proc_ctx->frame_store[FRAME_OUT_CURRENT].obj_surface = obj_surf;
            proc_ctx->current_output = FRAME_OUT_CURRENT;
        }
    } else {
        proc_ctx->frame_store[FRAME_OUT_CURRENT].surface_id = VA_INVALID_ID;
        proc_ctx->frame_store[FRAME_OUT_CURRENT].is_internal_surface = 0;
        proc_ctx->frame_store[FRAME_OUT_CURRENT].obj_surface = obj_surf;
        proc_ctx->current_output = FRAME_OUT_CURRENT;
    }

    return VA_STATUS_SUCCESS;
}

void hsw_veb_surface_unreference(VADriverContextP ctx,
                                 struct intel_vebox_context *proc_ctx)
{
    /* unreference the input surface */ 
    proc_ctx->frame_store[FRAME_IN_CURRENT].surface_id = VA_INVALID_ID;
    proc_ctx->frame_store[FRAME_IN_CURRENT].is_internal_surface = 0;
    proc_ctx->frame_store[FRAME_IN_CURRENT].obj_surface = NULL;

    /* unreference the shared output surface */ 
    if (proc_ctx->filters_mask == VPP_DNDI_DN) {
        proc_ctx->frame_store[FRAME_OUT_CURRENT_DN].surface_id = VA_INVALID_ID;
        proc_ctx->frame_store[FRAME_OUT_CURRENT_DN].is_internal_surface = 0;
        proc_ctx->frame_store[FRAME_OUT_CURRENT_DN].obj_surface = NULL;
    } else {
        proc_ctx->frame_store[FRAME_OUT_CURRENT].surface_id = VA_INVALID_ID;
        proc_ctx->frame_store[FRAME_OUT_CURRENT].is_internal_surface = 0;
        proc_ctx->frame_store[FRAME_OUT_CURRENT].obj_surface = NULL;
    }
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
        obj_surf_input->fourcc ==  VA_FOURCC_RGBA){

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
       
         vpp_surface_convert(ctx, proc_ctx->surface_input_vebox_object, proc_ctx->surface_input_object);
      }

      /* create one temporary NV12 surfaces for conversion*/
     if(obj_surf_output->fourcc ==  VA_FOURCC_YV12 ||
        obj_surf_output->fourcc ==  VA_FOURCC_I420 ||
        obj_surf_output->fourcc ==  VA_FOURCC_IMC1 ||
        obj_surf_output->fourcc ==  VA_FOURCC_IMC3 ||
        obj_surf_output->fourcc ==  VA_FOURCC_RGBA) {

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
        vpp_surface_convert(ctx,proc_ctx->surface_output_object, obj_surface);
    } else if(!(proc_ctx->format_convert_flags & POST_FORMAT_CONVERT) &&
       !(proc_ctx->format_convert_flags & POST_SCALING_CONVERT)){
        /* Output surface format is covered by vebox pipeline and 
         * processed picture is already store in output surface 
         * so nothing will be done here */
    } else if ((proc_ctx->format_convert_flags & POST_FORMAT_CONVERT) &&
               !(proc_ctx->format_convert_flags & POST_SCALING_CONVERT)){
       /* convert and copy NV12 to YV12/IMC3/IMC2/RGBA output*/
        vpp_surface_convert(ctx,proc_ctx->surface_output_object, obj_surface);

    } else if(proc_ctx->format_convert_flags & POST_SCALING_CONVERT) {
       /* scaling, convert and copy NV12 to YV12/IMC3/IMC2/RGBA output*/
        assert(obj_surface->fourcc == VA_FOURCC_NV12);
     
        /* first step :surface scaling */
        vpp_surface_scaling(ctx,proc_ctx->surface_output_scaled_object, obj_surface);

        /* second step: color format convert and copy to output */
        obj_surface = proc_ctx->surface_output_object;

        if(obj_surface->fourcc ==  VA_FOURCC_NV12 ||
           obj_surface->fourcc ==  VA_FOURCC_YV12 ||
           obj_surface->fourcc ==  VA_FOURCC_I420 ||
           obj_surface->fourcc ==  VA_FOURCC_YUY2 ||
           obj_surface->fourcc ==  VA_FOURCC_IMC1 ||
           obj_surface->fourcc ==  VA_FOURCC_IMC3 ||
           obj_surface->fourcc ==  VA_FOURCC_RGBA) {
           vpp_surface_convert(ctx, proc_ctx->surface_output_object, proc_ctx->surface_output_scaled_object);
       }else {
           assert(0); 
       }
   }

    return 0;
}

VAStatus gen75_vebox_process_picture(VADriverContextP ctx,
                         struct intel_vebox_context *proc_ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
 
    VAProcPipelineParameterBuffer *pipe = proc_ctx->pipeline_param;
    VAProcFilterParameterBuffer* filter = NULL;
    struct object_buffer *obj_buf = NULL;
    unsigned int i;

    for (i = 0; i < pipe->num_filters; i ++) {
         obj_buf = BUFFER(pipe->filters[i]);
         
         assert(obj_buf && obj_buf->buffer_store);

         if (!obj_buf || !obj_buf->buffer_store)
             goto error;

         filter = (VAProcFilterParameterBuffer*)obj_buf-> buffer_store->buffer;
            
         if (filter->type == VAProcFilterNoiseReduction) {
             proc_ctx->filters_mask |= VPP_DNDI_DN;
             proc_ctx->filter_dn = filter;
         } else if (filter->type == VAProcFilterDeinterlacing) {
             proc_ctx->filters_mask |= VPP_DNDI_DI;
             proc_ctx->filter_di = filter;
         } else if (filter->type == VAProcFilterColorBalance) {
             proc_ctx->filters_mask |= VPP_IECP_PRO_AMP;
             proc_ctx->filter_iecp_amp = filter;
             proc_ctx->filter_iecp_amp_num_elements = obj_buf->num_elements;
         } else if (filter->type == VAProcFilterSkinToneEnhancement) {
             proc_ctx->filters_mask |= VPP_IECP_STD_STE;
             proc_ctx->filter_iecp_std = filter;
         }
    }

    hsw_veb_pre_format_convert(ctx, proc_ctx);
    hsw_veb_surface_reference(ctx, proc_ctx);

    if (proc_ctx->frame_order == -1) {
        hsw_veb_resource_prepare(ctx, proc_ctx);
    }

    if (proc_ctx->format_convert_flags & POST_COPY_CONVERT) {
        assert(proc_ctx->frame_order == 1);
        /* directly copy the saved frame in the second call */
    } else {
        intel_batchbuffer_start_atomic_veb(proc_ctx->batch, 0x1000);
        intel_batchbuffer_emit_mi_flush(proc_ctx->batch);
        hsw_veb_surface_state(ctx, proc_ctx, INPUT_SURFACE); 
        hsw_veb_surface_state(ctx, proc_ctx, OUTPUT_SURFACE); 
        hsw_veb_state_table_setup(ctx, proc_ctx);

        hsw_veb_state_command(ctx, proc_ctx);		
        hsw_veb_dndi_iecp_command(ctx, proc_ctx);
        intel_batchbuffer_end_atomic(proc_ctx->batch);
        intel_batchbuffer_flush(proc_ctx->batch);
    }

    hsw_veb_post_format_convert(ctx, proc_ctx);
    // hsw_veb_surface_unreference(ctx, proc_ctx);

    proc_ctx->frame_order = (proc_ctx->frame_order + 1) % 2;
     
    return VA_STATUS_SUCCESS;

error:
    return VA_STATUS_ERROR_INVALID_PARAMETER;
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

    for(i = 0; i < FRAME_STORE_SUM; i ++) {
        if (proc_ctx->frame_store[i].is_internal_surface == 1) {
            assert(proc_ctx->frame_store[i].surface_id != VA_INVALID_ID);

            if (proc_ctx->frame_store[i].surface_id != VA_INVALID_ID)
                i965_DestroySurfaces(ctx, &proc_ctx->frame_store[i].surface_id, 1);
        }

        proc_ctx->frame_store[i].surface_id = VA_INVALID_ID;
        proc_ctx->frame_store[i].is_internal_surface = 0;
        proc_ctx->frame_store[i].obj_surface = NULL;
    }

    /* dndi state table  */
    dri_bo_unreference(proc_ctx->dndi_state_table.bo);
    proc_ctx->dndi_state_table.bo = NULL;

    /* iecp state table  */
    dri_bo_unreference(proc_ctx->iecp_state_table.bo);
    proc_ctx->dndi_state_table.bo = NULL;
 
    /* gamut statu table */
    dri_bo_unreference(proc_ctx->gamut_state_table.bo);
    proc_ctx->gamut_state_table.bo = NULL;

    /* vertex state table  */
    dri_bo_unreference(proc_ctx->vertex_state_table.bo);
    proc_ctx->vertex_state_table.bo = NULL;

    intel_batchbuffer_free(proc_ctx->batch);

    free(proc_ctx);
}

struct intel_vebox_context * gen75_vebox_context_init(VADriverContextP ctx)
{
    struct intel_driver_data *intel = intel_driver_data(ctx);
    struct intel_vebox_context *proc_context = calloc(1, sizeof(struct intel_vebox_context));
    int i;

    proc_context->batch = intel_batchbuffer_new(intel, I915_EXEC_VEBOX, 0);
    memset(proc_context->frame_store, 0, sizeof(VEBFrameStore)*FRAME_STORE_SUM);

    for (i = 0; i < FRAME_STORE_SUM; i ++) {
        proc_context->frame_store[i].surface_id = VA_INVALID_ID;
        proc_context->frame_store[i].is_internal_surface = 0;
        proc_context->frame_store[i].obj_surface = NULL;
    }
  
    proc_context->filters_mask          = 0;
    proc_context->frame_order           = -1; /* the first frame */
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
    unsigned int is_dn_enabled   = (proc_ctx->filters_mask & 0x01)? 1: 0;
    unsigned int is_di_enabled   = (proc_ctx->filters_mask & 0x02)? 1: 0;
    unsigned int is_iecp_enabled = (proc_ctx->filters_mask & 0xff00)?1:0;
    unsigned int is_first_frame  = !!((proc_ctx->frame_order == -1) &&
                                      (is_di_enabled ||
                                       is_dn_enabled));
    unsigned int di_output_frames_flag = 2; /* Output Current Frame Only */

    if(proc_ctx->fourcc_input != proc_ctx->fourcc_output ||
       (is_dn_enabled == 0 && is_di_enabled == 0)){
       is_iecp_enabled = 1;
    }

    if (is_di_enabled) {
        VAProcFilterParameterBufferDeinterlacing *di_param =
            (VAProcFilterParameterBufferDeinterlacing *)proc_ctx->filter_di;

        assert(di_param);
        
        if (di_param->algorithm == VAProcDeinterlacingBob)
            is_first_frame = 1;

        if ((di_param->algorithm == VAProcDeinterlacingMotionAdaptive ||
            di_param->algorithm == VAProcDeinterlacingMotionCompensated) &&
            proc_ctx->frame_order != -1)
            di_output_frames_flag = 0; /* Output both Current Frame and Previous Frame */
    }

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
                  di_output_frames_flag << 8  |       // DI output frame
                  1 << 7  |       // 444->422 downsample method
                  1 << 6  |       // 422->420 downsample method
                  is_first_frame  << 5  |   // DN/DI first frame
                  is_di_enabled   << 4  |             // DI enable
                  is_dn_enabled   << 3  |             // DN enable
                  is_iecp_enabled << 2  |             // global IECP enabled
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
    unsigned int startingX = 0;
    unsigned int endingX = (proc_ctx->width_input + 63 ) / 64 * 64;

    BEGIN_VEB_BATCH(batch, 0x14);
    OUT_VEB_BATCH(batch, VEB_DNDI_IECP_STATE | (0x14 - 2));//DWord 0
    OUT_VEB_BATCH(batch,
                  startingX << 16 |
                  endingX -1);//DWord 1

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

VAStatus gen8_vebox_process_picture(VADriverContextP ctx,
                         struct intel_vebox_context *proc_ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
 
    VAProcPipelineParameterBuffer *pipe = proc_ctx->pipeline_param;
    VAProcFilterParameterBuffer* filter = NULL;
    struct object_buffer *obj_buf = NULL;
    unsigned int i;

    for (i = 0; i < pipe->num_filters; i ++) {
         obj_buf = BUFFER(pipe->filters[i]);
         
         assert(obj_buf && obj_buf->buffer_store);

         if (!obj_buf || !obj_buf->buffer_store)
             goto error;

         filter = (VAProcFilterParameterBuffer*)obj_buf-> buffer_store->buffer;
            
         if (filter->type == VAProcFilterNoiseReduction) {
             proc_ctx->filters_mask |= VPP_DNDI_DN;
             proc_ctx->filter_dn = filter;
         } else if (filter->type == VAProcFilterDeinterlacing) {
             proc_ctx->filters_mask |= VPP_DNDI_DI;
             proc_ctx->filter_di = filter;
         } else if (filter->type == VAProcFilterColorBalance) {
             proc_ctx->filters_mask |= VPP_IECP_PRO_AMP;
             proc_ctx->filter_iecp_amp = filter;
             proc_ctx->filter_iecp_amp_num_elements = obj_buf->num_elements;
         } else if (filter->type == VAProcFilterSkinToneEnhancement) {
             proc_ctx->filters_mask |= VPP_IECP_STD_STE;
             proc_ctx->filter_iecp_std = filter;
         }
    }

    hsw_veb_pre_format_convert(ctx, proc_ctx);
    hsw_veb_surface_reference(ctx, proc_ctx);

    if (proc_ctx->frame_order == -1) {
        hsw_veb_resource_prepare(ctx, proc_ctx);
    }

    if (proc_ctx->format_convert_flags & POST_COPY_CONVERT) {
        assert(proc_ctx->frame_order == 1);
        /* directly copy the saved frame in the second call */
    } else {
        intel_batchbuffer_start_atomic_veb(proc_ctx->batch, 0x1000);
        intel_batchbuffer_emit_mi_flush(proc_ctx->batch);
        hsw_veb_surface_state(ctx, proc_ctx, INPUT_SURFACE); 
        hsw_veb_surface_state(ctx, proc_ctx, OUTPUT_SURFACE); 
        hsw_veb_state_table_setup(ctx, proc_ctx);

        bdw_veb_state_command(ctx, proc_ctx);		
        bdw_veb_dndi_iecp_command(ctx, proc_ctx);
        intel_batchbuffer_end_atomic(proc_ctx->batch);
        intel_batchbuffer_flush(proc_ctx->batch);
    }

    hsw_veb_post_format_convert(ctx, proc_ctx);
    // hsw_veb_surface_unreference(ctx, proc_ctx);

    proc_ctx->frame_order = (proc_ctx->frame_order + 1) % 2;
     
    return VA_STATUS_SUCCESS;

error:
    return VA_STATUS_ERROR_INVALID_PARAMETER;
}

