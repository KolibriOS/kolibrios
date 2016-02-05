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
 *    Zhou Chang <chang.zhou@intel.com>
 *    Xiang, Haihao <haihao.xiang@intel.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "intel_batchbuffer.h"
#include "i965_defines.h"
#include "i965_structs.h"
#include "i965_drv_video.h"
#include "i965_encoder.h"
#include "i965_encoder_utils.h"
#include "gen6_mfc.h"
#include "gen6_vme.h"

#define SURFACE_STATE_PADDED_SIZE               MAX(SURFACE_STATE_PADDED_SIZE_GEN6, SURFACE_STATE_PADDED_SIZE_GEN7)
#define SURFACE_STATE_OFFSET(index)             (SURFACE_STATE_PADDED_SIZE * index)
#define BINDING_TABLE_OFFSET(index)             (SURFACE_STATE_OFFSET(MAX_MEDIA_SURFACES_GEN6) + sizeof(unsigned int) * index)

extern void
gen6_mfc_pipe_buf_addr_state(VADriverContextP ctx, 
                             struct intel_encoder_context *encoder_context);
extern void
gen6_mfc_bsp_buf_base_addr_state(VADriverContextP ctx, 
                                 struct intel_encoder_context *encoder_context);
extern void 
gen6_mfc_init(VADriverContextP ctx, 
              struct encode_state *encode_state,
              struct intel_encoder_context *encoder_context);

extern VAStatus
gen6_mfc_run(VADriverContextP ctx, 
             struct encode_state *encode_state,
             struct intel_encoder_context *encoder_context);

extern VAStatus
gen6_mfc_stop(VADriverContextP ctx, 
              struct encode_state *encode_state,
              struct intel_encoder_context *encoder_context,
              int *encoded_bits_size);

extern VAStatus
gen6_mfc_avc_encode_picture(VADriverContextP ctx, 
                            struct encode_state *encode_state,
                            struct intel_encoder_context *encoder_context);

static const uint32_t gen7_mfc_batchbuffer_avc_intra[][4] = {
#include "shaders/utils/mfc_batchbuffer_avc_intra.g7b"
};

static const uint32_t gen7_mfc_batchbuffer_avc_inter[][4] = {
#include "shaders/utils/mfc_batchbuffer_avc_inter.g7b"
};

static struct i965_kernel gen7_mfc_kernels[] = {
    {
        "MFC AVC INTRA BATCHBUFFER ",
        MFC_BATCHBUFFER_AVC_INTRA,
        gen7_mfc_batchbuffer_avc_intra,
        sizeof(gen7_mfc_batchbuffer_avc_intra),
        NULL
    },

    {
        "MFC AVC INTER BATCHBUFFER ",
        MFC_BATCHBUFFER_AVC_INTER,
        gen7_mfc_batchbuffer_avc_inter,
        sizeof(gen7_mfc_batchbuffer_avc_inter),
        NULL
    },
};

static void
gen7_mfc_pipe_mode_select(VADriverContextP ctx,
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
                  (1 << 10) | /* Stream-Out Enable */
                  ((!!mfc_context->post_deblocking_output.bo) << 9)  | /* Post Deblocking Output */
                  ((!!mfc_context->pre_deblocking_output.bo) << 8)  | /* Pre Deblocking Output */
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

static void
gen7_mfc_surface_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
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
gen7_mfc_ind_obj_base_addr_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct gen6_vme_context *vme_context = encoder_context->vme_context;

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
gen7_mfc_avc_img_state(VADriverContextP ctx, struct encode_state *encode_state,  
                       struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    VAEncPictureParameterBufferH264 *pPicParameter = (VAEncPictureParameterBufferH264 *)encode_state->pic_param_ext->buffer;

    int width_in_mbs = (mfc_context->surface_state.width + 15) / 16;
    int height_in_mbs = (mfc_context->surface_state.height + 15) / 16;

    BEGIN_BCS_BATCH(batch, 16);

    OUT_BCS_BATCH(batch, MFX_AVC_IMG_STATE | (16 - 2));
    /*DW1 frame size */
    OUT_BCS_BATCH(batch,
                  ((width_in_mbs * height_in_mbs - 1) & 0xFFFF));
    OUT_BCS_BATCH(batch, 
                  ((height_in_mbs - 1) << 16) | 
                  ((width_in_mbs - 1) << 0));
    /*DW3 Qp setting */
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
                  (0 << 9)  |   /* FIXME: MbMvFormatFlag */
                  (pPicParameter->pic_fields.bits.entropy_coding_mode_flag << 7)  |   /*0:CAVLC encoding mode,1:CABAC*/
                  (0 << 6)  |   /* Only valid for VLD decoding mode */
                  (0 << 5)  |   /* Constrained Intra Predition Flag, from PPS */
                  (0 << 4)  |   /* Direct 8x8 inference flag */
                  (pPicParameter->pic_fields.bits.transform_8x8_mode_flag << 3)  |   /*8x8 or 4x4 IDCT Transform Mode Flag*/
                  (1 << 2)  |   /* Frame MB only flag */
                  (0 << 1)  |   /* MBAFF mode is in active */
                  (0 << 0));    /* Field picture flag */
    /*DW5 trequllis quantization */
    OUT_BCS_BATCH(batch, 0);    /* Mainly about MB rate control and debug, just ignoring */
    OUT_BCS_BATCH(batch,        /* Inter and Intra Conformance Max size limit */
                  (0xBB8 << 16) |       /* InterMbMaxSz */
                  (0xEE8) );            /* IntraMbMaxSz */
    /* DW7 */
    OUT_BCS_BATCH(batch, 0);            /* Reserved */
    OUT_BCS_BATCH(batch, 0);            /* Slice QP Delta for bitrate control */
    OUT_BCS_BATCH(batch, 0);            /* Slice QP Delta for bitrate control */
    /* DW10 frame bit setting */
    OUT_BCS_BATCH(batch, 0x8C000000);
    OUT_BCS_BATCH(batch, 0x00010000);
    OUT_BCS_BATCH(batch, 0);
    /* DW13 Ref setting */
    OUT_BCS_BATCH(batch, 0x02010100);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);
}

