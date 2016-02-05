/*
 * Copyright © 2010-2012 Intel Corporation
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
#include <math.h>
#include <assert.h>

#include "intel_batchbuffer.h"
#include "i965_defines.h"
#include "i965_structs.h"
#include "i965_drv_video.h"
#include "i965_encoder.h"
#include "i965_encoder_utils.h"
#include "gen6_mfc.h"
#include "gen6_vme.h"
#include "intel_media.h"

#define	AVC_INTRA_RDO_OFFSET	4
#define	AVC_INTER_RDO_OFFSET	10
#define	AVC_INTER_MSG_OFFSET	8
#define	AVC_INTER_MV_OFFSET	48
#define	AVC_RDO_MASK		0xFFFF

#define	MFC_SOFTWARE_HASWELL	0

#define SURFACE_STATE_PADDED_SIZE               MAX(SURFACE_STATE_PADDED_SIZE_GEN6, SURFACE_STATE_PADDED_SIZE_GEN7)
#define SURFACE_STATE_OFFSET(index)             (SURFACE_STATE_PADDED_SIZE * index)
#define BINDING_TABLE_OFFSET(index)             (SURFACE_STATE_OFFSET(MAX_MEDIA_SURFACES_GEN6) + sizeof(unsigned int) * index)

#define B0_STEP_REV		2
#define IS_STEPPING_BPLUS(i965)	((i965->intel.revision) >= B0_STEP_REV)

static const uint32_t gen75_mfc_batchbuffer_avc[][4] = {
#include "shaders/utils/mfc_batchbuffer_hsw.g75b"
};

static struct i965_kernel gen75_mfc_kernels[] = {
    {
        "MFC AVC INTRA BATCHBUFFER ",
        MFC_BATCHBUFFER_AVC_INTRA,
        gen75_mfc_batchbuffer_avc,
        sizeof(gen75_mfc_batchbuffer_avc),
        NULL
    },
};

#define		INTER_MODE_MASK		0x03
#define		INTER_8X8		0x03
#define		INTER_16X8		0x01
#define		INTER_8X16		0x02
#define		SUBMB_SHAPE_MASK	0x00FF00

#define		INTER_MV8		(4 << 20)
#define		INTER_MV32		(6 << 20)


static void
gen75_mfc_pipe_mode_select(VADriverContextP ctx,
                           int standard_select,
                           struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    assert(standard_select == MFX_FORMAT_MPEG2 ||
           standard_select == MFX_FORMAT_AVC);

    BEGIN_BCS_BATCH(batch, 5);

    OUT_BCS_BATCH(batch, MFX_PIPE_MODE_SELECT | (5 - 2));
    OUT_BCS_BATCH(batch,
                  (MFX_LONG_MODE << 17) | /* Must be long format for encoder */
                  (MFD_MODE_VLD << 15) | /* VLD mode */
                  (0 << 10) | /* Stream-Out Enable */
                  ((!!mfc_context->post_deblocking_output.bo) << 9)  | /* Post Deblocking Output */
                  ((!!mfc_context->pre_deblocking_output.bo) << 8)  | /* Pre Deblocking Output */
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

static void
gen75_mfc_surface_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;

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
gen75_mfc_ind_obj_base_addr_state_bplus(VADriverContextP ctx,
                                        struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct gen6_vme_context *vme_context = encoder_context->vme_context;

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
	
    OUT_BCS_RELOC(batch,
                  mfc_context->mfc_indirect_pak_bse_object.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  mfc_context->mfc_indirect_pak_bse_object.end_offset);
    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);
}

static void
gen75_mfc_ind_obj_base_addr_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    struct i965_driver_data *i965 = i965_driver_data(ctx);

    if (IS_STEPPING_BPLUS(i965)) {
	gen75_mfc_ind_obj_base_addr_state_bplus(ctx, encoder_context);
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
    OUT_BCS_RELOC(batch,
                  mfc_context->mfc_indirect_pak_bse_object.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  mfc_context->mfc_indirect_pak_bse_object.end_offset);

    ADVANCE_BCS_BATCH(batch);
}

static void
gen75_mfc_avc_img_state(VADriverContextP ctx, struct encode_state *encode_state,  
                        struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    VAEncPictureParameterBufferH264 *pPicParameter = (VAEncPictureParameterBufferH264 *)encode_state->pic_param_ext->buffer;

    int width_in_mbs = (mfc_context->surface_state.width + 15) / 16;
    int height_in_mbs = (mfc_context->surface_state.height + 15) / 16;

    BEGIN_BCS_BATCH(batch, 16);

    OUT_BCS_BATCH(batch, MFX_AVC_IMG_STATE | (16 - 2));
    /*DW1. MB setting of frame */
    OUT_BCS_BATCH(batch,
                  ((width_in_mbs * height_in_mbs - 1) & 0xFFFF));
    OUT_BCS_BATCH(batch, 
                  ((height_in_mbs - 1) << 16) | 
                  ((width_in_mbs - 1) << 0));
    /* DW3 QP setting */
    OUT_BCS_BATCH(batch, 
                  (0 << 24) |	/* Second Chroma QP Offset */
                  (0 << 16) |	/* Chroma QP Offset */
                  (0 << 14) |   /* Max-bit conformance Intra flag */
                  (0 << 13) |   /* Max Macroblock size conformance Inter flag */
                  (pPicParameter->pic_fields.bits.weighted_pred_flag << 12) |   /*Weighted_Pred_Flag */
                  (pPicParameter->pic_fields.bits.weighted_bipred_idc << 10) |  /* Weighted_BiPred_Idc */
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
                  (pPicParameter->pic_fields.bits.entropy_coding_mode_flag << 7)  |   /*0:CAVLC encoding mode,1:CABAC*/
                  (0 << 6)  |   /* Only valid for VLD decoding mode */
                  (0 << 5)  |   /* Constrained Intra Predition Flag, from PPS */
                  (0 << 4)  |   /* Direct 8x8 inference flag */
                  (pPicParameter->pic_fields.bits.transform_8x8_mode_flag << 3)  |   /*8x8 or 4x4 IDCT Transform Mode Flag*/
                  (1 << 2)  |   /* Frame MB only flag */
                  (0 << 1)  |   /* MBAFF mode is in active */
                  (0 << 0));    /* Field picture flag */
    /* DW5 Trellis quantization */
    OUT_BCS_BATCH(batch, 0);    /* Mainly about MB rate control and debug, just ignoring */
    OUT_BCS_BATCH(batch,        /* Inter and Intra Conformance Max size limit */
                  (0xBB8 << 16) |       /* InterMbMaxSz */
                  (0xEE8) );            /* IntraMbMaxSz */
    OUT_BCS_BATCH(batch, 0);            /* Reserved */
    /* DW8. QP delta */
    OUT_BCS_BATCH(batch, 0);            /* Slice QP Delta for bitrate control */
    OUT_BCS_BATCH(batch, 0);            /* Slice QP Delta for bitrate control */
    /* DW10. Bit setting for MB */
    OUT_BCS_BATCH(batch, 0x8C000000);
    OUT_BCS_BATCH(batch, 0x00010000);
    /* DW12. */
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0x02010100);
    /* DW14. For short format */
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);
}

static void
gen75_mfc_qm_state(VADriverContextP ctx,
                   int qm_type,
                   unsigned int *qm,
                   int qm_length,
                   struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    unsigned int qm_buffer[16];

    assert(qm_length <= 16);
    assert(sizeof(*qm) == 4);
    memcpy(qm_buffer, qm, qm_length * 4);

    BEGIN_BCS_BATCH(batch, 18);
    OUT_BCS_BATCH(batch, MFX_QM_STATE | (18 - 2));
    OUT_BCS_BATCH(batch, qm_type << 0);
    intel_batchbuffer_data(batch, qm_buffer, 16 * 4);
    ADVANCE_BCS_BATCH(batch);
}

static void
gen75_mfc_avc_qm_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
{
    unsigned int qm[16] = {
        0x10101010, 0x10101010, 0x10101010, 0x10101010,
        0x10101010, 0x10101010, 0x10101010, 0x10101010,
        0x10101010, 0x10101010, 0x10101010, 0x10101010,
        0x10101010, 0x10101010, 0x10101010, 0x10101010
    };

    gen75_mfc_qm_state(ctx, MFX_QM_AVC_4X4_INTRA_MATRIX, qm, 12, encoder_context);
    gen75_mfc_qm_state(ctx, MFX_QM_AVC_4X4_INTER_MATRIX, qm, 12, encoder_context);
    gen75_mfc_qm_state(ctx, MFX_QM_AVC_8x8_INTRA_MATRIX, qm, 16, encoder_context);
    gen75_mfc_qm_state(ctx, MFX_QM_AVC_8x8_INTER_MATRIX, qm, 16, encoder_context);
}

static void
gen75_mfc_fqm_state(VADriverContextP ctx,
                    int fqm_type,
                    unsigned int *fqm,
                    int fqm_length,
                    struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    unsigned int fqm_buffer[32];

    assert(fqm_length <= 32);
    assert(sizeof(*fqm) == 4);
    memcpy(fqm_buffer, fqm, fqm_length * 4);

    BEGIN_BCS_BATCH(batch, 34);
    OUT_BCS_BATCH(batch, MFX_FQM_STATE | (34 - 2));
    OUT_BCS_BATCH(batch, fqm_type << 0);
    intel_batchbuffer_data(batch, fqm_buffer, 32 * 4);
    ADVANCE_BCS_BATCH(batch);
}

static void
gen75_mfc_avc_fqm_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
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

    gen75_mfc_fqm_state(ctx, MFX_QM_AVC_4X4_INTRA_MATRIX, qm, 24, encoder_context);
    gen75_mfc_fqm_state(ctx, MFX_QM_AVC_4X4_INTER_MATRIX, qm, 24, encoder_context);
    gen75_mfc_fqm_state(ctx, MFX_QM_AVC_8x8_INTRA_MATRIX, qm, 32, encoder_context);
    gen75_mfc_fqm_state(ctx, MFX_QM_AVC_8x8_INTER_MATRIX, qm, 32, encoder_context);
}

static void
gen75_mfc_avc_insert_object(VADriverContextP ctx, struct intel_encoder_context *encoder_context,
                            unsigned int *insert_data, int lenght_in_dws, int data_bits_in_last_dw,
                            int skip_emul_byte_count, int is_last_header, int is_end_of_slice, int emulation_flag,
                            struct intel_batchbuffer *batch)
{
    if (batch == NULL)
        batch = encoder_context->base.batch;

    BEGIN_BCS_BATCH(batch, lenght_in_dws + 2);

    OUT_BCS_BATCH(batch, MFX_INSERT_OBJECT | (lenght_in_dws + 2 - 2));
    OUT_BCS_BATCH(batch,
                  (0 << 16) |   /* always start at offset 0 */
                  (data_bits_in_last_dw << 8) |
                  (skip_emul_byte_count << 4) |
                  (!!emulation_flag << 3) |
                  ((!!is_last_header) << 2) |
                  ((!!is_end_of_slice) << 1) |
                  (0 << 0));    /* FIXME: ??? */
    intel_batchbuffer_data(batch, insert_data, lenght_in_dws * 4);

    ADVANCE_BCS_BATCH(batch);
}


