/*
 * Copyright © 2010-2011 Intel Corporation
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
 *    Zhao Yakui <yakui.zhao@intel.com>
 *    Xiang Haihao <haihao.xiang@intel.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "assert.h"
#include "intel_batchbuffer.h"
#include "i965_defines.h"
#include "i965_structs.h"
#include "i965_drv_video.h"
#include "i965_encoder.h"

#define B0_STEP_REV		2
#define IS_STEPPING_BPLUS(i965)	((i965->intel.revision) >= B0_STEP_REV)

static void
gen75_mfc_pipe_mode_select(VADriverContextP ctx,
                           int standard_select,
                           struct gen6_encoder_context *gen6_encoder_context,
                           struct intel_batchbuffer *batch)
{
    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    assert(standard_select == MFX_FORMAT_MPEG2 ||
           standard_select == MFX_FORMAT_AVC);

    BEGIN_BCS_BATCH(batch, 5);
    OUT_BCS_BATCH(batch, MFX_PIPE_MODE_SELECT | (5 - 2));
    OUT_BCS_BATCH(batch,
                  (MFX_LONG_MODE << 17) | /* Must be long format for encoder */
                  (MFD_MODE_VLD << 15) | /* VLD mode */
                  (0 << 10) | /* disable Stream-Out */
                  (1 << 9)  | /* Post Deblocking Output */
                  (0 << 8)  | /* Pre Deblocking Output */
                  (0 << 5)  | /* not in stitch mode */
                  (1 << 4)  | /* encoding mode */
                  (standard_select << 0));  /* standard select: avc or mpeg2 */
    OUT_BCS_BATCH(batch,
                  (0 << 7)  | /* expand NOA bus flag */
                  (0 << 6)  | /* disable slice-level clock gating */
                  (0 << 5)  | /* disable clock gating for NOA */
                  (0 << 4)  | /* terminate if AVC motion and POC table error occurs */
                  (0 << 3)  | /* terminate if AVC mbdata error occurs */
                  (0 << 2)  | /* terminate if AVC CABAC/CAVLC decode error occurs */
                  (0 << 1)  |
                  (0 << 0));
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);
}

#define		INTER_MODE_MASK		0x03
#define		INTER_8X8		0x03
#define		SUBMB_SHAPE_MASK	0x00FF00

#define		INTER_MV8		(4 << 20)
#define		INTER_MV32		(6 << 20)


static void
gen75_mfc_surface_state(VADriverContextP ctx,
                        struct gen6_encoder_context *gen6_encoder_context,
                        struct intel_batchbuffer *batch)
{
    struct gen6_mfc_context *mfc_context = &gen6_encoder_context->mfc_context;

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    BEGIN_BCS_BATCH(batch, 6);

    OUT_BCS_BATCH(batch, MFX_SURFACE_STATE | (6 - 2));
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch,
                  ((mfc_context->surface_state.height - 1) << 18) |
                  ((mfc_context->surface_state.width - 1) << 4));
    OUT_BCS_BATCH(batch,
                  (MFX_SURFACE_PLANAR_420_8 << 28) | /* 420 planar YUV surface */
                  (1 << 27) | /* must be 1 for interleave U/V, hardware requirement */
                  (0 << 22) | /* surface object control state, FIXME??? */
                  ((mfc_context->surface_state.w_pitch - 1) << 3) | /* pitch */
                  (0 << 2)  | /* must be 0 for interleave U/V */
                  (1 << 1)  | /* must be tiled */
                  (I965_TILEWALK_YMAJOR << 0));  /* tile walk, TILEWALK_YMAJOR */
    OUT_BCS_BATCH(batch,
                  (0 << 16) | 								/* must be 0 for interleave U/V */
                  (mfc_context->surface_state.h_pitch)); 		/* y offset for U(cb) */
    OUT_BCS_BATCH(batch, 0);
    ADVANCE_BCS_BATCH(batch);
}

static void
gen75_mfc_pipe_buf_addr_state_bplus(VADriverContextP ctx,
                                    struct gen6_encoder_context *gen6_encoder_context,
                                    struct intel_batchbuffer *batch)
{
    struct gen6_mfc_context *mfc_context = &gen6_encoder_context->mfc_context;
    int i;

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    BEGIN_BCS_BATCH(batch, 61);

    OUT_BCS_BATCH(batch, MFX_PIPE_BUF_ADDR_STATE | (61 - 2));

    /* the DW1-3 is for pre_deblocking */
        OUT_BCS_BATCH(batch, 0);											/* pre output addr   */

        OUT_BCS_BATCH(batch, 0);
        OUT_BCS_BATCH(batch, 0);
     /* the DW4-6 is for the post_deblocking */

    if (mfc_context->post_deblocking_output.bo)
        OUT_BCS_RELOC(batch, mfc_context->post_deblocking_output.bo,
                      I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                      0);											/* post output addr  */	
    else
        OUT_BCS_BATCH(batch, 0);

        OUT_BCS_BATCH(batch, 0);
        OUT_BCS_BATCH(batch, 0);

     /* the DW7-9 is for the uncompressed_picture */
    OUT_BCS_RELOC(batch, mfc_context->uncompressed_picture_source.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  0); /* uncompressed data */

        OUT_BCS_BATCH(batch, 0);
        OUT_BCS_BATCH(batch, 0);

     /* the DW10-12 is for the mb status */
        OUT_BCS_BATCH(batch, 0);
        OUT_BCS_BATCH(batch, 0);
        OUT_BCS_BATCH(batch, 0);

     /* the DW13-15 is for the intra_row_store_scratch */
    OUT_BCS_RELOC(batch, mfc_context->intra_row_store_scratch_buffer.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  0);	
        OUT_BCS_BATCH(batch, 0);
        OUT_BCS_BATCH(batch, 0);

     /* the DW16-18 is for the deblocking filter */
    OUT_BCS_RELOC(batch, mfc_context->deblocking_filter_row_store_scratch_buffer.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  0);
        OUT_BCS_BATCH(batch, 0);
        OUT_BCS_BATCH(batch, 0);

