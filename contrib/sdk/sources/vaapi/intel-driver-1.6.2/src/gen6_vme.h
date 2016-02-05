/*
 * Copyright © 2009 Intel Corporation
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
 * SOFTWAR
 *
 * Authors:
 *    Zhou Chang <chang.zhou@intel.com>
 *
 */

#ifndef _GEN6_VME_H_
#define _GEN6_VME_H_

#include <xf86drm.h>
#include <drm.h>
#include <i915_drm.h>
#include <intel_bufmgr.h>

#include "i965_gpe_utils.h"

#define INTRA_VME_OUTPUT_IN_BYTES       16      /* in bytes */
#define INTRA_VME_OUTPUT_IN_DWS         (INTRA_VME_OUTPUT_IN_BYTES / 4)
#define INTER_VME_OUTPUT_IN_BYTES       160     /* the first 128 bytes for MVs and the last 32 bytes for other info */
#define INTER_VME_OUTPUT_IN_DWS         (INTER_VME_OUTPUT_IN_BYTES / 4)

#define MAX_INTERFACE_DESC_GEN6         MAX_GPE_KERNELS
#define MAX_MEDIA_SURFACES_GEN6         34

#define GEN6_VME_KERNEL_NUMBER          3

struct encode_state;
struct intel_encoder_context;

struct gen6_vme_context
{
    struct i965_gpe_context gpe_context;

    struct {
        dri_bo *bo;
    } vme_state;

    struct i965_buffer_surface vme_output;
    struct i965_buffer_surface vme_batchbuffer;


    void (*vme_surface2_setup)(VADriverContextP ctx,
                               struct i965_gpe_context *gpe_context,
                               struct object_surface *obj_surface,
                               unsigned long binding_table_offset,
                               unsigned long surface_state_offset);
    void (*vme_media_rw_surface_setup)(VADriverContextP ctx,
                                       struct i965_gpe_context *gpe_context,
                                       struct object_surface *obj_surface,
                                       unsigned long binding_table_offset,
                                       unsigned long surface_state_offset);
    void (*vme_buffer_suface_setup)(VADriverContextP ctx,
                                    struct i965_gpe_context *gpe_context,
                                    struct i965_buffer_surface *buffer_surface,
                                    unsigned long binding_table_offset,
                                    unsigned long surface_state_offset);
    void (*vme_media_chroma_surface_setup)(VADriverContextP ctx,
                                           struct i965_gpe_context *gpe_context,
                                           struct object_surface *obj_surface,
                                           unsigned long binding_table_offset,
                                           unsigned long surface_state_offset);
    void *vme_state_message;
    unsigned int h264_level;
    unsigned int hevc_level;
    unsigned int video_coding_type;
    unsigned int vme_kernel_sum;
    unsigned int mpeg2_level;

    struct object_surface *used_reference_objects[2];
    void *used_references[2];
    unsigned int ref_index_in_mb[2];
};

#define MPEG2_PIC_WIDTH_HEIGHT	30
#define	MPEG2_MV_RANGE		29
#define	MPEG2_LEVEL_MASK	0x0f
#define	MPEG2_LEVEL_LOW		0x0a
#define	MPEG2_LEVEL_MAIN	0x08
#define	MPEG2_LEVEL_HIGH	0x04


Bool gen75_vme_context_init(VADriverContextP ctx, struct intel_encoder_context *encoder_context);

extern void intel_vme_update_mbmv_cost(VADriverContextP ctx,
                                       struct encode_state *encode_state,
                                       struct intel_encoder_context *encoder_context);

void intel_vme_vp8_update_mbmv_cost(VADriverContextP ctx,
                                struct encode_state *encode_state,
                                struct intel_encoder_context *encoder_context);

Bool gen7_vme_context_init(VADriverContextP ctx, struct intel_encoder_context *encoder_context);

#define		MODE_INTRA_NONPRED	0
#define		MODE_INTRA_16X16	1
#define		MODE_INTRA_8X8		2
#define		MODE_INTRA_4X4		3
#define		MODE_INTER_16X8		4
#define		MODE_INTER_8X16		4
#define		MODE_INTER_8X8		5
#define		MODE_INTER_8X4		6
#define		MODE_INTER_4X8		6
#define		MODE_INTER_4X4		7
#define		MODE_INTER_16X16	8
#define		MODE_INTER_BWD		9
#define		MODE_REFID_COST		10
#define		MODE_CHROMA_INTRA	11

#define		MODE_INTER_MV0		12
#define		MODE_INTER_MV1		13
#define		MODE_INTER_MV2		14

#define		MODE_INTER_MV3		15
#define		MODE_INTER_MV4		16
#define		MODE_INTER_MV5		17
#define		MODE_INTER_MV6		18
#define		MODE_INTER_MV7		19

#define		INTRA_PRED_AVAIL_FLAG_AE	0x60
#define		INTRA_PRED_AVAIL_FLAG_B		0x10
#define		INTRA_PRED_AVAIL_FLAG_C       	0x8
#define		INTRA_PRED_AVAIL_FLAG_D		0x4
#define		INTRA_PRED_AVAIL_FLAG_BCD_MASK	0x1C

extern void
gen7_vme_walker_fill_vme_batchbuffer(VADriverContextP ctx, 
                                     struct encode_state *encode_state,
                                     int mb_width, int mb_height,
                                     int kernel,
                                     int transform_8x8_mode_flag,
                                     struct intel_encoder_context *encoder_context);

extern void 
gen7_vme_scoreboard_init(VADriverContextP ctx, struct gen6_vme_context *vme_context);

extern void
intel_vme_mpeg2_state_setup(VADriverContextP ctx,
                            struct encode_state *encode_state,
                            struct intel_encoder_context *encoder_context);

extern void
gen7_vme_mpeg2_walker_fill_vme_batchbuffer(VADriverContextP ctx, 
                                           struct encode_state *encode_state,
                                           int mb_width, int mb_height,
                                           int kernel,
                                           struct intel_encoder_context *encoder_context);

void
intel_avc_vme_reference_state(VADriverContextP ctx,
                              struct encode_state *encode_state,
                              struct intel_encoder_context *encoder_context,
                              int list_index,
                              int surface_index,
                              void (* vme_source_surface_state)(
                                  VADriverContextP ctx,
                                  int index,
                                  struct object_surface *obj_surface,
                                  struct intel_encoder_context *encoder_context));

/* HEVC */
void
intel_hevc_vme_reference_state(VADriverContextP ctx,
                              struct encode_state *encode_state,
                              struct intel_encoder_context *encoder_context,
                              int list_index,
                              int surface_index,
                              void (* vme_source_surface_state)(
                                  VADriverContextP ctx,
                                  int index,
                                  struct object_surface *obj_surface,
                                  struct intel_encoder_context *encoder_context));

void intel_vme_hevc_update_mbmv_cost(VADriverContextP ctx,
                                struct encode_state *encode_state,
                                struct intel_encoder_context *encoder_context);


extern Bool gen8_vme_context_init(VADriverContextP ctx, struct intel_encoder_context *encoder_context);

extern Bool gen9_vme_context_init(VADriverContextP ctx, struct intel_encoder_context *encoder_context);
#endif /* _GEN6_VME_H_ */