static void gen75_mfc_init(VADriverContextP ctx,
                           struct encode_state *encode_state,
                           struct intel_encoder_context *encoder_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    dri_bo *bo;
    int i;
    int width_in_mbs = 0;
    int height_in_mbs = 0;
    int slice_batchbuffer_size;

    if (encoder_context->codec == CODEC_H264 ||
        encoder_context->codec == CODEC_H264_MVC) {
        VAEncSequenceParameterBufferH264 *pSequenceParameter = (VAEncSequenceParameterBufferH264 *)encode_state->seq_param_ext->buffer;
        width_in_mbs = pSequenceParameter->picture_width_in_mbs;
        height_in_mbs = pSequenceParameter->picture_height_in_mbs;
    } else {
        VAEncSequenceParameterBufferMPEG2 *pSequenceParameter = (VAEncSequenceParameterBufferMPEG2 *)encode_state->seq_param_ext->buffer;

        assert(encoder_context->codec == CODEC_MPEG2);

        width_in_mbs = ALIGN(pSequenceParameter->picture_width, 16) / 16;
        height_in_mbs = ALIGN(pSequenceParameter->picture_height, 16) / 16;
    }

    slice_batchbuffer_size = 64 * width_in_mbs * height_in_mbs + 4096 +
		(SLICE_HEADER + SLICE_TAIL) * encode_state->num_slice_params_ext;

    /*Encode common setup for MFC*/
    dri_bo_unreference(mfc_context->post_deblocking_output.bo);
    mfc_context->post_deblocking_output.bo = NULL;

    dri_bo_unreference(mfc_context->pre_deblocking_output.bo);
    mfc_context->pre_deblocking_output.bo = NULL;

    dri_bo_unreference(mfc_context->uncompressed_picture_source.bo);
    mfc_context->uncompressed_picture_source.bo = NULL;

    dri_bo_unreference(mfc_context->mfc_indirect_pak_bse_object.bo); 
    mfc_context->mfc_indirect_pak_bse_object.bo = NULL;

    for (i = 0; i < NUM_MFC_DMV_BUFFERS; i++){
        if (mfc_context->direct_mv_buffers[i].bo != NULL)
            dri_bo_unreference(mfc_context->direct_mv_buffers[i].bo);
        mfc_context->direct_mv_buffers[i].bo = NULL;
    }

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

    dri_bo_unreference(mfc_context->macroblock_status_buffer.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "Buffer",
                      width_in_mbs * height_in_mbs * 16,
                      64);
    assert(bo);
    mfc_context->macroblock_status_buffer.bo = bo;

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

    dri_bo_unreference(mfc_context->mfc_batchbuffer_surface.bo);
    mfc_context->mfc_batchbuffer_surface.bo = NULL;

    dri_bo_unreference(mfc_context->aux_batchbuffer_surface.bo);
    mfc_context->aux_batchbuffer_surface.bo = NULL;

    if (mfc_context->aux_batchbuffer)
        intel_batchbuffer_free(mfc_context->aux_batchbuffer);

    mfc_context->aux_batchbuffer = intel_batchbuffer_new(&i965->intel, I915_EXEC_BSD,
							slice_batchbuffer_size);
    mfc_context->aux_batchbuffer_surface.bo = mfc_context->aux_batchbuffer->buffer;
    dri_bo_reference(mfc_context->aux_batchbuffer_surface.bo);
    mfc_context->aux_batchbuffer_surface.pitch = 16;
    mfc_context->aux_batchbuffer_surface.num_blocks = mfc_context->aux_batchbuffer->size / 16;
    mfc_context->aux_batchbuffer_surface.size_block = 16;

    i965_gpe_context_init(ctx, &mfc_context->gpe_context);
}

static void
gen75_mfc_pipe_buf_addr_state_bplus(VADriverContextP ctx,
                                    struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    int i;

    BEGIN_BCS_BATCH(batch, 61);

    OUT_BCS_BATCH(batch, MFX_PIPE_BUF_ADDR_STATE | (61 - 2));

    /* the DW1-3 is for pre_deblocking */
    if (mfc_context->pre_deblocking_output.bo)
        OUT_BCS_RELOC(batch, mfc_context->pre_deblocking_output.bo,
                      I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                      0);
    else
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
    OUT_BCS_RELOC(batch, mfc_context->macroblock_status_buffer.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  0); /* StreamOut data*/
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
    OUT_BCS_RELOC(batch, mfc_context->macroblock_status_buffer.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  0);											/* Macroblock status buffer*/
	
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
gen75_mfc_pipe_buf_addr_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    int i;

    if (IS_STEPPING_BPLUS(i965)) {
	gen75_mfc_pipe_buf_addr_state_bplus(ctx, encoder_context);
	return;
    }

    BEGIN_BCS_BATCH(batch, 25);

    OUT_BCS_BATCH(batch, MFX_PIPE_BUF_ADDR_STATE | (25 - 2));

    if (mfc_context->pre_deblocking_output.bo)
        OUT_BCS_RELOC(batch, mfc_context->pre_deblocking_output.bo,
                      I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                      0);
    else
        OUT_BCS_BATCH(batch, 0);											/* pre output addr   */

    if (mfc_context->post_deblocking_output.bo)
        OUT_BCS_RELOC(batch, mfc_context->post_deblocking_output.bo,
                      I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                      0);											/* post output addr  */	
    else
        OUT_BCS_BATCH(batch, 0);

    OUT_BCS_RELOC(batch, mfc_context->uncompressed_picture_source.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  0);											/* uncompressed data */
    OUT_BCS_RELOC(batch, mfc_context->macroblock_status_buffer.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  0);											/* StreamOut data*/
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
    OUT_BCS_RELOC(batch, mfc_context->macroblock_status_buffer.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  0);											/* Macroblock status buffer*/

    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);
}

static void
gen75_mfc_avc_directmode_state_bplus(VADriverContextP ctx,
                                     struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;

    int i;

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

static void
gen75_mfc_avc_directmode_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    int i;

    if (IS_STEPPING_BPLUS(i965)) {
	gen75_mfc_avc_directmode_state_bplus(ctx, encoder_context);
	return;
    }

    BEGIN_BCS_BATCH(batch, 69);

    OUT_BCS_BATCH(batch, MFX_AVC_DIRECTMODE_STATE | (69 - 2));

    /* Reference frames and Current frames */
    for(i = 0; i < NUM_MFC_DMV_BUFFERS; i++) {
        if ( mfc_context->direct_mv_buffers[i].bo != NULL) { 
            OUT_BCS_RELOC(batch, mfc_context->direct_mv_buffers[i].bo,
                          I915_GEM_DOMAIN_INSTRUCTION, 0,
                          0);
        } else {
            OUT_BCS_BATCH(batch, 0);
        }
    }

    /* POL list */
    for(i = 0; i < 32; i++) {
        OUT_BCS_BATCH(batch, i/2);
    }
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);
}