    /* the DW 19-50 is for Reference pictures*/
    for (i = 0; i < ARRAY_ELEMS(mfc_context->reference_surfaces); i++) {
        if ( mfc_context->reference_surfaces[i].bo != NULL) {
            OUT_BCS_RELOC(batch, mfc_context->reference_surfaces[i].bo,
                          I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                          0);			
        } else {
            OUT_BCS_BATCH(batch, 0);
        }
	OUT_BCS_BATCH(batch, 0);
    }
        OUT_BCS_BATCH(batch, 0);

	/* The DW 52-54 is for the MB status buffer */
        OUT_BCS_BATCH(batch, 0);
	
        OUT_BCS_BATCH(batch, 0);
        OUT_BCS_BATCH(batch, 0);

	/* the DW 55-57 is the ILDB buffer */
        OUT_BCS_BATCH(batch, 0);
        OUT_BCS_BATCH(batch, 0);
        OUT_BCS_BATCH(batch, 0);

	/* the DW 58-60 is the second ILDB buffer */
        OUT_BCS_BATCH(batch, 0);
        OUT_BCS_BATCH(batch, 0);
        OUT_BCS_BATCH(batch, 0);
    ADVANCE_BCS_BATCH(batch);
}

static void
gen75_mfc_pipe_buf_addr_state(VADriverContextP ctx,
                              struct gen6_encoder_context *gen6_encoder_context,
                              struct intel_batchbuffer *batch)
{
    struct gen6_mfc_context *mfc_context = &gen6_encoder_context->mfc_context;
    int i;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
 
    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    if (IS_STEPPING_BPLUS(i965)) {
	gen75_mfc_pipe_buf_addr_state_bplus(ctx, gen6_encoder_context, batch);
	return;
    }

    BEGIN_BCS_BATCH(batch, 25);

    OUT_BCS_BATCH(batch, MFX_PIPE_BUF_ADDR_STATE | (25 - 2));

    OUT_BCS_BATCH(batch, 0);											/* pre output addr   */

    OUT_BCS_RELOC(batch, mfc_context->post_deblocking_output.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  0);											/* post output addr  */	

    OUT_BCS_RELOC(batch, mfc_context->uncompressed_picture_source.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  0);											/* uncompressed data */

    OUT_BCS_BATCH(batch, 0);											/* StreamOut data*/
    OUT_BCS_RELOC(batch, mfc_context->intra_row_store_scratch_buffer.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  0);	
    OUT_BCS_RELOC(batch, mfc_context->deblocking_filter_row_store_scratch_buffer.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  0);
    /* 7..22 Reference pictures*/
    for (i = 0; i < ARRAY_ELEMS(mfc_context->reference_surfaces); i++) {
        if ( mfc_context->reference_surfaces[i].bo != NULL) {
            OUT_BCS_RELOC(batch, mfc_context->reference_surfaces[i].bo,
                          I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                          0);			
        } else {
            OUT_BCS_BATCH(batch, 0);
        }
    }
    OUT_BCS_BATCH(batch, 0);   											/* no block status  */

    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);
}


static void
gen75_mfc_ind_obj_base_addr_state_bplus(VADriverContextP ctx,
                                        struct gen6_encoder_context *gen6_encoder_context,
                                        struct intel_batchbuffer *batch)
{
    struct gen6_vme_context *vme_context = &gen6_encoder_context->vme_context;
    struct gen6_mfc_context *mfc_context = &gen6_encoder_context->mfc_context;

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    BEGIN_BCS_BATCH(batch, 26);

    OUT_BCS_BATCH(batch, MFX_IND_OBJ_BASE_ADDR_STATE | (26 - 2));
	/* the DW1-3 is for the MFX indirect bistream offset */
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
	/* the DW4-5 is the MFX upper bound */
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);

    /* the DW6-10 is for MFX Indirect MV Object Base Address */
    OUT_BCS_RELOC(batch, vme_context->vme_output.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0x80000000); /* must set, up to 2G */
    OUT_BCS_BATCH(batch, 0);

     /* the DW11-15 is for MFX IT-COFF. Not used on encoder */
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);

     /* the DW16-20 is for MFX indirect DBLK. Not used on encoder */	
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);

    /* the DW21-25 is for MFC Indirect PAK-BSE Object Base Address for Encoder*/	
    OUT_BCS_RELOC(batch,
                  mfc_context->mfc_indirect_pak_bse_object.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
	
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0x00000000);

    ADVANCE_BCS_BATCH(batch);
}

static void
gen75_mfc_ind_obj_base_addr_state(VADriverContextP ctx,
                                  struct gen6_encoder_context *gen6_encoder_context,
                                  struct intel_batchbuffer *batch)
{
    struct gen6_vme_context *vme_context = &gen6_encoder_context->vme_context;
    struct gen6_mfc_context *mfc_context = &gen6_encoder_context->mfc_context;
    struct i965_driver_data *i965 = i965_driver_data(ctx);

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    if (IS_STEPPING_BPLUS(i965)) {
	gen75_mfc_ind_obj_base_addr_state_bplus(ctx, gen6_encoder_context, batch);
	return;
    }

    BEGIN_BCS_BATCH(batch, 11);

    OUT_BCS_BATCH(batch, MFX_IND_OBJ_BASE_ADDR_STATE | (11 - 2));
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    /* MFX Indirect MV Object Base Address */
    OUT_BCS_RELOC(batch, vme_context->vme_output.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);
    OUT_BCS_BATCH(batch, 0x80000000); /* must set, up to 2G */
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    /*MFC Indirect PAK-BSE Object Base Address for Encoder*/	
    OUT_BCS_RELOC(batch,
                  mfc_context->mfc_indirect_pak_bse_object.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  0);
    OUT_BCS_BATCH(batch, 0x00000000); /* must set, up to 2G */

