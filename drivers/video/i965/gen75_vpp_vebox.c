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

#define PI  3.1415926

extern VAStatus 
i965_CreateSurfaces(VADriverContextP ctx,
                    int width,
                    int height,
                    int format,
                    int num_surfaces,
                    VASurfaceID *surfaces);      

int format_convert(float src, int out_int_bits, int out_frac_bits,int out_sign_flag)
{
     unsigned char negative_flag = (src < 0.0) ? 1 : 0;
     float src_1 = (!negative_flag)? src: -src ;
     unsigned int factor = 1 << out_frac_bits;
     int output_value = 0;         
 
     unsigned int integer_part = 0;//floor(src_1);
     unsigned int fraction_part = ((int)((src_1 - integer_part) * factor)) & (factor - 1) ;

     output_value = (integer_part << out_frac_bits) | fraction_part;

     if(negative_flag)
         output_value = (~output_value + 1) & ((1 <<(out_int_bits + out_frac_bits)) -1);

     if(out_sign_flag == 1 && negative_flag)
     {
          output_value |= negative_flag <<(out_int_bits + out_frac_bits);
     }
     return output_value;
}

void hsw_veb_dndi_table(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    unsigned int* p_table ;
    /*
    VAProcFilterParameterBufferDeinterlacing *di_param =
            (VAProcFilterParameterBufferDeinterlacing *) proc_ctx->filter_di;

    VAProcFilterParameterBuffer * dn_param =
            (VAProcFilterParameterBuffer *) proc_ctx->filter_dn;
    */
    p_table = (unsigned int *)proc_ctx->dndi_state_table.ptr;

    *p_table ++ = 0;               // reserved  . w0
    *p_table ++ = ( 0   << 24 |    // denoise STAD threshold . w1
                    128 << 16 |    // dnmh_history_max
                    0   << 12 |    // reserved
                    8   << 8  |    // dnmh_delta[3:0]
                    0 );           // denoise ASD threshold

    *p_table ++ = ( 0  << 30 |    // reserved . w2
                    16 << 24 |    // temporal diff th
                    0  << 22 |    // reserved.
                    8  << 16 |    // low temporal diff th
                    0  << 13 |    // STMM C2
                    0  << 8  |    // denoise moving pixel th
                    64 );         // denoise th for sum of complexity measure

    *p_table ++ = ( 0 << 30  |   // reserved . w3
                    4 << 24  |   // good neighbor th[5:0]
                    9 << 20  |   // CAT slope minus 1
                    5 << 16  |   // SAD Tight in
                    0 << 14  |   // smooth mv th
                    0 << 12  |   // reserved
                    1 << 8   |   // bne_edge_th[3:0]
                    15 );        // block noise estimate noise th

    *p_table ++ = ( 0  << 31  |  // STMM blending constant select. w4
                    64 << 24  |  // STMM trc1
                    0  << 16  |  // STMM trc2
                    0  << 14  |  // reserved
                    2  << 8   |  // VECM_mul
                    128 );       // maximum STMM

    *p_table ++ = ( 0  << 24  |  // minumum STMM  . W5
                    0  << 22  |  // STMM shift down
                    0  << 20  |  // STMM shift up
                    7  << 16  |  // STMM output shift
                    128 << 8  |  // SDI threshold
                    8 );         // SDI delta

    *p_table ++ = ( 0 << 24  |   // SDI fallback mode 1 T1 constant . W6
                    0 << 16  |   // SDI fallback mode 1 T2 constant
                    0 << 8   |   // SDI fallback mode 2 constant(angle2x1)
                    0 );         // FMD temporal difference threshold

    *p_table ++ = ( 32 << 24  |  // FMD #1 vertical difference th . w7
                    32 << 16  |  // FMD #2 vertical difference th
                    1  << 14  |  // CAT th1
                    32 << 8   |  // FMD tear threshold
                    0  << 7   |  // MCDI Enable, use motion compensated deinterlace algorithm
                    0  << 6   |  // progressive DN
                    0  << 4   |  // reserved
                    0  << 3   |  // DN/DI Top First
                    0 );         // reserved

    *p_table ++ = ( 0  << 29  |  // reserved . W8
                    0  << 23  |  // dnmh_history_init[5:0]
                    10 << 19  |  // neighborPixel th
                    0  << 18  |  // reserved
                    0  << 16  |  // FMD for 2nd field of previous frame
                    25 << 10  |  // MC pixel consistency th
                    0  << 8   |  // FMD for 1st field for current frame
                    10 << 4   |  // SAD THB
                    5 );         // SAD THA

    *p_table ++ = ( 0 << 24  |  // reserved
                    0 << 16  |  // chr_dnmh_stad_th
                    0 << 13  |  // reserved
                    0 << 12  |  // chrome denoise enable
                    0 << 6   |  // chr temp diff th
                    0 );        // chr temp diff low

}

