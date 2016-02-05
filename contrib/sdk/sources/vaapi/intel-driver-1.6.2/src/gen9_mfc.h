/*
 * Copyright © 2014 Intel Corporation
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
 *    Qu Pengfei <Pengfei.Qu@intel.com>
 *
 */

#ifndef GEN9_MFC_H
#define GEN9_MFC_H

#include <drm.h>
#include <i915_drm.h>
#include <intel_bufmgr.h>

#include "i965_gpe_utils.h"

struct encode_state;

#define MAX_HCP_REFERENCE_SURFACES      8
#define NUM_HCP_CURRENT_COLLOCATED_MV_TEMPORAL_BUFFERS             9

#define INTRA_MB_FLAG_MASK              0x00002000

/* The space required for slice header SLICE_STATE + header.
 * Is it enough? */
#define SLICE_HEADER            80

/* the space required for slice tail. */
#define SLICE_TAIL          16

#define __SOFTWARE__    0

#define HCP_BATCHBUFFER_HEVC_INTRA       0
#define HCP_BATCHBUFFER_HEVC_INTER       1
#define NUM_HCP_KERNEL                   2

#define BIND_IDX_VME_OUTPUT             0
#define BIND_IDX_HCP_SLICE_HEADER       1
#define BIND_IDX_HCP_BATCHBUFFER        2

#define CMD_LEN_IN_OWORD        4

struct gen9_hcpe_context {
    struct {
        unsigned int width;
        unsigned int height;
        unsigned int w_pitch;
        unsigned int h_pitch;
    } surface_state;

    //HCP_PIPE_BUF_ADDR_STATE

    struct {
        dri_bo *bo;
    } deblocking_filter_line_buffer;            //OUTPUT: reconstructed picture with deblocked

    struct {
        dri_bo *bo;
    } deblocking_filter_tile_line_buffer;       //OUTPUT: reconstructed picture with deblocked

    struct {
        dri_bo *bo;
    } deblocking_filter_tile_column_buffer;     //OUTPUT: reconstructed picture with deblocked

    struct {
        dri_bo *bo;
    } uncompressed_picture_source;              //INPUT: original compressed image

    struct {
        dri_bo *bo;
    } metadata_line_buffer;                     //INTERNAL:metadata

    struct {
        dri_bo *bo;
    } metadata_tile_line_buffer;                //INTERNAL:metadata

    struct {
        dri_bo *bo;
    } metadata_tile_column_buffer;              //INTERNAL:metadata

    struct {
        dri_bo *bo;
    } sao_line_buffer;                     //INTERNAL:SAO not used in skylake

    struct {
        dri_bo *bo;
    } sao_tile_line_buffer;                //INTERNAL:SAO not used in skylake

    struct {
        dri_bo *bo;
    } sao_tile_column_buffer;              //INTERNAL:SAO not used in skylake

    struct {
        dri_bo *bo;
    } current_collocated_mv_temporal_buffer[NUM_HCP_CURRENT_COLLOCATED_MV_TEMPORAL_BUFFERS];       //

    struct {
        dri_bo *bo;
    } reference_surfaces[MAX_HCP_REFERENCE_SURFACES];   //INTERNAL: refrence surfaces

    //HCP_IND_OBJ_BASE_ADDR_STATE
    struct {
        dri_bo *bo;
    } hcp_indirect_cu_object;           //INPUT: the cu' mv info

    struct {
        dri_bo *bo;
        int offset;
        int end_offset;
    } hcp_indirect_pak_bse_object;      //OUTPUT: the compressed bitstream

    //Bit rate tracking context
    struct {
        unsigned int QpPrimeY;
        unsigned int MaxQpNegModifier;
        unsigned int MaxQpPosModifier;
        unsigned char MaxSizeInWord;
        unsigned char TargetSizeInWord;
        unsigned char Correct[6];
        unsigned char GrowInit;
        unsigned char GrowResistance;
        unsigned char ShrinkInit;
        unsigned char ShrinkResistance;

        unsigned int target_mb_size;
        unsigned int target_frame_size;
    } bit_rate_control_context[3];      //INTERNAL: for I, P, B frames

    struct {
        int mode;
        int gop_nums[3];
        int target_frame_size[3]; // I,P,B
        double bits_per_frame;
        double qpf_rounding_accumulator;

        double saved_bps;
        double saved_fps;
        int saved_intra_period;
        int saved_ip_period;
        int saved_idr_period;
    } brc;

    struct {
        double current_buffer_fullness;
        double target_buffer_fullness;
        double buffer_capacity;
        unsigned int buffer_size;
        unsigned int violation_noted;
    } hrd;