    ADVANCE_BCS_BATCH(batch);
}

static void
gen75_mfc_bsp_buf_base_addr_state_bplus(VADriverContextP ctx,
                                        struct gen6_encoder_context *gen6_encoder_context,
                                        struct intel_batchbuffer *batch)
{
    struct gen6_mfc_context *mfc_context = &gen6_encoder_context->mfc_context;

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    BEGIN_BCS_BATCH(batch, 10);

    OUT_BCS_BATCH(batch, MFX_BSP_BUF_BASE_ADDR_STATE | (10 - 2));
    OUT_BCS_RELOC(batch, mfc_context->bsd_mpc_row_store_scratch_buffer.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
	
	/* the DW4-6 is for MPR Row Store Scratch Buffer Base Address */
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);

	/* the DW7-9 is for Bitplane Read Buffer Base Address */
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);
}

static void
gen75_mfc_bsp_buf_base_addr_state(VADriverContextP ctx,
                                  struct gen6_encoder_context *gen6_encoder_context,
                                  struct intel_batchbuffer *batch)
{
    struct gen6_mfc_context *mfc_context = &gen6_encoder_context->mfc_context;
    struct i965_driver_data *i965 = i965_driver_data(ctx);

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    if (IS_STEPPING_BPLUS(i965)) {
	gen75_mfc_bsp_buf_base_addr_state_bplus(ctx, gen6_encoder_context, batch);
	return;
    }
 

    BEGIN_BCS_BATCH(batch, 4);

    OUT_BCS_BATCH(batch, MFX_BSP_BUF_BASE_ADDR_STATE | (4 - 2));
    OUT_BCS_RELOC(batch, mfc_context->bsd_mpc_row_store_scratch_buffer.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);
}

static void
gen75_mfc_avc_img_state(VADriverContextP ctx,
                        struct gen6_encoder_context *gen6_encoder_context,
                        struct intel_batchbuffer *batch)
{
    struct gen6_mfc_context *mfc_context = &gen6_encoder_context->mfc_context;
    int width_in_mbs = (mfc_context->surface_state.width + 15) / 16;
    int height_in_mbs = (mfc_context->surface_state.height + 15) / 16;

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    BEGIN_BCS_BATCH(batch, 16);
    OUT_BCS_BATCH(batch, MFX_AVC_IMG_STATE | (16 - 2));
    OUT_BCS_BATCH(batch,
                  ((width_in_mbs * height_in_mbs) & 0xFFFF));
    OUT_BCS_BATCH(batch, 
                  ((height_in_mbs - 1) << 16) | 
                  ((width_in_mbs - 1) << 0));
    OUT_BCS_BATCH(batch, 
                  (0 << 24) |	/* Second Chroma QP Offset */
                  (0 << 16) |	/* Chroma QP Offset */
                  (0 << 14) |   /* Max-bit conformance Intra flag */
                  (0 << 13) |   /* Max Macroblock size conformance Inter flag */
                  (0 << 12) |   /* FIXME: Weighted_Pred_Flag */
                  (0 << 10) |   /* FIXME: Weighted_BiPred_Idc */
                  (0 << 8)  |   /* FIXME: Image Structure */
                  (0 << 0) );   /* Current Decoed Image Frame Store ID, reserved in Encode mode */
    OUT_BCS_BATCH(batch,
                  (0 << 16) |   /* Mininum Frame size */
                  (0 << 15) |   /* Disable reading of Macroblock Status Buffer */
                  (0 << 14) |   /* Load BitStream Pointer only once, 1 slic 1 frame */
                  (0 << 13) |   /* CABAC 0 word insertion test enable */
                  (1 << 12) |   /* MVUnpackedEnable,compliant to DXVA */
                  (1 << 10) |   /* Chroma Format IDC, 4:2:0 */
                  (0 << 8)  |   /* FIXME: MbMvFormatFlag */
                  (1 << 7)  |   /* 0:CAVLC encoding mode,1:CABAC */
                  (0 << 6)  |   /* Only valid for VLD decoding mode */
                  (0 << 5)  |   /* Constrained Intra Predition Flag, from PPS */
                  (0 << 4)  |   /* Direct 8x8 inference flag */
                  (0 << 3)  |   /* Only 8x8 IDCT Transform Mode Flag */
                  (1 << 2)  |   /* Frame MB only flag */
                  (0 << 1)  |   /* MBAFF mode is in active */
                  (0 << 0));    /* Field picture flag */
    OUT_BCS_BATCH(batch, 0);    /* Mainly about MB rate control and debug, just ignoring */
    OUT_BCS_BATCH(batch,        /* Inter and Intra Conformance Max size limit */
                  (0xBB8 << 16) |       /* InterMbMaxSz */
                  (0xEE8) );            /* IntraMbMaxSz */
    OUT_BCS_BATCH(batch, 0);            /* Reserved */
    OUT_BCS_BATCH(batch, 0);            /* Slice QP Delta for bitrate control */
    OUT_BCS_BATCH(batch, 0);            /* Slice QP Delta for bitrate control */	
    OUT_BCS_BATCH(batch, 0x8C000000);
    OUT_BCS_BATCH(batch, 0x00010000);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);
}


static void
gen75_mfc_avc_directmode_state_bplus(VADriverContextP ctx,
                                     struct gen6_encoder_context *gen6_encoder_context,
                                     struct intel_batchbuffer *batch)
{
    struct gen6_mfc_context *mfc_context = &gen6_encoder_context->mfc_context;
    int i;

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    BEGIN_BCS_BATCH(batch, 71);

    OUT_BCS_BATCH(batch, MFX_AVC_DIRECTMODE_STATE | (71 - 2));

