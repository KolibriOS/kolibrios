/*
 * Copyright © 2010 Intel Corporation
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
 *
 */

#ifndef _GEN6_MFC_H_
#define _GEN6_MFC_H_

#include <drm.h>
#include <i915_drm.h>
#include <intel_bufmgr.h>

#include "i965_gpe_utils.h"
#include "i965_encoder.h"

struct encode_state;

#define MAX_MFC_REFERENCE_SURFACES      16
#define NUM_MFC_DMV_BUFFERS             34

#define INTRA_MB_FLAG_MASK              0x00002000

/* The space required for slice header SLICE_STATE + header.
 * Is it enough? */
#define SLICE_HEADER			80

/* the space required for slice tail. */
#define SLICE_TAIL			16

#define __SOFTWARE__    0

#define MFC_BATCHBUFFER_AVC_INTRA       0
#define MFC_BATCHBUFFER_AVC_INTER       1
#define NUM_MFC_KERNEL                  2

#define BIND_IDX_VME_OUTPUT             0
#define BIND_IDX_MFC_SLICE_HEADER       1
#define BIND_IDX_MFC_BATCHBUFFER        2

#define CMD_LEN_IN_OWORD        4

#define BRC_CLIP(x, min, max)                                   \
    {                                                           \
        x = ((x > (max)) ? (max) : ((x < (min)) ? (min) : x));  \
    }

#define BRC_P_B_QP_DIFF 4
#define BRC_I_P_QP_DIFF 2
#define BRC_I_B_QP_DIFF (BRC_I_P_QP_DIFF + BRC_P_B_QP_DIFF)

#define BRC_PWEIGHT 0.6  /* weight if P slice with comparison to I slice */
#define BRC_BWEIGHT 0.25 /* weight if B slice with comparison to I slice */

#define BRC_QP_MAX_CHANGE 5 /* maximum qp modification */
#define BRC_CY 0.1 /* weight for */
#define BRC_CX_UNDERFLOW 5.
#define BRC_CX_OVERFLOW -4.

#define BRC_PI_0_5 1.5707963267948966192313216916398

typedef enum {
   VME_V_PRED = 0,
   VME_H_PRED = 1,
   VME_DC_PRED = 2,
   VME_PL_PRED = 3,

   VME_MB_INTRA_MODE_COUNT
} VME_MB_INTRA_PRED_MODE;

typedef enum {
   PAK_DC_PRED = 0,
   PAK_V_PRED = 1,
   PAK_H_PRED = 2,
   PAK_TM_PRED = 3,

   PAK_MB_INTRA_MODE_COUNT
} VP8_PAK_MB_INTRA_PRED_MODE;

typedef enum
{
    VME_B_V_PRED = 0,
    VME_B_H_PRED = 1,
    VME_B_DC_PRED = 2,
    VME_B_DL_PRED = 3,
    VME_B_DR_PRED = 4,
    VME_B_VR_PRED = 5,
    VME_B_HD_PRED = 6,
    VME_B_VL_PRED = 7,
    VME_B_HU_PRED = 8,

    VME_B_INTRA_MODE_COUNT
} VME_BLOCK_INTRA_PRED_MODE;

typedef enum
{
    PAK_B_DC_PRED = 0,
    PAK_B_TM_PRED = 1,
    PAK_B_VE_PRED = 2,
    PAK_B_HE_PRED = 3,
    PAK_B_LD_PRED = 4,
    PAK_B_RD_PRED = 5,
    PAK_B_VR_PRED = 6,
    PAK_B_VL_PRED = 7,
    PAK_B_HD_PRED = 8,
    PAK_B_HU_PRED = 9,

    PAK_B_INTRA_MODE_COUNT
} VP8_PAK_BLOCK_INTRA_PRED_MODE;

typedef struct
{
  int vme_intra_mb_mode;
  int vp8_pak_intra_mb_mode;
} vp8_intra_mb_mode_map_t;

typedef struct
{
  int vme_intra_block_mode;
  int vp8_pak_intra_block_mode;
} vp8_intra_block_mode_map_t;

typedef enum _gen6_brc_status
{
    BRC_NO_HRD_VIOLATION = 0,
    BRC_UNDERFLOW = 1,
    BRC_OVERFLOW = 2,
    BRC_UNDERFLOW_WITH_MAX_QP = 3,
    BRC_OVERFLOW_WITH_MIN_QP = 4,
} gen6_brc_status;

struct gen6_mfc_avc_surface_aux
{
    dri_bo *dmv_top;
    dri_bo *dmv_bottom;
};

struct gen6_mfc_context
{
    struct {
        unsigned int width;
        unsigned int height;
        unsigned int w_pitch;
        unsigned int h_pitch;
    } surface_state;

    //MFX_PIPE_BUF_ADDR_STATE
    struct {
        dri_bo *bo;                            	
    } post_deblocking_output;           //OUTPUT: reconstructed picture                           
    
    struct {  
        dri_bo *bo;							   	
    } pre_deblocking_output;            //OUTPUT: reconstructed picture with deblocked                           

    struct {
        dri_bo *bo;
    } uncompressed_picture_source;      //INPUT: original compressed image

    struct {
        dri_bo *bo;							  	
    } intra_row_store_scratch_buffer;   //INTERNAL:

    struct {
        dri_bo *bo;
    } macroblock_status_buffer;         //INTERNAL:

    struct {
        dri_bo *bo;								
    } deblocking_filter_row_store_scratch_buffer;       //INTERNAL:

    struct {                                    
        dri_bo *bo; 
    } reference_surfaces[MAX_MFC_REFERENCE_SURFACES];   //INTERNAL: refrence surfaces