    //HRD control context
    struct {
        int i_bit_rate_value; // scale?
        int i_cpb_size_value; // scale?

        int i_initial_cpb_removal_delay;
        int i_cpb_removal_delay;

        int i_frame_number;

        int i_initial_cpb_removal_delay_length;
        int i_cpb_removal_delay_length;
        int i_dpb_output_delay_length;
    } vui_hrd;

    // picture width and height
    struct {
        uint16_t picture_width_in_samples;
        uint16_t picture_height_in_samples;
        uint16_t picture_width_in_ctbs;
        uint16_t picture_height_in_ctbs;
        uint16_t picture_width_in_min_cb_minus1;
        uint16_t picture_height_in_min_cb_minus1;
        uint16_t picture_width_in_mbs; /* to use on skylake */
        uint16_t picture_height_in_mbs;/* to sue on skylake */
        uint8_t ctb_size;
        uint8_t min_cb_size;
    } pic_size;

    VAQMatrixBufferHEVC  iq_matrix_hevc;

    struct i965_gpe_context gpe_context;
    struct i965_buffer_surface hcp_batchbuffer_surface;
    struct intel_batchbuffer *aux_batchbuffer;
    struct i965_buffer_surface aux_batchbuffer_surface;

    void (*pipe_mode_select)(VADriverContextP ctx,
                             int standard_select,
                             struct intel_encoder_context *encoder_context);
    void (*set_surface_state)(VADriverContextP ctx, struct encode_state *encode_state,
                              struct intel_encoder_context *encoder_context);
    void (*ind_obj_base_addr_state)(VADriverContextP ctx,
                                    struct intel_encoder_context *encoder_context);
    void (*fqm_state)(VADriverContextP ctx,
                      struct intel_encoder_context *encoder_context);
    void (*qm_state)(VADriverContextP ctx,
                     struct intel_encoder_context *encoder_context);
    void (*pic_state)(VADriverContextP ctx,
                      struct encode_state *encode_state,
                      struct intel_encoder_context *encoder_context);
    void (*insert_object)(VADriverContextP ctx,
                          struct intel_encoder_context *encoder_context,
                          unsigned int *insert_data,
                          int lenght_in_dws, int data_bits_in_last_dw,
                          int skip_emul_byte_count,
                          int is_last_header, int is_end_of_slice,
                          int emulation_flag,
                          struct intel_batchbuffer *batch);
    void (*buffer_suface_setup)(VADriverContextP ctx,
                                struct i965_gpe_context *gpe_context,
                                struct i965_buffer_surface *buffer_surface,
                                unsigned long binding_table_offset,
                                unsigned long surface_state_offset);
};

VAStatus gen9_hcpe_pipeline(VADriverContextP ctx,
                            VAProfile profile,
                            struct encode_state *encode_state,
                            struct intel_encoder_context *encoder_context);

/* HEVC BRC */
extern int intel_hcpe_update_hrd(struct encode_state *encode_state,
                                 struct gen9_hcpe_context *hcpe_context,
                                 int frame_bits);

extern int intel_hcpe_brc_postpack(struct encode_state *encode_state,
                                   struct gen9_hcpe_context *hcpe_context,
                                   int frame_bits);

extern void intel_hcpe_hrd_context_update(struct encode_state *encode_state,
        struct gen9_hcpe_context *hcpe_context);

extern int intel_hcpe_interlace_check(VADriverContextP ctx,
                                      struct encode_state *encode_state,
                                      struct intel_encoder_context *encoder_context);

extern void intel_hcpe_brc_prepare(struct encode_state *encode_state,
                                   struct intel_encoder_context *encoder_context);

/* HEVC HCP pipeline */
extern void intel_hcpe_hevc_pipeline_header_programing(VADriverContextP ctx,
        struct encode_state *encode_state,
        struct intel_encoder_context *encoder_context,
        struct intel_batchbuffer *slice_batch);

extern VAStatus intel_hcpe_hevc_prepare(VADriverContextP ctx,
                                        struct encode_state *encode_state,
                                        struct intel_encoder_context *encoder_context);

extern void
intel_hcpe_hevc_ref_idx_state(VADriverContextP ctx,
                              struct encode_state *encode_state,
                              struct intel_encoder_context *encoder_context);

extern void
intel_hevc_slice_insert_packed_data(VADriverContextP ctx,
                                    struct encode_state *encode_state,
                                    struct intel_encoder_context *encoder_context,
                                    int slice_index,
                                    struct intel_batchbuffer *slice_batch);

extern
Bool gen9_hcpe_context_init(VADriverContextP ctx, struct intel_encoder_context *encoder_context);

void gen9_hcpe_context_destroy(void *context);

#endif  /* GEN9_MFC_H */