    /* Reference frames and Current frames */
    /* the DW1-32 is for the direct MV for reference */
    for(i = 0; i < NUM_MFC_DMV_BUFFERS - 2; i += 2) {
        if ( mfc_context->direct_mv_buffers[i].bo != NULL) { 
            OUT_BCS_RELOC(batch, mfc_context->direct_mv_buffers[i].bo,
                          I915_GEM_DOMAIN_INSTRUCTION, 0,
                          0);
            OUT_BCS_BATCH(batch, 0);
        } else {
            OUT_BCS_BATCH(batch, 0);
            OUT_BCS_BATCH(batch, 0);
        }
    }
	OUT_BCS_BATCH(batch, 0);

	/* the DW34-36 is the MV for the current reference */
        OUT_BCS_RELOC(batch, mfc_context->direct_mv_buffers[NUM_MFC_DMV_BUFFERS - 2].bo,
                          I915_GEM_DOMAIN_INSTRUCTION, 0,
                          0);

	OUT_BCS_BATCH(batch, 0);
	OUT_BCS_BATCH(batch, 0);

    /* POL list */
    for(i = 0; i < 32; i++) {
        OUT_BCS_BATCH(batch, i/2);
    }
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);
}

static void gen75_mfc_avc_directmode_state(VADriverContextP ctx,
                                           struct gen6_encoder_context *gen6_encoder_context,
                                           struct intel_batchbuffer *batch)
{
    struct gen6_mfc_context *mfc_context = &gen6_encoder_context->mfc_context;
    int i;
    struct i965_driver_data *i965 = i965_driver_data(ctx);

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    if (IS_STEPPING_BPLUS(i965)) {
	gen75_mfc_avc_directmode_state_bplus(ctx, gen6_encoder_context, batch);
	return;
    }

    BEGIN_BCS_BATCH(batch, 69);

    OUT_BCS_BATCH(batch, MFX_AVC_DIRECTMODE_STATE | (69 - 2));
    //TODO: reference DMV
    for (i = 0; i < NUM_MFC_DMV_BUFFERS - 2; i++){
	if (mfc_context->direct_mv_buffers[i].bo)
    		OUT_BCS_RELOC(batch, mfc_context->direct_mv_buffers[i].bo,
                	  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  0);
	else
        	OUT_BCS_BATCH(batch, 0);
    }

    //TODO: current DMV just for test
#if 0
    OUT_BCS_RELOC(batch, mfc_context->direct_mv_buffers[0].bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  0);
#else
    //drm_intel_bo_pin(mfc_context->direct_mv_buffers[0].bo, 0x1000);
    //OUT_BCS_BATCH(batch, mfc_context->direct_mv_buffers[0].bo->offset);
    OUT_BCS_RELOC(batch, mfc_context->direct_mv_buffers[NUM_MFC_DMV_BUFFERS - 2].bo,
               	  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  0);
#endif


    OUT_BCS_BATCH(batch, 0);

    //TODO: POL list
    for(i = 0; i < 34; i++) {
        OUT_BCS_BATCH(batch, 0);
    }

    ADVANCE_BCS_BATCH(batch);
}

static void gen75_mfc_avc_slice_state(VADriverContextP ctx,
                                      int intra_slice,
                                      struct gen6_encoder_context *gen6_encoder_context,
                                      struct intel_batchbuffer *batch)
{
    struct gen6_mfc_context *mfc_context = &gen6_encoder_context->mfc_context;

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    BEGIN_BCS_BATCH(batch, 11);;

    OUT_BCS_BATCH(batch, MFX_AVC_SLICE_STATE | (11 - 2) );

    if ( intra_slice )
        OUT_BCS_BATCH(batch, 2);			/*Slice Type: I Slice*/
    else
        OUT_BCS_BATCH(batch, 0);			/*Slice Type: P Slice*/

    if ( intra_slice )
        OUT_BCS_BATCH(batch, 0);			/*no reference frames and pred_weight_table*/
    else 
        OUT_BCS_BATCH(batch, 0x00010000); 	/*1 reference frame*/

    OUT_BCS_BATCH(batch, (0<<24) |                /*Enable deblocking operation*/
                  (26<<16) | 			/*Slice Quantization Parameter*/
                  0x0202 );
    OUT_BCS_BATCH(batch, 0);			/*First MB X&Y , the postion of current slice*/
    OUT_BCS_BATCH(batch, ( ((mfc_context->surface_state.height+15)/16) << 16) );

    OUT_BCS_BATCH(batch, 
                  (0<<31) |		/*RateControlCounterEnable = disable*/
                  (1<<30) |		/*ResetRateControlCounter*/
                  (2<<28) |		/*RC Triggle Mode = Loose Rate Control*/
                  (1<<19) | 	        /*IsLastSlice*/
                  (0<<18) | 	        /*BitstreamOutputFlag Compressed BitStream Output Disable Flag 0:enable 1:disable*/
                  (0<<17) |	        /*HeaderPresentFlag*/	
                  (1<<16) |	        /*SliceData PresentFlag*/
                  (0<<15) |	        /*TailPresentFlag*/
                  (1<<13) |	        /*RBSP NAL TYPE*/	
                  (0<<12) );	        /*CabacZeroWordInsertionEnable*/
	

    OUT_BCS_BATCH(batch, mfc_context->mfc_indirect_pak_bse_object.offset);

    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);
}

static void
gen75_mfc_qm_state(VADriverContextP ctx,
                   int qm_type,
                   unsigned int *qm,
                   int qm_length,
                   struct gen6_encoder_context *gen6_encoder_context,
                   struct intel_batchbuffer *batch)
{
    unsigned int qm_buffer[16];

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    assert(qm_length <= 16);
    assert(sizeof(*qm) == 4);
    memcpy(qm_buffer, qm, qm_length * 4);

    BEGIN_BCS_BATCH(batch, 18);
    OUT_BCS_BATCH(batch, MFX_QM_STATE | (18 - 2));
    OUT_BCS_BATCH(batch, qm_type << 0);
    intel_batchbuffer_data(batch, qm_buffer, 16 * 4);
    ADVANCE_BCS_BATCH(batch);
}