void hsw_veb_iecp_std_table(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    unsigned int *p_table = proc_ctx->iecp_state_table.ptr + 0 ;
    /*
      VAProcFilterParameterBuffer * std_param =
             (VAProcFilterParameterBuffer *) proc_ctx->filter_std;
    */
    if(!(proc_ctx->filters_mask & VPP_IECP_STD_STE)){ 
        memset(p_table, 0, 29 * 4);
    }else{
        *p_table ++ = 0x9a6e39f0;
        *p_table ++ = 0x400c0000;
        *p_table ++ = 0x00001180;
        *p_table ++ = 0xfe2f2e00;
        *p_table ++ = 0x000000ff;

        *p_table ++ = 0x00140000;
        *p_table ++ = 0xd82e0000;
        *p_table ++ = 0x8285ecec;
        *p_table ++ = 0x00008282;
        *p_table ++ = 0x00000000;

        *p_table ++ = 0x02117000;
        *p_table ++ = 0xa38fec96;
        *p_table ++ = 0x0000c8c8;
        *p_table ++ = 0x00000000;
        *p_table ++ = 0x01478000;
 
        *p_table ++ = 0x0007c306;
        *p_table ++ = 0x00000000;
        *p_table ++ = 0x00000000;
        *p_table ++ = 0x1c1bd000;
        *p_table ++ = 0x00000000;

        *p_table ++ = 0x00000000;
        *p_table ++ = 0x00000000;
        *p_table ++ = 0x0007cf80;
        *p_table ++ = 0x00000000;
        *p_table ++ = 0x00000000;

        *p_table ++ = 0x1c080000;
        *p_table ++ = 0x00000000;
        *p_table ++ = 0x00000000;
        *p_table ++ = 0x00000000;
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
    /*
      VAProcFilterParameterBuffer * tcc_param =
              (VAProcFilterParameterBuffer *) proc_ctx->filter_iecp_tcc;
   */
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
        float  tmp_value = 0.0;
        float  src_saturation = 1.0;
        float  src_hue = 0.0;
        float  src_contrast   = 1.0;
        /*
        float  src_brightness = 0.0;

        VAProcFilterParameterBufferColorBalance * amp_param =
        (VAProcFilterParameterBufferColorBalance *) proc_ctx->filter_iecp_amp;
        VAProcColorBalanceType attrib = amp_param->attrib;

        if(attrib == VAProcColorBalanceHue) {
           src_hue = amp_param->value;         //(-180.0, 180.0)
        }else if(attrib == VAProcColorBalanceSaturation) {
           src_saturation = amp_param->value; //(0.0, 10.0)
        }else if(attrib == VAProcColorBalanceBrightness) {
           src_brightness = amp_param->value; // (-100.0, 100.0)
           brightness = format_convert(src_brightness, 7, 4, 1);
        }else if(attrib == VAProcColorBalanceContrast) {
           src_contrast = amp_param->value;  //  (0.0, 10.0)
           contrast = format_convert(src_contrast, 4, 7, 0);
        }
        */
        tmp_value = cos(src_hue/180*PI) * src_contrast * src_saturation;
        cos_c_s = format_convert(tmp_value, 7, 8, 1);
        
        tmp_value = sin(src_hue/180*PI) * src_contrast * src_saturation;
        sin_c_s = format_convert(tmp_value, 7, 8, 1);
     
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
    /*
    VAProcColorStandardType   in_color_std  = proc_ctx->pipeline_param->surface_color_standard;
    VAProcColorStandardType   out_color_std = proc_ctx->pipeline_param->output_color_standard;
    assert(in_color_std == out_color_std);  
    */
    if(proc_ctx->fourcc_input == VA_FOURCC('R','G','B','A') &&
       (proc_ctx->fourcc_output == VA_FOURCC('N','V','1','2') ||
        proc_ctx->fourcc_output == VA_FOURCC('Y','V','1','2') ||
        proc_ctx->fourcc_output == VA_FOURCC('Y','V','Y','2') ||
        proc_ctx->fourcc_output == VA_FOURCC('A','Y','U','V'))) {

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
    }else if((proc_ctx->fourcc_input  == VA_FOURCC('N','V','1','2') || 
              proc_ctx->fourcc_input  == VA_FOURCC('Y','V','1','2') || 
              proc_ctx->fourcc_input  == VA_FOURCC('Y','U','Y','2') ||
              proc_ctx->fourcc_input  == VA_FOURCC('A','Y','U','V'))&&
              proc_ctx->fourcc_output == VA_FOURCC('R','G','B','A')) {

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
                        format_convert(tran_coef[1], 2, 10, 1) << 16 | //c1, s2.10 format
                        format_convert(tran_coef[0], 2, 10, 1) << 3 |  //c0, s2.10 format
                        0 << 2 | //reserved
                        0 << 1 | // yuv_channel swap
                        is_transform_enabled);                

        *p_table ++ = ( 0 << 26 | //reserved
                        format_convert(tran_coef[3], 2, 10, 1) << 13 | 
                        format_convert(tran_coef[2], 2, 10, 1));
    
        *p_table ++ = ( 0 << 26 | //reserved
                        format_convert(tran_coef[5], 2, 10, 1) << 13 | 
                        format_convert(tran_coef[4], 2, 10, 1));

        *p_table ++ = ( 0 << 26 | //reserved
                        format_convert(tran_coef[7], 2, 10, 1) << 13 | 
                        format_convert(tran_coef[6], 2, 10, 1));

        *p_table ++ = ( 0 << 13 | //reserved
                        format_convert(tran_coef[8], 2, 10, 1));

        *p_table ++ = ( 0 << 22 | //reserved
                        format_convert(u_coef[0], 10, 0, 1) << 11 | 
                        format_convert(v_coef[0], 10, 0, 1));

        *p_table ++ = ( 0 << 22 | //reserved
                        format_convert(u_coef[1], 10, 0, 1) << 11 | 
                        format_convert(v_coef[1], 10, 0, 1));

        *p_table ++ = ( 0 << 22 | //reserved
                        format_convert(u_coef[2], 10, 0, 1) << 11 | 
                        format_convert(v_coef[2], 10, 0, 1));
    }
}