static void
gen75_mfc_bsp_buf_base_addr_state_bplus(VADriverContextP ctx,
                                        struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;

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
gen75_mfc_bsp_buf_base_addr_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct i965_driver_data *i965 = i965_driver_data(ctx);

    if (IS_STEPPING_BPLUS(i965)) {
	gen75_mfc_bsp_buf_base_addr_state_bplus(ctx, encoder_context);
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


static void gen75_mfc_avc_pipeline_picture_programing( VADriverContextP ctx,
                                                       struct encode_state *encode_state,
                                                       struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;

    mfc_context->pipe_mode_select(ctx, MFX_FORMAT_AVC, encoder_context);
    mfc_context->set_surface_state(ctx, encoder_context);
    mfc_context->ind_obj_base_addr_state(ctx, encoder_context);
    gen75_mfc_pipe_buf_addr_state(ctx, encoder_context);
    gen75_mfc_bsp_buf_base_addr_state(ctx, encoder_context);
    mfc_context->avc_img_state(ctx, encode_state, encoder_context);
    mfc_context->avc_qm_state(ctx, encoder_context);
    mfc_context->avc_fqm_state(ctx, encoder_context);
    gen75_mfc_avc_directmode_state(ctx, encoder_context); 
    intel_mfc_avc_ref_idx_state(ctx, encode_state, encoder_context);
}


static VAStatus gen75_mfc_run(VADriverContextP ctx, 
                              struct encode_state *encode_state,
                              struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;

    intel_batchbuffer_flush(batch);		//run the pipeline

    return VA_STATUS_SUCCESS;
}


static VAStatus
gen75_mfc_stop(VADriverContextP ctx, 
               struct encode_state *encode_state,
               struct intel_encoder_context *encoder_context,
               int *encoded_bits_size)
{
    VAStatus vaStatus = VA_STATUS_ERROR_UNKNOWN;
    VAEncPictureParameterBufferH264 *pPicParameter = (VAEncPictureParameterBufferH264 *)encode_state->pic_param_ext->buffer;
    VACodedBufferSegment *coded_buffer_segment;
    
    vaStatus = i965_MapBuffer(ctx, pPicParameter->coded_buf, (void **)&coded_buffer_segment);
    assert(vaStatus == VA_STATUS_SUCCESS);
    *encoded_bits_size = coded_buffer_segment->size * 8;
    i965_UnmapBuffer(ctx, pPicParameter->coded_buf);

    return VA_STATUS_SUCCESS;
}


static void
gen75_mfc_avc_slice_state(VADriverContextP ctx,
                          VAEncPictureParameterBufferH264 *pic_param,
                          VAEncSliceParameterBufferH264 *slice_param,
                          struct encode_state *encode_state,
                          struct intel_encoder_context *encoder_context,
                          int rate_control_enable,
                          int qp,
                          struct intel_batchbuffer *batch)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    int width_in_mbs = (mfc_context->surface_state.width + 15) / 16;
    int height_in_mbs = (mfc_context->surface_state.height + 15) / 16;
    int beginmb = slice_param->macroblock_address;
    int endmb = beginmb + slice_param->num_macroblocks;
    int beginx = beginmb % width_in_mbs;
    int beginy = beginmb / width_in_mbs;
    int nextx =  endmb % width_in_mbs;
    int nexty = endmb / width_in_mbs;
    int slice_type = intel_avc_enc_slice_type_fixup(slice_param->slice_type);
    int last_slice = (endmb == (width_in_mbs * height_in_mbs));
    int maxQpN, maxQpP;
    unsigned char correct[6], grow, shrink;
    int i;
    int weighted_pred_idc = 0;
    unsigned int luma_log2_weight_denom = slice_param->luma_log2_weight_denom;
    unsigned int chroma_log2_weight_denom = slice_param->chroma_log2_weight_denom;
    int num_ref_l0 = 0, num_ref_l1 = 0;

    if (batch == NULL)
        batch = encoder_context->base.batch;

    if (slice_type == SLICE_TYPE_I) {
        luma_log2_weight_denom = 0;
        chroma_log2_weight_denom = 0;
    } else if (slice_type == SLICE_TYPE_P) {
        weighted_pred_idc = pic_param->pic_fields.bits.weighted_pred_flag;
        num_ref_l0 = pic_param->num_ref_idx_l0_active_minus1 + 1;

        if (slice_param->num_ref_idx_active_override_flag)
            num_ref_l0 = slice_param->num_ref_idx_l0_active_minus1 + 1;
    } else if (slice_type == SLICE_TYPE_B) {
        weighted_pred_idc = pic_param->pic_fields.bits.weighted_bipred_idc;
        num_ref_l0 = pic_param->num_ref_idx_l0_active_minus1 + 1;
        num_ref_l1 = pic_param->num_ref_idx_l1_active_minus1 + 1;

        if (slice_param->num_ref_idx_active_override_flag) {
            num_ref_l0 = slice_param->num_ref_idx_l0_active_minus1 + 1;
            num_ref_l1 = slice_param->num_ref_idx_l1_active_minus1 + 1;
        }

        if (weighted_pred_idc == 2) {
            /* 8.4.3 - Derivation process for prediction weights (8-279) */
            luma_log2_weight_denom = 5;
            chroma_log2_weight_denom = 5;
        }
    }

    maxQpN = mfc_context->bit_rate_control_context[slice_type].MaxQpNegModifier;
    maxQpP = mfc_context->bit_rate_control_context[slice_type].MaxQpPosModifier;

    for (i = 0; i < 6; i++)
        correct[i] = mfc_context->bit_rate_control_context[slice_type].Correct[i];

    grow = mfc_context->bit_rate_control_context[slice_type].GrowInit + 
        (mfc_context->bit_rate_control_context[slice_type].GrowResistance << 4);
    shrink = mfc_context->bit_rate_control_context[slice_type].ShrinkInit + 
        (mfc_context->bit_rate_control_context[slice_type].ShrinkResistance << 4);

    BEGIN_BCS_BATCH(batch, 11);;

    OUT_BCS_BATCH(batch, MFX_AVC_SLICE_STATE | (11 - 2) );
    OUT_BCS_BATCH(batch, slice_type);			/*Slice Type: I:P:B Slice*/

    OUT_BCS_BATCH(batch,
                  (num_ref_l0 << 16) |
                  (num_ref_l1 << 24) |
                  (chroma_log2_weight_denom << 8) |
                  (luma_log2_weight_denom << 0));

    OUT_BCS_BATCH(batch, 
                  (weighted_pred_idc << 30) |
                  (slice_param->direct_spatial_mv_pred_flag<<29) |             /*Direct Prediction Type*/
                  (slice_param->disable_deblocking_filter_idc << 27) |
                  (slice_param->cabac_init_idc << 24) |
                  (qp<<16) | 			/*Slice Quantization Parameter*/
                  ((slice_param->slice_beta_offset_div2 & 0xf) << 8) |
                  ((slice_param->slice_alpha_c0_offset_div2 & 0xf) << 0));
    OUT_BCS_BATCH(batch,
                  (beginy << 24) |			/*First MB X&Y , the begin postion of current slice*/
                  (beginx << 16) |
                  slice_param->macroblock_address );
    OUT_BCS_BATCH(batch, (nexty << 16) | nextx);                       /*Next slice first MB X&Y*/
    OUT_BCS_BATCH(batch, 
                  (0/*rate_control_enable*/ << 31) |		/*in CBR mode RateControlCounterEnable = enable*/
                  (1 << 30) |		/*ResetRateControlCounter*/
                  (0 << 28) |		/*RC Triggle Mode = Always Rate Control*/
                  (4 << 24) |     /*RC Stable Tolerance, middle level*/
                  (0/*rate_control_enable*/ << 23) |     /*RC Panic Enable*/                 
                  (0 << 22) |     /*QP mode, don't modfiy CBP*/
                  (0 << 21) |     /*MB Type Direct Conversion Enabled*/ 
                  (0 << 20) |     /*MB Type Skip Conversion Enabled*/ 
                  (last_slice << 19) |     /*IsLastSlice*/
                  (0 << 18) | 	/*BitstreamOutputFlag Compressed BitStream Output Disable Flag 0:enable 1:disable*/
                  (1 << 17) |	    /*HeaderPresentFlag*/	
                  (1 << 16) |	    /*SliceData PresentFlag*/
                  (1 << 15) |	    /*TailPresentFlag*/
                  (1 << 13) |	    /*RBSP NAL TYPE*/	
                  (0 << 12) );    /*CabacZeroWordInsertionEnable*/
    OUT_BCS_BATCH(batch, mfc_context->mfc_indirect_pak_bse_object.offset);
    OUT_BCS_BATCH(batch,
                  (maxQpN << 24) |     /*Target QP - 24 is lowest QP*/ 
                  (maxQpP << 16) |     /*Target QP + 20 is highest QP*/
                  (shrink << 8)  |
                  (grow << 0));   
    OUT_BCS_BATCH(batch,
                  (correct[5] << 20) |
                  (correct[4] << 16) |
                  (correct[3] << 12) |
                  (correct[2] << 8) |
                  (correct[1] << 4) |
                  (correct[0] << 0));
    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);
}


#if MFC_SOFTWARE_HASWELL

static int
gen75_mfc_avc_pak_object_intra(VADriverContextP ctx, int x, int y, int end_mb,
                               int qp,unsigned int *msg,
                               struct intel_encoder_context *encoder_context,
                               unsigned char target_mb_size, unsigned char max_mb_size,
                               struct intel_batchbuffer *batch)
{
    int len_in_dwords = 12;
    unsigned int intra_msg;
#define		INTRA_MSG_FLAG		(1 << 13)
#define		INTRA_MBTYPE_MASK	(0x1F0000)
    if (batch == NULL)
        batch = encoder_context->base.batch;

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

    OUT_BCS_BATCH(batch, (0xFFFF << 16) | (y << 8) | x);		/* Code Block Pattern for Y*/
    OUT_BCS_BATCH(batch, 0x000F000F);							/* Code Block Pattern */		
    OUT_BCS_BATCH(batch, (0 << 27) | (end_mb << 26) | qp);	/* Last MB */

    /*Stuff for Intra MB*/
    OUT_BCS_BATCH(batch, msg[1]);			/* We using Intra16x16 no 4x4 predmode*/	
    OUT_BCS_BATCH(batch, msg[2]);	
    OUT_BCS_BATCH(batch, msg[3]&0xFF);	
    
    /*MaxSizeInWord and TargetSzieInWord*/
    OUT_BCS_BATCH(batch, (max_mb_size << 24) |
                  (target_mb_size << 16) );

    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);

    return len_in_dwords;
}

static int
gen75_mfc_avc_pak_object_inter(VADriverContextP ctx, int x, int y, int end_mb, int qp,
                               unsigned int *msg, unsigned int offset,
                               struct intel_encoder_context *encoder_context,
                               unsigned char target_mb_size,unsigned char max_mb_size, int slice_type,
                               struct intel_batchbuffer *batch)
{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    int len_in_dwords = 12;
    unsigned int inter_msg = 0;
    if (batch == NULL)
        batch = encoder_context->base.batch;
    {
#define MSG_MV_OFFSET	4
	unsigned int *mv_ptr;
	mv_ptr = msg + MSG_MV_OFFSET;
	/* MV of VME output is based on 16 sub-blocks. So it is necessary
         * to convert them to be compatible with the format of AVC_PAK
         * command.
         */
	if ((msg[0] & INTER_MODE_MASK) == INTER_8X16) {
            /* MV[0] and MV[2] are replicated */
            mv_ptr[4] = mv_ptr[0];
            mv_ptr[5] = mv_ptr[1];
            mv_ptr[2] = mv_ptr[8];
            mv_ptr[3] = mv_ptr[9];
            mv_ptr[6] = mv_ptr[8];
            mv_ptr[7] = mv_ptr[9];
	} else if ((msg[0] & INTER_MODE_MASK) == INTER_16X8) {
            /* MV[0] and MV[1] are replicated */
            mv_ptr[2] = mv_ptr[0];
            mv_ptr[3] = mv_ptr[1];
            mv_ptr[4] = mv_ptr[16];
            mv_ptr[5] = mv_ptr[17];
            mv_ptr[6] = mv_ptr[24];
            mv_ptr[7] = mv_ptr[25];
	} else if (((msg[0] & INTER_MODE_MASK) == INTER_8X8) &&
                   !(msg[1] & SUBMB_SHAPE_MASK)) {
            /* Don't touch MV[0] or MV[1] */
            mv_ptr[2] = mv_ptr[8];
            mv_ptr[3] = mv_ptr[9];
            mv_ptr[4] = mv_ptr[16];
            mv_ptr[5] = mv_ptr[17];
            mv_ptr[6] = mv_ptr[24];
            mv_ptr[7] = mv_ptr[25];
	}
    }

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
    inter_msg |= ((1 << 19) | (1 << 18) | (1 << 17));
    if (((msg[0] & INTER_MODE_MASK) == INTER_8X8) &&
        (msg[1] & SUBMB_SHAPE_MASK)) {
        inter_msg |= INTER_MV32;
    }

    OUT_BCS_BATCH(batch, inter_msg);

    OUT_BCS_BATCH(batch, (0xFFFF<<16) | (y << 8) | x);        /* Code Block Pattern for Y*/
    OUT_BCS_BATCH(batch, 0x000F000F);                         /* Code Block Pattern */  
#if 0 
    if ( slice_type == SLICE_TYPE_B) {
        OUT_BCS_BATCH(batch, (0xF<<28) | (end_mb << 26) | qp);	/* Last MB */
    } else {
        OUT_BCS_BATCH(batch, (end_mb << 26) | qp);	/* Last MB */
    }
#else
    OUT_BCS_BATCH(batch, (end_mb << 26) | qp);	/* Last MB */
#endif

    inter_msg = msg[1] >> 8;
    /*Stuff for Inter MB*/
    OUT_BCS_BATCH(batch, inter_msg);        
    OUT_BCS_BATCH(batch, vme_context->ref_index_in_mb[0]);
    OUT_BCS_BATCH(batch, vme_context->ref_index_in_mb[1]);

    /*MaxSizeInWord and TargetSzieInWord*/
    OUT_BCS_BATCH(batch, (max_mb_size << 24) |
                  (target_mb_size << 16) );

    OUT_BCS_BATCH(batch, 0x0);    

    ADVANCE_BCS_BATCH(batch);

    return len_in_dwords;
}