static void gen75_mfc_avc_qm_state(VADriverContextP ctx,
                                   struct gen6_encoder_context *gen6_encoder_context,
                                   struct intel_batchbuffer *batch)
{
    unsigned int qm[16] = {
        0x10101010, 0x10101010, 0x10101010, 0x10101010,
        0x10101010, 0x10101010, 0x10101010, 0x10101010,
        0x10101010, 0x10101010, 0x10101010, 0x10101010,
        0x10101010, 0x10101010, 0x10101010, 0x10101010
    };

    gen75_mfc_qm_state(ctx, MFX_QM_AVC_4X4_INTRA_MATRIX, qm, 12, gen6_encoder_context, batch);
    gen75_mfc_qm_state(ctx, MFX_QM_AVC_4X4_INTER_MATRIX, qm, 12, gen6_encoder_context, batch);
    gen75_mfc_qm_state(ctx, MFX_QM_AVC_8x8_INTRA_MATRIX, qm, 16, gen6_encoder_context, batch);
    gen75_mfc_qm_state(ctx, MFX_QM_AVC_8x8_INTER_MATRIX, qm, 16, gen6_encoder_context, batch);
}

static void
gen75_mfc_fqm_state(VADriverContextP ctx,
                    int fqm_type,
                    unsigned int *fqm,
                    int fqm_length,
                    struct gen6_encoder_context *gen6_encoder_context,
                    struct intel_batchbuffer *batch)
{
    unsigned int fqm_buffer[32];

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    assert(fqm_length <= 32);
    assert(sizeof(*fqm) == 4);
    memcpy(fqm_buffer, fqm, fqm_length * 4);

    BEGIN_BCS_BATCH(batch, 34);
    OUT_BCS_BATCH(batch, MFX_FQM_STATE | (34 - 2));
    OUT_BCS_BATCH(batch, fqm_type << 0);
    intel_batchbuffer_data(batch, fqm_buffer, 32 * 4);
    ADVANCE_BCS_BATCH(batch);
}

static void gen75_mfc_avc_fqm_state(VADriverContextP ctx,
                                    struct gen6_encoder_context *gen6_encoder_context,
                                    struct intel_batchbuffer *batch)
{
    unsigned int qm[32] = {
        0x10001000, 0x10001000, 0x10001000, 0x10001000,
        0x10001000, 0x10001000, 0x10001000, 0x10001000,
        0x10001000, 0x10001000, 0x10001000, 0x10001000,
        0x10001000, 0x10001000, 0x10001000, 0x10001000,
        0x10001000, 0x10001000, 0x10001000, 0x10001000,
        0x10001000, 0x10001000, 0x10001000, 0x10001000,
        0x10001000, 0x10001000, 0x10001000, 0x10001000,
        0x10001000, 0x10001000, 0x10001000, 0x10001000
    };

    gen75_mfc_fqm_state(ctx, MFX_QM_AVC_4X4_INTRA_MATRIX, qm, 24, gen6_encoder_context, batch);
    gen75_mfc_fqm_state(ctx, MFX_QM_AVC_4X4_INTER_MATRIX, qm, 24, gen6_encoder_context, batch);
    gen75_mfc_fqm_state(ctx, MFX_QM_AVC_8x8_INTRA_MATRIX, qm, 32, gen6_encoder_context, batch);
    gen75_mfc_fqm_state(ctx, MFX_QM_AVC_8x8_INTER_MATRIX, qm, 32, gen6_encoder_context, batch);
}

static void gen75_mfc_avc_ref_idx_state(VADriverContextP ctx,
                                        struct gen6_encoder_context *gen6_encoder_context,
                                        struct intel_batchbuffer *batch)
{
    int i;

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    BEGIN_BCS_BATCH(batch, 10);

    OUT_BCS_BATCH(batch, MFX_AVC_REF_IDX_STATE | 8);
    OUT_BCS_BATCH(batch, 0);                  //Select L0

    OUT_BCS_BATCH(batch, 0x80808000);         //Only 1 reference
    for(i = 0; i < 7; i++) {
        OUT_BCS_BATCH(batch, 0x80808080);
    }

    ADVANCE_BCS_BATCH(batch);
}
	
static int
gen75_mfc_avc_pak_object_intra(VADriverContextP ctx, int x, int y, int end_mb, int qp,unsigned int *msg,
                               struct gen6_encoder_context *gen6_encoder_context,
                               struct intel_batchbuffer *batch)
{
    int len_in_dwords = 12;

    unsigned int intra_msg;
#define		INTRA_MSG_FLAG		(1 << 13)
#define		INTRA_MBTYPE_MASK	(0x1F0000)

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    BEGIN_BCS_BATCH(batch, len_in_dwords);

    intra_msg = msg[0] & 0xC0FF;
    intra_msg |= INTRA_MSG_FLAG;
    intra_msg |= ((msg[0] & INTRA_MBTYPE_MASK) >> 8);
    OUT_BCS_BATCH(batch, MFC_AVC_PAK_OBJECT | (len_in_dwords - 2));
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 
                  (0 << 24) |		/* PackedMvNum, Debug*/
                  (0 << 20) | 		/* No motion vector */
                  (1 << 19) |		/* CbpDcY */
                  (1 << 18) |		/* CbpDcU */
                  (1 << 17) |		/* CbpDcV */
                  intra_msg);

    OUT_BCS_BATCH(batch, (0xFFFF<<16) | (y << 8) | x);		/* Code Block Pattern for Y*/
    OUT_BCS_BATCH(batch, 0x000F000F);							/* Code Block Pattern */		
    OUT_BCS_BATCH(batch, (0 << 27) | (end_mb << 26) | qp);	/* Last MB */

    /*Stuff for Intra MB*/
    OUT_BCS_BATCH(batch, msg[1]);			/* We using Intra16x16 no 4x4 predmode*/	
    OUT_BCS_BATCH(batch, msg[2]);	
    OUT_BCS_BATCH(batch, msg[3]&0xFC);		

    OUT_BCS_BATCH(batch, 0x00000);	/*MaxSizeInWord and TargetSzieInWord*/
	OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);

    return len_in_dwords;
}