    //MFX_IND_OBJ_BASE_ADDR_STATE
    struct{
        dri_bo *bo;
    } mfc_indirect_mv_object;           //INPUT: the blocks' mv info

    struct {
        dri_bo *bo;
        int offset;
        int end_offset;
    } mfc_indirect_pak_bse_object;      //OUTPUT: the compressed bitstream 

    //MFX_BSP_BUF_BASE_ADDR_STATE
    struct {
        dri_bo *bo;
    } bsd_mpc_row_store_scratch_buffer; //INTERNAL:
	
    //MFX_AVC_DIRECTMODE_STATE
    struct {
        dri_bo *bo;
    } direct_mv_buffers[NUM_MFC_DMV_BUFFERS];   //INTERNAL: 0-31 as input,32 and 33 as output

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
        int i_bit_rate_value;
        int i_cpb_size_value;

        int i_initial_cpb_removal_delay;
        int i_cpb_removal_delay;

        int i_frame_number;

        int i_initial_cpb_removal_delay_length;
        int i_cpb_removal_delay_length;
        int i_dpb_output_delay_length;
    }vui_hrd;
 
    struct {
        unsigned char *vp8_frame_header;
        unsigned int frame_header_bit_count;
        unsigned int frame_header_qindex_update_pos;
        unsigned int frame_header_lf_update_pos;
        unsigned int frame_header_token_update_pos;
        unsigned int frame_header_bin_mv_upate_pos;

        unsigned int intermediate_partition_offset[8];
        unsigned int intermediate_buffer_max_size;
        unsigned int final_frame_byte_offset;

        unsigned char mb_segment_tree_probs[3];
        unsigned char y_mode_probs[4];
        unsigned char uv_mode_probs[3];
        unsigned char mv_probs[2][19];

        unsigned char prob_skip_false;
        unsigned char prob_intra;
        unsigned char prob_last;
        unsigned char prob_gf;
        
        dri_bo *frame_header_bo;
        dri_bo *intermediate_bo;
        dri_bo *final_frame_bo;
        dri_bo *stream_out_bo;
        dri_bo *coeff_probs_stream_in_bo;
        dri_bo *token_statistics_bo;
        dri_bo *mpc_row_store_bo;
    }vp8_state;

    //"buffered_QMatrix" will be used to buffer the QMatrix if the app sends one.
    // Or else, we will load a default QMatrix from the driver for JPEG encode.
    VAQMatrixBufferJPEG buffered_qmatrix;
    struct i965_gpe_context gpe_context;
    struct i965_buffer_surface mfc_batchbuffer_surface;
    struct intel_batchbuffer *aux_batchbuffer;
    struct i965_buffer_surface aux_batchbuffer_surface;

    void (*pipe_mode_select)(VADriverContextP ctx,
                             int standard_select,
                             struct intel_encoder_context *encoder_context);
    void (*set_surface_state)(VADriverContextP ctx,
                              struct intel_encoder_context *encoder_context);
    void (*ind_obj_base_addr_state)(VADriverContextP ctx,
                                    struct intel_encoder_context *encoder_context);
    void (*avc_img_state)(VADriverContextP ctx,
                          struct encode_state *encode_state,
                          struct intel_encoder_context *encoder_context);
    void (*avc_qm_state)(VADriverContextP ctx,
                         struct intel_encoder_context *encoder_context);
    void (*avc_fqm_state)(VADriverContextP ctx,
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

VAStatus gen6_mfc_pipeline(VADriverContextP ctx,
                           VAProfile profile,
                           struct encode_state *encode_state,
                           struct intel_encoder_context *encoder_context);
void gen6_mfc_context_destroy(void *context);

extern
Bool gen75_mfc_context_init(VADriverContextP ctx, struct intel_encoder_context *encoder_context);


extern int intel_mfc_update_hrd(struct encode_state *encode_state,
                                struct gen6_mfc_context *mfc_context,
                                int frame_bits);

extern int intel_mfc_brc_postpack(struct encode_state *encode_state,
                                  struct gen6_mfc_context *mfc_context,
                                  int frame_bits);

extern void intel_mfc_hrd_context_update(struct encode_state *encode_state,
                                         struct gen6_mfc_context *mfc_context);

extern int intel_mfc_interlace_check(VADriverContextP ctx,
                                     struct encode_state *encode_state,
                                     struct intel_encoder_context *encoder_context);

extern void intel_mfc_brc_prepare(struct encode_state *encode_state,
                                  struct intel_encoder_context *encoder_context);

extern void intel_mfc_avc_pipeline_header_programing(VADriverContextP ctx,
                                                     struct encode_state *encode_state,
                                                     struct intel_encoder_context *encoder_context,
                                                     struct intel_batchbuffer *slice_batch);

extern VAStatus intel_mfc_avc_prepare(VADriverContextP ctx, 
                                      struct encode_state *encode_state,
                                      struct intel_encoder_context *encoder_context);

extern int intel_avc_enc_slice_type_fixup(int type);

extern void
intel_mfc_avc_ref_idx_state(VADriverContextP ctx,
                            struct encode_state *encode_state,
                            struct intel_encoder_context *encoder_context);

extern
Bool gen8_mfc_context_init(VADriverContextP ctx, struct intel_encoder_context *encoder_context);

extern void
intel_avc_slice_insert_packed_data(VADriverContextP ctx,
                             struct encode_state *encode_state,
                             struct intel_encoder_context *encoder_context,
                             int slice_index,
                             struct intel_batchbuffer *slice_batch);

extern
Bool gen9_mfc_context_init(VADriverContextP ctx, struct intel_encoder_context *encoder_context);

#endif	/* _GEN6_MFC_BCS_H_ */