void hsw_veb_iecp_aoi_table(VADriverContextP ctx, struct intel_vebox_context *proc_ctx)
{
    unsigned int *p_table = (unsigned int*)(proc_ctx->iecp_state_table.ptr + 252);
    /*
     VAProcFilterParameterBuffer * tcc_param =
             (VAProcFilterParameterBuffer *) proc_ctx->filter_iecp_tcc;
    */
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

    if(proc_ctx->filters_mask & 0x0000ff00 ||
       proc_ctx->fourcc_input != proc_ctx->fourcc_output) {
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

    BEGIN_VEB_BATCH(batch, 6);
    OUT_VEB_BATCH(batch, VEB_STATE | (6 - 2));
    OUT_VEB_BATCH(batch,
                  0 << 26 |       // state surface control bits
                  0 << 11 |       // reserved.
                  0 << 10 |       // pipe sync disable
                  2 << 8  |       // DI output frame
                  0 << 7  |       // 444->422 downsample method
                  0 << 6  |       // 422->420 downsample method
                  !!(proc_ctx->is_first_frame && (is_di_enabled || is_dn_enabled)) << 5  |   // DN/DI first frame
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
    struct  i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = proc_ctx->batch;
    unsigned int u_offset_y = 0, v_offset_y = 0;
    unsigned int is_uv_interleaved = 0, tiling = 0, swizzle = 0;
    unsigned int surface_format = PLANAR_420_8;
    struct object_surface* obj_surf = NULL;
    unsigned int surface_pitch = 0;
    unsigned int half_pitch_chroma = 0;

    if(is_output){   
         obj_surf = SURFACE(proc_ctx->frame_store[FRAME_OUT_CURRENT].surface_id);
    }else {
         obj_surf = SURFACE(proc_ctx->frame_store[FRAME_IN_CURRENT].surface_id);
    }

    if (obj_surf->fourcc == VA_FOURCC_NV12) {
        surface_format = PLANAR_420_8;
        surface_pitch = obj_surf->width; 
        printf("NV12, is_output=%d, width = %d, pitch is =  %d\n",is_output, obj_surf->orig_width, obj_surf->width);
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
                  (proc_ctx->pic_height - 1) << 18 |  // height . w3
                  (proc_ctx->pic_width )  << 4  |  // width
                  0);                                 // reserve

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
    unsigned int endingX = proc_ctx->pic_width;

    BEGIN_VEB_BATCH(batch, 10);
    OUT_VEB_BATCH(batch, VEB_DNDI_IECP_STATE | (10 - 2));
    OUT_VEB_BATCH(batch,
                  startingX << 16 |
                  endingX);
    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_IN_CURRENT].bo,
              I915_GEM_DOMAIN_RENDER, 0, frame_ctrl_bits);
    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_IN_PREVIOUS].bo,
              I915_GEM_DOMAIN_RENDER, 0, frame_ctrl_bits);
    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_IN_STMM].bo,
              I915_GEM_DOMAIN_RENDER, 0, frame_ctrl_bits);
    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_OUT_STMM].bo,
              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER, frame_ctrl_bits);
    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_OUT_CURRENT_DN].bo,
              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER, frame_ctrl_bits);
    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_OUT_CURRENT].bo,
              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER, frame_ctrl_bits);
    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_OUT_PREVIOUS].bo,
              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER, frame_ctrl_bits);
    OUT_RELOC(batch,
              proc_ctx->frame_store[FRAME_OUT_STATISTIC].bo,
              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER, frame_ctrl_bits);

    ADVANCE_VEB_BATCH(batch);
}