static int gen75_mfc_avc_pak_object_inter(VADriverContextP ctx, int x, int y, int end_mb, int qp,
                                          unsigned int offset, unsigned int *msg, struct gen6_encoder_context *gen6_encoder_context,
                                          struct intel_batchbuffer *batch)
{
    int len_in_dwords = 12;
    unsigned int inter_msg;

    if (batch == NULL)
        batch = gen6_encoder_context->base.batch;

    BEGIN_BCS_BATCH(batch, len_in_dwords);

    OUT_BCS_BATCH(batch, MFC_AVC_PAK_OBJECT | (len_in_dwords - 2));

	inter_msg = 32;
 	/* MV quantity */
	if ((msg[0] & INTER_MODE_MASK) == INTER_8X8) {
		if (msg[1] & SUBMB_SHAPE_MASK)
			inter_msg = 128;
	}
    OUT_BCS_BATCH(batch, inter_msg);         /* 32 MV*/

    OUT_BCS_BATCH(batch, offset);
	inter_msg = msg[0] & (0x1F00FFFF);
	inter_msg |= INTER_MV8;
	if (((msg[0] & INTER_MODE_MASK) == INTER_8X8) &&
	     		(msg[1] & SUBMB_SHAPE_MASK)) {
		inter_msg |= INTER_MV32;
	}

    OUT_BCS_BATCH(batch, inter_msg);

    OUT_BCS_BATCH(batch, (0xFFFF<<16) | (y << 8) | x);        /* Code Block Pattern for Y*/
    OUT_BCS_BATCH(batch, 0x000F000F);                         /* Code Block Pattern */    
    OUT_BCS_BATCH(batch, (0 << 27) | (end_mb << 26) | qp);    /* Last MB */

    /*Stuff for Inter MB*/
	inter_msg = msg[1] >> 8;
    OUT_BCS_BATCH(batch, inter_msg);        
    OUT_BCS_BATCH(batch, 0x0);    
    OUT_BCS_BATCH(batch, 0x0);        

    OUT_BCS_BATCH(batch, 0x00000000); /*MaxSizeInWord and TargetSzieInWord*/

    OUT_BCS_BATCH(batch, 0x0);        

    ADVANCE_BCS_BATCH(batch);

    return len_in_dwords;
}

static void gen75_mfc_init(VADriverContextP ctx,
                           struct encode_state *encode_state,
                           struct gen6_encoder_context *gen6_encoder_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct gen6_mfc_context *mfc_context = &gen6_encoder_context->mfc_context;
    dri_bo *bo;
    int i;
    VAEncSequenceParameterBufferH264 *pSequenceParameter = (VAEncSequenceParameterBufferH264 *)encode_state->seq_param->buffer;
    int width_in_mbs = pSequenceParameter->picture_width_in_mbs;

    /*Encode common setup for MFC*/
    dri_bo_unreference(mfc_context->post_deblocking_output.bo);
    mfc_context->post_deblocking_output.bo = NULL;

    dri_bo_unreference(mfc_context->pre_deblocking_output.bo);
    mfc_context->pre_deblocking_output.bo = NULL;

    dri_bo_unreference(mfc_context->uncompressed_picture_source.bo);
    mfc_context->uncompressed_picture_source.bo = NULL;

    dri_bo_unreference(mfc_context->mfc_indirect_pak_bse_object.bo); 
    mfc_context->mfc_indirect_pak_bse_object.bo = NULL;

    for (i = 0; i < MAX_MFC_REFERENCE_SURFACES; i++){
        if (mfc_context->reference_surfaces[i].bo != NULL)
            dri_bo_unreference(mfc_context->reference_surfaces[i].bo);
        mfc_context->reference_surfaces[i].bo = NULL;  
    }

    dri_bo_unreference(mfc_context->intra_row_store_scratch_buffer.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "Buffer",
                      width_in_mbs * 64,
                      64);
    assert(bo);
    mfc_context->intra_row_store_scratch_buffer.bo = bo;

    dri_bo_unreference(mfc_context->deblocking_filter_row_store_scratch_buffer.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "Buffer",
                      4 * width_in_mbs * 64,  /* 4 * width_in_mbs * 64 */
                      64);
    assert(bo);
    mfc_context->deblocking_filter_row_store_scratch_buffer.bo = bo;

    dri_bo_unreference(mfc_context->bsd_mpc_row_store_scratch_buffer.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "Buffer",
                      2 * width_in_mbs * 64, /* 2 * width_in_mbs * 64 */
                      0x1000);
    assert(bo);
    mfc_context->bsd_mpc_row_store_scratch_buffer.bo = bo;
}

#define		INTRA_RDO_OFFSET	4
#define		INTER_RDO_OFFSET	54
#define		INTER_MSG_OFFSET	52
#define		INTER_MV_OFFSET		224
#define		RDO_MASK		0xFFFF