static void 
gen75_mfc_avc_pipeline_slice_programing(VADriverContextP ctx,
                                        struct encode_state *encode_state,
                                        struct intel_encoder_context *encoder_context,
                                        int slice_index,
                                        struct intel_batchbuffer *slice_batch)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    VAEncSequenceParameterBufferH264 *pSequenceParameter = (VAEncSequenceParameterBufferH264 *)encode_state->seq_param_ext->buffer;
    VAEncPictureParameterBufferH264 *pPicParameter = (VAEncPictureParameterBufferH264 *)encode_state->pic_param_ext->buffer;
    VAEncSliceParameterBufferH264 *pSliceParameter = (VAEncSliceParameterBufferH264 *)encode_state->slice_params_ext[slice_index]->buffer; 
    unsigned int *msg = NULL, offset = 0;
    unsigned char *msg_ptr = NULL;
    int width_in_mbs = (mfc_context->surface_state.width + 15) / 16;
    int height_in_mbs = (mfc_context->surface_state.height + 15) / 16;
    int last_slice = (pSliceParameter->macroblock_address + pSliceParameter->num_macroblocks) == (width_in_mbs * height_in_mbs);
    int i,x,y;
    int qp = pPicParameter->pic_init_qp + pSliceParameter->slice_qp_delta;
    unsigned int rate_control_mode = encoder_context->rate_control_mode;
    unsigned int tail_data[] = { 0x0, 0x0 };
    int slice_type = intel_avc_enc_slice_type_fixup(pSliceParameter->slice_type);
    int is_intra = slice_type == SLICE_TYPE_I;
    int qp_slice;

    qp_slice = qp;
    if (rate_control_mode == VA_RC_CBR) {
        qp = mfc_context->bit_rate_control_context[slice_type].QpPrimeY;
        if (encode_state->slice_header_index[slice_index] == 0) {
            pSliceParameter->slice_qp_delta = qp - pPicParameter->pic_init_qp;
            qp_slice = qp;
        }
    }

    /* only support for 8-bit pixel bit-depth */
    assert(pSequenceParameter->bit_depth_luma_minus8 == 0);
    assert(pSequenceParameter->bit_depth_chroma_minus8 == 0);
    assert(pPicParameter->pic_init_qp >= 0 && pPicParameter->pic_init_qp < 52);
    assert(qp >= 0 && qp < 52);

    gen75_mfc_avc_slice_state(ctx,
                              pPicParameter,
                              pSliceParameter,
                              encode_state, encoder_context,
                              (rate_control_mode == VA_RC_CBR), qp_slice, slice_batch);

    if ( slice_index == 0)
        intel_mfc_avc_pipeline_header_programing(ctx, encode_state, encoder_context, slice_batch);

    intel_avc_slice_insert_packed_data(ctx, encode_state, encoder_context, slice_index, slice_batch);

    dri_bo_map(vme_context->vme_output.bo , 1);
    msg_ptr = (unsigned char *)vme_context->vme_output.bo->virtual;

    if (is_intra) {
        msg = (unsigned int *) (msg_ptr + pSliceParameter->macroblock_address * vme_context->vme_output.size_block);
    } else {
        msg = (unsigned int *) (msg_ptr + pSliceParameter->macroblock_address * vme_context->vme_output.size_block);
    }
   
    for (i = pSliceParameter->macroblock_address; 
         i < pSliceParameter->macroblock_address + pSliceParameter->num_macroblocks; i++) {
        int last_mb = (i == (pSliceParameter->macroblock_address + pSliceParameter->num_macroblocks - 1) );
        x = i % width_in_mbs;
        y = i / width_in_mbs;
        msg = (unsigned int *) (msg_ptr + i * vme_context->vme_output.size_block);

        if (is_intra) {
            assert(msg);
            gen75_mfc_avc_pak_object_intra(ctx, x, y, last_mb, qp, msg, encoder_context, 0, 0, slice_batch);
        } else {
	    int inter_rdo, intra_rdo;
	    inter_rdo = msg[AVC_INTER_RDO_OFFSET] & AVC_RDO_MASK;
	    intra_rdo = msg[AVC_INTRA_RDO_OFFSET] & AVC_RDO_MASK;
	    offset = i * vme_context->vme_output.size_block + AVC_INTER_MV_OFFSET;
	    if (intra_rdo < inter_rdo) { 
                gen75_mfc_avc_pak_object_intra(ctx, x, y, last_mb, qp, msg, encoder_context, 0, 0, slice_batch);
            } else {
		msg += AVC_INTER_MSG_OFFSET;
                gen75_mfc_avc_pak_object_inter(ctx, x, y, last_mb, qp, msg, offset, encoder_context, 0, 0, slice_type, slice_batch);
            }
        }
    }
   
    dri_bo_unmap(vme_context->vme_output.bo);

    if ( last_slice ) {    
        mfc_context->insert_object(ctx, encoder_context,
                                   tail_data, 2, 8,
                                   2, 1, 1, 0, slice_batch);
    } else {
        mfc_context->insert_object(ctx, encoder_context,
                                   tail_data, 1, 8,
                                   1, 1, 1, 0, slice_batch);
    }
}

static dri_bo *
gen75_mfc_avc_software_batchbuffer(VADriverContextP ctx,
                                   struct encode_state *encode_state,
                                   struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct intel_batchbuffer *batch;
    dri_bo *batch_bo;
    int i;
    int buffer_size;

    batch = mfc_context->aux_batchbuffer;
    batch_bo = batch->buffer;
    for (i = 0; i < encode_state->num_slice_params_ext; i++) {
        gen75_mfc_avc_pipeline_slice_programing(ctx, encode_state, encoder_context, i, batch);
    }

    intel_batchbuffer_align(batch, 8);
    
    BEGIN_BCS_BATCH(batch, 2);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, MI_BATCH_BUFFER_END);
    ADVANCE_BCS_BATCH(batch);

    dri_bo_reference(batch_bo);

    intel_batchbuffer_free(batch);
    mfc_context->aux_batchbuffer = NULL;

    return batch_bo;
}

#else

static void
gen75_mfc_batchbuffer_surfaces_input(VADriverContextP ctx,
                                     struct encode_state *encode_state,
                                     struct intel_encoder_context *encoder_context)

{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;

    assert(vme_context->vme_output.bo);
    mfc_context->buffer_suface_setup(ctx,
                                     &mfc_context->gpe_context,
                                     &vme_context->vme_output,
                                     BINDING_TABLE_OFFSET(BIND_IDX_VME_OUTPUT),
                                     SURFACE_STATE_OFFSET(BIND_IDX_VME_OUTPUT));
}

static void
gen75_mfc_batchbuffer_surfaces_output(VADriverContextP ctx,
                                      struct encode_state *encode_state,
                                      struct intel_encoder_context *encoder_context)

{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    assert(mfc_context->aux_batchbuffer_surface.bo);
    mfc_context->buffer_suface_setup(ctx,
                                     &mfc_context->gpe_context,
                                     &mfc_context->aux_batchbuffer_surface,
                                     BINDING_TABLE_OFFSET(BIND_IDX_MFC_BATCHBUFFER),
                                     SURFACE_STATE_OFFSET(BIND_IDX_MFC_BATCHBUFFER));
}

static void
gen75_mfc_batchbuffer_surfaces_setup(VADriverContextP ctx, 
                                     struct encode_state *encode_state,
                                     struct intel_encoder_context *encoder_context)
{
    gen75_mfc_batchbuffer_surfaces_input(ctx, encode_state, encoder_context);
    gen75_mfc_batchbuffer_surfaces_output(ctx, encode_state, encoder_context);
}

static void
gen75_mfc_batchbuffer_idrt_setup(VADriverContextP ctx, 
                                 struct encode_state *encode_state,
                                 struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct gen6_interface_descriptor_data *desc;   
    int i;
    dri_bo *bo;

    bo = mfc_context->gpe_context.idrt.bo;
    dri_bo_map(bo, 1);
    assert(bo->virtual);
    desc = bo->virtual;

    for (i = 0; i < mfc_context->gpe_context.num_kernels; i++) {
        struct i965_kernel *kernel;

        kernel = &mfc_context->gpe_context.kernels[i];
        assert(sizeof(*desc) == 32);

        /*Setup the descritor table*/
        memset(desc, 0, sizeof(*desc));
        desc->desc0.kernel_start_pointer = (kernel->bo->offset >> 6);
        desc->desc2.sampler_count = 0;
        desc->desc2.sampler_state_pointer = 0;
        desc->desc3.binding_table_entry_count = 2;
        desc->desc3.binding_table_pointer = (BINDING_TABLE_OFFSET(0) >> 5);
        desc->desc4.constant_urb_entry_read_offset = 0;
        desc->desc4.constant_urb_entry_read_length = 4;
 		
        /*kernel start*/
        dri_bo_emit_reloc(bo,	
                          I915_GEM_DOMAIN_INSTRUCTION, 0,
                          0,
                          i * sizeof(*desc) + offsetof(struct gen6_interface_descriptor_data, desc0),
                          kernel->bo);
        desc++;
    }

    dri_bo_unmap(bo);
}

static void
gen75_mfc_batchbuffer_constant_setup(VADriverContextP ctx, 
                                     struct encode_state *encode_state,
                                     struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    
    (void)mfc_context;
}

#define AVC_PAK_LEN_IN_BYTE	48
#define AVC_PAK_LEN_IN_OWORD	3