void hsw_veb_surface_reference(VADriverContextP ctx,
                              struct intel_vebox_context *proc_ctx)
{
    struct object_surface * obj_surf; 
    struct i965_driver_data *i965 = i965_driver_data(ctx);

    /* update the input surface */ 
     obj_surf = SURFACE(proc_ctx->surface_input);
     proc_ctx->frame_store[FRAME_IN_CURRENT].surface_id = proc_ctx->surface_input;
     proc_ctx->frame_store[FRAME_IN_CURRENT].bo = obj_surf->bo;
     proc_ctx->frame_store[FRAME_IN_CURRENT].is_internal_surface = 0;
     dri_bo_reference(proc_ctx->frame_store[FRAME_IN_CURRENT].bo);

     /* update the output surface */ 
     if(proc_ctx->filters_mask == VPP_DNDI_DN){
         obj_surf = SURFACE(proc_ctx->surface_output);
         proc_ctx->frame_store[FRAME_OUT_CURRENT_DN].surface_id = proc_ctx->surface_output;
         proc_ctx->frame_store[FRAME_OUT_CURRENT_DN].bo = obj_surf->bo;
         proc_ctx->frame_store[FRAME_OUT_CURRENT_DN].is_internal_surface = 0;
         dri_bo_reference(proc_ctx->frame_store[FRAME_OUT_CURRENT_DN].bo);
     }else {
         obj_surf = SURFACE(proc_ctx->surface_output);
         proc_ctx->frame_store[FRAME_OUT_CURRENT].surface_id = proc_ctx->surface_output;
         proc_ctx->frame_store[FRAME_OUT_CURRENT].bo = obj_surf->bo;
         proc_ctx->frame_store[FRAME_OUT_CURRENT].is_internal_surface = 0;
         dri_bo_reference(proc_ctx->frame_store[FRAME_OUT_CURRENT].bo);
     } 
}