static void gen75_mfc_avc_pipeline_programing(VADriverContextP ctx,
                                      struct encode_state *encode_state,
                                      struct gen6_encoder_context *gen6_encoder_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *main_batch = gen6_encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = &gen6_encoder_context->mfc_context;
    struct gen6_vme_context *vme_context = &gen6_encoder_context->vme_context;
    VAEncSequenceParameterBufferH264 *pSequenceParameter = (VAEncSequenceParameterBufferH264 *)encode_state->seq_param->buffer;
    VAEncSliceParameterBuffer *pSliceParameter = (VAEncSliceParameterBuffer *)encode_state->slice_params[0]->buffer; /* FIXME: multi slices */
    unsigned int *msg = NULL, offset = 0;
    unsigned char *msg_ptr = NULL;
    int emit_new_state = 1, object_len_in_bytes;
    int is_intra = pSliceParameter->slice_flags.bits.is_intra;
    int width_in_mbs = (mfc_context->surface_state.width + 15) / 16;
    int height_in_mbs = (mfc_context->surface_state.height + 15) / 16;
    int x,y, mb_index;
    int inter_rdo, intra_rdo;
    struct intel_batchbuffer *batch = intel_batchbuffer_new(&i965->intel, I915_EXEC_BSD, width_in_mbs * height_in_mbs * 12 * 4 + 0x800);

    intel_batchbuffer_start_atomic_bcs(batch, width_in_mbs * height_in_mbs * 12 * 4 + 0x700);

    dri_bo_map(vme_context->vme_output.bo , 1);
    msg_ptr = (unsigned char *)vme_context->vme_output.bo->virtual;
    if (is_intra) {
	msg = (unsigned int *) (msg_ptr + 0 * vme_context->vme_output.size_block);
    } else {
	msg = (unsigned int *) (msg_ptr + 0 * vme_context->vme_output.size_block);
	offset = 0; 
    }

    for (y = 0; y < height_in_mbs; y++) {
        for (x = 0; x < width_in_mbs; x++) { 
            int last_mb = (y == (height_in_mbs-1)) && ( x == (width_in_mbs-1) );
            int qp = pSequenceParameter->initial_qp;
	     mb_index = (y * width_in_mbs) + x;
            if (emit_new_state) {
                intel_batchbuffer_emit_mi_flush(batch);
                
                gen75_mfc_pipe_mode_select(ctx, MFX_FORMAT_AVC, gen6_encoder_context, batch);
                gen75_mfc_surface_state(ctx, gen6_encoder_context, batch);
                gen75_mfc_ind_obj_base_addr_state(ctx, gen6_encoder_context, batch);

                gen75_mfc_pipe_buf_addr_state(ctx, gen6_encoder_context, batch);
                gen75_mfc_bsp_buf_base_addr_state(ctx, gen6_encoder_context, batch);

                gen75_mfc_avc_img_state(ctx, gen6_encoder_context, batch);
                gen75_mfc_avc_qm_state(ctx, gen6_encoder_context, batch);
                gen75_mfc_avc_fqm_state(ctx, gen6_encoder_context, batch);
                gen75_mfc_avc_directmode_state(ctx, gen6_encoder_context, batch);

                gen75_mfc_avc_ref_idx_state(ctx, gen6_encoder_context, batch);
                gen75_mfc_avc_slice_state(ctx, is_intra, gen6_encoder_context, batch);
                emit_new_state = 0;
            }

	    msg = (unsigned int *) (msg_ptr + mb_index * vme_context->vme_output.size_block);
            if (is_intra) {
                object_len_in_bytes = gen75_mfc_avc_pak_object_intra(ctx, x, y, last_mb, qp, msg, gen6_encoder_context, batch);
            } else {
		inter_rdo = msg[INTER_RDO_OFFSET] & RDO_MASK;
		intra_rdo = msg[INTRA_RDO_OFFSET] & RDO_MASK;
		if (intra_rdo < inter_rdo) {
                    object_len_in_bytes = gen75_mfc_avc_pak_object_intra(ctx, x, y, last_mb, qp, msg, gen6_encoder_context, batch);
		} else {
                    msg += INTER_MSG_OFFSET;
                    offset = mb_index * vme_context->vme_output.size_block + INTER_MV_OFFSET;
                    object_len_in_bytes = gen75_mfc_avc_pak_object_inter(ctx, x, y, last_mb, qp, offset, msg, gen6_encoder_context, batch);
		}
	    }
            if (intel_batchbuffer_check_free_space(batch, object_len_in_bytes) == 0) {
                intel_batchbuffer_end_atomic(batch);
                intel_batchbuffer_flush(batch);
                emit_new_state = 1;
                intel_batchbuffer_start_atomic_bcs(batch, 0x1000);
            }
        }
    }

    dri_bo_unmap(vme_context->vme_output.bo);
	
    intel_batchbuffer_align(batch, 8);

    BEGIN_BCS_BATCH(batch, 2);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, MI_BATCH_BUFFER_END);
    ADVANCE_BCS_BATCH(batch);

    intel_batchbuffer_end_atomic(batch);

    /* chain to the main batch buffer */
    intel_batchbuffer_start_atomic_bcs(main_batch, 0x100);
    intel_batchbuffer_emit_mi_flush(main_batch);
    BEGIN_BCS_BATCH(main_batch, 2);
    OUT_BCS_BATCH(main_batch, MI_BATCH_BUFFER_START | (1 << 8));
    OUT_BCS_RELOC(main_batch,
                  batch->buffer,
                  I915_GEM_DOMAIN_COMMAND, 0,
                  0);
    ADVANCE_BCS_BATCH(main_batch);
    intel_batchbuffer_end_atomic(main_batch);

    // end programing             
    intel_batchbuffer_free(batch);	
}