static void
gen75_mfc_batchbuffer_emit_object_command(struct intel_batchbuffer *batch,
                                          uint32_t intra_flag,
                                          int head_offset,
                                          int number_mb_cmds,
                                          int slice_end_x,
                                          int slice_end_y,
                                          int mb_x,
                                          int mb_y,
                                          int width_in_mbs,
                                          int qp,
					  uint32_t fwd_ref,
					  uint32_t bwd_ref)
{
    uint32_t temp_value;
    BEGIN_BATCH(batch, 14);
    
    OUT_BATCH(batch, CMD_MEDIA_OBJECT | (14 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
   
    /*inline data */
    OUT_BATCH(batch, head_offset / 16);
    OUT_BATCH(batch, (intra_flag) | (qp << 16));
    temp_value = (mb_x | (mb_y << 8) | (width_in_mbs << 16));
    OUT_BATCH(batch, temp_value);

    OUT_BATCH(batch, number_mb_cmds);

    OUT_BATCH(batch,
              ((slice_end_y << 8) | (slice_end_x)));
    OUT_BATCH(batch, fwd_ref);
    OUT_BATCH(batch, bwd_ref);

    OUT_BATCH(batch, MI_NOOP);

    ADVANCE_BATCH(batch);
}

static void
gen75_mfc_avc_batchbuffer_slice_command(VADriverContextP ctx,
                                        struct intel_encoder_context *encoder_context,
                                        VAEncSliceParameterBufferH264 *slice_param,
                                        int head_offset,
                                        int qp,
                                        int last_slice)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    int width_in_mbs = (mfc_context->surface_state.width + 15) / 16;
    int total_mbs = slice_param->num_macroblocks;
    int slice_type = intel_avc_enc_slice_type_fixup(slice_param->slice_type);
    int number_mb_cmds = 128;
    int starting_offset = 0;
    int mb_x, mb_y;
    int last_mb, slice_end_x, slice_end_y;
    int remaining_mb = total_mbs;
    uint32_t fwd_ref , bwd_ref, mb_flag;

    last_mb = slice_param->macroblock_address + total_mbs - 1;
    slice_end_x = last_mb % width_in_mbs;
    slice_end_y = last_mb / width_in_mbs;

    if (slice_type == SLICE_TYPE_I) {
	fwd_ref = 0;
	bwd_ref = 0;
	mb_flag = 1;
    } else {
	fwd_ref = vme_context->ref_index_in_mb[0];
	bwd_ref = vme_context->ref_index_in_mb[1];
	mb_flag = 0;
    }

    if (width_in_mbs >= 100) {
	number_mb_cmds = width_in_mbs / 5;
    } else if (width_in_mbs >= 80) {
	number_mb_cmds = width_in_mbs / 4;
    } else if (width_in_mbs >= 60) {
	number_mb_cmds = width_in_mbs / 3;
    } else if (width_in_mbs >= 40) {
	number_mb_cmds = width_in_mbs / 2;
    } else {
	number_mb_cmds = width_in_mbs;
    }

    do {
	if (number_mb_cmds >= remaining_mb) {
		number_mb_cmds = remaining_mb;
	}
	mb_x = (slice_param->macroblock_address + starting_offset) % width_in_mbs;
	mb_y = (slice_param->macroblock_address + starting_offset) / width_in_mbs;

        gen75_mfc_batchbuffer_emit_object_command(batch,
						  mb_flag,
                                                  head_offset,
                                                  number_mb_cmds,
						  slice_end_x,
						  slice_end_y,
                                                  mb_x,
                                                  mb_y,
                                                  width_in_mbs,
                                                  qp,
						  fwd_ref,
						  bwd_ref);

	head_offset += (number_mb_cmds * AVC_PAK_LEN_IN_BYTE);
	remaining_mb -= number_mb_cmds;
	starting_offset += number_mb_cmds;
    } while (remaining_mb > 0);
}
                          
/*
 * return size in Owords (16bytes)
 */         
static void
gen75_mfc_avc_batchbuffer_slice(VADriverContextP ctx,
                                struct encode_state *encode_state,
                                struct intel_encoder_context *encoder_context,
                                int slice_index)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct intel_batchbuffer *slice_batch = mfc_context->aux_batchbuffer;
    VAEncSequenceParameterBufferH264 *pSequenceParameter = (VAEncSequenceParameterBufferH264 *)encode_state->seq_param_ext->buffer;
    VAEncPictureParameterBufferH264 *pPicParameter = (VAEncPictureParameterBufferH264 *)encode_state->pic_param_ext->buffer;
    VAEncSliceParameterBufferH264 *pSliceParameter = (VAEncSliceParameterBufferH264 *)encode_state->slice_params_ext[slice_index]->buffer; 
    int width_in_mbs = (mfc_context->surface_state.width + 15) / 16;
    int height_in_mbs = (mfc_context->surface_state.height + 15) / 16;
    int last_slice = (pSliceParameter->macroblock_address + pSliceParameter->num_macroblocks) == (width_in_mbs * height_in_mbs);
    int qp = pPicParameter->pic_init_qp + pSliceParameter->slice_qp_delta;
    unsigned int rate_control_mode = encoder_context->rate_control_mode;
    unsigned int tail_data[] = { 0x0, 0x0 };
    long head_offset;
    int slice_type = intel_avc_enc_slice_type_fixup(pSliceParameter->slice_type);
    int qp_slice;

    qp_slice = qp;
    if (rate_control_mode == VA_RC_CBR) {
        qp = mfc_context->bit_rate_control_context[slice_type].QpPrimeY;
        if (encode_state->slice_header_index[slice_index] == 0) {
            pSliceParameter->slice_qp_delta = qp - pPicParameter->pic_init_qp;
            qp_slice = qp;
        }
    }

    /* only support for 8-bit pixel bit-depth */
    assert(pSequenceParameter->bit_depth_luma_minus8 == 0);
    assert(pSequenceParameter->bit_depth_chroma_minus8 == 0);
    assert(pPicParameter->pic_init_qp >= 0 && pPicParameter->pic_init_qp < 52);
    assert(qp >= 0 && qp < 52);

    gen75_mfc_avc_slice_state(ctx,
                              pPicParameter,
                              pSliceParameter,
                              encode_state,
                              encoder_context,
                              (rate_control_mode == VA_RC_CBR),
                              qp_slice,
                              slice_batch);

    if (slice_index == 0)
        intel_mfc_avc_pipeline_header_programing(ctx, encode_state, encoder_context, slice_batch);

    intel_avc_slice_insert_packed_data(ctx, encode_state, encoder_context, slice_index, slice_batch);

    intel_batchbuffer_align(slice_batch, 16); /* aligned by an Oword */
    head_offset = intel_batchbuffer_used_size(slice_batch);

    slice_batch->ptr += pSliceParameter->num_macroblocks * AVC_PAK_LEN_IN_BYTE;

    gen75_mfc_avc_batchbuffer_slice_command(ctx,
                                            encoder_context,
                                            pSliceParameter,
                                            head_offset,
                                            qp,
                                            last_slice);


    /* Aligned for tail */
    intel_batchbuffer_align(slice_batch, 16); /* aligned by an Oword */
    if (last_slice) {    
        mfc_context->insert_object(ctx,
                                   encoder_context,
                                   tail_data,
                                   2,
                                   8,
                                   2,
                                   1,
                                   1,
                                   0,
                                   slice_batch);
    } else {
        mfc_context->insert_object(ctx,
                                   encoder_context,
                                   tail_data,
                                   1,
                                   8,
                                   1,
                                   1,
                                   1,
                                   0,
                                   slice_batch);
    }

    return;
}

static void
gen75_mfc_avc_batchbuffer_pipeline(VADriverContextP ctx,
                                   struct encode_state *encode_state,
                                   struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    int i;
    intel_batchbuffer_start_atomic(batch, 0x4000); 
    gen6_gpe_pipeline_setup(ctx, &mfc_context->gpe_context, batch);

    for ( i = 0; i < encode_state->num_slice_params_ext; i++) {
        gen75_mfc_avc_batchbuffer_slice(ctx, encode_state, encoder_context, i);
    }
    {
	struct intel_batchbuffer *slice_batch = mfc_context->aux_batchbuffer;
	intel_batchbuffer_align(slice_batch, 8);
    	BEGIN_BCS_BATCH(slice_batch, 2);
	OUT_BCS_BATCH(slice_batch, 0);
	OUT_BCS_BATCH(slice_batch, MI_BATCH_BUFFER_END);
	ADVANCE_BCS_BATCH(slice_batch);
	mfc_context->aux_batchbuffer = NULL;
	intel_batchbuffer_free(slice_batch);
    }
    intel_batchbuffer_end_atomic(batch);
    intel_batchbuffer_flush(batch);
}

static void
gen75_mfc_build_avc_batchbuffer(VADriverContextP ctx, 
                                struct encode_state *encode_state,
                                struct intel_encoder_context *encoder_context)
{
    gen75_mfc_batchbuffer_surfaces_setup(ctx, encode_state, encoder_context);
    gen75_mfc_batchbuffer_idrt_setup(ctx, encode_state, encoder_context);
    gen75_mfc_batchbuffer_constant_setup(ctx, encode_state, encoder_context);
    gen75_mfc_avc_batchbuffer_pipeline(ctx, encode_state, encoder_context);
}

static dri_bo *
gen75_mfc_avc_hardware_batchbuffer(VADriverContextP ctx,
                                   struct encode_state *encode_state,
                                   struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;

    dri_bo_reference(mfc_context->aux_batchbuffer_surface.bo);
    gen75_mfc_build_avc_batchbuffer(ctx, encode_state, encoder_context);

    return mfc_context->aux_batchbuffer_surface.bo;
}

#endif

static void
gen75_mfc_avc_pipeline_programing(VADriverContextP ctx,
                                  struct encode_state *encode_state,
                                  struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    dri_bo *slice_batch_bo;

    if ( intel_mfc_interlace_check(ctx, encode_state, encoder_context) ) {
        fprintf(stderr, "Current VA driver don't support interlace mode!\n");
        assert(0);
        return; 
    }

#if MFC_SOFTWARE_HASWELL
    slice_batch_bo = gen75_mfc_avc_software_batchbuffer(ctx, encode_state, encoder_context);
#else
    slice_batch_bo = gen75_mfc_avc_hardware_batchbuffer(ctx, encode_state, encoder_context);
#endif

    // begin programing
    intel_batchbuffer_start_atomic_bcs(batch, 0x4000); 
    intel_batchbuffer_emit_mi_flush(batch);
    
    // picture level programing
    gen75_mfc_avc_pipeline_picture_programing(ctx, encode_state, encoder_context);

    BEGIN_BCS_BATCH(batch, 2);
    OUT_BCS_BATCH(batch, MI_BATCH_BUFFER_START | (1 << 8));
    OUT_BCS_RELOC(batch,
                  slice_batch_bo,
                  I915_GEM_DOMAIN_COMMAND, 0, 
                  0);
    ADVANCE_BCS_BATCH(batch);

    // end programing
    intel_batchbuffer_end_atomic(batch);

    dri_bo_unreference(slice_batch_bo);
}


static VAStatus
gen75_mfc_avc_encode_picture(VADriverContextP ctx, 
                             struct encode_state *encode_state,
                             struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    unsigned int rate_control_mode = encoder_context->rate_control_mode;
    int current_frame_bits_size;
    int sts;
 
    for (;;) {
        gen75_mfc_init(ctx, encode_state, encoder_context);
        intel_mfc_avc_prepare(ctx, encode_state, encoder_context);
        /*Programing bcs pipeline*/
        gen75_mfc_avc_pipeline_programing(ctx, encode_state, encoder_context);	//filling the pipeline
        gen75_mfc_run(ctx, encode_state, encoder_context);
        if (rate_control_mode == VA_RC_CBR /*|| rate_control_mode == VA_RC_VBR*/) {
            gen75_mfc_stop(ctx, encode_state, encoder_context, &current_frame_bits_size);
            sts = intel_mfc_brc_postpack(encode_state, mfc_context, current_frame_bits_size);
            if (sts == BRC_NO_HRD_VIOLATION) {
                intel_mfc_hrd_context_update(encode_state, mfc_context);
                break;
            }
            else if (sts == BRC_OVERFLOW_WITH_MIN_QP || sts == BRC_UNDERFLOW_WITH_MAX_QP) {
                if (!mfc_context->hrd.violation_noted) {
                    fprintf(stderr, "Unrepairable %s!\n", (sts == BRC_OVERFLOW_WITH_MIN_QP)? "overflow": "underflow");
                    mfc_context->hrd.violation_noted = 1;
                }
                return VA_STATUS_SUCCESS;
            }
        } else {
            break;
        }
    }

    return VA_STATUS_SUCCESS;
}

/*
 * MPEG-2
 */

static const int
va_to_gen75_mpeg2_picture_type[3] = {
    1,  /* I */
    2,  /* P */
    3   /* B */
};