static void
gen7_mfc_qm_state(VADriverContextP ctx,
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
gen7_mfc_avc_qm_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
{
    unsigned int qm[16] = {
        0x10101010, 0x10101010, 0x10101010, 0x10101010,
        0x10101010, 0x10101010, 0x10101010, 0x10101010,
        0x10101010, 0x10101010, 0x10101010, 0x10101010,
        0x10101010, 0x10101010, 0x10101010, 0x10101010
    };

    gen7_mfc_qm_state(ctx, MFX_QM_AVC_4X4_INTRA_MATRIX, qm, 12, encoder_context);
    gen7_mfc_qm_state(ctx, MFX_QM_AVC_4X4_INTER_MATRIX, qm, 12, encoder_context);
    gen7_mfc_qm_state(ctx, MFX_QM_AVC_8x8_INTRA_MATRIX, qm, 16, encoder_context);
    gen7_mfc_qm_state(ctx, MFX_QM_AVC_8x8_INTER_MATRIX, qm, 16, encoder_context);
}

static void
gen7_mfc_fqm_state(VADriverContextP ctx,
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
gen7_mfc_avc_fqm_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
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

    gen7_mfc_fqm_state(ctx, MFX_QM_AVC_4X4_INTRA_MATRIX, qm, 24, encoder_context);
    gen7_mfc_fqm_state(ctx, MFX_QM_AVC_4X4_INTER_MATRIX, qm, 24, encoder_context);
    gen7_mfc_fqm_state(ctx, MFX_QM_AVC_8x8_INTRA_MATRIX, qm, 32, encoder_context);
    gen7_mfc_fqm_state(ctx, MFX_QM_AVC_8x8_INTER_MATRIX, qm, 32, encoder_context);
}

static void
gen7_mfc_avc_insert_object(VADriverContextP ctx, struct intel_encoder_context *encoder_context,
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

static const int
va_to_gen7_mpeg2_picture_type[3] = {
    1,  /* I */
    2,  /* P */
    3   /* B */
};

static void
gen7_mfc_mpeg2_pic_state(VADriverContextP ctx,
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
                  va_to_gen7_mpeg2_picture_type[pic_param->picture_type] << 9 |
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
gen7_mfc_mpeg2_qm_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
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

    gen7_mfc_qm_state(ctx, MFX_QM_MPEG_INTRA_QUANTIZER_MATRIX, (unsigned int *)intra_qm, 16, encoder_context);
    gen7_mfc_qm_state(ctx, MFX_QM_MPEG_NON_INTRA_QUANTIZER_MATRIX, (unsigned int *)non_intra_qm, 16,encoder_context);
}

static void
gen7_mfc_mpeg2_fqm_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
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

    gen7_mfc_fqm_state(ctx, MFX_QM_MPEG_INTRA_QUANTIZER_MATRIX, (unsigned int *)intra_fqm, 32, encoder_context);
    gen7_mfc_fqm_state(ctx, MFX_QM_MPEG_NON_INTRA_QUANTIZER_MATRIX, (unsigned int *)non_intra_fqm, 32, encoder_context);
}

static void
gen7_mfc_mpeg2_slicegroup_state(VADriverContextP ctx,
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
gen7_mfc_mpeg2_pak_object_intra(VADriverContextP ctx,
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

#define MV_OFFSET_IN_WORD       112

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
gen7_mfc_mpeg2_pak_object_inter(VADriverContextP ctx,
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

    mvptr = (short *)msg;
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
gen7_mfc_mpeg2_pipeline_header_programing(VADriverContextP ctx,
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
gen7_mfc_mpeg2_pipeline_slice_group(VADriverContextP ctx,
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

    gen7_mfc_mpeg2_slicegroup_state(ctx,
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
        gen7_mfc_mpeg2_pipeline_header_programing(ctx, encode_state, encoder_context, slice_batch);

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

            if (slice_param->is_intra_slice) {
                gen7_mfc_mpeg2_pak_object_intra(ctx,
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
                msg = (unsigned int *)(msg_ptr + (slice_param->macroblock_address + j) * vme_context->vme_output.size_block);

                if(msg[32] & INTRA_MB_FLAG_MASK) {
                    gen7_mfc_mpeg2_pak_object_intra(ctx,
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

                    gen7_mfc_mpeg2_pak_object_inter(ctx,
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
gen7_mfc_mpeg2_software_slice_batchbuffer(VADriverContextP ctx,
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

        gen7_mfc_mpeg2_pipeline_slice_group(ctx, encode_state, encoder_context, i, next_slice_group_param, batch);
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
gen7_mfc_mpeg2_pipeline_picture_programing(VADriverContextP ctx,
                                           struct encode_state *encode_state,
                                           struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;

    mfc_context->pipe_mode_select(ctx, MFX_FORMAT_MPEG2, encoder_context);
    mfc_context->set_surface_state(ctx, encoder_context);
    mfc_context->ind_obj_base_addr_state(ctx, encoder_context);
    gen6_mfc_pipe_buf_addr_state(ctx, encoder_context);
    gen6_mfc_bsp_buf_base_addr_state(ctx, encoder_context);
    gen7_mfc_mpeg2_pic_state(ctx, encoder_context, encode_state);
    gen7_mfc_mpeg2_qm_state(ctx, encoder_context);
    gen7_mfc_mpeg2_fqm_state(ctx, encoder_context);
}

static void
gen7_mfc_mpeg2_pipeline_programing(VADriverContextP ctx,
                                   struct encode_state *encode_state,
                                   struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    dri_bo *slice_batch_bo;

    slice_batch_bo = gen7_mfc_mpeg2_software_slice_batchbuffer(ctx, encode_state, encoder_context);

    // begin programing
    intel_batchbuffer_start_atomic_bcs(batch, 0x4000); 
    intel_batchbuffer_emit_mi_flush(batch);
    
    // picture level programing
    gen7_mfc_mpeg2_pipeline_picture_programing(ctx, encode_state, encoder_context);

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
gen7_mfc_mpeg2_prepare(VADriverContextP ctx,
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
gen7_mfc_mpeg2_encode_picture(VADriverContextP ctx, 
                              struct encode_state *encode_state,
                              struct intel_encoder_context *encoder_context)
{
    gen6_mfc_init(ctx, encode_state, encoder_context);
    gen7_mfc_mpeg2_prepare(ctx, encode_state, encoder_context);
    /*Programing bcs pipeline*/
    gen7_mfc_mpeg2_pipeline_programing(ctx, encode_state, encoder_context);
    gen6_mfc_run(ctx, encode_state, encoder_context);

    return VA_STATUS_SUCCESS;
}

VAStatus
gen7_mfc_pipeline(VADriverContextP ctx,
                  VAProfile profile,
                  struct encode_state *encode_state,
                  struct intel_encoder_context *encoder_context)
{
    VAStatus vaStatus;

    switch (profile) {
    case VAProfileH264ConstrainedBaseline:
    case VAProfileH264Main:
    case VAProfileH264High:
        vaStatus = gen6_mfc_avc_encode_picture(ctx, encode_state, encoder_context);
        break;

    case VAProfileMPEG2Simple:
    case VAProfileMPEG2Main:
        vaStatus = gen7_mfc_mpeg2_encode_picture(ctx, encode_state, encoder_context);
        break;

        /* FIXME: add for other profile */
    default:
        vaStatus = VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
        break;
    }

    return vaStatus;
}

Bool
gen7_mfc_context_init(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
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
                          gen7_mfc_kernels,
                          NUM_MFC_KERNEL);

    mfc_context->pipe_mode_select = gen7_mfc_pipe_mode_select;
    mfc_context->set_surface_state = gen7_mfc_surface_state;
    mfc_context->ind_obj_base_addr_state = gen7_mfc_ind_obj_base_addr_state;
    mfc_context->avc_img_state = gen7_mfc_avc_img_state;
    mfc_context->avc_qm_state = gen7_mfc_avc_qm_state;
    mfc_context->avc_fqm_state = gen7_mfc_avc_fqm_state;
    mfc_context->insert_object = gen7_mfc_avc_insert_object;
    mfc_context->buffer_suface_setup = gen7_gpe_buffer_suface_setup;

    encoder_context->mfc_context = mfc_context;
    encoder_context->mfc_context_destroy = gen6_mfc_context_destroy;
    encoder_context->mfc_pipeline = gen7_mfc_pipeline;
    encoder_context->mfc_brc_prepare = intel_mfc_brc_prepare;

    return True;
}