static VAStatus gen75_mfc_avc_prepare(VADriverContextP ctx, 
                                     struct encode_state *encode_state,
                                     struct gen6_encoder_context *gen6_encoder_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct gen6_mfc_context *mfc_context = &gen6_encoder_context->mfc_context;
    struct object_surface *obj_surface;	
    struct object_buffer *obj_buffer;
    dri_bo *bo;
    VAEncPictureParameterBufferH264 *pPicParameter = (VAEncPictureParameterBufferH264 *)encode_state->pic_param->buffer;
    VAStatus vaStatus = VA_STATUS_SUCCESS;

    /*Setup all the input&output object*/
    obj_surface = SURFACE(pPicParameter->reconstructed_picture);
    assert(obj_surface);
    i965_check_alloc_surface_bo(ctx, obj_surface, 1, VA_FOURCC('N','V','1','2'), SUBSAMPLE_YUV420);
    mfc_context->post_deblocking_output.bo = obj_surface->bo;
    dri_bo_reference(mfc_context->post_deblocking_output.bo);

    mfc_context->surface_state.width = obj_surface->orig_width;
    mfc_context->surface_state.height = obj_surface->orig_height;
    mfc_context->surface_state.w_pitch = obj_surface->width;
    mfc_context->surface_state.h_pitch = obj_surface->height;

    obj_surface = SURFACE(pPicParameter->reference_picture);
    assert(obj_surface);
    if (obj_surface->bo != NULL) {
        mfc_context->reference_surfaces[0].bo = obj_surface->bo;
        dri_bo_reference(obj_surface->bo);
    }
	
    obj_surface = SURFACE(encode_state->current_render_target);
    assert(obj_surface && obj_surface->bo);
    mfc_context->uncompressed_picture_source.bo = obj_surface->bo;
    dri_bo_reference(mfc_context->uncompressed_picture_source.bo);

    obj_buffer = BUFFER (pPicParameter->coded_buf); /* FIXME: fix this later */
    bo = obj_buffer->buffer_store->bo;
    assert(bo);
    mfc_context->mfc_indirect_pak_bse_object.bo = bo;
    mfc_context->mfc_indirect_pak_bse_object.offset = ALIGN(sizeof(VACodedBufferSegment), 64);
    dri_bo_reference(mfc_context->mfc_indirect_pak_bse_object.bo);

    /*Programing bcs pipeline*/
    gen75_mfc_avc_pipeline_programing(ctx, encode_state, gen6_encoder_context);	//filling the pipeline
	
    return vaStatus;
}

static VAStatus gen75_mfc_run(VADriverContextP ctx, 
                             struct encode_state *encode_state,
                             struct gen6_encoder_context *gen6_encoder_context)
{
    struct intel_batchbuffer *batch = gen6_encoder_context->base.batch;

    intel_batchbuffer_flush(batch);		//run the pipeline

    return VA_STATUS_SUCCESS;
}

static VAStatus gen75_mfc_stop(VADriverContextP ctx, 
                              struct encode_state *encode_state,
                              struct gen6_encoder_context *gen6_encoder_context)
{
#if 0
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct gen6_mfc_context *mfc_context = &gen6_encoder_context->mfc_context;
	
    VAEncPictureParameterBufferH264 *pPicParameter = (VAEncPictureParameterBufferH264 *)encode_state->pic_param->buffer;
	
    struct object_surface *obj_surface = SURFACE(pPicParameter->reconstructed_picture);
    //struct object_surface *obj_surface = SURFACE(pPicParameter->reference_picture[0]);
    //struct object_surface *obj_surface = SURFACE(encode_state->current_render_target);
    my_debug(obj_surface);

#endif

    return VA_STATUS_SUCCESS;
}

static VAStatus
gen75_mfc_avc_encode_picture(VADriverContextP ctx, 
                            struct encode_state *encode_state,
                            struct gen6_encoder_context *gen6_encoder_context)
{
    gen75_mfc_init(ctx, encode_state, gen6_encoder_context);
    gen75_mfc_avc_prepare(ctx, encode_state, gen6_encoder_context);
    gen75_mfc_run(ctx, encode_state, gen6_encoder_context);
    gen75_mfc_stop(ctx, encode_state, gen6_encoder_context);

    return VA_STATUS_SUCCESS;
}

VAStatus
gen75_mfc_pipeline(VADriverContextP ctx,
                  VAProfile profile,
                  struct encode_state *encode_state,
                  struct gen6_encoder_context *gen6_encoder_context)
{
    VAStatus vaStatus;

    switch (profile) {
    case VAProfileH264Baseline:
        vaStatus = gen75_mfc_avc_encode_picture(ctx, encode_state, gen6_encoder_context);
        break;

        /* FIXME: add for other profile */
    default:
        vaStatus = VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
        break;
    }

    return vaStatus;
}

Bool gen75_mfc_context_init(VADriverContextP ctx, struct gen6_mfc_context *mfc_context)
{
    int i;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    dri_bo *bo;
	
    for (i = 0; i < NUM_MFC_DMV_BUFFERS; i++){
        dri_bo_unreference(mfc_context->direct_mv_buffers[i].bo);
        mfc_context->direct_mv_buffers[i].bo = NULL;
    }
    bo = dri_bo_alloc(i965->intel.bufmgr,
                        "Buffer",
                         68*8192,
                         64);
    mfc_context->direct_mv_buffers[0].bo = bo;
    bo = dri_bo_alloc(i965->intel.bufmgr,
                        "Buffer",
                         68*8192,
                         64);
    mfc_context->direct_mv_buffers[NUM_MFC_DMV_BUFFERS - 2].bo = bo;
    return True;
}

Bool gen75_mfc_context_destroy(struct gen6_mfc_context *mfc_context)
{
    int i;

    dri_bo_unreference(mfc_context->post_deblocking_output.bo);
    mfc_context->post_deblocking_output.bo = NULL;

    dri_bo_unreference(mfc_context->pre_deblocking_output.bo);
    mfc_context->pre_deblocking_output.bo = NULL;

    dri_bo_unreference(mfc_context->uncompressed_picture_source.bo);
    mfc_context->uncompressed_picture_source.bo = NULL;

    dri_bo_unreference(mfc_context->mfc_indirect_pak_bse_object.bo); 
    mfc_context->mfc_indirect_pak_bse_object.bo = NULL;

    for (i = 0; i < NUM_MFC_DMV_BUFFERS; i++){
        dri_bo_unreference(mfc_context->direct_mv_buffers[i].bo);
        mfc_context->direct_mv_buffers[i].bo = NULL;
    }

    dri_bo_unreference(mfc_context->intra_row_store_scratch_buffer.bo);
    mfc_context->intra_row_store_scratch_buffer.bo = NULL;

    dri_bo_unreference(mfc_context->deblocking_filter_row_store_scratch_buffer.bo);
    mfc_context->deblocking_filter_row_store_scratch_buffer.bo = NULL;

    dri_bo_unreference(mfc_context->bsd_mpc_row_store_scratch_buffer.bo);
    mfc_context->bsd_mpc_row_store_scratch_buffer.bo = NULL;

    return True;
}
