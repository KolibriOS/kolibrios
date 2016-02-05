/*
 * Copyright © 2012 Intel Corporation
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
#include <va/va_enc_jpeg.h>
#include "vp8_probs.h"

#define SURFACE_STATE_PADDED_SIZE               SURFACE_STATE_PADDED_SIZE_GEN8
#define SURFACE_STATE_OFFSET(index)             (SURFACE_STATE_PADDED_SIZE * index)
#define BINDING_TABLE_OFFSET(index)             (SURFACE_STATE_OFFSET(MAX_MEDIA_SURFACES_GEN6) + sizeof(unsigned int) * index)

#define MFC_SOFTWARE_HASWELL	1

#define B0_STEP_REV		2
#define IS_STEPPING_BPLUS(i965)	((i965->intel.revision) >= B0_STEP_REV)

//Zigzag scan order of the the Luma and Chroma components
//Note: Jpeg Spec ISO/IEC 10918-1, Figure A.6 shows the zigzag order differently.
//The Spec is trying to show the zigzag pattern with number positions. The below
//table will use the pattern shown by A.6 and map the position of the elements in the array
static const uint32_t zigzag_direct[64] = {
    0,   1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

//Default Luminance quantization table
//Source: Jpeg Spec ISO/IEC 10918-1, Annex K, Table K.1
static const uint8_t jpeg_luma_quant[64] = {
    16, 11, 10, 16, 24,  40,  51,  61,
    12, 12, 14, 19, 26,  58,  60,  55,
    14, 13, 16, 24, 40,  57,  69,  56,
    14, 17, 22, 29, 51,  87,  80,  62,
    18, 22, 37, 56, 68,  109, 103, 77,
    24, 35, 55, 64, 81,  104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101,
    72, 92, 95, 98, 112, 100, 103, 99    
};

//Default Chroma quantization table
//Source: Jpeg Spec ISO/IEC 10918-1, Annex K, Table K.2
static const uint8_t jpeg_chroma_quant[64] = {
    17, 18, 24, 47, 99, 99, 99, 99,
    18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99,
    47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99
};


static const int va_to_gen7_jpeg_hufftable[2] = {
    MFX_HUFFTABLE_ID_Y,
    MFX_HUFFTABLE_ID_UV
};

static const uint32_t gen8_mfc_batchbuffer_avc_intra[][4] = {
#include "shaders/utils/mfc_batchbuffer_avc_intra.g7b"
};

static const uint32_t gen8_mfc_batchbuffer_avc_inter[][4] = {
#include "shaders/utils/mfc_batchbuffer_avc_inter.g7b"
};

static struct i965_kernel gen8_mfc_kernels[] = {
    {
        "MFC AVC INTRA BATCHBUFFER ",
        MFC_BATCHBUFFER_AVC_INTRA,
        gen8_mfc_batchbuffer_avc_intra,
        sizeof(gen8_mfc_batchbuffer_avc_intra),
        NULL
    },

    {
        "MFC AVC INTER BATCHBUFFER ",
        MFC_BATCHBUFFER_AVC_INTER,
        gen8_mfc_batchbuffer_avc_inter,
        sizeof(gen8_mfc_batchbuffer_avc_inter),
        NULL
    },
};

#define		INTER_MODE_MASK		0x03
#define		INTER_8X8		0x03
#define		INTER_16X8		0x01
#define		INTER_8X16		0x02
#define		SUBMB_SHAPE_MASK	0x00FF00
#define		INTER_16X16		0x00

#define		INTER_MV8		(4 << 20)
#define		INTER_MV32		(6 << 20)


static void
gen8_mfc_pipe_mode_select(VADriverContextP ctx,
                          int standard_select,
                          struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;

    assert(standard_select == MFX_FORMAT_MPEG2 ||
           standard_select == MFX_FORMAT_AVC   ||
           standard_select == MFX_FORMAT_JPEG  ||
           standard_select == MFX_FORMAT_VP8);

    BEGIN_BCS_BATCH(batch, 5);

    OUT_BCS_BATCH(batch, MFX_PIPE_MODE_SELECT | (5 - 2));
    OUT_BCS_BATCH(batch,
                  (MFX_LONG_MODE << 17) | /* Must be long format for encoder */
                  (MFD_MODE_VLD << 15) | /* VLD mode */
                  (0 << 10) | /* Stream-Out Enable */
                  ((!!mfc_context->post_deblocking_output.bo) << 9)  | /* Post Deblocking Output */
                  ((!!mfc_context->pre_deblocking_output.bo) << 8)  | /* Pre Deblocking Output */
                  (0 << 6)  | /* frame statistics stream-out enable*/
                  (0 << 5)  | /* not in stitch mode */
                  (1 << 4)  | /* encoding mode */
                  (standard_select << 0));  /* standard select: avc or mpeg2 or jpeg*/
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
gen8_mfc_surface_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
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
gen8_mfc_ind_obj_base_addr_state(VADriverContextP ctx,
                                 struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    int vme_size;
    unsigned int bse_offset;

    BEGIN_BCS_BATCH(batch, 26);

    OUT_BCS_BATCH(batch, MFX_IND_OBJ_BASE_ADDR_STATE | (26 - 2));
    /* the DW1-3 is for the MFX indirect bistream offset */
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);

    /* the DW4-5 is the MFX upper bound */
    if (encoder_context->codec == CODEC_VP8) {
        OUT_BCS_RELOC(batch,
                mfc_context->mfc_indirect_pak_bse_object.bo,
                I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                mfc_context->mfc_indirect_pak_bse_object.end_offset);
        OUT_BCS_BATCH(batch, 0);
    } else {
        OUT_BCS_BATCH(batch, 0);
        OUT_BCS_BATCH(batch, 0);
    }

    if(encoder_context->codec != CODEC_JPEG) {
        vme_size = vme_context->vme_output.size_block * vme_context->vme_output.num_blocks;
        /* the DW6-10 is for MFX Indirect MV Object Base Address */
        OUT_BCS_RELOC(batch, vme_context->vme_output.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);
        OUT_BCS_BATCH(batch, 0);
        OUT_BCS_BATCH(batch, 0);
        OUT_BCS_RELOC(batch, vme_context->vme_output.bo, I915_GEM_DOMAIN_INSTRUCTION, 0, vme_size);
        OUT_BCS_BATCH(batch, 0);
    } else {
        /* No VME for JPEG */
        OUT_BCS_BATCH(batch, 0);
        OUT_BCS_BATCH(batch, 0);
        OUT_BCS_BATCH(batch, 0);
        OUT_BCS_BATCH(batch, 0);
        OUT_BCS_BATCH(batch, 0);
    }

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
    bse_offset = (encoder_context->codec == CODEC_JPEG) ? (mfc_context->mfc_indirect_pak_bse_object.offset) : 0;
    OUT_BCS_RELOC(batch,
                  mfc_context->mfc_indirect_pak_bse_object.bo,
                  I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION,
                  bse_offset);
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
gen8_mfc_avc_img_state(VADriverContextP ctx, struct encode_state *encode_state,  
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
gen8_mfc_qm_state(VADriverContextP ctx,
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
gen8_mfc_avc_qm_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
{
    unsigned int qm[16] = {
        0x10101010, 0x10101010, 0x10101010, 0x10101010,
        0x10101010, 0x10101010, 0x10101010, 0x10101010,
        0x10101010, 0x10101010, 0x10101010, 0x10101010,
        0x10101010, 0x10101010, 0x10101010, 0x10101010
    };

    gen8_mfc_qm_state(ctx, MFX_QM_AVC_4X4_INTRA_MATRIX, qm, 12, encoder_context);
    gen8_mfc_qm_state(ctx, MFX_QM_AVC_4X4_INTER_MATRIX, qm, 12, encoder_context);
    gen8_mfc_qm_state(ctx, MFX_QM_AVC_8x8_INTRA_MATRIX, qm, 16, encoder_context);
    gen8_mfc_qm_state(ctx, MFX_QM_AVC_8x8_INTER_MATRIX, qm, 16, encoder_context);
}

static void
gen8_mfc_fqm_state(VADriverContextP ctx,
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
gen8_mfc_avc_fqm_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
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

    gen8_mfc_fqm_state(ctx, MFX_QM_AVC_4X4_INTRA_MATRIX, qm, 24, encoder_context);
    gen8_mfc_fqm_state(ctx, MFX_QM_AVC_4X4_INTER_MATRIX, qm, 24, encoder_context);
    gen8_mfc_fqm_state(ctx, MFX_QM_AVC_8x8_INTRA_MATRIX, qm, 32, encoder_context);
    gen8_mfc_fqm_state(ctx, MFX_QM_AVC_8x8_INTER_MATRIX, qm, 32, encoder_context);
}

static void
gen8_mfc_avc_insert_object(VADriverContextP ctx, struct intel_encoder_context *encoder_context,
                           unsigned int *insert_data, int lenght_in_dws, int data_bits_in_last_dw,
                           int skip_emul_byte_count, int is_last_header, int is_end_of_slice, int emulation_flag,
                           struct intel_batchbuffer *batch)
{
    if (batch == NULL)
        batch = encoder_context->base.batch;

    if (data_bits_in_last_dw == 0)
	data_bits_in_last_dw = 32;

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


static void gen8_mfc_init(VADriverContextP ctx,
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
    } else if (encoder_context->codec == CODEC_MPEG2) {
        VAEncSequenceParameterBufferMPEG2 *pSequenceParameter = (VAEncSequenceParameterBufferMPEG2 *)encode_state->seq_param_ext->buffer;

        assert(encoder_context->codec == CODEC_MPEG2);

        width_in_mbs = ALIGN(pSequenceParameter->picture_width, 16) / 16;
        height_in_mbs = ALIGN(pSequenceParameter->picture_height, 16) / 16;
    } else {
        assert(encoder_context->codec == CODEC_JPEG);
        VAEncPictureParameterBufferJPEG *pic_param = (VAEncPictureParameterBufferJPEG *)encode_state->pic_param_ext->buffer;

        width_in_mbs = ALIGN(pic_param->picture_width, 16) / 16;
        height_in_mbs = ALIGN(pic_param->picture_height, 16) / 16;
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

    mfc_context->aux_batchbuffer = intel_batchbuffer_new(&i965->intel, I915_EXEC_BSD, slice_batchbuffer_size);
    mfc_context->aux_batchbuffer_surface.bo = mfc_context->aux_batchbuffer->buffer;
    dri_bo_reference(mfc_context->aux_batchbuffer_surface.bo);
    mfc_context->aux_batchbuffer_surface.pitch = 16;
    mfc_context->aux_batchbuffer_surface.num_blocks = mfc_context->aux_batchbuffer->size / 16;
    mfc_context->aux_batchbuffer_surface.size_block = 16;

    i965_gpe_context_init(ctx, &mfc_context->gpe_context);
}

static void
gen8_mfc_pipe_buf_addr_state(VADriverContextP ctx,
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
gen8_mfc_avc_directmode_state(VADriverContextP ctx,
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
gen8_mfc_bsp_buf_base_addr_state(VADriverContextP ctx,
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


static void gen8_mfc_avc_pipeline_picture_programing( VADriverContextP ctx,
                                                      struct encode_state *encode_state,
                                                      struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;

    mfc_context->pipe_mode_select(ctx, MFX_FORMAT_AVC, encoder_context);
    mfc_context->set_surface_state(ctx, encoder_context);
    mfc_context->ind_obj_base_addr_state(ctx, encoder_context);
    gen8_mfc_pipe_buf_addr_state(ctx, encoder_context);
    gen8_mfc_bsp_buf_base_addr_state(ctx, encoder_context);
    mfc_context->avc_img_state(ctx, encode_state, encoder_context);
    mfc_context->avc_qm_state(ctx, encoder_context);
    mfc_context->avc_fqm_state(ctx, encoder_context);
    gen8_mfc_avc_directmode_state(ctx, encoder_context); 
    intel_mfc_avc_ref_idx_state(ctx, encode_state, encoder_context);
}


static VAStatus gen8_mfc_run(VADriverContextP ctx, 
                             struct encode_state *encode_state,
                             struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;

    intel_batchbuffer_flush(batch);		//run the pipeline

    return VA_STATUS_SUCCESS;
}


static VAStatus
gen8_mfc_stop(VADriverContextP ctx, 
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
gen8_mfc_avc_slice_state(VADriverContextP ctx,
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


#ifdef MFC_SOFTWARE_HASWELL

static int
gen8_mfc_avc_pak_object_intra(VADriverContextP ctx, int x, int y, int end_mb,
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
gen8_mfc_avc_pak_object_inter(VADriverContextP ctx, int x, int y, int end_mb, int qp,
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

#define		AVC_INTRA_RDO_OFFSET	4
#define		AVC_INTER_RDO_OFFSET	10
#define		AVC_INTER_MSG_OFFSET	8	
#define		AVC_INTER_MV_OFFSET		48
#define		AVC_RDO_MASK		0xFFFF

static void 
gen8_mfc_avc_pipeline_slice_programing(VADriverContextP ctx,
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

    gen8_mfc_avc_slice_state(ctx,
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
            gen8_mfc_avc_pak_object_intra(ctx, x, y, last_mb, qp, msg, encoder_context, 0, 0, slice_batch);
        } else {
	    int inter_rdo, intra_rdo;
	    inter_rdo = msg[AVC_INTER_RDO_OFFSET] & AVC_RDO_MASK;
	    intra_rdo = msg[AVC_INTRA_RDO_OFFSET] & AVC_RDO_MASK;
	    offset = i * vme_context->vme_output.size_block + AVC_INTER_MV_OFFSET;
	    if (intra_rdo < inter_rdo) { 
                gen8_mfc_avc_pak_object_intra(ctx, x, y, last_mb, qp, msg, encoder_context, 0, 0, slice_batch);
            } else {
		msg += AVC_INTER_MSG_OFFSET;
                gen8_mfc_avc_pak_object_inter(ctx, x, y, last_mb, qp, msg, offset, encoder_context, 0, 0, pSliceParameter->slice_type, slice_batch);
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
gen8_mfc_avc_software_batchbuffer(VADriverContextP ctx,
                                  struct encode_state *encode_state,
                                  struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct intel_batchbuffer *batch;
    dri_bo *batch_bo;
    int i;

    batch = mfc_context->aux_batchbuffer;
    batch_bo = batch->buffer;
    for (i = 0; i < encode_state->num_slice_params_ext; i++) {
        gen8_mfc_avc_pipeline_slice_programing(ctx, encode_state, encoder_context, i, batch);
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
gen8_mfc_batchbuffer_surfaces_input(VADriverContextP ctx,
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
    assert(mfc_context->aux_batchbuffer_surface.bo);
    mfc_context->buffer_suface_setup(ctx,
                                     &mfc_context->gpe_context,
                                     &mfc_context->aux_batchbuffer_surface,
                                     BINDING_TABLE_OFFSET(BIND_IDX_MFC_SLICE_HEADER),
                                     SURFACE_STATE_OFFSET(BIND_IDX_MFC_SLICE_HEADER));
}

static void
gen8_mfc_batchbuffer_surfaces_output(VADriverContextP ctx,
                                     struct encode_state *encode_state,
                                     struct intel_encoder_context *encoder_context)

{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    VAEncSequenceParameterBufferH264 *pSequenceParameter = (VAEncSequenceParameterBufferH264 *)encode_state->seq_param_ext->buffer;
    int width_in_mbs = pSequenceParameter->picture_width_in_mbs;
    int height_in_mbs = pSequenceParameter->picture_height_in_mbs;
    mfc_context->mfc_batchbuffer_surface.num_blocks = width_in_mbs * height_in_mbs + encode_state->num_slice_params_ext * 8 + 1;
    mfc_context->mfc_batchbuffer_surface.size_block = 16 * CMD_LEN_IN_OWORD; /* 3 OWORDs */
    mfc_context->mfc_batchbuffer_surface.pitch = 16;
    mfc_context->mfc_batchbuffer_surface.bo = dri_bo_alloc(i965->intel.bufmgr, 
                                                           "MFC batchbuffer",
                                                           mfc_context->mfc_batchbuffer_surface.num_blocks * mfc_context->mfc_batchbuffer_surface.size_block,
                                                           0x1000);
    mfc_context->buffer_suface_setup(ctx,
                                     &mfc_context->gpe_context,
                                     &mfc_context->mfc_batchbuffer_surface,
                                     BINDING_TABLE_OFFSET(BIND_IDX_MFC_BATCHBUFFER),
                                     SURFACE_STATE_OFFSET(BIND_IDX_MFC_BATCHBUFFER));
}

static void
gen8_mfc_batchbuffer_surfaces_setup(VADriverContextP ctx, 
                                    struct encode_state *encode_state,
                                    struct intel_encoder_context *encoder_context)
{
    gen8_mfc_batchbuffer_surfaces_input(ctx, encode_state, encoder_context);
    gen8_mfc_batchbuffer_surfaces_output(ctx, encode_state, encoder_context);
}

static void
gen8_mfc_batchbuffer_idrt_setup(VADriverContextP ctx, 
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
gen8_mfc_batchbuffer_constant_setup(VADriverContextP ctx, 
                                    struct encode_state *encode_state,
                                    struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    
    (void)mfc_context;
}

static void
gen8_mfc_batchbuffer_emit_object_command(struct intel_batchbuffer *batch,
                                         int index,
                                         int head_offset,
                                         int batchbuffer_offset,
                                         int head_size,
                                         int tail_size,
                                         int number_mb_cmds,
                                         int first_object,
                                         int last_object,
                                         int last_slice,
                                         int mb_x,
                                         int mb_y,
                                         int width_in_mbs,
                                         int qp)
{
    BEGIN_BATCH(batch, 12);
    
    OUT_BATCH(batch, CMD_MEDIA_OBJECT | (12 - 2));
    OUT_BATCH(batch, index);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
   
    /*inline data */
    OUT_BATCH(batch, head_offset);
    OUT_BATCH(batch, batchbuffer_offset);
    OUT_BATCH(batch, 
              head_size << 16 |
              tail_size);
    OUT_BATCH(batch,
              number_mb_cmds << 16 |
              first_object << 2 |
              last_object << 1 |
              last_slice);
    OUT_BATCH(batch,
              mb_y << 8 |
              mb_x);
    OUT_BATCH(batch,
              qp << 16 |
              width_in_mbs);

    ADVANCE_BATCH(batch);
}

static void
gen8_mfc_avc_batchbuffer_slice_command(VADriverContextP ctx,
                                       struct intel_encoder_context *encoder_context,
                                       VAEncSliceParameterBufferH264 *slice_param,
                                       int head_offset,
                                       unsigned short head_size,
                                       unsigned short tail_size,
                                       int batchbuffer_offset,
                                       int qp,
                                       int last_slice)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    int width_in_mbs = (mfc_context->surface_state.width + 15) / 16;
    int total_mbs = slice_param->num_macroblocks;
    int number_mb_cmds = 128;
    int starting_mb = 0;
    int last_object = 0;
    int first_object = 1;
    int i;
    int mb_x, mb_y;
    int index = (slice_param->slice_type == SLICE_TYPE_I) ? MFC_BATCHBUFFER_AVC_INTRA : MFC_BATCHBUFFER_AVC_INTER;

    for (i = 0; i < total_mbs / number_mb_cmds; i++) {
        last_object = (total_mbs - starting_mb) == number_mb_cmds;
        mb_x = (slice_param->macroblock_address + starting_mb) % width_in_mbs;
        mb_y = (slice_param->macroblock_address + starting_mb) / width_in_mbs;
        assert(mb_x <= 255 && mb_y <= 255);

        starting_mb += number_mb_cmds;

        gen8_mfc_batchbuffer_emit_object_command(batch,
                                                 index,
                                                 head_offset,
                                                 batchbuffer_offset,
                                                 head_size,
                                                 tail_size,
                                                 number_mb_cmds,
                                                 first_object,
                                                 last_object,
                                                 last_slice,
                                                 mb_x,
                                                 mb_y,
                                                 width_in_mbs,
                                                 qp);

        if (first_object) {
            head_offset += head_size;
            batchbuffer_offset += head_size;
        }

        if (last_object) {
            head_offset += tail_size;
            batchbuffer_offset += tail_size;
        }

        batchbuffer_offset += number_mb_cmds * CMD_LEN_IN_OWORD;

        first_object = 0;
    }

    if (!last_object) {
        last_object = 1;
        number_mb_cmds = total_mbs % number_mb_cmds;
        mb_x = (slice_param->macroblock_address + starting_mb) % width_in_mbs;
        mb_y = (slice_param->macroblock_address + starting_mb) / width_in_mbs;
        assert(mb_x <= 255 && mb_y <= 255);
        starting_mb += number_mb_cmds;

        gen8_mfc_batchbuffer_emit_object_command(batch,
                                                 index,
                                                 head_offset,
                                                 batchbuffer_offset,
                                                 head_size,
                                                 tail_size,
                                                 number_mb_cmds,
                                                 first_object,
                                                 last_object,
                                                 last_slice,
                                                 mb_x,
                                                 mb_y,
                                                 width_in_mbs,
                                                 qp);
    }
}
                          
/*
 * return size in Owords (16bytes)
 */         
static int
gen8_mfc_avc_batchbuffer_slice(VADriverContextP ctx,
                               struct encode_state *encode_state,
                               struct intel_encoder_context *encoder_context,
                               int slice_index,
                               int batchbuffer_offset)
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
    int old_used = intel_batchbuffer_used_size(slice_batch), used;
    unsigned short head_size, tail_size;
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

    head_offset = old_used / 16;
    gen8_mfc_avc_slice_state(ctx,
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
    used = intel_batchbuffer_used_size(slice_batch);
    head_size = (used - old_used) / 16;
    old_used = used;

    /* tail */
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

    intel_batchbuffer_align(slice_batch, 16); /* aligned by an Oword */
    used = intel_batchbuffer_used_size(slice_batch);
    tail_size = (used - old_used) / 16;

   
    gen8_mfc_avc_batchbuffer_slice_command(ctx,
                                           encoder_context,
                                           pSliceParameter,
                                           head_offset,
                                           head_size,
                                           tail_size,
                                           batchbuffer_offset,
                                           qp,
                                           last_slice);

    return head_size + tail_size + pSliceParameter->num_macroblocks * CMD_LEN_IN_OWORD;
}

static void
gen8_mfc_avc_batchbuffer_pipeline(VADriverContextP ctx,
                                  struct encode_state *encode_state,
                                  struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    int i, size, offset = 0;
    intel_batchbuffer_start_atomic(batch, 0x4000); 
    gen6_gpe_pipeline_setup(ctx, &mfc_context->gpe_context, batch);

    for ( i = 0; i < encode_state->num_slice_params_ext; i++) {
        size = gen8_mfc_avc_batchbuffer_slice(ctx, encode_state, encoder_context, i, offset);
        offset += size;
    }

    intel_batchbuffer_end_atomic(batch);
    intel_batchbuffer_flush(batch);
}

static void
gen8_mfc_build_avc_batchbuffer(VADriverContextP ctx, 
                               struct encode_state *encode_state,
                               struct intel_encoder_context *encoder_context)
{
    gen8_mfc_batchbuffer_surfaces_setup(ctx, encode_state, encoder_context);
    gen8_mfc_batchbuffer_idrt_setup(ctx, encode_state, encoder_context);
    gen8_mfc_batchbuffer_constant_setup(ctx, encode_state, encoder_context);
    gen8_mfc_avc_batchbuffer_pipeline(ctx, encode_state, encoder_context);
}

static dri_bo *
gen8_mfc_avc_hardware_batchbuffer(VADriverContextP ctx,
                                  struct encode_state *encode_state,
                                  struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;

    gen8_mfc_build_avc_batchbuffer(ctx, encode_state, encoder_context);
    dri_bo_reference(mfc_context->mfc_batchbuffer_surface.bo);

    return mfc_context->mfc_batchbuffer_surface.bo;
}

#endif

static void
gen8_mfc_avc_pipeline_programing(VADriverContextP ctx,
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

#ifdef MFC_SOFTWARE_HASWELL
    slice_batch_bo = gen8_mfc_avc_software_batchbuffer(ctx, encode_state, encoder_context);
#else
    slice_batch_bo = gen8_mfc_avc_hardware_batchbuffer(ctx, encode_state, encoder_context);
#endif

    // begin programing
    intel_batchbuffer_start_atomic_bcs(batch, 0x4000); 
    intel_batchbuffer_emit_mi_flush(batch);
    
    // picture level programing
    gen8_mfc_avc_pipeline_picture_programing(ctx, encode_state, encoder_context);

    BEGIN_BCS_BATCH(batch, 3);
    OUT_BCS_BATCH(batch, MI_BATCH_BUFFER_START | (1 << 8) | (1 << 0));
    OUT_BCS_RELOC(batch,
                  slice_batch_bo,
                  I915_GEM_DOMAIN_COMMAND, 0, 
                  0);
    OUT_BCS_BATCH(batch, 0);
    ADVANCE_BCS_BATCH(batch);

    // end programing
    intel_batchbuffer_end_atomic(batch);

    dri_bo_unreference(slice_batch_bo);
}


static VAStatus
gen8_mfc_avc_encode_picture(VADriverContextP ctx, 
                            struct encode_state *encode_state,
                            struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    unsigned int rate_control_mode = encoder_context->rate_control_mode;
    int current_frame_bits_size;
    int sts;
 
    for (;;) {
        gen8_mfc_init(ctx, encode_state, encoder_context);
        intel_mfc_avc_prepare(ctx, encode_state, encoder_context);
        /*Programing bcs pipeline*/
        gen8_mfc_avc_pipeline_programing(ctx, encode_state, encoder_context);	//filling the pipeline
        gen8_mfc_run(ctx, encode_state, encoder_context);
        if (rate_control_mode == VA_RC_CBR /*|| rate_control_mode == VA_RC_VBR*/) {
            gen8_mfc_stop(ctx, encode_state, encoder_context, &current_frame_bits_size);
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
va_to_gen8_mpeg2_picture_type[3] = {
    1,  /* I */
    2,  /* P */
    3   /* B */
};

static void
gen8_mfc_mpeg2_pic_state(VADriverContextP ctx,
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
                  va_to_gen8_mpeg2_picture_type[pic_param->picture_type] << 9 |
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
gen8_mfc_mpeg2_qm_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
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

    gen8_mfc_qm_state(ctx, MFX_QM_MPEG_INTRA_QUANTIZER_MATRIX, (unsigned int *)intra_qm, 16, encoder_context);
    gen8_mfc_qm_state(ctx, MFX_QM_MPEG_NON_INTRA_QUANTIZER_MATRIX, (unsigned int *)non_intra_qm, 16,encoder_context);
}

static void
gen8_mfc_mpeg2_fqm_state(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
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

    gen8_mfc_fqm_state(ctx, MFX_QM_MPEG_INTRA_QUANTIZER_MATRIX, (unsigned int *)intra_fqm, 32, encoder_context);
    gen8_mfc_fqm_state(ctx, MFX_QM_MPEG_NON_INTRA_QUANTIZER_MATRIX, (unsigned int *)non_intra_fqm, 32, encoder_context);
}

static void
gen8_mfc_mpeg2_slicegroup_state(VADriverContextP ctx,
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
gen8_mfc_mpeg2_pak_object_intra(VADriverContextP ctx,
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

/* Byte offset */
#define MPEG2_INTER_MV_OFFSET   48 

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
gen8_mfc_mpeg2_pak_object_inter(VADriverContextP ctx,
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

    mvptr = (short *)((unsigned char *)msg + MPEG2_INTER_MV_OFFSET);;
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
gen8_mfc_mpeg2_pipeline_slice_group(VADriverContextP ctx,
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

    gen8_mfc_mpeg2_slicegroup_state(ctx,
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
                gen8_mfc_mpeg2_pak_object_intra(ctx,
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
                    gen8_mfc_mpeg2_pak_object_intra(ctx,
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
                    gen8_mfc_mpeg2_pak_object_inter(ctx,
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
gen8_mfc_mpeg2_software_slice_batchbuffer(VADriverContextP ctx,
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

        gen8_mfc_mpeg2_pipeline_slice_group(ctx, encode_state, encoder_context, i, next_slice_group_param, batch);
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
gen8_mfc_mpeg2_pipeline_picture_programing(VADriverContextP ctx,
                                           struct encode_state *encode_state,
                                           struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;

    mfc_context->pipe_mode_select(ctx, MFX_FORMAT_MPEG2, encoder_context);
    mfc_context->set_surface_state(ctx, encoder_context);
    mfc_context->ind_obj_base_addr_state(ctx, encoder_context);
    gen8_mfc_pipe_buf_addr_state(ctx, encoder_context);
    gen8_mfc_bsp_buf_base_addr_state(ctx, encoder_context);
    gen8_mfc_mpeg2_pic_state(ctx, encoder_context, encode_state);
    gen8_mfc_mpeg2_qm_state(ctx, encoder_context);
    gen8_mfc_mpeg2_fqm_state(ctx, encoder_context);
}

static void
gen8_mfc_mpeg2_pipeline_programing(VADriverContextP ctx,
                                   struct encode_state *encode_state,
                                   struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    dri_bo *slice_batch_bo;

    slice_batch_bo = gen8_mfc_mpeg2_software_slice_batchbuffer(ctx, encode_state, encoder_context);

    // begin programing
    intel_batchbuffer_start_atomic_bcs(batch, 0x4000); 
    intel_batchbuffer_emit_mi_flush(batch);
    
    // picture level programing
    gen8_mfc_mpeg2_pipeline_picture_programing(ctx, encode_state, encoder_context);

    BEGIN_BCS_BATCH(batch, 4);
    OUT_BCS_BATCH(batch, MI_BATCH_BUFFER_START | (1 << 8) | (1 << 0));
    OUT_BCS_RELOC(batch,
                  slice_batch_bo,
                  I915_GEM_DOMAIN_COMMAND, 0, 
                  0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
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
gen8_mfc_mpeg2_encode_picture(VADriverContextP ctx, 
                              struct encode_state *encode_state,
                              struct intel_encoder_context *encoder_context)
{
    gen8_mfc_init(ctx, encode_state, encoder_context);
    intel_mfc_mpeg2_prepare(ctx, encode_state, encoder_context);
    /*Programing bcs pipeline*/
    gen8_mfc_mpeg2_pipeline_programing(ctx, encode_state, encoder_context);
    gen8_mfc_run(ctx, encode_state, encoder_context);

    return VA_STATUS_SUCCESS;
}

/* JPEG encode methods */

static VAStatus
intel_mfc_jpeg_prepare(VADriverContextP ctx, 
                        struct encode_state *encode_state,
                        struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct object_surface *obj_surface; 
    struct object_buffer *obj_buffer;
    struct i965_coded_buffer_segment *coded_buffer_segment;
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    dri_bo *bo;
   
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


static void 
gen8_mfc_jpeg_set_surface_state(VADriverContextP ctx,
                        struct intel_encoder_context *encoder_context,
                        struct encode_state *encode_state)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct object_surface *obj_surface = encode_state->input_yuv_object;
    unsigned int input_fourcc;
    unsigned int y_cb_offset;
    unsigned int y_cr_offset;
    unsigned int surface_format;

    assert(obj_surface);

    y_cb_offset = obj_surface->y_cb_offset;
    y_cr_offset = obj_surface->y_cr_offset;
    input_fourcc = obj_surface->fourcc;

    surface_format = (obj_surface->fourcc == VA_FOURCC_Y800) ?
        MFX_SURFACE_MONOCHROME : MFX_SURFACE_PLANAR_420_8;
        
        
     switch (input_fourcc) {
        case VA_FOURCC_Y800: {
            surface_format = MFX_SURFACE_MONOCHROME;
            break;
        }
        case VA_FOURCC_NV12: { 
            surface_format = MFX_SURFACE_PLANAR_420_8;
            break;
        }      
        case VA_FOURCC_UYVY: { 
            surface_format = MFX_SURFACE_YCRCB_SWAPY;
            break;
        }
        case VA_FOURCC_YUY2: { 
            surface_format = MFX_SURFACE_YCRCB_NORMAL;
            break;
        }
        case VA_FOURCC_RGBA:
        case VA_FOURCC_444P: {
            surface_format = MFX_SURFACE_R8G8B8A8_UNORM;
            break;
        }
    }

    BEGIN_BCS_BATCH(batch, 6);

    OUT_BCS_BATCH(batch, MFX_SURFACE_STATE | (6 - 2));
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch,
                  ((obj_surface->orig_height - 1) << 18) |
                  ((obj_surface->orig_width - 1) << 4));
    OUT_BCS_BATCH(batch,
                  (surface_format << 28) | /* Surface Format */
                  (0 << 27) | /* must be 1 for interleave U/V, hardware requirement for AVC/VC1/MPEG and 0 for JPEG */
                  (0 << 22) | /* surface object control state, FIXME??? */
                  ((obj_surface->width - 1) << 3) | /* pitch */
                  (0 << 2)  | /* must be 0 for interleave U/V */
                  (1 << 1)  | /* must be tiled */
                  (I965_TILEWALK_YMAJOR << 0));  /* tile walk, TILEWALK_YMAJOR */
    OUT_BCS_BATCH(batch,
                  (0 << 16) | /* X offset for U(Cb), must be 0 */
                  (y_cb_offset << 0)); /* Y offset for U(Cb) */
    OUT_BCS_BATCH(batch,
                  (0 << 16) | /* X offset for V(Cr), must be 0 */
                  (y_cr_offset << 0)); /* Y offset for V(Cr), must be 0 for video codec, non-zoeo for JPEG */
                 

    ADVANCE_BCS_BATCH(batch);
}

static void
gen8_mfc_jpeg_pic_state(VADriverContextP ctx,
                        struct intel_encoder_context *encoder_context,
                        struct encode_state *encode_state)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct object_surface *obj_surface = encode_state->input_yuv_object;
    VAEncPictureParameterBufferJPEG *pic_param;
    unsigned int  surface_format;
    unsigned int  frame_width_in_blks;
    unsigned int  frame_height_in_blks;
    unsigned int  pixels_in_horizontal_lastMCU;
    unsigned int  pixels_in_vertical_lastMCU;
    unsigned int  input_surface_format;
    unsigned int  output_mcu_format;
    unsigned int  picture_width;
    unsigned int  picture_height;  

    assert(encode_state->pic_param_ext && encode_state->pic_param_ext->buffer);
    assert(obj_surface);
    pic_param = (VAEncPictureParameterBufferJPEG *)encode_state->pic_param_ext->buffer;
    surface_format = obj_surface->fourcc;
    picture_width = pic_param->picture_width;
    picture_height = pic_param->picture_height;
    
    switch (surface_format) {
        case VA_FOURCC_Y800: {
            input_surface_format = JPEG_ENC_SURFACE_Y8; 
            output_mcu_format = JPEG_ENC_MCU_YUV400;
            break;
        }
        case VA_FOURCC_NV12: { 
            input_surface_format = JPEG_ENC_SURFACE_NV12; 
            output_mcu_format = JPEG_ENC_MCU_YUV420; 
            break;
        }      
        case VA_FOURCC_UYVY: { 
            input_surface_format = JPEG_ENC_SURFACE_UYVY; 
            output_mcu_format = JPEG_ENC_MCU_YUV422H_2Y; 
            break;
        }
        case VA_FOURCC_YUY2: { 
            input_surface_format = JPEG_ENC_SURFACE_YUY2; 
            output_mcu_format = JPEG_ENC_MCU_YUV422H_2Y; 
            break;
        }

        case VA_FOURCC_RGBA:
        case VA_FOURCC_444P: { 
            input_surface_format = JPEG_ENC_SURFACE_RGB; 
            output_mcu_format = JPEG_ENC_MCU_RGB; 
            break;
        }
        default : {
            input_surface_format = JPEG_ENC_SURFACE_NV12; 
            output_mcu_format = JPEG_ENC_MCU_YUV420;
            break;
        }
    }

    
    switch (output_mcu_format) {
        
        case JPEG_ENC_MCU_YUV400:
        case JPEG_ENC_MCU_RGB: {
            pixels_in_horizontal_lastMCU = (picture_width % 8);
            pixels_in_vertical_lastMCU = (picture_height % 8); 

            //H1=1,V1=1 for YUV400 and YUV444. So, compute these values accordingly
            frame_width_in_blks = ((picture_width + 7) / 8); 
            frame_height_in_blks = ((picture_height + 7) / 8);
            break;
        }
        
        case JPEG_ENC_MCU_YUV420: {        
            if((picture_width % 2) == 0) 
                pixels_in_horizontal_lastMCU = picture_width % 16; 
            else 
                pixels_in_horizontal_lastMCU   = ((picture_width % 16) + 1) % 16; 
            
            if((picture_height % 2) == 0) 
                pixels_in_vertical_lastMCU     = picture_height % 16; 
            else 
                pixels_in_vertical_lastMCU   = ((picture_height % 16) + 1) % 16; 

            //H1=2,V1=2 for YUV420. So, compute these values accordingly
            frame_width_in_blks = ((picture_width + 15) / 16) * 2;
            frame_height_in_blks = ((picture_height + 15) / 16) * 2;
            break;
        }
        
        case JPEG_ENC_MCU_YUV422H_2Y: {
            if(picture_width % 2 == 0) 
                pixels_in_horizontal_lastMCU = picture_width % 16; 
            else 
                pixels_in_horizontal_lastMCU = ((picture_width % 16) + 1) % 16; 
            
            pixels_in_vertical_lastMCU = picture_height % 8;
            
            //H1=2,V1=1 for YUV422H_2Y. So, compute these values accordingly
            frame_width_in_blks = ((picture_width + 15) / 16) * 2;
            frame_height_in_blks = ((picture_height + 7) / 8);
            break;            
        }       
    } //end of switch
   
    BEGIN_BCS_BATCH(batch, 3);
    /* DWORD 0 */
    OUT_BCS_BATCH(batch, MFX_JPEG_PIC_STATE | (3 - 2)); 
    /* DWORD 1 */
    OUT_BCS_BATCH(batch,
                  ( pixels_in_horizontal_lastMCU << 26) |    /* Pixels In Horizontal Last MCU */
                  ( pixels_in_vertical_lastMCU << 21)   |    /* Pixels In Vertical Last MCU */
                  ( input_surface_format << 8)          |    /* Input Surface format */
                  ( output_mcu_format << 0));                /* Output MCU Structure */
    /* DWORD 2 */
    OUT_BCS_BATCH(batch,
                  ((frame_height_in_blks - 1) << 16)    |   /* Frame Height In Blks Minus 1 */
                  (JPEG_ENC_ROUND_QUANT_DEFAULT  << 13) |   /* Rounding Quant set to default value 0 */
                  ((frame_width_in_blks - 1) << 0));        /* Frame Width In Blks Minus 1 */
    ADVANCE_BCS_BATCH(batch);
}

static void 
get_reciprocal_dword_qm(unsigned char *raster_qm, uint32_t *dword_qm)
{
    int i = 0, j = 0;
    short reciprocal_qm[64];
    
    for(i=0; i<64; i++) {
        reciprocal_qm[i] = 65535/(raster_qm[i]);           
    }
    
    for(i=0; i<64; i++) {
        dword_qm[j] = ((reciprocal_qm[i+1] <<16) | (reciprocal_qm[i]));
        j++;
        i++;
    }    
    
}


static void 
gen8_mfc_jpeg_fqm_state(VADriverContextP ctx,
                        struct intel_encoder_context *encoder_context,
                        struct encode_state *encode_state)
{
    unsigned int quality = 0;
    uint32_t temp, i = 0, j = 0, dword_qm[32];
    VAEncPictureParameterBufferJPEG *pic_param;
    VAQMatrixBufferJPEG *qmatrix;
    unsigned char raster_qm[64], column_raster_qm[64];
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    
    assert(encode_state->pic_param_ext && encode_state->pic_param_ext->buffer);
    pic_param = (VAEncPictureParameterBufferJPEG *)encode_state->pic_param_ext->buffer;
    quality = pic_param->quality;
    
    //If the app sends the qmatrix, use it, buffer it for using it with the next frames 
    //The app can send qmatrix for the first frame and not send for the subsequent frames
    if(encode_state->q_matrix && encode_state->q_matrix->buffer) {
        qmatrix = (VAQMatrixBufferJPEG *)encode_state->q_matrix->buffer;

        mfc_context->buffered_qmatrix.load_lum_quantiser_matrix = 1;
        memcpy(mfc_context->buffered_qmatrix.lum_quantiser_matrix, qmatrix->lum_quantiser_matrix, 64 * (sizeof(unsigned char)));

        if(pic_param->num_components > 1) {
            mfc_context->buffered_qmatrix.load_chroma_quantiser_matrix = 1;
            memcpy(mfc_context->buffered_qmatrix.chroma_quantiser_matrix, qmatrix->chroma_quantiser_matrix, 64 * (sizeof(unsigned char)));
        } else {
            mfc_context->buffered_qmatrix.load_chroma_quantiser_matrix = 0;
        }

    } else {
        //If the app doesnt send the qmatrix, use the buffered/default qmatrix
        qmatrix = &mfc_context->buffered_qmatrix;
        qmatrix->load_lum_quantiser_matrix = 1;
        qmatrix->load_chroma_quantiser_matrix = (pic_param->num_components > 1) ? 1 : 0;
    }   


    //As per the design, normalization of the quality factor and scaling of the Quantization tables
    //based on the quality factor needs to be done in the driver before sending the values to the HW.
    //But note, the driver expects the scaled quantization tables (as per below logic) to be sent as
    //packed header information. The packed header is written as the header of the jpeg file. This
    //header information is used to decode the jpeg file. So, it is the app's responsibility to send
    //the correct header information (See build_packed_jpeg_header_buffer() in jpegenc.c in LibVa on
    //how to do this). QTables can be different for different applications. If no tables are provided,
    //the default tables in the driver are used.

    //Normalization of the quality factor
    if (quality > 100) quality=100;
    if (quality == 0)  quality=1;
    quality = (quality < 50) ? (5000/quality) : (200 - (quality*2)); 
    
    //Step 1. Apply Quality factor and clip to range [1, 255] for luma and chroma Quantization matrices
    //Step 2. HW expects the 1/Q[i] values in the qm sent, so get reciprocals
    //Step 3. HW also expects 32 dwords, hence combine 2 (1/Q) values into 1 dword
    //Step 4. Send the Quantization matrix to the HW, use gen8_mfc_fqm_state
    
    //For luma (Y or R)
    if(qmatrix->load_lum_quantiser_matrix) {
        //apply quality to lum_quantiser_matrix
        for(i=0; i < 64; i++) {
            temp = (qmatrix->lum_quantiser_matrix[i] * quality)/100;
            //clamp to range [1,255]
            temp = (temp > 255) ? 255 : temp;
            temp = (temp < 1) ? 1 : temp;
            qmatrix->lum_quantiser_matrix[i] = (unsigned char)temp;
        }       
        
        //For VAAPI, the VAQMatrixBuffer needs to be in zigzag order. 
        //The App should send it in zigzag. Now, the driver has to extract the raster from it. 
        for (j = 0; j < 64; j++)
            raster_qm[zigzag_direct[j]] = qmatrix->lum_quantiser_matrix[j];

        //Convert the raster order(row-ordered) to the column-raster (column by column).
        //To be consistent with the other encoders, send it in column order.
        //Need to double check if our HW expects col or row raster.
        for (j = 0; j < 64; j++) {
            int row = j / 8, col = j % 8;
            column_raster_qm[col * 8 + row] = raster_qm[j];
        }
        
        //Convert to raster QM to reciprocal. HW expects values in reciprocal.
        get_reciprocal_dword_qm(column_raster_qm, dword_qm);
        
        //send the luma qm to the command buffer
        gen8_mfc_fqm_state(ctx, MFX_QM_JPEG_LUMA_Y_QUANTIZER_MATRIX, dword_qm, 32, encoder_context);
    } 
    
    //For Chroma, if chroma exists (Cb, Cr or G, B)
    if(qmatrix->load_chroma_quantiser_matrix) {
        //apply quality to chroma_quantiser_matrix
        for(i=0; i < 64; i++) {
            temp = (qmatrix->chroma_quantiser_matrix[i] * quality)/100;
            //clamp to range [1,255]
            temp = (temp > 255) ? 255 : temp;
            temp = (temp < 1) ? 1 : temp;
            qmatrix->chroma_quantiser_matrix[i] = (unsigned char)temp;
        }
        
        //For VAAPI, the VAQMatrixBuffer needs to be in zigzag order. 
        //The App should send it in zigzag. Now, the driver has to extract the raster from it. 
        for (j = 0; j < 64; j++)
            raster_qm[zigzag_direct[j]] = qmatrix->chroma_quantiser_matrix[j];
        
        //Convert the raster order(row-ordered) to the column-raster (column by column).
        //To be consistent with the other encoders, send it in column order.
        //Need to double check if our HW expects col or row raster.
        for (j = 0; j < 64; j++) {
            int row = j / 8, col = j % 8;
            column_raster_qm[col * 8 + row] = raster_qm[j];
        }


        //Convert to raster QM to reciprocal. HW expects values in reciprocal.
        get_reciprocal_dword_qm(column_raster_qm, dword_qm);

        //send the same chroma qm to the command buffer (for both U,V or G,B)
        gen8_mfc_fqm_state(ctx, MFX_QM_JPEG_CHROMA_CB_QUANTIZER_MATRIX, dword_qm, 32, encoder_context);
        gen8_mfc_fqm_state(ctx, MFX_QM_JPEG_CHROMA_CR_QUANTIZER_MATRIX, dword_qm, 32, encoder_context);        
    }
}


//Translation of Table K.5 into code: This method takes the huffval from the 
//Huffmantable buffer and converts into index for the coefficients and size tables
uint8_t map_huffval_to_index(uint8_t huff_val) 
{
    uint8_t index = 0;

    if(huff_val < 0xF0) {
        index = (((huff_val >> 4) & 0x0F) * 0xA) + (huff_val & 0x0F);
    } else {
        index = 1 + (((huff_val >> 4) & 0x0F) * 0xA) + (huff_val & 0x0F);
    }

    return index;
}


//Implementation of Flow chart Annex C  - Figure C.1
static void
generate_huffman_codesizes_table(uint8_t *bits, uint8_t *huff_size_table, uint8_t *lastK) 
{
    uint8_t i=1, j=1, k=0;

    while(i <= 16) {
        while(j <= (uint8_t)bits[i-1]) {
            huff_size_table[k] = i;
            k = k+1;
            j = j+1;
        }
        
        i = i+1;
        j = 1;
    }
    huff_size_table[k] = 0;
    (*lastK) = k;    
}

//Implementation of Flow chart Annex C - Figure C.2
static void
generate_huffman_codes_table(uint8_t *huff_size_table, uint16_t *huff_code_table)
{
    uint8_t k=0;
    uint16_t code=0;
    uint8_t si=huff_size_table[k];
    
    while(huff_size_table[k] != 0) {
    
        while(huff_size_table[k] == si) {
            
            // An huffman code can never be 0xFFFF. Replace it with 0 if 0xFFFF 
            if(code == 0xFFFF) {
                code = 0x0000;
            }

            huff_code_table[k] = code;
            code = code+1;
            k = k+1;
        }
    
        code <<= 1;
        si = si+1;
    }
    
}

//Implementation of Flow chat Annex C - Figure C.3
static void
generate_ordered_codes_table(uint8_t *huff_vals, uint8_t *huff_size_table, uint16_t *huff_code_table, uint8_t type, uint8_t lastK)
{
    uint8_t huff_val_size=0, i=0, k=0;
    
    huff_val_size = (type == 0) ? 12 : 162; 
    uint8_t huff_si_table[huff_val_size]; 
    uint16_t huff_co_table[huff_val_size];
    
    memset(huff_si_table, 0, sizeof(huff_si_table));
    memset(huff_co_table, 0, sizeof(huff_co_table));
    
    do {
        i = map_huffval_to_index(huff_vals[k]);
        huff_co_table[i] = huff_code_table[k];
        huff_si_table[i] = huff_size_table[k];
        k++;
    } while(k < lastK);
    
    memcpy(huff_size_table, huff_si_table, sizeof(uint8_t)*huff_val_size);
    memcpy(huff_code_table, huff_co_table, sizeof(uint16_t)*huff_val_size);
}


//This method converts the huffman table to code words which is needed by the HW
//Flowcharts from Jpeg Spec Annex C - Figure C.1, Figure C.2, Figure C.3 are used here
static void
convert_hufftable_to_codes(VAHuffmanTableBufferJPEGBaseline *huff_buffer, uint32_t *table, uint8_t type, uint8_t index)
{
    uint8_t lastK = 0, i=0; 
    uint8_t huff_val_size = 0;
    uint8_t *huff_bits, *huff_vals;

    huff_val_size = (type == 0) ? 12 : 162; 
    uint8_t huff_size_table[huff_val_size+1]; //The +1 for adding 0 at the end of huff_val_size
    uint16_t huff_code_table[huff_val_size];

    memset(huff_size_table, 0, sizeof(huff_size_table));
    memset(huff_code_table, 0, sizeof(huff_code_table));

    huff_bits = (type == 0) ? (huff_buffer->huffman_table[index].num_dc_codes) : (huff_buffer->huffman_table[index].num_ac_codes);
    huff_vals = (type == 0) ? (huff_buffer->huffman_table[index].dc_values) : (huff_buffer->huffman_table[index].ac_values);
    

    //Generation of table of Huffman code sizes
    generate_huffman_codesizes_table(huff_bits, huff_size_table, &lastK);
       
    //Generation of table of Huffman codes
    generate_huffman_codes_table(huff_size_table, huff_code_table);
       
    //Ordering procedure for encoding procedure code tables
    generate_ordered_codes_table(huff_vals, huff_size_table, huff_code_table, type, lastK);

    //HW expects Byte0: Code length; Byte1,Byte2: Code Word, Byte3: Dummy
    //Since IA is littlended, &, | and << accordingly to store the values in the DWord.
    for(i=0; i<huff_val_size; i++) {
        table[i] = 0;
        table[i] = ((huff_size_table[i] & 0xFF) | ((huff_code_table[i] & 0xFFFF) << 8));
    }

}

//send the huffman table using MFC_JPEG_HUFF_TABLE_STATE
static void
gen8_mfc_jpeg_huff_table_state(VADriverContextP ctx,
                                           struct encode_state *encode_state,
                                           struct intel_encoder_context *encoder_context,
                                           int num_tables)
{
    VAHuffmanTableBufferJPEGBaseline *huff_buffer;
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    uint8_t index;
    uint32_t dc_table[12], ac_table[162]; 
    
    assert(encode_state->huffman_table && encode_state->huffman_table->buffer);
    huff_buffer = (VAHuffmanTableBufferJPEGBaseline *)encode_state->huffman_table->buffer;

    memset(dc_table, 0, 12);
    memset(ac_table, 0, 162);

    for (index = 0; index < num_tables; index++) {
        int id = va_to_gen7_jpeg_hufftable[index];
 
        if (!huff_buffer->load_huffman_table[index])
            continue;
     
        //load DC table with 12 DWords
        convert_hufftable_to_codes(huff_buffer, dc_table, 0, index);  //0 for Dc

        //load AC table with 162 DWords 
        convert_hufftable_to_codes(huff_buffer, ac_table, 1, index);  //1 for AC 

        BEGIN_BCS_BATCH(batch, 176);
        OUT_BCS_BATCH(batch, MFC_JPEG_HUFF_TABLE_STATE | (176 - 2));
        OUT_BCS_BATCH(batch, id); //Huff table id

        //DWord 2 - 13 has DC_TABLE
        intel_batchbuffer_data(batch, dc_table, 12*4);

        //Dword 14 -175 has AC_TABLE
        intel_batchbuffer_data(batch, ac_table, 162*4);
        ADVANCE_BCS_BATCH(batch);
    }    
}


//This method is used to compute the MCU count used for setting MFC_JPEG_SCAN_OBJECT
static void get_Y_sampling_factors(uint32_t surface_format, uint8_t *h_factor, uint8_t *v_factor)
{ 
    switch (surface_format) {
        case VA_FOURCC_Y800: {
            (* h_factor) = 1; 
            (* v_factor) = 1;
            break;
        }
        case VA_FOURCC_NV12: { 
            (* h_factor) = 2;             
            (* v_factor) = 2;
            break;
        }      
        case VA_FOURCC_UYVY: { 
            (* h_factor) = 2; 
            (* v_factor) = 1;
            break;
        }
        case VA_FOURCC_YUY2: { 
            (* h_factor) = 2; 
            (* v_factor) = 1;
            break;
        }
        case VA_FOURCC_RGBA:
        case VA_FOURCC_444P: { 
            (* h_factor) = 1; 
            (* v_factor) = 1;
            break;
        }
        default : { //May be  have to insert error handling here. For now just use as below
            (* h_factor) = 1; 
            (* v_factor) = 1;
            break;
        }
    }
}

//set MFC_JPEG_SCAN_OBJECT
static void
gen8_mfc_jpeg_scan_object(VADriverContextP ctx,
                                           struct encode_state *encode_state,
                                           struct intel_encoder_context *encoder_context)
{
    uint32_t mcu_count, surface_format, Mx, My;
    uint8_t i, horizontal_sampling_factor, vertical_sampling_factor, huff_ac_table=0, huff_dc_table=0;
    uint8_t is_last_scan = 1;    //Jpeg has only 1 scan per frame. When last scan, HW inserts EOI code.
    uint8_t head_present_flag=1; //Header has tables and app data 
    uint16_t num_components, restart_interval;   //Specifies number of MCUs in an ECS.
    VAEncSliceParameterBufferJPEG *slice_param;
    VAEncPictureParameterBufferJPEG *pic_param;
    
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct object_surface *obj_surface = encode_state->input_yuv_object;
    
    assert(encode_state->slice_params_ext[0] && encode_state->slice_params_ext[0]->buffer);
    assert(encode_state->pic_param_ext && encode_state->pic_param_ext->buffer);
    assert(obj_surface);
    pic_param = (VAEncPictureParameterBufferJPEG *)encode_state->pic_param_ext->buffer;
    slice_param = (VAEncSliceParameterBufferJPEG *)encode_state->slice_params_ext[0]->buffer;
    surface_format = obj_surface->fourcc;
    
    get_Y_sampling_factors(surface_format, &horizontal_sampling_factor, &vertical_sampling_factor);
    
    // Mx = #MCUs in a row, My = #MCUs in a column
    Mx = (pic_param->picture_width + (horizontal_sampling_factor*8 -1))/(horizontal_sampling_factor*8);
    My = (pic_param->picture_height + (vertical_sampling_factor*8 -1))/(vertical_sampling_factor*8);
    mcu_count = (Mx * My);
 
    num_components = pic_param->num_components;    
    restart_interval = slice_param->restart_interval;
    
    //Depending on number of components and values set for table selectors, 
    //only those bits are set in 24:22 for AC table, 20:18 for DC table
    for(i=0; i<num_components; i++) {
        huff_ac_table |= ((slice_param->components[i].ac_table_selector)<<i);
        huff_dc_table |= ((slice_param->components[i].dc_table_selector)<<i);
    }
    
    
    BEGIN_BCS_BATCH(batch, 3);
    /* DWORD 0 */
    OUT_BCS_BATCH(batch, MFC_JPEG_SCAN_OBJECT | (3 - 2)); 
    /* DWORD 1 */
    OUT_BCS_BATCH(batch, mcu_count << 0);       //MCU Count
    /* DWORD 2 */
    OUT_BCS_BATCH(batch,
                  (huff_ac_table << 22)     |   //Huffman AC Table
                  (huff_dc_table << 18)     |   //Huffman DC Table
                  (head_present_flag << 17) |   //Head present flag
                  (is_last_scan << 16)      |   //Is last scan
                  (restart_interval << 0));     //Restart Interval
    ADVANCE_BCS_BATCH(batch);
}

static void
gen8_mfc_jpeg_pak_insert_object(struct intel_encoder_context *encoder_context, unsigned int *insert_data, 
                                int length_in_dws, int data_bits_in_last_dw, int is_last_header, 
                                int is_end_of_slice)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    assert(batch);
    
    if (data_bits_in_last_dw == 0)
        data_bits_in_last_dw = 32;

    BEGIN_BCS_BATCH(batch, length_in_dws + 2);

    OUT_BCS_BATCH(batch, MFX_INSERT_OBJECT | (length_in_dws + 2 - 2));
    //DWord 1
    OUT_BCS_BATCH(batch,
                  (0 << 16) |                    //DataByteOffset 0 for JPEG Encoder
                  (0 << 15) |                    //HeaderLengthExcludeFrmSize 0 for JPEG Encoder
                  (data_bits_in_last_dw << 8) |  //DataBitsInLastDW
                  (0 << 4) |                     //SkipEmulByteCount 0 for JPEG Encoder
                  (0 << 3) |                     //EmulationFlag 0 for JPEG Encoder
                  ((!!is_last_header) << 2) |    //LastHeaderFlag
                  ((!!is_end_of_slice) << 1) |   //EndOfSliceFlag
                  (1 << 0));                     //BitstreamStartReset 1 for JPEG Encoder
    //Data Paylaod
    intel_batchbuffer_data(batch, insert_data, length_in_dws*4);

    ADVANCE_BCS_BATCH(batch);
}


//send the jpeg headers to HW using MFX_PAK_INSERT_OBJECT
static void
gen8_mfc_jpeg_add_headers(VADriverContextP ctx,
                                           struct encode_state *encode_state,
                                           struct intel_encoder_context *encoder_context)
{
    if (encode_state->packed_header_data_ext) {
        VAEncPackedHeaderParameterBuffer *param = NULL;
        unsigned int *header_data = (unsigned int *)(*encode_state->packed_header_data_ext)->buffer;
        unsigned int length_in_bits;

        param = (VAEncPackedHeaderParameterBuffer *)(*encode_state->packed_header_params_ext)->buffer;
        length_in_bits = param->bit_length;

        gen8_mfc_jpeg_pak_insert_object(encoder_context, 
                                        header_data, 
                                        ALIGN(length_in_bits, 32) >> 5,
                                        length_in_bits & 0x1f,
                                        1,
                                        1);
    }
}

//Initialize the buffered_qmatrix with the default qmatrix in the driver.
//If the app sends the qmatrix, this will be replaced with the one app sends.
static void 
jpeg_init_default_qmatrix(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
{
    int i=0;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
 
    //Load the the QM in zigzag order. If app sends QM, it is always in zigzag order.
    for(i=0; i<64; i++)
       mfc_context->buffered_qmatrix.lum_quantiser_matrix[i] = jpeg_luma_quant[zigzag_direct[i]];

    for(i=0; i<64; i++)
        mfc_context->buffered_qmatrix.chroma_quantiser_matrix[i] = jpeg_chroma_quant[zigzag_direct[i]];
}    
 
/* This is at the picture level */
static void
gen8_mfc_jpeg_pipeline_picture_programing(VADriverContextP ctx,
                                           struct encode_state *encode_state,
                                           struct intel_encoder_context *encoder_context)
{
    int i, j, component, max_selector = 0;
    VAEncSliceParameterBufferJPEG *slice_param;
    
    gen8_mfc_pipe_mode_select(ctx, MFX_FORMAT_JPEG, encoder_context);
    gen8_mfc_jpeg_set_surface_state(ctx, encoder_context, encode_state);
    gen8_mfc_pipe_buf_addr_state(ctx, encoder_context);
    gen8_mfc_ind_obj_base_addr_state(ctx, encoder_context);
    gen8_mfc_bsp_buf_base_addr_state(ctx, encoder_context);
    gen8_mfc_jpeg_pic_state(ctx, encoder_context, encode_state);
    
    //do the slice level encoding here
    gen8_mfc_jpeg_fqm_state(ctx, encoder_context, encode_state);

    //I dont think I need this for loop. Just to be consistent with other encoding logic...
    for(i = 0; i < encode_state->num_slice_params_ext; i++) {
        assert(encode_state->slice_params && encode_state->slice_params_ext[i]->buffer);
        slice_param = (VAEncSliceParameterBufferJPEG *)encode_state->slice_params_ext[i]->buffer;
        
        for(j = 0; j < encode_state->slice_params_ext[i]->num_elements; j++) {
            
            for(component = 0; component < slice_param->num_components; component++) {
                if(max_selector < slice_param->components[component].dc_table_selector)
                    max_selector = slice_param->components[component].dc_table_selector;
                
                if (max_selector < slice_param->components[component].ac_table_selector)
                    max_selector = slice_param->components[component].ac_table_selector;
            }
            
            slice_param++;
        }
    }    

    assert(max_selector < 2);
    //send the huffman table using MFC_JPEG_HUFF_TABLE
    gen8_mfc_jpeg_huff_table_state(ctx, encode_state, encoder_context, max_selector+1);
    //set MFC_JPEG_SCAN_OBJECT
    gen8_mfc_jpeg_scan_object(ctx, encode_state, encoder_context);
    //add headers using MFX_PAK_INSERT_OBJECT (it is refered as MFX_INSERT_OBJECT in this driver code)
    gen8_mfc_jpeg_add_headers(ctx, encode_state, encoder_context);
       
}

static void
gen8_mfc_jpeg_pipeline_programing(VADriverContextP ctx,
                                   struct encode_state *encode_state,
                                   struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    
    // begin programing
    intel_batchbuffer_start_atomic_bcs(batch, 0x4000); 
    intel_batchbuffer_emit_mi_flush(batch);
    
    // picture level programing
    gen8_mfc_jpeg_pipeline_picture_programing(ctx, encode_state, encoder_context);

    // end programing
    intel_batchbuffer_end_atomic(batch);

}


static VAStatus
gen8_mfc_jpeg_encode_picture(VADriverContextP ctx, 
                              struct encode_state *encode_state,
                              struct intel_encoder_context *encoder_context)
{
    gen8_mfc_init(ctx, encode_state, encoder_context);
    intel_mfc_jpeg_prepare(ctx, encode_state, encoder_context);
    /*Programing bcs pipeline*/
    gen8_mfc_jpeg_pipeline_programing(ctx, encode_state, encoder_context);
    gen8_mfc_run(ctx, encode_state, encoder_context);

    return VA_STATUS_SUCCESS;
}

static int gen8_mfc_vp8_qindex_estimate(struct encode_state *encode_state,
                                        struct gen6_mfc_context *mfc_context,
                                        int target_frame_size,
                                        int is_key_frame)
{
    VAEncSequenceParameterBufferVP8 *seq_param = (VAEncSequenceParameterBufferVP8 *)encode_state->seq_param_ext->buffer;
    VAEncPictureParameterBufferVP8 *pic_param = (VAEncPictureParameterBufferVP8 *)encode_state->pic_param_ext->buffer;
    unsigned int max_qindex = pic_param->clamp_qindex_high;
    unsigned int min_qindex = pic_param->clamp_qindex_low;
    int width_in_mbs = ALIGN(seq_param->frame_width, 16) / 16;
    int height_in_mbs = ALIGN(seq_param->frame_height, 16) / 16;
    int target_mb_size;
    int last_size_gap  = -1;
    int per_mb_size_at_qindex;
    int target_qindex = min_qindex, i;

    /* make sure would not overflow*/
    if (target_frame_size >= (0x7fffffff >> 9))
        target_mb_size = (target_frame_size / width_in_mbs / height_in_mbs) << 9;
    else
        target_mb_size = (target_frame_size << 9) / width_in_mbs / height_in_mbs;

    for (i = min_qindex; i <= max_qindex; i++) {
        per_mb_size_at_qindex = vp8_bits_per_mb[!is_key_frame][i];
        target_qindex = i;
        if (per_mb_size_at_qindex <= target_mb_size) {
            if (target_mb_size - per_mb_size_at_qindex < last_size_gap)
                target_qindex--;
            break;
        }
        else
            last_size_gap = per_mb_size_at_qindex - target_mb_size;
    }

    return target_qindex;
}

static void
gen8_mfc_vp8_bit_rate_control_context_init(struct encode_state *encode_state,
                                        struct gen6_mfc_context *mfc_context)
{
    VAEncSequenceParameterBufferVP8 *seq_param = (VAEncSequenceParameterBufferVP8 *)encode_state->seq_param_ext->buffer;
    VAEncMiscParameterBuffer *misc_param_frame_rate_buffer = (VAEncMiscParameterBuffer*)encode_state->misc_param[VAEncMiscParameterTypeFrameRate]->buffer;
    VAEncMiscParameterFrameRate* param_frame_rate = (VAEncMiscParameterFrameRate*)misc_param_frame_rate_buffer->data;
    int width_in_mbs = ALIGN(seq_param->frame_width, 16) / 16;
    int height_in_mbs = ALIGN(seq_param->frame_height, 16) / 16;
    float fps = param_frame_rate->framerate;
    int inter_mb_size = seq_param->bits_per_second * 1.0 / (fps+4.0) / width_in_mbs / height_in_mbs;
    int intra_mb_size = inter_mb_size * 5.0;

    mfc_context->bit_rate_control_context[SLICE_TYPE_I].target_mb_size = intra_mb_size;
    mfc_context->bit_rate_control_context[SLICE_TYPE_I].target_frame_size = intra_mb_size * width_in_mbs * height_in_mbs;
    mfc_context->bit_rate_control_context[SLICE_TYPE_P].target_mb_size = inter_mb_size;
    mfc_context->bit_rate_control_context[SLICE_TYPE_P].target_frame_size = inter_mb_size * width_in_mbs * height_in_mbs;

    mfc_context->bit_rate_control_context[SLICE_TYPE_I].TargetSizeInWord = (intra_mb_size + 16)/ 16;
    mfc_context->bit_rate_control_context[SLICE_TYPE_P].TargetSizeInWord = (inter_mb_size + 16)/ 16;

    mfc_context->bit_rate_control_context[SLICE_TYPE_I].MaxSizeInWord = mfc_context->bit_rate_control_context[SLICE_TYPE_I].TargetSizeInWord * 1.5;
    mfc_context->bit_rate_control_context[SLICE_TYPE_P].MaxSizeInWord = mfc_context->bit_rate_control_context[SLICE_TYPE_P].TargetSizeInWord * 1.5;
}

static void gen8_mfc_vp8_brc_init(struct encode_state *encode_state,
                               struct intel_encoder_context* encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    VAEncSequenceParameterBufferVP8 *seq_param = (VAEncSequenceParameterBufferVP8 *)encode_state->seq_param_ext->buffer;
    VAEncMiscParameterBuffer* misc_param_hrd = (VAEncMiscParameterBuffer*)encode_state->misc_param[VAEncMiscParameterTypeHRD]->buffer;
    VAEncMiscParameterHRD* param_hrd = (VAEncMiscParameterHRD*)misc_param_hrd->data;
    VAEncMiscParameterBuffer* misc_param_frame_rate_buffer = (VAEncMiscParameterBuffer*)encode_state->misc_param[VAEncMiscParameterTypeFrameRate]->buffer;
    VAEncMiscParameterFrameRate* param_frame_rate = (VAEncMiscParameterFrameRate*)misc_param_frame_rate_buffer->data;
    double bitrate = seq_param->bits_per_second;
    unsigned int frame_rate = param_frame_rate->framerate;
    int inum = 1, pnum = 0;
    int intra_period = seq_param->intra_period;
    int width_in_mbs = ALIGN(seq_param->frame_width, 16) / 16;
    int height_in_mbs = ALIGN(seq_param->frame_height, 16) / 16;
    int max_frame_size =  (vp8_bits_per_mb[0][0] >> 9) * width_in_mbs * height_in_mbs;/* vp8_bits_per_mb table mutilpled 512 */

    pnum = intra_period  - 1;

    mfc_context->brc.mode = encoder_context->rate_control_mode;

    mfc_context->brc.target_frame_size[SLICE_TYPE_I] = (int)((double)((bitrate * intra_period)/frame_rate) /
                                                             (double)(inum + BRC_PWEIGHT * pnum ));
    mfc_context->brc.target_frame_size[SLICE_TYPE_P] = BRC_PWEIGHT * mfc_context->brc.target_frame_size[SLICE_TYPE_I];

    mfc_context->brc.gop_nums[SLICE_TYPE_I] = inum;
    mfc_context->brc.gop_nums[SLICE_TYPE_P] = pnum;

    mfc_context->brc.bits_per_frame = bitrate/frame_rate;

    mfc_context->bit_rate_control_context[SLICE_TYPE_I].QpPrimeY = gen8_mfc_vp8_qindex_estimate(encode_state,
                                                                   mfc_context,
                                                                   mfc_context->brc.target_frame_size[SLICE_TYPE_I],
                                                                   1);
    mfc_context->bit_rate_control_context[SLICE_TYPE_P].QpPrimeY = gen8_mfc_vp8_qindex_estimate(encode_state,
                                                                   mfc_context,
                                                                   mfc_context->brc.target_frame_size[SLICE_TYPE_P],
                                                                   0);

    mfc_context->hrd.buffer_size = (double)param_hrd->buffer_size;
    mfc_context->hrd.current_buffer_fullness =
        (double)(param_hrd->initial_buffer_fullness < mfc_context->hrd.buffer_size)?
        param_hrd->initial_buffer_fullness: mfc_context->hrd.buffer_size/2.;
    mfc_context->hrd.target_buffer_fullness = (double)mfc_context->hrd.buffer_size/2.;
    mfc_context->hrd.buffer_capacity = (double)mfc_context->hrd.buffer_size/max_frame_size;
    mfc_context->hrd.violation_noted = 0;
}

static int gen8_mfc_vp8_brc_postpack(struct encode_state *encode_state,
                           struct gen6_mfc_context *mfc_context,
                           int frame_bits)
{
    gen6_brc_status sts = BRC_NO_HRD_VIOLATION;
    VAEncPictureParameterBufferVP8 *pic_param = (VAEncPictureParameterBufferVP8 *)encode_state->pic_param_ext->buffer;
    int is_key_frame = !pic_param->pic_flags.bits.frame_type;
    int slicetype = (is_key_frame ? SLICE_TYPE_I : SLICE_TYPE_P);
    int qpi = mfc_context->bit_rate_control_context[SLICE_TYPE_I].QpPrimeY;
    int qpp = mfc_context->bit_rate_control_context[SLICE_TYPE_P].QpPrimeY;
    int qp; // quantizer of previously encoded slice of current type
    int qpn; // predicted quantizer for next frame of current type in integer format
    double qpf; // predicted quantizer for next frame of current type in float format
    double delta_qp; // QP correction
    int target_frame_size, frame_size_next;
    /* Notes:
     *  x - how far we are from HRD buffer borders
     *  y - how far we are from target HRD buffer fullness
     */
    double x, y;
    double frame_size_alpha;
    unsigned int max_qindex = pic_param->clamp_qindex_high;
    unsigned int min_qindex = pic_param->clamp_qindex_low;

    qp = mfc_context->bit_rate_control_context[slicetype].QpPrimeY;

    target_frame_size = mfc_context->brc.target_frame_size[slicetype];
    if (mfc_context->hrd.buffer_capacity < 5)
        frame_size_alpha = 0;
    else
        frame_size_alpha = (double)mfc_context->brc.gop_nums[slicetype];
    if (frame_size_alpha > 30) frame_size_alpha = 30;
    frame_size_next = target_frame_size + (double)(target_frame_size - frame_bits) /
        (double)(frame_size_alpha + 1.);

    /* frame_size_next: avoiding negative number and too small value */
    if ((double)frame_size_next < (double)(target_frame_size * 0.25))
        frame_size_next = (int)((double)target_frame_size * 0.25);

    qpf = (double)qp * target_frame_size / frame_size_next;
    qpn = (int)(qpf + 0.5);

    if (qpn == qp) {
        /* setting qpn we round qpf making mistakes: now we are trying to compensate this */
        mfc_context->brc.qpf_rounding_accumulator += qpf - qpn;
        if (mfc_context->brc.qpf_rounding_accumulator > 1.0) {
            qpn++;
            mfc_context->brc.qpf_rounding_accumulator = 0.;
        } else if (mfc_context->brc.qpf_rounding_accumulator < -1.0) {
            qpn--;
            mfc_context->brc.qpf_rounding_accumulator = 0.;
        }
    }

    /* making sure that QP is not changing too fast */
    if ((qpn - qp) > BRC_QP_MAX_CHANGE) qpn = qp + BRC_QP_MAX_CHANGE;
    else if ((qpn - qp) < -BRC_QP_MAX_CHANGE) qpn = qp - BRC_QP_MAX_CHANGE;
    /* making sure that with QP predictions we did do not leave QPs range */
    BRC_CLIP(qpn, min_qindex, max_qindex);

    /* checking wthether HRD compliance is still met */
    sts = intel_mfc_update_hrd(encode_state, mfc_context, frame_bits);

    /* calculating QP delta as some function*/
    x = mfc_context->hrd.target_buffer_fullness - mfc_context->hrd.current_buffer_fullness;
    if (x > 0) {
        x /= mfc_context->hrd.target_buffer_fullness;
        y = mfc_context->hrd.current_buffer_fullness;
    }
    else {
        x /= (mfc_context->hrd.buffer_size - mfc_context->hrd.target_buffer_fullness);
        y = mfc_context->hrd.buffer_size - mfc_context->hrd.current_buffer_fullness;
    }
    if (y < 0.01) y = 0.01;
    if (x > 1) x = 1;
    else if (x < -1) x = -1;

    delta_qp = BRC_QP_MAX_CHANGE*exp(-1/y)*sin(BRC_PI_0_5 * x);
    qpn = (int)(qpn + delta_qp + 0.5);

    /* making sure that with QP predictions we did do not leave QPs range */
    BRC_CLIP(qpn, min_qindex, max_qindex);

    if (sts == BRC_NO_HRD_VIOLATION) { // no HRD violation
        /* correcting QPs of slices of other types */
        if (!is_key_frame) {
            if (abs(qpn - BRC_I_P_QP_DIFF - qpi) > 4)
                mfc_context->bit_rate_control_context[SLICE_TYPE_I].QpPrimeY += (qpn - BRC_I_P_QP_DIFF - qpi) >> 2;
        } else {
            if (abs(qpn + BRC_I_P_QP_DIFF - qpp) > 4)
                mfc_context->bit_rate_control_context[SLICE_TYPE_P].QpPrimeY += (qpn + BRC_I_P_QP_DIFF - qpp) >> 2;
        }
        BRC_CLIP(mfc_context->bit_rate_control_context[SLICE_TYPE_I].QpPrimeY, min_qindex, max_qindex);
        BRC_CLIP(mfc_context->bit_rate_control_context[SLICE_TYPE_P].QpPrimeY, min_qindex, max_qindex);
    } else if (sts == BRC_UNDERFLOW) { // underflow
        if (qpn <= qp) qpn = qp + 2;
        if (qpn > max_qindex) {
            qpn = max_qindex;
            sts = BRC_UNDERFLOW_WITH_MAX_QP; //underflow with maxQP
        }
    } else if (sts == BRC_OVERFLOW) {
        if (qpn >= qp) qpn = qp - 2;
        if (qpn < min_qindex) { // < 0 (?) overflow with minQP
            qpn = min_qindex;
            sts = BRC_OVERFLOW_WITH_MIN_QP; // bit stuffing to be done
        }
    }

    mfc_context->bit_rate_control_context[slicetype].QpPrimeY = qpn;

    return sts;
}

static void gen8_mfc_vp8_hrd_context_init(struct encode_state *encode_state,
                                       struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    VAEncSequenceParameterBufferVP8 *seq_param = (VAEncSequenceParameterBufferVP8 *)encode_state->seq_param_ext->buffer;
    unsigned int rate_control_mode = encoder_context->rate_control_mode;
    int target_bit_rate = seq_param->bits_per_second;

    // current we only support CBR mode.
    if (rate_control_mode == VA_RC_CBR) {
        mfc_context->vui_hrd.i_bit_rate_value = target_bit_rate >> 10;
        mfc_context->vui_hrd.i_cpb_size_value = (target_bit_rate * 8) >> 10;
        mfc_context->vui_hrd.i_initial_cpb_removal_delay = mfc_context->vui_hrd.i_cpb_size_value * 0.5 * 1024 / target_bit_rate * 90000;
        mfc_context->vui_hrd.i_cpb_removal_delay = 2;
        mfc_context->vui_hrd.i_frame_number = 0;

        mfc_context->vui_hrd.i_initial_cpb_removal_delay_length = 24;
        mfc_context->vui_hrd.i_cpb_removal_delay_length = 24;
        mfc_context->vui_hrd.i_dpb_output_delay_length = 24;
    }

}

static void gen8_mfc_vp8_hrd_context_update(struct encode_state *encode_state,
                             struct gen6_mfc_context *mfc_context)
{
    mfc_context->vui_hrd.i_frame_number++;
}

/*
 * Check whether the parameters related with CBR are updated and decide whether
 * it needs to reinitialize the configuration related with CBR.
 * Currently it will check the following parameters:
 *      bits_per_second
 *      frame_rate
 *      gop_configuration(intra_period, ip_period, intra_idr_period)
 */
static bool gen8_mfc_vp8_brc_updated_check(struct encode_state *encode_state,
                           struct intel_encoder_context *encoder_context)
{
    unsigned int rate_control_mode = encoder_context->rate_control_mode;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    double cur_fps, cur_bitrate;
    VAEncSequenceParameterBufferVP8 *seq_param = (VAEncSequenceParameterBufferVP8 *)encode_state->seq_param_ext->buffer;
    VAEncMiscParameterBuffer *misc_param_frame_rate_buf = (VAEncMiscParameterBuffer*)encode_state->misc_param[VAEncMiscParameterTypeFrameRate]->buffer;
    VAEncMiscParameterFrameRate *param_frame_rate = (VAEncMiscParameterFrameRate*)misc_param_frame_rate_buf->data;
    unsigned int frame_rate = param_frame_rate->framerate;

    if (rate_control_mode != VA_RC_CBR) {
        return false;
    }

    cur_bitrate = seq_param->bits_per_second;
    cur_fps = frame_rate;

    if ((cur_bitrate == mfc_context->brc.saved_bps) &&
        (cur_fps == mfc_context->brc.saved_fps) &&
        (seq_param->intra_period == mfc_context->brc.saved_intra_period)) {
        /* the parameters related with CBR are not updaetd */
        return false;
    }

    mfc_context->brc.saved_intra_period = seq_param->intra_period;
    mfc_context->brc.saved_fps = cur_fps;
    mfc_context->brc.saved_bps = cur_bitrate;
    return true;
}

static void gen8_mfc_vp8_brc_prepare(struct encode_state *encode_state,
                           struct intel_encoder_context *encoder_context)
{
    unsigned int rate_control_mode = encoder_context->rate_control_mode;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;

    if (rate_control_mode == VA_RC_CBR) {
        bool brc_updated;
        assert(encoder_context->codec != CODEC_MPEG2);

        brc_updated = gen8_mfc_vp8_brc_updated_check(encode_state, encoder_context);

        /*Programing bit rate control */
        if ((mfc_context->bit_rate_control_context[SLICE_TYPE_I].MaxSizeInWord == 0) ||
             brc_updated) {
            gen8_mfc_vp8_bit_rate_control_context_init(encode_state, mfc_context);
            gen8_mfc_vp8_brc_init(encode_state, encoder_context);
        }

        /*Programing HRD control */
        if ((mfc_context->vui_hrd.i_cpb_size_value == 0) || brc_updated )
            gen8_mfc_vp8_hrd_context_init(encode_state, encoder_context);
    }
}

static void vp8_enc_state_init(struct gen6_mfc_context *mfc_context,
                               VAEncPictureParameterBufferVP8 *pic_param,
                               VAQMatrixBufferVP8 *q_matrix)
{

    int is_key_frame = !pic_param->pic_flags.bits.frame_type;
    unsigned char *coeff_probs_stream_in_buffer;
    
    mfc_context->vp8_state.frame_header_lf_update_pos = 0;
    mfc_context->vp8_state.frame_header_qindex_update_pos = 0;
    mfc_context->vp8_state.frame_header_token_update_pos = 0;
    mfc_context->vp8_state.frame_header_bin_mv_upate_pos = 0;

    mfc_context->vp8_state.prob_skip_false = 255;
    memset(mfc_context->vp8_state.mb_segment_tree_probs, 0, sizeof(mfc_context->vp8_state.mb_segment_tree_probs));
    memcpy(mfc_context->vp8_state.mv_probs, vp8_default_mv_context, sizeof(mfc_context->vp8_state.mv_probs));
    
    if (is_key_frame) {
        memcpy(mfc_context->vp8_state.y_mode_probs, vp8_kf_ymode_prob, sizeof(mfc_context->vp8_state.y_mode_probs));
        memcpy(mfc_context->vp8_state.uv_mode_probs, vp8_kf_uv_mode_prob, sizeof(mfc_context->vp8_state.uv_mode_probs));

        mfc_context->vp8_state.prob_intra = 255;
        mfc_context->vp8_state.prob_last = 128;
        mfc_context->vp8_state.prob_gf = 128;
    } else {
        memcpy(mfc_context->vp8_state.y_mode_probs, vp8_ymode_prob, sizeof(mfc_context->vp8_state.y_mode_probs));
        memcpy(mfc_context->vp8_state.uv_mode_probs, vp8_uv_mode_prob, sizeof(mfc_context->vp8_state.uv_mode_probs));

        mfc_context->vp8_state.prob_intra = 63;
        mfc_context->vp8_state.prob_last = 128;
        mfc_context->vp8_state.prob_gf = 128;
    }
    
    mfc_context->vp8_state.prob_skip_false = vp8_base_skip_false_prob[q_matrix->quantization_index[0]];
  
    dri_bo_map(mfc_context->vp8_state.coeff_probs_stream_in_bo, 1);
    coeff_probs_stream_in_buffer = (unsigned char *)mfc_context->vp8_state.coeff_probs_stream_in_bo->virtual;
    assert(coeff_probs_stream_in_buffer);
    memcpy(coeff_probs_stream_in_buffer, vp8_default_coef_probs, sizeof(vp8_default_coef_probs));
    dri_bo_unmap(mfc_context->vp8_state.coeff_probs_stream_in_bo);
}

static void vp8_enc_state_update(struct gen6_mfc_context *mfc_context,
                                 VAQMatrixBufferVP8 *q_matrix)
{

    /*some other probabilities need to be updated*/
}

extern void binarize_vp8_frame_header(VAEncSequenceParameterBufferVP8 *seq_param,
                           VAEncPictureParameterBufferVP8 *pic_param,
                           VAQMatrixBufferVP8 *q_matrix,
                           struct gen6_mfc_context *mfc_context,
                           struct intel_encoder_context *encoder_context);

static void vp8_enc_frame_header_binarize(struct encode_state *encode_state,
                                          struct intel_encoder_context *encoder_context,
                                          struct gen6_mfc_context *mfc_context)
{
    VAEncSequenceParameterBufferVP8 *seq_param = (VAEncSequenceParameterBufferVP8 *)encode_state->seq_param_ext->buffer;
    VAEncPictureParameterBufferVP8 *pic_param = (VAEncPictureParameterBufferVP8 *)encode_state->pic_param_ext->buffer;
    VAQMatrixBufferVP8 *q_matrix = (VAQMatrixBufferVP8 *)encode_state->q_matrix->buffer;
    unsigned char *frame_header_buffer;

    binarize_vp8_frame_header(seq_param, pic_param, q_matrix, mfc_context, encoder_context);
 
    dri_bo_map(mfc_context->vp8_state.frame_header_bo, 1);
    frame_header_buffer = (unsigned char *)mfc_context->vp8_state.frame_header_bo->virtual;
    assert(frame_header_buffer);
    memcpy(frame_header_buffer, mfc_context->vp8_state.vp8_frame_header, (mfc_context->vp8_state.frame_header_bit_count + 7) / 8);
    dri_bo_unmap(mfc_context->vp8_state.frame_header_bo);
}

#define MAX_VP8_FRAME_HEADER_SIZE              0x2000
#define VP8_TOKEN_STATISTICS_BUFFER_SIZE       0x2000

static void gen8_mfc_vp8_init(VADriverContextP ctx,
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
    int is_key_frame, slice_type, rate_control_mode;

    VAEncSequenceParameterBufferVP8 *pSequenceParameter = (VAEncSequenceParameterBufferVP8 *)encode_state->seq_param_ext->buffer;
    VAEncPictureParameterBufferVP8 *pic_param = (VAEncPictureParameterBufferVP8 *)encode_state->pic_param_ext->buffer;
    VAQMatrixBufferVP8 *q_matrix = (VAQMatrixBufferVP8 *)encode_state->q_matrix->buffer;

    width_in_mbs = ALIGN(pSequenceParameter->frame_height, 16) / 16;
    height_in_mbs = ALIGN(pSequenceParameter->frame_height, 16) / 16;

    is_key_frame = !pic_param->pic_flags.bits.frame_type;
    slice_type = (is_key_frame ? SLICE_TYPE_I : SLICE_TYPE_P);
    rate_control_mode = encoder_context->rate_control_mode;

    if (rate_control_mode == VA_RC_CBR) {
        q_matrix->quantization_index[0] = mfc_context->bit_rate_control_context[slice_type].QpPrimeY;
        for (i = 1; i < 4; i++)
            q_matrix->quantization_index[i] = q_matrix->quantization_index[0];
        for (i = 0; i < 5; i++)
            q_matrix->quantization_index_delta[i] = 0;
    }

    slice_batchbuffer_size = 64 * width_in_mbs * height_in_mbs + 4096 +
        (SLICE_HEADER + SLICE_TAIL);

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
        if ( mfc_context->direct_mv_buffers[i].bo != NULL)
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
                      width_in_mbs * 64 * 16,
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
                      16 * width_in_mbs * 64,  /* 16 * width_in_mbs * 64 */
                      64);
    assert(bo);
    mfc_context->deblocking_filter_row_store_scratch_buffer.bo = bo;

    dri_bo_unreference(mfc_context->bsd_mpc_row_store_scratch_buffer.bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "Buffer",
                      16 * width_in_mbs * 64, /* 16 * width_in_mbs * 64 */
                      0x1000);
    assert(bo);
    mfc_context->bsd_mpc_row_store_scratch_buffer.bo = bo;

    dri_bo_unreference(mfc_context->mfc_batchbuffer_surface.bo);
    mfc_context->mfc_batchbuffer_surface.bo = NULL;

    dri_bo_unreference(mfc_context->aux_batchbuffer_surface.bo);
    mfc_context->aux_batchbuffer_surface.bo = NULL;

    if (mfc_context->aux_batchbuffer)
        intel_batchbuffer_free(mfc_context->aux_batchbuffer);

    mfc_context->aux_batchbuffer = intel_batchbuffer_new(&i965->intel, I915_EXEC_BSD, slice_batchbuffer_size);
    mfc_context->aux_batchbuffer_surface.bo = mfc_context->aux_batchbuffer->buffer;
    dri_bo_reference(mfc_context->aux_batchbuffer_surface.bo);
    mfc_context->aux_batchbuffer_surface.pitch = 16;
    mfc_context->aux_batchbuffer_surface.num_blocks = mfc_context->aux_batchbuffer->size / 16;
    mfc_context->aux_batchbuffer_surface.size_block = 16;

    i965_gpe_context_init(ctx, &mfc_context->gpe_context);

    /* alloc vp8 encoding buffers*/
    dri_bo_unreference(mfc_context->vp8_state.frame_header_bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "Buffer",
                      MAX_VP8_FRAME_HEADER_SIZE,
                      0x1000);
    assert(bo);
    mfc_context->vp8_state.frame_header_bo = bo;

    mfc_context->vp8_state.intermediate_buffer_max_size = width_in_mbs * height_in_mbs * 384 * 9;
    for(i = 0; i < 8; i++) {
        mfc_context->vp8_state.intermediate_partition_offset[i] = width_in_mbs * height_in_mbs * 384 * (i + 1);
    }
    dri_bo_unreference(mfc_context->vp8_state.intermediate_bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "Buffer",
                      mfc_context->vp8_state.intermediate_buffer_max_size,
                      0x1000);
    assert(bo);
    mfc_context->vp8_state.intermediate_bo = bo;

    dri_bo_unreference(mfc_context->vp8_state.stream_out_bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "Buffer",
                      width_in_mbs * height_in_mbs * 16,
                      0x1000);
    assert(bo);
    mfc_context->vp8_state.stream_out_bo = bo;

    dri_bo_unreference(mfc_context->vp8_state.coeff_probs_stream_in_bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "Buffer",
                      sizeof(vp8_default_coef_probs),
                      0x1000);
    assert(bo);
    mfc_context->vp8_state.coeff_probs_stream_in_bo = bo;

    dri_bo_unreference(mfc_context->vp8_state.token_statistics_bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "Buffer",
                      VP8_TOKEN_STATISTICS_BUFFER_SIZE,
                      0x1000);
    assert(bo);
    mfc_context->vp8_state.token_statistics_bo = bo;

    dri_bo_unreference(mfc_context->vp8_state.mpc_row_store_bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "Buffer",
                      width_in_mbs * 16 * 64,
                      0x1000);
    assert(bo);
    mfc_context->vp8_state.mpc_row_store_bo = bo;

    vp8_enc_state_init(mfc_context, pic_param, q_matrix);
    vp8_enc_frame_header_binarize(encode_state, encoder_context, mfc_context);
}

static VAStatus
intel_mfc_vp8_prepare(VADriverContextP ctx,
                        struct encode_state *encode_state,
                        struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct object_surface *obj_surface;
    struct object_buffer *obj_buffer;
    struct i965_coded_buffer_segment *coded_buffer_segment;
    VAEncPictureParameterBufferVP8 *pic_param = (VAEncPictureParameterBufferVP8 *)encode_state->pic_param_ext->buffer;
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    dri_bo *bo;
    int i;

    /* reconstructed surface */
    obj_surface = encode_state->reconstructed_object;
    i965_check_alloc_surface_bo(ctx, obj_surface, 1, VA_FOURCC('N','V','1','2'), SUBSAMPLE_YUV420);
    if (pic_param->loop_filter_level[0] == 0) {
        mfc_context->pre_deblocking_output.bo = obj_surface->bo;
        dri_bo_reference(mfc_context->pre_deblocking_output.bo);
    } else {
        mfc_context->post_deblocking_output.bo = obj_surface->bo;
        dri_bo_reference(mfc_context->post_deblocking_output.bo);
    }

    mfc_context->surface_state.width = obj_surface->orig_width;
    mfc_context->surface_state.height = obj_surface->orig_height;
    mfc_context->surface_state.w_pitch = obj_surface->width;
    mfc_context->surface_state.h_pitch = obj_surface->height;

    /* set vp8 reference frames */
    for (i = 0; i < ARRAY_ELEMS(mfc_context->reference_surfaces); i++) {
        obj_surface = encode_state->reference_objects[i];

        if (obj_surface && obj_surface->bo) {
            mfc_context->reference_surfaces[i].bo = obj_surface->bo;
            dri_bo_reference(mfc_context->reference_surfaces[i].bo);
        } else {
            mfc_context->reference_surfaces[i].bo = NULL;
        }
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

    dri_bo_unreference(mfc_context->vp8_state.final_frame_bo);
    mfc_context->vp8_state.final_frame_bo = mfc_context->mfc_indirect_pak_bse_object.bo;
    mfc_context->vp8_state.final_frame_byte_offset = I965_CODEDBUFFER_HEADER_SIZE;
    dri_bo_reference(mfc_context->vp8_state.final_frame_bo);

    /* set the internal flag to 0 to indicate the coded size is unknown */
    dri_bo_map(bo, 1);
    coded_buffer_segment = (struct i965_coded_buffer_segment *)bo->virtual;
    coded_buffer_segment->mapped = 0;
    coded_buffer_segment->codec = encoder_context->codec;
    dri_bo_unmap(bo);

    return vaStatus;
}

static void
gen8_mfc_vp8_encoder_cfg(VADriverContextP ctx, 
                         struct encode_state *encode_state,
                         struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    VAEncSequenceParameterBufferVP8 *seq_param = (VAEncSequenceParameterBufferVP8 *)encode_state->seq_param_ext->buffer;
    VAEncPictureParameterBufferVP8 *pic_param = (VAEncPictureParameterBufferVP8 *)encode_state->pic_param_ext->buffer;

    BEGIN_BCS_BATCH(batch, 30);
    OUT_BCS_BATCH(batch, MFX_VP8_ENCODER_CFG | (30 - 2)); /* SKL should be 31-2 ? */

    OUT_BCS_BATCH(batch,
                  0 << 9 | /* compressed bitstream output disable */
                  1 << 7 | /* disable per-segment delta qindex and loop filter in RC */
                  1 << 6 | /* RC initial pass */
                  0 << 4 | /* upate segment feature date flag */
                  1 << 3 | /* bitstream statistics output enable */
                  1 << 2 | /* token statistics output enable */
                  0 << 1 | /* final bitstream output disable */
                  0 << 0); /*DW1*/
    
    OUT_BCS_BATCH(batch, 0); /*DW2*/

    OUT_BCS_BATCH(batch, 
                  0xfff << 16 | /* max intra mb bit count limit */
                  0xfff << 0  /* max inter mb bit count limit */
                  ); /*DW3*/

    OUT_BCS_BATCH(batch, 0); /*DW4*/
    OUT_BCS_BATCH(batch, 0); /*DW5*/
    OUT_BCS_BATCH(batch, 0); /*DW6*/
    OUT_BCS_BATCH(batch, 0); /*DW7*/
    OUT_BCS_BATCH(batch, 0); /*DW8*/
    OUT_BCS_BATCH(batch, 0); /*DW9*/
    OUT_BCS_BATCH(batch, 0); /*DW10*/
    OUT_BCS_BATCH(batch, 0); /*DW11*/
    OUT_BCS_BATCH(batch, 0); /*DW12*/
    OUT_BCS_BATCH(batch, 0); /*DW13*/
    OUT_BCS_BATCH(batch, 0); /*DW14*/
    OUT_BCS_BATCH(batch, 0); /*DW15*/
    OUT_BCS_BATCH(batch, 0); /*DW16*/
    OUT_BCS_BATCH(batch, 0); /*DW17*/
    OUT_BCS_BATCH(batch, 0); /*DW18*/
    OUT_BCS_BATCH(batch, 0); /*DW19*/
    OUT_BCS_BATCH(batch, 0); /*DW20*/
    OUT_BCS_BATCH(batch, 0); /*DW21*/

    OUT_BCS_BATCH(batch, 
                 pic_param->pic_flags.bits.show_frame << 23 |
                 pic_param->pic_flags.bits.version << 20
                 ); /*DW22*/

    OUT_BCS_BATCH(batch,
                 (seq_param->frame_height_scale << 14 | seq_param->frame_height) << 16 |
                 (seq_param->frame_width_scale << 14 | seq_param->frame_width) << 0
                 );

    /*DW24*/
    OUT_BCS_BATCH(batch, mfc_context->vp8_state.frame_header_bit_count); /* frame header bit count */

    /*DW25*/
    OUT_BCS_BATCH(batch, mfc_context->vp8_state.frame_header_qindex_update_pos); /* frame header bin buffer qindex update pointer */

    /*DW26*/
    OUT_BCS_BATCH(batch, mfc_context->vp8_state.frame_header_lf_update_pos); /* frame header bin buffer loop filter update pointer*/

    /*DW27*/
    OUT_BCS_BATCH(batch, mfc_context->vp8_state.frame_header_token_update_pos); /* frame header bin buffer token update pointer */

    /*DW28*/
    OUT_BCS_BATCH(batch, mfc_context->vp8_state.frame_header_bin_mv_upate_pos); /*frame header bin buffer mv update pointer */

    /*DW29*/
    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);
}

static void
gen8_mfc_vp8_pic_state(VADriverContextP ctx,
                       struct encode_state *encode_state,
                       struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    VAEncSequenceParameterBufferVP8 *seq_param = (VAEncSequenceParameterBufferVP8 *)encode_state->seq_param_ext->buffer;
    VAEncPictureParameterBufferVP8 *pic_param = (VAEncPictureParameterBufferVP8 *)encode_state->pic_param_ext->buffer;
    VAQMatrixBufferVP8 *q_matrix = (VAQMatrixBufferVP8 *)encode_state->q_matrix->buffer;
    int i, j, log2num;

    log2num = pic_param->pic_flags.bits.num_token_partitions;

    /*update mode and token probs*/
    vp8_enc_state_update(mfc_context, q_matrix);

    BEGIN_BCS_BATCH(batch, 38);
    OUT_BCS_BATCH(batch, MFX_VP8_PIC_STATE | (38 - 2));
    OUT_BCS_BATCH(batch,
                  (ALIGN(seq_param->frame_height, 16) / 16 - 1) << 16 |
                  (ALIGN(seq_param->frame_width, 16) / 16 - 1) << 0);
 
    OUT_BCS_BATCH(batch,
                  log2num << 24 |
                  pic_param->sharpness_level << 16 |
                  pic_param->pic_flags.bits.sign_bias_alternate << 13 |
                  pic_param->pic_flags.bits.sign_bias_golden << 12 |
                  pic_param->pic_flags.bits.loop_filter_adj_enable << 11 |
                  pic_param->pic_flags.bits.mb_no_coeff_skip << 10 |
                  pic_param->pic_flags.bits.update_mb_segmentation_map << 9 |
                  pic_param->pic_flags.bits.segmentation_enabled << 8 |
                  !pic_param->pic_flags.bits.frame_type << 5 | /* 0 indicate an intra frame in VP8 stream/spec($9.1)*/
                  (pic_param->pic_flags.bits.version / 2) << 4 |
                  (pic_param->pic_flags.bits.version == 3) << 1 | /* full pixel mode for version 3 */
                  !!pic_param->pic_flags.bits.version << 0); /* version 0: 6 tap */
 
    OUT_BCS_BATCH(batch,
                  pic_param->loop_filter_level[3] << 24 |
                  pic_param->loop_filter_level[2] << 16 |
                  pic_param->loop_filter_level[1] <<  8 |
                  pic_param->loop_filter_level[0] <<  0);

    OUT_BCS_BATCH(batch,
                  q_matrix->quantization_index[3] << 24 |
                  q_matrix->quantization_index[2] << 16 |
                  q_matrix->quantization_index[1] <<  8 |
                  q_matrix->quantization_index[0] << 0);

    OUT_BCS_BATCH(batch,
                 ((unsigned short)(q_matrix->quantization_index_delta[4]) >> 15) << 28 | 
                 abs(q_matrix->quantization_index_delta[4]) << 24 |
                 ((unsigned short)(q_matrix->quantization_index_delta[3]) >> 15) << 20 | 
                 abs(q_matrix->quantization_index_delta[3]) << 16 |
                 ((unsigned short)(q_matrix->quantization_index_delta[2]) >> 15) << 12 | 
                 abs(q_matrix->quantization_index_delta[2]) << 8 |
                 ((unsigned short)(q_matrix->quantization_index_delta[1]) >> 15) << 4 | 
                 abs(q_matrix->quantization_index_delta[1]) << 0);

    OUT_BCS_BATCH(batch,
                 ((unsigned short)(q_matrix->quantization_index_delta[0]) >> 15) << 4 |
                 abs(q_matrix->quantization_index_delta[0]) << 0);
    
    OUT_BCS_BATCH(batch,
                 pic_param->clamp_qindex_high << 8 |
                 pic_param->clamp_qindex_low << 0);

    for (i = 8; i < 19; i++) {
         OUT_BCS_BATCH(batch, 0xffffffff);
    }

    OUT_BCS_BATCH(batch,
                  mfc_context->vp8_state.mb_segment_tree_probs[2] << 16 |
                  mfc_context->vp8_state.mb_segment_tree_probs[1] <<  8 |
                  mfc_context->vp8_state.mb_segment_tree_probs[0] <<  0);

    OUT_BCS_BATCH(batch,
                  mfc_context->vp8_state.prob_skip_false << 24 |
                  mfc_context->vp8_state.prob_intra      << 16 |
                  mfc_context->vp8_state.prob_last       <<  8 |
                  mfc_context->vp8_state.prob_gf         <<  0);

    OUT_BCS_BATCH(batch,
                  mfc_context->vp8_state.y_mode_probs[3] << 24 |
                  mfc_context->vp8_state.y_mode_probs[2] << 16 |
                  mfc_context->vp8_state.y_mode_probs[1] <<  8 |
                  mfc_context->vp8_state.y_mode_probs[0] <<  0);

    OUT_BCS_BATCH(batch,
                  mfc_context->vp8_state.uv_mode_probs[2] << 16 |
                  mfc_context->vp8_state.uv_mode_probs[1] <<  8 |
                  mfc_context->vp8_state.uv_mode_probs[0] <<  0);
    
    /* MV update value, DW23-DW32 */
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 20; j += 4) {
            OUT_BCS_BATCH(batch,
                          (j + 3 == 19 ? 0 : mfc_context->vp8_state.mv_probs[i][j + 3]) << 24 |
                          mfc_context->vp8_state.mv_probs[i][j + 2] << 16 |
                          mfc_context->vp8_state.mv_probs[i][j + 1] <<  8 |
                          mfc_context->vp8_state.mv_probs[i][j + 0] <<  0);
        }
    }

    OUT_BCS_BATCH(batch,
                  (pic_param->ref_lf_delta[3] & 0x7f) << 24 |
                  (pic_param->ref_lf_delta[2] & 0x7f) << 16 |
                  (pic_param->ref_lf_delta[1] & 0x7f) <<  8 |
                  (pic_param->ref_lf_delta[0] & 0x7f) <<  0);

    OUT_BCS_BATCH(batch,
                  (pic_param->mode_lf_delta[3] & 0x7f) << 24 |
                  (pic_param->mode_lf_delta[2] & 0x7f) << 16 |
                  (pic_param->mode_lf_delta[1] & 0x7f) <<  8 |
                  (pic_param->mode_lf_delta[0] & 0x7f) <<  0);

    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);

    ADVANCE_BCS_BATCH(batch);
}

#define OUT_VP8_BUFFER(bo, offset)                                      \
    if (bo)                                                             \
        OUT_BCS_RELOC(batch,                                            \
                      bo,                                               \
                      I915_GEM_DOMAIN_INSTRUCTION, I915_GEM_DOMAIN_INSTRUCTION, \
                      offset);                                           \
    else                                                                \
        OUT_BCS_BATCH(batch, 0);                                        \
    OUT_BCS_BATCH(batch, 0);                                            \
    OUT_BCS_BATCH(batch, 0);

static void 
gen8_mfc_vp8_bsp_buf_base_addr_state(VADriverContextP ctx, 
                                     struct encode_state *encode_state,
                                     struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;

    BEGIN_BCS_BATCH(batch, 32);
    OUT_BCS_BATCH(batch, MFX_VP8_BSP_BUF_BASE_ADDR_STATE | (32 - 2));

    OUT_VP8_BUFFER(mfc_context->vp8_state.frame_header_bo, 0);

    OUT_VP8_BUFFER(mfc_context->vp8_state.intermediate_bo, 0);
    OUT_BCS_BATCH(batch, mfc_context->vp8_state.intermediate_partition_offset[0]);
    OUT_BCS_BATCH(batch, mfc_context->vp8_state.intermediate_partition_offset[1]);
    OUT_BCS_BATCH(batch, mfc_context->vp8_state.intermediate_partition_offset[2]);
    OUT_BCS_BATCH(batch, mfc_context->vp8_state.intermediate_partition_offset[3]);
    OUT_BCS_BATCH(batch, mfc_context->vp8_state.intermediate_partition_offset[4]);
    OUT_BCS_BATCH(batch, mfc_context->vp8_state.intermediate_partition_offset[5]);
    OUT_BCS_BATCH(batch, mfc_context->vp8_state.intermediate_partition_offset[6]);
    OUT_BCS_BATCH(batch, mfc_context->vp8_state.intermediate_partition_offset[7]);
    OUT_BCS_BATCH(batch, mfc_context->vp8_state.intermediate_buffer_max_size);

    OUT_VP8_BUFFER(mfc_context->vp8_state.final_frame_bo, I965_CODEDBUFFER_HEADER_SIZE);
    OUT_BCS_BATCH(batch, 0);

    OUT_VP8_BUFFER(mfc_context->vp8_state.stream_out_bo, 0);
    OUT_VP8_BUFFER(mfc_context->vp8_state.coeff_probs_stream_in_bo, 0);
    OUT_VP8_BUFFER(mfc_context->vp8_state.token_statistics_bo, 0);
    OUT_VP8_BUFFER(mfc_context->vp8_state.mpc_row_store_bo, 0);

    ADVANCE_BCS_BATCH(batch);
}

static void
gen8_mfc_vp8_pipeline_picture_programing(VADriverContextP ctx,
                                           struct encode_state *encode_state,
                                           struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;

    mfc_context->pipe_mode_select(ctx, MFX_FORMAT_VP8, encoder_context);
    mfc_context->set_surface_state(ctx, encoder_context);
    mfc_context->ind_obj_base_addr_state(ctx, encoder_context);
    gen8_mfc_pipe_buf_addr_state(ctx, encoder_context);
    gen8_mfc_bsp_buf_base_addr_state(ctx, encoder_context);
    gen8_mfc_vp8_bsp_buf_base_addr_state(ctx, encode_state, encoder_context);
    gen8_mfc_vp8_pic_state(ctx, encode_state,encoder_context);
    gen8_mfc_vp8_encoder_cfg(ctx, encode_state, encoder_context);
}

static const unsigned char
vp8_intra_mb_mode_map[VME_MB_INTRA_MODE_COUNT] = {
    PAK_V_PRED,
    PAK_H_PRED,
    PAK_DC_PRED,
    PAK_TM_PRED
};

static const unsigned char
vp8_intra_block_mode_map[VME_B_INTRA_MODE_COUNT] = {
    PAK_B_VE_PRED,
    PAK_B_HE_PRED,
    PAK_B_DC_PRED,
    PAK_B_LD_PRED,
    PAK_B_RD_PRED,
    PAK_B_VR_PRED,
    PAK_B_HD_PRED,
    PAK_B_VL_PRED,
    PAK_B_HU_PRED
};

static int inline gen8_mfc_vp8_intra_mb_mode_map(unsigned int vme_pred_mode, int is_luma_4x4)
{
    unsigned int i, pak_pred_mode = 0;
    unsigned int vme_sub_blocks_pred_mode[8], pak_sub_blocks_pred_mode[8]; /* 8 blocks's intra mode */

    if (!is_luma_4x4) {
        pak_pred_mode = vp8_intra_mb_mode_map[vme_pred_mode & 0x3];
    } else {
        for (i = 0; i < 8; i++) { 
            vme_sub_blocks_pred_mode[i] = ((vme_pred_mode >> (4 * i)) & 0xf);
            assert(vme_sub_blocks_pred_mode[i] < VME_B_INTRA_MODE_COUNT);
            pak_sub_blocks_pred_mode[i] = vp8_intra_block_mode_map[vme_sub_blocks_pred_mode[i]];
            pak_pred_mode |= (pak_sub_blocks_pred_mode[i] << (4 * i));
        }
    }

    return pak_pred_mode;
}
static void
gen8_mfc_vp8_pak_object_intra(VADriverContextP ctx, 
                              struct intel_encoder_context *encoder_context,
                              unsigned int *msg,
                              int x, int y,
                              struct intel_batchbuffer *batch)
{
    unsigned int vme_intra_mb_mode, vme_chroma_pred_mode;
    unsigned int pak_intra_mb_mode, pak_chroma_pred_mode;
    unsigned int vme_luma_pred_mode[2], pak_luma_pred_mode[2];

    if (batch == NULL)
        batch = encoder_context->base.batch;

    vme_intra_mb_mode = ((msg[0] & 0x30) >> 4);
    assert((vme_intra_mb_mode == 0) || (vme_intra_mb_mode == 2)); //vp8 only support intra_16x16 and intra_4x4
    pak_intra_mb_mode = (vme_intra_mb_mode >> 1);

    vme_luma_pred_mode[0] = msg[1];
    vme_luma_pred_mode[1] = msg[2];
    vme_chroma_pred_mode = msg[3] & 0x3;

    pak_luma_pred_mode[0] = gen8_mfc_vp8_intra_mb_mode_map(vme_luma_pred_mode[0], pak_intra_mb_mode);
    pak_luma_pred_mode[1] = gen8_mfc_vp8_intra_mb_mode_map(vme_luma_pred_mode[1], pak_intra_mb_mode);
    pak_chroma_pred_mode = gen8_mfc_vp8_intra_mb_mode_map(vme_chroma_pred_mode, 0);

    BEGIN_BCS_BATCH(batch, 7);

    OUT_BCS_BATCH(batch, MFX_VP8_PAK_OBJECT | (7 - 2));
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch,
                  (0 << 20) |                    /* mv format: intra mb */
                  (0 << 18) |                    /* Segment ID */
                  (0 << 17) |                    /* disable coeff clamp */
                  (1 << 13) |                    /* intra mb flag */
                  (0 << 11) | 		         /* refer picture select: last frame */
                  (pak_intra_mb_mode << 8) |     /* mb type */
                  (pak_chroma_pred_mode << 4) |  /* mb uv mode */
                  (0 << 2) |                     /* skip mb flag: disable */
                  0);

    OUT_BCS_BATCH(batch, (y << 16) | x);
    OUT_BCS_BATCH(batch, pak_luma_pred_mode[0]);
    OUT_BCS_BATCH(batch, pak_luma_pred_mode[1]);

    ADVANCE_BCS_BATCH(batch);
}

static void
gen8_mfc_vp8_pak_object_inter(VADriverContextP ctx, 
                              struct intel_encoder_context *encoder_context,
                              unsigned int *msg,
                              int offset,
                              int x, int y,
                              struct intel_batchbuffer *batch)
{
    int i;

    if (batch == NULL)
        batch = encoder_context->base.batch;

    /* only support inter_16x16 now */
    assert((msg[AVC_INTER_MSG_OFFSET] & INTER_MODE_MASK) == INTER_16X16);
    /* for inter_16x16, all 16 MVs should be same, 
     * and move mv to the vme mb start address to make sure offset is 64 bytes aligned
     * as vp8 spec, all vp8 luma motion vectors are doulbled stored
     */
    msg[0] = (((msg[AVC_INTER_MV_OFFSET/4] & 0xffff0000) << 1) | ((msg[AVC_INTER_MV_OFFSET/4] << 1) & 0xffff));

    for (i = 1; i < 16; i++) {
        msg[i] = msg[0];
    }
    
    BEGIN_BCS_BATCH(batch, 7);

    OUT_BCS_BATCH(batch, MFX_VP8_PAK_OBJECT | (7 - 2));
    OUT_BCS_BATCH(batch,
                  (0 << 29) |           /* enable inline mv data: disable */
                  64);
    OUT_BCS_BATCH(batch,
                  offset);
    OUT_BCS_BATCH(batch,
                  (4 << 20) |           /* mv format: inter */
                  (0 << 18) |           /* Segment ID */
                  (0 << 17) |           /* coeff clamp: disable */
                  (0 << 13) |		/* intra mb flag: inter mb */
                  (0 << 11) | 		/* refer picture select: last frame */
                  (0 << 8) |            /* mb type: 16x16 */
                  (0 << 4) |		/* mb uv mode: dc_pred */
                  (0 << 2) |		/* skip mb flag: disable */
                  0);

    OUT_BCS_BATCH(batch, (y << 16) | x);

    /*new mv*/
    OUT_BCS_BATCH(batch, 0x8);
    OUT_BCS_BATCH(batch, 0x8);

    ADVANCE_BCS_BATCH(batch);
}

static void
gen8_mfc_vp8_pak_pipeline(VADriverContextP ctx,
                          struct encode_state *encode_state,
                          struct intel_encoder_context *encoder_context,
                          struct intel_batchbuffer *slice_batch)
{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    VAEncSequenceParameterBufferVP8 *seq_param = (VAEncSequenceParameterBufferVP8 *)encode_state->seq_param_ext->buffer;
    VAEncPictureParameterBufferVP8 *pic_param = (VAEncPictureParameterBufferVP8 *)encode_state->pic_param_ext->buffer;
    int width_in_mbs = ALIGN(seq_param->frame_width, 16) / 16;
    int height_in_mbs = ALIGN(seq_param->frame_height, 16) / 16;
    unsigned int *msg = NULL;
    unsigned char *msg_ptr = NULL;
    unsigned int i, offset, is_intra_frame;

    is_intra_frame = !pic_param->pic_flags.bits.frame_type;

    dri_bo_map(vme_context->vme_output.bo , 1);
    msg_ptr = (unsigned char *)vme_context->vme_output.bo->virtual;

    for( i = 0; i < width_in_mbs * height_in_mbs; i++) {
        int h_pos = i % width_in_mbs;
        int v_pos = i / width_in_mbs;
        msg = (unsigned int *) (msg_ptr + i * vme_context->vme_output.size_block);
        
        if (is_intra_frame) {
            gen8_mfc_vp8_pak_object_intra(ctx,
                    encoder_context,
                    msg,
                    h_pos, v_pos,
                    slice_batch);
        } else {
            int inter_rdo, intra_rdo;
            inter_rdo = msg[AVC_INTER_RDO_OFFSET] & AVC_RDO_MASK;
            intra_rdo = msg[AVC_INTRA_RDO_OFFSET] & AVC_RDO_MASK;

            if (intra_rdo < inter_rdo) {
                gen8_mfc_vp8_pak_object_intra(ctx,
                        encoder_context,
                        msg,
                        h_pos, v_pos,
                        slice_batch);
            } else {
                offset = i * vme_context->vme_output.size_block;
                gen8_mfc_vp8_pak_object_inter(ctx,
                        encoder_context,
                        msg,
                        offset,
                        h_pos, v_pos,
                        slice_batch);
            }
        }
    }

    dri_bo_unmap(vme_context->vme_output.bo);
}

/*
 * A batch buffer for vp8 pak object commands
 */
static dri_bo *
gen8_mfc_vp8_software_batchbuffer(VADriverContextP ctx,
                                          struct encode_state *encode_state,
                                          struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct intel_batchbuffer *batch;
    dri_bo *batch_bo;

    batch = mfc_context->aux_batchbuffer;
    batch_bo = batch->buffer;

    gen8_mfc_vp8_pak_pipeline(ctx, encode_state, encoder_context, batch);

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
gen8_mfc_vp8_pipeline_programing(VADriverContextP ctx,
                                   struct encode_state *encode_state,
                                   struct intel_encoder_context *encoder_context)
{
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    dri_bo *slice_batch_bo;

    slice_batch_bo = gen8_mfc_vp8_software_batchbuffer(ctx, encode_state, encoder_context);

    // begin programing
    intel_batchbuffer_start_atomic_bcs(batch, 0x4000);
    intel_batchbuffer_emit_mi_flush(batch);

    // picture level programing
    gen8_mfc_vp8_pipeline_picture_programing(ctx, encode_state, encoder_context);

    BEGIN_BCS_BATCH(batch, 4);
    OUT_BCS_BATCH(batch, MI_BATCH_BUFFER_START | (1 << 8) | (1 << 0));
    OUT_BCS_RELOC(batch,
                  slice_batch_bo,
                  I915_GEM_DOMAIN_COMMAND, 0,
                  0);
    OUT_BCS_BATCH(batch, 0);
    OUT_BCS_BATCH(batch, 0);
    ADVANCE_BCS_BATCH(batch);

    // end programing
    intel_batchbuffer_end_atomic(batch);

    dri_bo_unreference(slice_batch_bo);
}

static int gen8_mfc_calc_vp8_coded_buffer_size(VADriverContextP ctx,
                          struct encode_state *encode_state,
                          struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    VAEncPictureParameterBufferVP8 *pic_param = (VAEncPictureParameterBufferVP8 *)encode_state->pic_param_ext->buffer;
    unsigned char is_intra_frame = !pic_param->pic_flags.bits.frame_type;
    unsigned int *vp8_encoding_status, i, first_partition_bytes, token_partition_bytes, vp8_coded_bytes;
    
    int partition_num = 1 << pic_param->pic_flags.bits.num_token_partitions;

    first_partition_bytes = token_partition_bytes = vp8_coded_bytes = 0;

    dri_bo_map(mfc_context->vp8_state.token_statistics_bo, 0);

    vp8_encoding_status = (unsigned int *)mfc_context->vp8_state.token_statistics_bo->virtual;
    first_partition_bytes = (vp8_encoding_status[0] + 7) / 8;

    for (i = 1; i <= partition_num; i++) 
        token_partition_bytes += (vp8_encoding_status[i] + 7) / 8;

    /*coded_bytes includes P0~P8 partitions bytes + uncompresse date bytes + partion_size bytes in bitstream + 3 extra bytes */
    /*it seems the last partition size in vp8 status buffer is smaller than reality. so add 3 extra bytes */
    vp8_coded_bytes = first_partition_bytes + token_partition_bytes + (3 + 7 * !!is_intra_frame) + (partition_num - 1) * 3 + 3;

    dri_bo_unmap(mfc_context->vp8_state.token_statistics_bo);

    dri_bo_map(mfc_context->vp8_state.final_frame_bo, 0);
    struct i965_coded_buffer_segment *coded_buffer_segment = (struct i965_coded_buffer_segment *)(mfc_context->vp8_state.final_frame_bo->virtual);
    coded_buffer_segment->base.size = vp8_coded_bytes;
    dri_bo_unmap(mfc_context->vp8_state.final_frame_bo);

    return vp8_coded_bytes;
}

static VAStatus
gen8_mfc_vp8_encode_picture(VADriverContextP ctx,
                              struct encode_state *encode_state,
                              struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    unsigned int rate_control_mode = encoder_context->rate_control_mode;
    int current_frame_bits_size;
    int sts;

    gen8_mfc_vp8_init(ctx, encode_state, encoder_context);
    intel_mfc_vp8_prepare(ctx, encode_state, encoder_context);
    /*Programing bcs pipeline*/
    gen8_mfc_vp8_pipeline_programing(ctx, encode_state, encoder_context);
    gen8_mfc_run(ctx, encode_state, encoder_context);
    current_frame_bits_size = 8 * gen8_mfc_calc_vp8_coded_buffer_size(ctx, encode_state, encoder_context);

    if (rate_control_mode == VA_RC_CBR /*|| rate_control_mode == VA_RC_VBR*/) {
        sts = gen8_mfc_vp8_brc_postpack(encode_state, mfc_context, current_frame_bits_size);
        if (sts == BRC_NO_HRD_VIOLATION) {
            gen8_mfc_vp8_hrd_context_update(encode_state, mfc_context);
        }
        else if (sts == BRC_OVERFLOW_WITH_MIN_QP || sts == BRC_UNDERFLOW_WITH_MAX_QP) {
            if (!mfc_context->hrd.violation_noted) {
                fprintf(stderr, "Unrepairable %s!\n", (sts == BRC_OVERFLOW_WITH_MIN_QP)? "overflow": "underflow");
                mfc_context->hrd.violation_noted = 1;
            }
            return VA_STATUS_SUCCESS;
        }
    }

    return VA_STATUS_SUCCESS;
}

static void
gen8_mfc_context_destroy(void *context)
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

    dri_bo_unreference(mfc_context->vp8_state.coeff_probs_stream_in_bo);
    mfc_context->vp8_state.coeff_probs_stream_in_bo = NULL;

    dri_bo_unreference(mfc_context->vp8_state.final_frame_bo);
    mfc_context->vp8_state.final_frame_bo = NULL;

    dri_bo_unreference(mfc_context->vp8_state.frame_header_bo);
    mfc_context->vp8_state.frame_header_bo = NULL;

    dri_bo_unreference(mfc_context->vp8_state.intermediate_bo);
    mfc_context->vp8_state.intermediate_bo = NULL;

    dri_bo_unreference(mfc_context->vp8_state.mpc_row_store_bo);
    mfc_context->vp8_state.mpc_row_store_bo = NULL;

    dri_bo_unreference(mfc_context->vp8_state.stream_out_bo);
    mfc_context->vp8_state.stream_out_bo = NULL;

    dri_bo_unreference(mfc_context->vp8_state.token_statistics_bo);
    mfc_context->vp8_state.token_statistics_bo = NULL;

    free(mfc_context);
}

static VAStatus gen8_mfc_pipeline(VADriverContextP ctx,
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
        vaStatus = gen8_mfc_avc_encode_picture(ctx, encode_state, encoder_context);
        break;

        /* FIXME: add for other profile */
    case VAProfileMPEG2Simple:
    case VAProfileMPEG2Main:
        vaStatus = gen8_mfc_mpeg2_encode_picture(ctx, encode_state, encoder_context);
        break;

    case VAProfileJPEGBaseline:
        jpeg_init_default_qmatrix(ctx, encoder_context);
        vaStatus = gen8_mfc_jpeg_encode_picture(ctx, encode_state, encoder_context);
        break;
 
    case VAProfileVP8Version0_3:
        vaStatus = gen8_mfc_vp8_encode_picture(ctx, encode_state, encoder_context);
        break;
 
    default:
        vaStatus = VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
        break;
    }

    return vaStatus;
}

Bool gen8_mfc_context_init(VADriverContextP ctx, struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = calloc(1, sizeof(struct gen6_mfc_context));
    assert(mfc_context);
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
                          gen8_mfc_kernels,
                          NUM_MFC_KERNEL);

    mfc_context->pipe_mode_select = gen8_mfc_pipe_mode_select;
    mfc_context->set_surface_state = gen8_mfc_surface_state;
    mfc_context->ind_obj_base_addr_state = gen8_mfc_ind_obj_base_addr_state;
    mfc_context->avc_img_state = gen8_mfc_avc_img_state;
    mfc_context->avc_qm_state = gen8_mfc_avc_qm_state;
    mfc_context->avc_fqm_state = gen8_mfc_avc_fqm_state;
    mfc_context->insert_object = gen8_mfc_avc_insert_object;
    mfc_context->buffer_suface_setup = gen8_gpe_buffer_suface_setup;

    encoder_context->mfc_context = mfc_context;
    encoder_context->mfc_context_destroy = gen8_mfc_context_destroy;
    encoder_context->mfc_pipeline = gen8_mfc_pipeline;

    if (encoder_context->codec == CODEC_VP8)
        encoder_context->mfc_brc_prepare = gen8_mfc_vp8_brc_prepare;
    else
        encoder_context->mfc_brc_prepare = intel_mfc_brc_prepare;

    return True;
}