static void
gen75_mfc_mpeg2_pic_state(VADriverContextP ctx,
                          struct intel_encoder_context *encoder_context,
                          struct encode_state *encode_state)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    VAEncPictureParameterBufferMPEG2 *pic_param;
    int width_in_mbs = (mfc_context->surface_state.width + 15) / 16;
    int height_in_mbs = (mfc_context->surface_state.height + 15) / 16;
    VAEncSliceParameterBufferMPEG2 *slice_param = NULL;

    assert(encode_state->pic_param_ext && encode_state->pic_param_ext->buffer);
    pic_param = (VAEncPictureParameterBufferMPEG2 *)encode_state->pic_param_ext->buffer;
    slice_param = (VAEncSliceParameterBufferMPEG2 *)encode_state->slice_params_ext[0]->buffer;

    BEGIN_BCS_BATCH(batch, 13);
    OUT_BCS_BATCH(batch, MFX_MPEG2_PIC_STATE | (13 - 2));
    OUT_BCS_BATCH(batch,
                  (pic_param->f_code[1][1] & 0xf) << 28 | /* f_code[1][1] */
                  (pic_param->f_code[1][0] & 0xf) << 24 | /* f_code[1][0] */
                  (pic_param->f_code[0][1] & 0xf) << 20 | /* f_code[0][1] */
                  (pic_param->f_code[0][0] & 0xf) << 16 | /* f_code[0][0] */
                  pic_param->picture_coding_extension.bits.intra_dc_precision << 14 |
                  pic_param->picture_coding_extension.bits.picture_structure << 12 |
                  pic_param->picture_coding_extension.bits.top_field_first << 11 |
                  pic_param->picture_coding_extension.bits.frame_pred_frame_dct << 10 |
                  pic_param->picture_coding_extension.bits.concealment_motion_vectors << 9 |
                  pic_param->picture_coding_extension.bits.q_scale_type << 8 |
                  pic_param->picture_coding_extension.bits.intra_vlc_format << 7 | 
                  pic_param->picture_coding_extension.bits.alternate_scan << 6);
    OUT_BCS_BATCH(batch,
                  0 << 14 |     /* LoadSlicePointerFlag, 0 means only loading bitstream pointer once */
                  va_to_gen75_mpeg2_picture_type[pic_param->picture_type] << 9 |
                  0);
    OUT_BCS_BATCH(batch,
                  1 << 31 |     /* slice concealment */
                  (height_in_mbs - 1) << 16 |
                  (width_in_mbs - 1));
    if (slice_param && slice_param->quantiser_scale_code >= 14) 
	OUT_BCS_BATCH(batch, (3 << 1) | (1 << 4) | (5 << 8) | (1 << 12));
    else
	OUT_BCS_BATCH(batch, 0);

    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch,
                  0xFFF << 16 | /* InterMBMaxSize */
                  0xFFF << 0 |  /* IntraMBMaxSize */
                  0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    ADVANCE_BCS_BATCH(batch);
}

static void
gen75_mfc_mpeg2_qm_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
{
    unsigned char intra_qm[64] = {
        8, 16, 19, 22, 26, 27, 29, 34,
        16, 16, 22, 24, 27, 29, 34, 37,
        19, 22, 26, 27, 29, 34, 34, 38,
        22, 22, 26, 27, 29, 34, 37, 40,
        22, 26, 27, 29, 32, 35, 40, 48,
        26, 27, 29, 32, 35, 40, 48, 58,
        26, 27, 29, 34, 38, 46, 56, 69,
        27, 29, 35, 38, 46, 56, 69, 83
    };

    unsigned char non_intra_qm[64] = {
        16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16
    };

    gen75_mfc_qm_state(ctx, MFX_QM_MPEG_INTRA_QUANTIZER_MATRIX, (unsigned int *)intra_qm, 16, encoder_context);
    gen75_mfc_qm_state(ctx, MFX_QM_MPEG_NON_INTRA_QUANTIZER_MATRIX, (unsigned int *)non_intra_qm, 16,encoder_context);
}

static void
gen75_mfc_mpeg2_fqm_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
{
    unsigned short intra_fqm[64] = {
        65536/0x8, 65536/0x10, 65536/0x13, 65536/0x16, 65536/0x16, 65536/0x1a, 65536/0x1a, 65536/0x1b,
        65536/0x10, 65536/0x10, 65536/0x16, 65536/0x16, 65536/0x1a, 65536/0x1b, 65536/0x1b, 65536/0x1d,
        65536/0x13, 65536/0x16, 65536/0x1a, 65536/0x1a, 65536/0x1b, 65536/0x1d, 65536/0x1d, 65536/0x23,
        65536/0x16, 65536/0x18, 65536/0x1b, 65536/0x1b, 65536/0x13, 65536/0x20, 65536/0x22, 65536/0x26,
        65536/0x1a, 65536/0x1b, 65536/0x13, 65536/0x13, 65536/0x20, 65536/0x23, 65536/0x26, 65536/0x2e,
        65536/0x1b, 65536/0x1d, 65536/0x22, 65536/0x22, 65536/0x23, 65536/0x28, 65536/0x2e, 65536/0x38,
        65536/0x1d, 65536/0x22, 65536/0x22, 65536/0x25, 65536/0x28, 65536/0x30, 65536/0x38, 65536/0x45,
        65536/0x22, 65536/0x25, 65536/0x26, 65536/0x28, 65536/0x30, 65536/0x3a, 65536/0x45, 65536/0x53,
    };

    unsigned short non_intra_fqm[64] = {
        0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
        0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
        0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
        0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
        0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
        0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
        0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
        0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    };

    gen75_mfc_fqm_state(ctx, MFX_QM_MPEG_INTRA_QUANTIZER_MATRIX, (unsigned int *)intra_fqm, 32, encoder_context);
    gen75_mfc_fqm_state(ctx, MFX_QM_MPEG_NON_INTRA_QUANTIZER_MATRIX, (unsigned int *)non_intra_fqm, 32, encoder_context);
}

static void
gen75_mfc_mpeg2_slicegroup_state(VADriverContextP ctx,
                                 struct intel_encoder_context *encoder_context,
                                 int x, int y,
                                 int next_x, int next_y,
                                 int is_fisrt_slice_group,
                                 int is_last_slice_group,
                                 int intra_slice,
                                 int qp,
                                 struct intel_batchbuffer *batch)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;

    if (batch == NULL)
        batch = encoder_context->base.batch;

    BEGIN_BCS_BATCH(batch, 8);

    OUT_BCS_BATCH(batch, MFC_MPEG2_SLICEGROUP_STATE | (8 - 2));
    OUT_BCS_BATCH(batch,
                  0 << 31 |                             /* MbRateCtrlFlag */
                  !!is_last_slice_group << 19 |         /* IsLastSliceGrp */
                  1 << 17 |                             /* Insert Header before the first slice group data */
                  1 << 16 |                             /* SliceData PresentFlag: always 1 */
                  1 << 15 |                             /* TailPresentFlag: always 1 */
                  0 << 14 |                             /* FirstSliceHdrDisabled: slice header for each slice */
                  !!intra_slice << 13 |                 /* IntraSlice */
                  !!intra_slice << 12 |                 /* IntraSliceFlag */
                  0);
    OUT_BCS_BATCH(batch,
                  next_y << 24 |
                  next_x << 16 |
                  y << 8 |
                  x << 0 |
                  0);
    OUT_BCS_BATCH(batch, qp);   /* FIXME: SliceGroupQp */
    /* bitstream pointer is only loaded once for the first slice of a frame when 
     * LoadSlicePointerFlag is 0
     */
    OUT_BCS_BATCH(batch, mfc_context->mfc_indirect_pak_bse_object.offset);
    OUT_BCS_BATCH(batch, 0);    /* FIXME: */
    OUT_BCS_BATCH(batch, 0);    /* FIXME: CorrectPoints */
    OUT_BCS_BATCH(batch, 0);    /* FIXME: CVxxx */

    ADVANCE_BCS_BATCH(batch);
}

static int
gen75_mfc_mpeg2_pak_object_intra(VADriverContextP ctx,
                                 struct intel_encoder_context *encoder_context,
                                 int x, int y,
                                 int first_mb_in_slice,
                                 int last_mb_in_slice,
                                 int first_mb_in_slice_group,
                                 int last_mb_in_slice_group,
                                 int mb_type,
                                 int qp_scale_code,
                                 int coded_block_pattern,
                                 unsigned char target_size_in_word,
                                 unsigned char max_size_in_word,
                                 struct intel_batchbuffer *batch)
{
    int len_in_dwords = 9;

    if (batch == NULL)
        batch = encoder_context->base.batch;

    BEGIN_BCS_BATCH(batch, len_in_dwords);

    OUT_BCS_BATCH(batch, MFC_MPEG2_PAK_OBJECT | (len_in_dwords - 2));
    OUT_BCS_BATCH(batch,
                  0 << 24 |     /* PackedMvNum */
                  0 << 20 |     /* MvFormat */
                  7 << 17 |     /* CbpDcY/CbpDcU/CbpDcV */
                  0 << 15 |     /* TransformFlag: frame DCT */
                  0 << 14 |     /* FieldMbFlag */
                  1 << 13 |     /* IntraMbFlag */
                  mb_type << 8 |   /* MbType: Intra */
                  0 << 2 |      /* SkipMbFlag */
                  0 << 0 |      /* InterMbMode */
                  0);
    OUT_BCS_BATCH(batch, y << 16 | x);
    OUT_BCS_BATCH(batch,
                  max_size_in_word << 24 |
                  target_size_in_word << 16 |
                  coded_block_pattern << 6 |      /* CBP */
                  0);
    OUT_BCS_BATCH(batch,
                  last_mb_in_slice << 31 |
                  first_mb_in_slice << 30 |
                  0 << 27 |     /* EnableCoeffClamp */
                  last_mb_in_slice_group << 26 |
                  0 << 25 |     /* MbSkipConvDisable */
                  first_mb_in_slice_group << 24 |
                  0 << 16 |     /* MvFieldSelect */
                  qp_scale_code << 0 |
                  0);
    OUT_BCS_BATCH(batch, 0);    /* MV[0][0] */
    OUT_BCS_BATCH(batch, 0);    /* MV[1][0] */
    OUT_BCS_BATCH(batch, 0);    /* MV[0][1] */
    OUT_BCS_BATCH(batch, 0);    /* MV[1][1] */

    ADVANCE_BCS_BATCH(batch);

    return len_in_dwords;
}

#define MPEG2_INTER_MV_OFFSET   12 

static struct _mv_ranges
{
    int low;    /* in the unit of 1/2 pixel */
    int high;   /* in the unit of 1/2 pixel */
} mv_ranges[] = {
    {0, 0},
    {-16, 15},
    {-32, 31},
    {-64, 63},
    {-128, 127},
    {-256, 255},
    {-512, 511},
    {-1024, 1023},
    {-2048, 2047},
    {-4096, 4095}
};

static int
mpeg2_motion_vector(int mv, int pos, int display_max, int f_code)
{
    if (mv + pos * 16 * 2 < 0 ||
        mv + (pos + 1) * 16 * 2 > display_max * 2)
        mv = 0;

    if (f_code > 0 && f_code < 10) {
        if (mv < mv_ranges[f_code].low)
            mv = mv_ranges[f_code].low;

        if (mv > mv_ranges[f_code].high)
            mv = mv_ranges[f_code].high;
    }

    return mv;
}

static int
gen75_mfc_mpeg2_pak_object_inter(VADriverContextP ctx,
                                 struct encode_state *encode_state,
                                 struct intel_encoder_context *encoder_context,
                                 unsigned int *msg,
                                 int width_in_mbs, int height_in_mbs,
                                 int x, int y,
                                 int first_mb_in_slice,
                                 int last_mb_in_slice,
                                 int first_mb_in_slice_group,
                                 int last_mb_in_slice_group,
                                 int qp_scale_code,
                                 unsigned char target_size_in_word,
                                 unsigned char max_size_in_word,
                                 struct intel_batchbuffer *batch)
{
    VAEncPictureParameterBufferMPEG2 *pic_param = (VAEncPictureParameterBufferMPEG2 *)encode_state->pic_param_ext->buffer;
    int len_in_dwords = 9;
    short *mvptr, mvx0, mvy0, mvx1, mvy1;
    
    if (batch == NULL)
        batch = encoder_context->base.batch;