void hsw_veb_surface_unreference(VADriverContextP ctx,
                                 struct intel_vebox_context *proc_ctx)
{
    /* unreference the input surface */ 
    dri_bo_unreference(proc_ctx->frame_store[FRAME_IN_CURRENT].bo);
    proc_ctx->frame_store[FRAME_IN_CURRENT].surface_id = -1;
    proc_ctx->frame_store[FRAME_IN_CURRENT].bo = NULL;
    proc_ctx->frame_store[FRAME_IN_CURRENT].is_internal_surface = 0;
    dri_bo_unreference(proc_ctx->frame_store[FRAME_IN_CURRENT].bo);

    /* unreference the shared output surface */ 
    if(proc_ctx->filters_mask == VPP_DNDI_DN){
       proc_ctx->frame_store[FRAME_OUT_CURRENT_DN].surface_id = -1;
       proc_ctx->frame_store[FRAME_OUT_CURRENT_DN].bo = NULL;
       proc_ctx->frame_store[FRAME_OUT_CURRENT_DN].is_internal_surface = 0;
       dri_bo_unreference(proc_ctx->frame_store[FRAME_OUT_CURRENT_DN].bo);
    }else{
        proc_ctx->frame_store[FRAME_OUT_CURRENT].surface_id = -1;
        proc_ctx->frame_store[FRAME_OUT_CURRENT].bo = NULL;
        proc_ctx->frame_store[FRAME_OUT_CURRENT].is_internal_surface = 0;
        dri_bo_unreference(proc_ctx->frame_store[FRAME_OUT_CURRENT].bo);
     }
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

    struct object_surface* obj_surf_in  = SURFACE(proc_ctx->surface_input);
    struct object_surface* obj_surf_out = SURFACE(proc_ctx->surface_output);
    
    assert(obj_surf_in->orig_width  == obj_surf_out->orig_width &&
           obj_surf_in->orig_height == obj_surf_out->orig_height);

    proc_ctx->pic_width   = obj_surf_in->orig_width;
    proc_ctx->pic_height  = obj_surf_in->orig_height;
  
    /* record vebox pipeline input surface format information*/
    if(obj_surf_in->bo == NULL){
        input_fourcc = VA_FOURCC('N','V','1','2');
        input_sampling = SUBSAMPLE_YUV420;
        input_tiling = 1;
        i965_check_alloc_surface_bo(ctx, obj_surf_in, input_tiling, input_fourcc, input_sampling);
    } else {
        input_fourcc = obj_surf_in->fourcc;
        input_sampling = obj_surf_in->subsampling;
        dri_bo_get_tiling(obj_surf_in->bo, &input_tiling, &swizzle);
        input_tiling = !!input_tiling;
    }

    /* record vebox pipeline output surface format information */
    if(obj_surf_out->bo == NULL){
        output_fourcc = VA_FOURCC('N','V','1','2');
        output_sampling = SUBSAMPLE_YUV420;
        output_tiling = 1;
        i965_check_alloc_surface_bo(ctx, obj_surf_out, output_tiling, output_fourcc, output_sampling);
    }else {
        output_fourcc   = obj_surf_out->fourcc;
        output_sampling = obj_surf_out->subsampling;
        dri_bo_get_tiling(obj_surf_out->bo, &output_tiling, &swizzle);
        output_tiling = !!output_tiling;
    }
   
    assert(input_fourcc == VA_FOURCC_NV12 ||
           input_fourcc == VA_FOURCC_YUY2 ||
           input_fourcc == VA_FOURCC_AYUV ||
           input_fourcc == VA_FOURCC_RGBA);
    assert(output_fourcc == VA_FOURCC_NV12 ||
           output_fourcc == VA_FOURCC_YUY2 ||
           output_fourcc == VA_FOURCC_AYUV ||
           output_fourcc == VA_FOURCC_RGBA);

    proc_ctx->fourcc_input = input_fourcc;
    proc_ctx->fourcc_output = output_fourcc;

    /* allocate vebox pipeline surfaces */
    VASurfaceID surfaces[FRAME_STORE_SUM];
    va_status = i965_CreateSurfaces(ctx,
                                   proc_ctx ->pic_width,
                                   proc_ctx ->pic_height,
                                   VA_RT_FORMAT_YUV420,
                                   FRAME_STORE_SUM,
                                   surfaces);
    assert(va_status == VA_STATUS_SUCCESS);