    mvptr = (short *)(msg + MPEG2_INTER_MV_OFFSET);
    mvx0 = mpeg2_motion_vector(mvptr[0] / 2, x, width_in_mbs * 16, pic_param->f_code[0][0]);
    mvy0 = mpeg2_motion_vector(mvptr[1] / 2, y, height_in_mbs * 16, pic_param->f_code[0][0]);
    mvx1 = mpeg2_motion_vector(mvptr[2] / 2, x, width_in_mbs * 16, pic_param->f_code[1][0]);
    mvy1 = mpeg2_motion_vector(mvptr[3] / 2, y, height_in_mbs * 16, pic_param->f_code[1][0]);

    BEGIN_BCS_BATCH(batch, len_in_dwords);

    OUT_BCS_BATCH(batch, MFC_MPEG2_PAK_OBJECT | (len_in_dwords - 2));
    OUT_BCS_BATCH(batch,
                  2 << 24 |     /* PackedMvNum */
                  7 << 20 |     /* MvFormat */
                  7 << 17 |     /* CbpDcY/CbpDcU/CbpDcV */
                  0 << 15 |     /* TransformFlag: frame DCT */
                  0 << 14 |     /* FieldMbFlag */
                  0 << 13 |     /* IntraMbFlag */
                  1 << 8 |      /* MbType: Frame-based */
                  0 << 2 |      /* SkipMbFlag */
                  0 << 0 |      /* InterMbMode */
                  0);
    OUT_BCS_BATCH(batch, y << 16 | x);
    OUT_BCS_BATCH(batch,
                  max_size_in_word << 24 |
                  target_size_in_word << 16 |
                  0x3f << 6 |   /* CBP */
                  0);
    OUT_BCS_BATCH(batch,
                  last_mb_in_slice << 31 |
                  first_mb_in_slice << 30 |
                  0 << 27 |     /* EnableCoeffClamp */
                  last_mb_in_slice_group << 26 |
                  0 << 25 |     /* MbSkipConvDisable */
                  first_mb_in_slice_group << 24 |
                  0 << 16 |     /* MvFieldSelect */
                  qp_scale_code << 0 |
                  0);

    OUT_BCS_BATCH(batch, (mvx0 & 0xFFFF) | mvy0 << 16);    /* MV[0][0] */
    OUT_BCS_BATCH(batch, (mvx1 & 0xFFFF) | mvy1 << 16);    /* MV[1][0] */
    OUT_BCS_BATCH(batch, 0);    /* MV[0][1] */
    OUT_BCS_BATCH(batch, 0);    /* MV[1][1] */

    ADVANCE_BCS_BATCH(batch);

    return len_in_dwords;
}

static void
intel_mfc_mpeg2_pipeline_header_programing(VADriverContextP ctx,
                                           struct encode_state *encode_state,
                                           struct intel_encoder_context *encoder_context,
                                           struct intel_batchbuffer *slice_batch)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    int idx = va_enc_packed_type_to_idx(VAEncPackedHeaderMPEG2_SPS);

    if (encode_state->packed_header_data[idx]) {
        VAEncPackedHeaderParameterBuffer *param = NULL;
        unsigned int *header_data = (unsigned int *)encode_state->packed_header_data[idx]->buffer;
        unsigned int length_in_bits;

        assert(encode_state->packed_header_param[idx]);
        param = (VAEncPackedHeaderParameterBuffer *)encode_state->packed_header_param[idx]->buffer;
        length_in_bits = param->bit_length;

        mfc_context->insert_object(ctx,
                                   encoder_context,
                                   header_data,
                                   ALIGN(length_in_bits, 32) >> 5,
                                   length_in_bits & 0x1f,
                                   5,   /* FIXME: check it */
                                   0,
                                   0,
                                   0,   /* Needn't insert emulation bytes for MPEG-2 */
                                   slice_batch);
    }

    idx = va_enc_packed_type_to_idx(VAEncPackedHeaderMPEG2_PPS);

    if (encode_state->packed_header_data[idx]) {
        VAEncPackedHeaderParameterBuffer *param = NULL;
        unsigned int *header_data = (unsigned int *)encode_state->packed_header_data[idx]->buffer;
        unsigned int length_in_bits;

        assert(encode_state->packed_header_param[idx]);
        param = (VAEncPackedHeaderParameterBuffer *)encode_state->packed_header_param[idx]->buffer;
        length_in_bits = param->bit_length;

        mfc_context->insert_object(ctx,
                                   encoder_context,
                                   header_data,
                                   ALIGN(length_in_bits, 32) >> 5,
                                   length_in_bits & 0x1f,
                                   5,   /* FIXME: check it */
                                   0,
                                   0,
                                   0,   /* Needn't insert emulation bytes for MPEG-2 */
                                   slice_batch);
    }
}

static void 
gen75_mfc_mpeg2_pipeline_slice_group(VADriverContextP ctx,
                                     struct encode_state *encode_state,
                                     struct intel_encoder_context *encoder_context,
                                     int slice_index,
                                     VAEncSliceParameterBufferMPEG2 *next_slice_group_param,
                                     struct intel_batchbuffer *slice_batch)
{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    VAEncSequenceParameterBufferMPEG2 *seq_param = (VAEncSequenceParameterBufferMPEG2 *)encode_state->seq_param_ext->buffer;
    VAEncSliceParameterBufferMPEG2 *slice_param = NULL;
    unsigned char tail_delimiter[] = {MPEG2_DELIMITER0, MPEG2_DELIMITER1, MPEG2_DELIMITER2, MPEG2_DELIMITER3, MPEG2_DELIMITER4, 0, 0, 0};
    unsigned char section_delimiter[] = {0x0, 0x0, 0x0, 0x0};
    int width_in_mbs = ALIGN(seq_param->picture_width, 16) / 16;
    int height_in_mbs = ALIGN(seq_param->picture_height, 16) / 16;
    int i, j;
    int h_start_pos, v_start_pos, h_next_start_pos, v_next_start_pos;
    unsigned int *msg = NULL;
    unsigned char *msg_ptr = NULL;

    slice_param = (VAEncSliceParameterBufferMPEG2 *)encode_state->slice_params_ext[slice_index]->buffer;
    h_start_pos = slice_param->macroblock_address % width_in_mbs;
    v_start_pos = slice_param->macroblock_address / width_in_mbs;
    assert(h_start_pos + slice_param->num_macroblocks <= width_in_mbs);

    dri_bo_map(vme_context->vme_output.bo , 0);
    msg_ptr = (unsigned char *)vme_context->vme_output.bo->virtual;

    if (next_slice_group_param) {
        h_next_start_pos = next_slice_group_param->macroblock_address % width_in_mbs;
        v_next_start_pos = next_slice_group_param->macroblock_address / width_in_mbs;
    } else {
        h_next_start_pos = 0;
        v_next_start_pos = height_in_mbs;
    }

    gen75_mfc_mpeg2_slicegroup_state(ctx,
                                     encoder_context,
                                     h_start_pos,
                                     v_start_pos,
                                     h_next_start_pos,
                                     v_next_start_pos,
                                     slice_index == 0,
                                     next_slice_group_param == NULL,
                                     slice_param->is_intra_slice,
                                     slice_param->quantiser_scale_code,
                                     slice_batch);

    if (slice_index == 0) 
        intel_mfc_mpeg2_pipeline_header_programing(ctx, encode_state, encoder_context, slice_batch);

    /* Insert '00' to make sure the header is valid */
    mfc_context->insert_object(ctx,
                               encoder_context,
                               (unsigned int*)section_delimiter,
                               1,
                               8,   /* 8bits in the last DWORD */
                               1,   /* 1 byte */
                               1,
                               0,
                               0,
                               slice_batch);

    for (i = 0; i < encode_state->slice_params_ext[slice_index]->num_elements; i++) {
        /* PAK for each macroblocks */
        for (j = 0; j < slice_param->num_macroblocks; j++) {
            int h_pos = (slice_param->macroblock_address + j) % width_in_mbs;
            int v_pos = (slice_param->macroblock_address + j) / width_in_mbs;
            int first_mb_in_slice = (j == 0);
            int last_mb_in_slice = (j == slice_param->num_macroblocks - 1);
            int first_mb_in_slice_group = (i == 0 && j == 0);
            int last_mb_in_slice_group = (i == encode_state->slice_params_ext[slice_index]->num_elements - 1 &&
                                          j == slice_param->num_macroblocks - 1);

            msg = (unsigned int *)(msg_ptr + (slice_param->macroblock_address + j) * vme_context->vme_output.size_block);

            if (slice_param->is_intra_slice) {
                gen75_mfc_mpeg2_pak_object_intra(ctx,
                                                 encoder_context,
                                                 h_pos, v_pos,
                                                 first_mb_in_slice,
                                                 last_mb_in_slice,
                                                 first_mb_in_slice_group,
                                                 last_mb_in_slice_group,
                                                 0x1a,
                                                 slice_param->quantiser_scale_code,
                                                 0x3f,
                                                 0,
                                                 0xff,
                                                 slice_batch);
            } else {
		int inter_rdo, intra_rdo;
		inter_rdo = msg[AVC_INTER_RDO_OFFSET] & AVC_RDO_MASK;
		intra_rdo = msg[AVC_INTRA_RDO_OFFSET] & AVC_RDO_MASK;

		if (intra_rdo < inter_rdo) 
                    gen75_mfc_mpeg2_pak_object_intra(ctx,
                                                     encoder_context,
                                                     h_pos, v_pos,
                                                     first_mb_in_slice,
                                                     last_mb_in_slice,
                                                     first_mb_in_slice_group,
                                                     last_mb_in_slice_group,
                                                     0x1a,
                                                     slice_param->quantiser_scale_code,
                                                     0x3f,
                                                     0,
                                                     0xff,
                                                     slice_batch);
		else
                    gen75_mfc_mpeg2_pak_object_inter(ctx,
                                                     encode_state,
                                                     encoder_context,
                                                     msg,
                                                     width_in_mbs, height_in_mbs,
                                                     h_pos, v_pos,
                                                     first_mb_in_slice,
                                                     last_mb_in_slice,
                                                     first_mb_in_slice_group,
                                                     last_mb_in_slice_group,
                                                     slice_param->quantiser_scale_code,
                                                     0,
                                                     0xff,
                                                     slice_batch);
            }
        }

        slice_param++;
    }

    dri_bo_unmap(vme_context->vme_output.bo);

    /* tail data */
    if (next_slice_group_param == NULL) { /* end of a picture */
        mfc_context->insert_object(ctx,
                                   encoder_context,
                                   (unsigned int *)tail_delimiter,
                                   2,
                                   8,   /* 8bits in the last DWORD */
                                   5,   /* 5 bytes */
                                   1,
                                   1,
                                   0,
                                   slice_batch);
    } else {        /* end of a lsice group */
        mfc_context->insert_object(ctx,
                                   encoder_context,
                                   (unsigned int *)section_delimiter,
                                   1,
                                   8,   /* 8bits in the last DWORD */
                                   1,   /* 1 byte */
                                   1,
                                   1,
                                   0,
                                   slice_batch);
    }
}

/* 
 * A batch buffer for all slices, including slice state, 
 * slice insert object and slice pak object commands
 *
 */
static dri_bo *
gen75_mfc_mpeg2_software_slice_batchbuffer(VADriverContextP ctx,
                                           struct encode_state *encode_state,
                                           struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct intel_batchbuffer *batch;
    VAEncSliceParameterBufferMPEG2 *next_slice_group_param = NULL;
    dri_bo *batch_bo;
    int i;

    batch = mfc_context->aux_batchbuffer;
    batch_bo = batch->buffer;

    for (i = 0; i < encode_state->num_slice_params_ext; i++) {
        if (i == encode_state->num_slice_params_ext - 1)
            next_slice_group_param = NULL;
        else
            next_slice_group_param = (VAEncSliceParameterBufferMPEG2 *)encode_state->slice_params_ext[i + 1]->buffer;

        gen75_mfc_mpeg2_pipeline_slice_group(ctx, encode_state, encoder_context, i, next_slice_group_param, batch);
    }

    intel_batchbuffer_align(batch, 8);
    
    BEGIN_BCS_BATCH(batch, 2);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, MI_BATCH_BUFFER_END);
    ADVANCE_BCS_BATCH(batch);

    dri_bo_reference(batch_bo);
    intel_batchbuffer_free(batch);
    mfc_context->aux_batchbuffer = NULL;

    return batch_bo;
}

static void
gen75_mfc_mpeg2_pipeline_picture_programing(VADriverContextP ctx,
                                            struct encode_state *encode_state,
                                            struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;

    mfc_context->pipe_mode_select(ctx, MFX_FORMAT_MPEG2, encoder_context);
    mfc_context->set_surface_state(ctx, encoder_context);
    mfc_context->ind_obj_base_addr_state(ctx, encoder_context);
    gen75_mfc_pipe_buf_addr_state(ctx, encoder_context);
    gen75_mfc_bsp_buf_base_addr_state(ctx, encoder_context);
    gen75_mfc_mpeg2_pic_state(ctx, encoder_context, encode_state);
    gen75_mfc_mpeg2_qm_state(ctx, encoder_context);
    gen75_mfc_mpeg2_fqm_state(ctx, encoder_context);
}

static void
gen75_mfc_mpeg2_pipeline_programing(VADriverContextP ctx,
                                    struct encode_state *encode_state,
                                    struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    dri_bo *slice_batch_bo;

    slice_batch_bo = gen75_mfc_mpeg2_software_slice_batchbuffer(ctx, encode_state, encoder_context);

    // begin programing
    intel_batchbuffer_start_atomic_bcs(batch, 0x4000); 
    intel_batchbuffer_emit_mi_flush(batch);
    
    // picture level programing
    gen75_mfc_mpeg2_pipeline_picture_programing(ctx, encode_state, encoder_context);

    BEGIN_BCS_BATCH(batch, 2);
    OUT_BCS_BATCH(batch, MI_BATCH_BUFFER_START | (1 << 8));
    OUT_BCS_RELOC(batch,
                  slice_batch_bo,
                  I915_GEM_DOMAIN_COMMAND, 0, 
                  0);
    ADVANCE_BCS_BATCH(batch);

    // end programing
    intel_batchbuffer_end_atomic(batch);

    dri_bo_unreference(slice_batch_bo);
}

static VAStatus
intel_mfc_mpeg2_prepare(VADriverContextP ctx, 
                        struct encode_state *encode_state,
                        struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct object_surface *obj_surface;	
    struct object_buffer *obj_buffer;
    struct i965_coded_buffer_segment *coded_buffer_segment;
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    dri_bo *bo;
    int i;

    /* reconstructed surface */
    obj_surface = encode_state->reconstructed_object;
    i965_check_alloc_surface_bo(ctx, obj_surface, 1, VA_FOURCC_NV12, SUBSAMPLE_YUV420);
    mfc_context->pre_deblocking_output.bo = obj_surface->bo;
    dri_bo_reference(mfc_context->pre_deblocking_output.bo);
    mfc_context->surface_state.width = obj_surface->orig_width;
    mfc_context->surface_state.height = obj_surface->orig_height;
    mfc_context->surface_state.w_pitch = obj_surface->width;
    mfc_context->surface_state.h_pitch = obj_surface->height;

    /* forward reference */
    obj_surface = encode_state->reference_objects[0];

    if (obj_surface && obj_surface->bo) {
        mfc_context->reference_surfaces[0].bo = obj_surface->bo;
        dri_bo_reference(mfc_context->reference_surfaces[0].bo);
    } else
        mfc_context->reference_surfaces[0].bo = NULL;

    /* backward reference */
    obj_surface = encode_state->reference_objects[1];

    if (obj_surface && obj_surface->bo) {
        mfc_context->reference_surfaces[1].bo = obj_surface->bo;
        dri_bo_reference(mfc_context->reference_surfaces[1].bo);
    } else {
        mfc_context->reference_surfaces[1].bo = mfc_context->reference_surfaces[0].bo;

        if (mfc_context->reference_surfaces[1].bo)
            dri_bo_reference(mfc_context->reference_surfaces[1].bo);
    }

    for (i = 2; i < ARRAY_ELEMS(mfc_context->reference_surfaces); i++) {
        mfc_context->reference_surfaces[i].bo = mfc_context->reference_surfaces[i & 1].bo;

        if (mfc_context->reference_surfaces[i].bo)
            dri_bo_reference(mfc_context->reference_surfaces[i].bo);
    }
    
    /* input YUV surface */
    obj_surface = encode_state->input_yuv_object;
    mfc_context->uncompressed_picture_source.bo = obj_surface->bo;
    dri_bo_reference(mfc_context->uncompressed_picture_source.bo);

    /* coded buffer */
    obj_buffer = encode_state->coded_buf_object;
    bo = obj_buffer->buffer_store->bo;
    mfc_context->mfc_indirect_pak_bse_object.bo = bo;
    mfc_context->mfc_indirect_pak_bse_object.offset = I965_CODEDBUFFER_HEADER_SIZE;
    mfc_context->mfc_indirect_pak_bse_object.end_offset = ALIGN(obj_buffer->size_element - 0x1000, 0x1000);
    dri_bo_reference(mfc_context->mfc_indirect_pak_bse_object.bo);

    /* set the internal flag to 0 to indicate the coded size is unknown */
    dri_bo_map(bo, 1);
    coded_buffer_segment = (struct i965_coded_buffer_segment *)bo->virtual;
    coded_buffer_segment->mapped = 0;
    coded_buffer_segment->codec = encoder_context->codec;
    dri_bo_unmap(bo);

    return vaStatus;
}

static VAStatus
gen75_mfc_mpeg2_encode_picture(VADriverContextP ctx, 
                               struct encode_state *encode_state,
                               struct intel_encoder_context *encoder_context)
{
    gen75_mfc_init(ctx, encode_state, encoder_context);
    intel_mfc_mpeg2_prepare(ctx, encode_state, encoder_context);
    /*Programing bcs pipeline*/
    gen75_mfc_mpeg2_pipeline_programing(ctx, encode_state, encoder_context);
    gen75_mfc_run(ctx, encode_state, encoder_context);

    return VA_STATUS_SUCCESS;
}

static void
gen75_mfc_context_destroy(void *context)
{
    struct gen6_mfc_context *mfc_context = context;
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

    dri_bo_unreference(mfc_context->macroblock_status_buffer.bo);
    mfc_context->macroblock_status_buffer.bo = NULL;

    dri_bo_unreference(mfc_context->deblocking_filter_row_store_scratch_buffer.bo);
    mfc_context->deblocking_filter_row_store_scratch_buffer.bo = NULL;

    dri_bo_unreference(mfc_context->bsd_mpc_row_store_scratch_buffer.bo);
    mfc_context->bsd_mpc_row_store_scratch_buffer.bo = NULL;

    for (i = 0; i < MAX_MFC_REFERENCE_SURFACES; i++){
        dri_bo_unreference(mfc_context->reference_surfaces[i].bo);
        mfc_context->reference_surfaces[i].bo = NULL;  
    }

    i965_gpe_context_destroy(&mfc_context->gpe_context);

    dri_bo_unreference(mfc_context->mfc_batchbuffer_surface.bo);
    mfc_context->mfc_batchbuffer_surface.bo = NULL;

    dri_bo_unreference(mfc_context->aux_batchbuffer_surface.bo);
    mfc_context->aux_batchbuffer_surface.bo = NULL;

    if (mfc_context->aux_batchbuffer)
        intel_batchbuffer_free(mfc_context->aux_batchbuffer);

    mfc_context->aux_batchbuffer = NULL;

    free(mfc_context);
}

static VAStatus gen75_mfc_pipeline(VADriverContextP ctx,
                                   VAProfile profile,
                                   struct encode_state *encode_state,
                                   struct intel_encoder_context *encoder_context)
{
    VAStatus vaStatus;

    switch (profile) {
    case VAProfileH264ConstrainedBaseline:
    case VAProfileH264Main:
    case VAProfileH264High:
    case VAProfileH264MultiviewHigh:
    case VAProfileH264StereoHigh:
        vaStatus = gen75_mfc_avc_encode_picture(ctx, encode_state, encoder_context);
        break;

        /* FIXME: add for other profile */
    case VAProfileMPEG2Simple:
    case VAProfileMPEG2Main:
        vaStatus = gen75_mfc_mpeg2_encode_picture(ctx, encode_state, encoder_context);
        break;

    default:
        vaStatus = VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
        break;
    }

    return vaStatus;
}

Bool gen75_mfc_context_init(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = calloc(1, sizeof(struct gen6_mfc_context));

    if (!mfc_context)
        return False;

    mfc_context->gpe_context.surface_state_binding_table.length = (SURFACE_STATE_PADDED_SIZE + sizeof(unsigned int)) * MAX_MEDIA_SURFACES_GEN6;

    mfc_context->gpe_context.idrt.max_entries = MAX_GPE_KERNELS;
    mfc_context->gpe_context.idrt.entry_size = sizeof(struct gen6_interface_descriptor_data);

    mfc_context->gpe_context.curbe.length = 32 * 4;

    mfc_context->gpe_context.vfe_state.max_num_threads = 60 - 1;
    mfc_context->gpe_context.vfe_state.num_urb_entries = 16;
    mfc_context->gpe_context.vfe_state.gpgpu_mode = 0;
    mfc_context->gpe_context.vfe_state.urb_entry_size = 59 - 1;
    mfc_context->gpe_context.vfe_state.curbe_allocation_size = 37 - 1;

    i965_gpe_load_kernels(ctx,
                          &mfc_context->gpe_context,
                          gen75_mfc_kernels,
                          1);

    mfc_context->pipe_mode_select = gen75_mfc_pipe_mode_select;
    mfc_context->set_surface_state = gen75_mfc_surface_state;
    mfc_context->ind_obj_base_addr_state = gen75_mfc_ind_obj_base_addr_state;
    mfc_context->avc_img_state = gen75_mfc_avc_img_state;
    mfc_context->avc_qm_state = gen75_mfc_avc_qm_state;
    mfc_context->avc_fqm_state = gen75_mfc_avc_fqm_state;
    mfc_context->insert_object = gen75_mfc_avc_insert_object;
    mfc_context->buffer_suface_setup = gen7_gpe_buffer_suface_setup;

    encoder_context->mfc_context = mfc_context;
    encoder_context->mfc_context_destroy = gen75_mfc_context_destroy;
    encoder_context->mfc_pipeline = gen75_mfc_pipeline;
    encoder_context->mfc_brc_prepare = intel_mfc_brc_prepare;

    return True;
}