    for(i = FRAME_IN_CURRENT; i < FRAME_STORE_SUM; i ++) {
        proc_ctx->frame_store[i].surface_id = surfaces[i];
        struct object_surface* obj_surf = SURFACE(surfaces[i]);
        if( i == FRAME_IN_CURRENT) {
            proc_ctx->frame_store[i].surface_id = proc_ctx->surface_input;
            proc_ctx->frame_store[i].bo = (SURFACE(proc_ctx->surface_input))->bo;
            proc_ctx->frame_store[i].is_internal_surface = 0;
            continue;
        }else if( i == FRAME_IN_PREVIOUS || i == FRAME_OUT_CURRENT_DN) {
            i965_check_alloc_surface_bo(ctx, obj_surf, input_tiling, input_fourcc, input_sampling);
        } else if( i == FRAME_IN_STMM || i == FRAME_OUT_STMM){
            i965_check_alloc_surface_bo(ctx, obj_surf, 1, input_fourcc, input_sampling);
        } else if( i >= FRAME_OUT_CURRENT){
            i965_check_alloc_surface_bo(ctx, obj_surf, output_tiling, output_fourcc, output_sampling);
        }
        proc_ctx->frame_store[i].bo = obj_surf->bo;
        dri_bo_reference(proc_ctx->frame_store[i].bo);
        proc_ctx->frame_store[i].is_internal_surface = 1;
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

VAStatus gen75_vebox_process_picture(VADriverContextP ctx,
                         struct intel_vebox_context *proc_ctx)
{
    VAStatus va_status = VA_STATUS_SUCCESS;
    
    if(proc_ctx->is_first_frame) 
       hsw_veb_resource_prepare(ctx, proc_ctx);
 
    hsw_veb_surface_reference(ctx, proc_ctx);

    intel_batchbuffer_start_atomic_veb(proc_ctx->batch, 0x1000);
    intel_batchbuffer_emit_mi_flush(proc_ctx->batch);
    hsw_veb_surface_state(ctx, proc_ctx, INPUT_SURFACE); 
    hsw_veb_surface_state(ctx, proc_ctx, OUTPUT_SURFACE); 
    hsw_veb_state_table_setup(ctx, proc_ctx);

    hsw_veb_state_command(ctx, proc_ctx);		
    hsw_veb_dndi_iecp_command(ctx, proc_ctx);
    intel_batchbuffer_end_atomic(proc_ctx->batch);
    intel_batchbuffer_flush(proc_ctx->batch);

    hsw_veb_surface_unreference(ctx, proc_ctx);

   if(proc_ctx->is_first_frame)
       proc_ctx->is_first_frame = 0; 
   
    return va_status;
}

void gen75_vebox_context_destroy(VADriverContextP ctx, 
                          struct intel_vebox_context *proc_ctx)
{
    int i;
    /* release vebox pipeline surface */
    for(i = 0; i < FRAME_STORE_SUM; i ++) {
        if(proc_ctx->frame_store[i].is_internal_surface){
            dri_bo_unreference(proc_ctx->frame_store[i].bo);
        }
        proc_ctx->frame_store[i].surface_id = -1;
        proc_ctx->frame_store[i].bo = NULL;
    }
    /* release dndi state table  */
    dri_bo_unreference(proc_ctx->dndi_state_table.bo);
    proc_ctx->dndi_state_table.bo = NULL;

    /* release iecp state table  */
    dri_bo_unreference(proc_ctx->iecp_state_table.bo);
    proc_ctx->dndi_state_table.bo = NULL;

    intel_batchbuffer_free(proc_ctx->batch);

    free(proc_ctx);
}

struct intel_vebox_context * gen75_vebox_context_init(VADriverContextP ctx)
{
    struct intel_driver_data *intel = intel_driver_data(ctx);
    struct intel_vebox_context *proc_context = calloc(1, sizeof(struct intel_vebox_context));

    proc_context->batch = intel_batchbuffer_new(intel, I915_EXEC_VEBOX, 0);
    memset(proc_context->frame_store, 0, sizeof(VEBFrameStore)*FRAME_STORE_SUM);
  
    proc_context->filters_mask             = 0;
    proc_context->is_first_frame           = 1;
    proc_context->filters_mask             = 0;

    return proc_context;
}

