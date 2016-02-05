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
 */

#include <stdlib.h>
#include <assert.h>

#include <va/va.h>
#include <va/va_enc_h264.h>
#include <va/va_enc_mpeg2.h>
#include <va/va_enc_vp8.h>
#include <va/va_enc_hevc.h>
#include <math.h>
#include "gen6_mfc.h"
#include "i965_encoder_utils.h"

#define BITSTREAM_ALLOCATE_STEPPING     4096

#define NAL_REF_IDC_NONE        0
#define NAL_REF_IDC_LOW         1
#define NAL_REF_IDC_MEDIUM      2
#define NAL_REF_IDC_HIGH        3

#define NAL_NON_IDR             1
#define NAL_IDR                 5
#define NAL_SPS                 7
#define NAL_PPS                 8
#define NAL_SEI                 6

#define SLICE_TYPE_P            0
#define SLICE_TYPE_B            1
#define SLICE_TYPE_I            2

#define IS_I_SLICE(type) (SLICE_TYPE_I == (type) || SLICE_TYPE_I == (type - 5))
#define IS_P_SLICE(type) (SLICE_TYPE_P == (type) || SLICE_TYPE_P == (type - 5))
#define IS_B_SLICE(type) (SLICE_TYPE_B == (type) || SLICE_TYPE_B == (type - 5))

#define ENTROPY_MODE_CAVLC      0
#define ENTROPY_MODE_CABAC      1

#define PROFILE_IDC_BASELINE    66
#define PROFILE_IDC_MAIN        77
#define PROFILE_IDC_HIGH        100

/*HEVC*/
#define VPS_NUT		32
#define SPS_NUT		33
#define PPS_NUT		34
#define IDR_WRADL_NUT	19
#define IDR_NLP_NUT	20
#define SLICE_TRAIL_N_NUT	0
#define SLICE_TRAIL_R_NUT	1
#define PREFIX_SEI_NUT	39
#define SUFFIX_SEI_NUT	40

struct __avc_bitstream {
    unsigned int *buffer;
    int bit_offset;
    int max_size_in_dword;
};

typedef struct __avc_bitstream avc_bitstream;

static unsigned int 
swap32(unsigned int val)
{
    unsigned char *pval = (unsigned char *)&val;

    return ((pval[0] << 24)     |
            (pval[1] << 16)     |
            (pval[2] << 8)      |
            (pval[3] << 0));
}

static void
avc_bitstream_start(avc_bitstream *bs)
{
    bs->max_size_in_dword = BITSTREAM_ALLOCATE_STEPPING;
    bs->buffer = calloc(bs->max_size_in_dword * sizeof(int), 1);
    bs->bit_offset = 0;
}

static void
avc_bitstream_end(avc_bitstream *bs)
{
    int pos = (bs->bit_offset >> 5);
    int bit_offset = (bs->bit_offset & 0x1f);
    int bit_left = 32 - bit_offset;

    if (bit_offset) {
        bs->buffer[pos] = swap32((bs->buffer[pos] << bit_left));
    }

    // free(bs->buffer);
}

static void
avc_bitstream_put_ui(avc_bitstream *bs, unsigned int val, int size_in_bits)
{
    int pos = (bs->bit_offset >> 5);
    int bit_offset = (bs->bit_offset & 0x1f);
    int bit_left = 32 - bit_offset;

    if (!size_in_bits)
        return;

    if (size_in_bits < 32)
        val &= (( 1 << size_in_bits) - 1);

    bs->bit_offset += size_in_bits;

    if (bit_left > size_in_bits) {
        bs->buffer[pos] = (bs->buffer[pos] << size_in_bits | val);
    } else {
        size_in_bits -= bit_left;
        bs->buffer[pos] = (bs->buffer[pos] << bit_left) | (val >> size_in_bits);
        bs->buffer[pos] = swap32(bs->buffer[pos]);

        if (pos + 1 == bs->max_size_in_dword) {
            bs->max_size_in_dword += BITSTREAM_ALLOCATE_STEPPING;
            bs->buffer = realloc(bs->buffer, bs->max_size_in_dword * sizeof(unsigned int));
            
            if (!bs->buffer)
                return;
        }

        bs->buffer[pos + 1] = val;
    }
}

static void
avc_bitstream_put_ue(avc_bitstream *bs, unsigned int val)
{
    int size_in_bits = 0;
    int tmp_val = ++val;

    while (tmp_val) {
        tmp_val >>= 1;
        size_in_bits++;
    }

    avc_bitstream_put_ui(bs, 0, size_in_bits - 1); // leading zero
    avc_bitstream_put_ui(bs, val, size_in_bits);
}

static void
avc_bitstream_put_se(avc_bitstream *bs, int val)
{
    unsigned int new_val;

    if (val <= 0)
        new_val = -2 * val;
    else
        new_val = 2 * val - 1;

    avc_bitstream_put_ue(bs, new_val);
}

static void
avc_bitstream_byte_aligning(avc_bitstream *bs, int bit)
{
    int bit_offset = (bs->bit_offset & 0x7);
    int bit_left = 8 - bit_offset;
    int new_val;

    if (!bit_offset)
        return;

    assert(bit == 0 || bit == 1);

    if (bit)
        new_val = (1 << bit_left) - 1;
    else
        new_val = 0;

    avc_bitstream_put_ui(bs, new_val, bit_left);
}
static void avc_rbsp_trailing_bits(avc_bitstream *bs)
{
    avc_bitstream_put_ui(bs, 1, 1);
    avc_bitstream_byte_aligning(bs, 0);
}
static void nal_start_code_prefix(avc_bitstream *bs)
{
    avc_bitstream_put_ui(bs, 0x00000001, 32);
}

static void nal_header(avc_bitstream *bs, int nal_ref_idc, int nal_unit_type)
{
    avc_bitstream_put_ui(bs, 0, 1);                /* forbidden_zero_bit: 0 */
    avc_bitstream_put_ui(bs, nal_ref_idc, 2);
    avc_bitstream_put_ui(bs, nal_unit_type, 5);
}

static void 
slice_header(avc_bitstream *bs,
             VAEncSequenceParameterBufferH264 *sps_param,
             VAEncPictureParameterBufferH264 *pic_param,
             VAEncSliceParameterBufferH264 *slice_param)
{
    int first_mb_in_slice = slice_param->macroblock_address;

    avc_bitstream_put_ue(bs, first_mb_in_slice);        /* first_mb_in_slice: 0 */
    avc_bitstream_put_ue(bs, slice_param->slice_type);  /* slice_type */
    avc_bitstream_put_ue(bs, slice_param->pic_parameter_set_id);        /* pic_parameter_set_id: 0 */
    avc_bitstream_put_ui(bs, pic_param->frame_num, sps_param->seq_fields.bits.log2_max_frame_num_minus4 + 4); /* frame_num */

    /* frame_mbs_only_flag == 1 */
    if (!sps_param->seq_fields.bits.frame_mbs_only_flag) {
        /* FIXME: */
        assert(0);
    }

    if (pic_param->pic_fields.bits.idr_pic_flag)
        avc_bitstream_put_ue(bs, slice_param->idr_pic_id);		/* idr_pic_id: 0 */

    if (sps_param->seq_fields.bits.pic_order_cnt_type == 0) {
        avc_bitstream_put_ui(bs, pic_param->CurrPic.TopFieldOrderCnt, sps_param->seq_fields.bits.log2_max_pic_order_cnt_lsb_minus4 + 4);
        /* pic_order_present_flag == 0 */
    } else {
        /* FIXME: */
        assert(0);
    }

    /* redundant_pic_cnt_present_flag == 0 */
    
    /* slice type */
    if (IS_P_SLICE(slice_param->slice_type)) {
        avc_bitstream_put_ui(bs, slice_param->num_ref_idx_active_override_flag, 1);            /* num_ref_idx_active_override_flag: */

        if (slice_param->num_ref_idx_active_override_flag)
            avc_bitstream_put_ue(bs, slice_param->num_ref_idx_l0_active_minus1);

        /* ref_pic_list_reordering */
        avc_bitstream_put_ui(bs, 0, 1);            /* ref_pic_list_reordering_flag_l0: 0 */
    } else if (IS_B_SLICE(slice_param->slice_type)) {
        avc_bitstream_put_ui(bs, slice_param->direct_spatial_mv_pred_flag, 1);            /* direct_spatial_mv_pred: 1 */

        avc_bitstream_put_ui(bs, slice_param->num_ref_idx_active_override_flag, 1);       /* num_ref_idx_active_override_flag: */

        if (slice_param->num_ref_idx_active_override_flag) {
            avc_bitstream_put_ue(bs, slice_param->num_ref_idx_l0_active_minus1);
            avc_bitstream_put_ue(bs, slice_param->num_ref_idx_l1_active_minus1);
        }

        /* ref_pic_list_reordering */
        avc_bitstream_put_ui(bs, 0, 1);            /* ref_pic_list_reordering_flag_l0: 0 */
        avc_bitstream_put_ui(bs, 0, 1);            /* ref_pic_list_reordering_flag_l1: 0 */
    } 

    if ((pic_param->pic_fields.bits.weighted_pred_flag && 
         IS_P_SLICE(slice_param->slice_type)) ||
        ((pic_param->pic_fields.bits.weighted_bipred_idc == 1) &&
         IS_B_SLICE(slice_param->slice_type))) {
        /* FIXME: fill weight/offset table */
        assert(0);
    }

    /* dec_ref_pic_marking */
    if (pic_param->pic_fields.bits.reference_pic_flag) {     /* nal_ref_idc != 0 */
        unsigned char no_output_of_prior_pics_flag = 0;
        unsigned char long_term_reference_flag = 0;
        unsigned char adaptive_ref_pic_marking_mode_flag = 0;

        if (pic_param->pic_fields.bits.idr_pic_flag) {
            avc_bitstream_put_ui(bs, no_output_of_prior_pics_flag, 1);            /* no_output_of_prior_pics_flag: 0 */
            avc_bitstream_put_ui(bs, long_term_reference_flag, 1);            /* long_term_reference_flag: 0 */
        } else {
            avc_bitstream_put_ui(bs, adaptive_ref_pic_marking_mode_flag, 1);            /* adaptive_ref_pic_marking_mode_flag: 0 */
        }
    }

    if (pic_param->pic_fields.bits.entropy_coding_mode_flag &&
        !IS_I_SLICE(slice_param->slice_type))
        avc_bitstream_put_ue(bs, slice_param->cabac_init_idc);               /* cabac_init_idc: 0 */

    avc_bitstream_put_se(bs, slice_param->slice_qp_delta);                   /* slice_qp_delta: 0 */

    /* ignore for SP/SI */

    if (pic_param->pic_fields.bits.deblocking_filter_control_present_flag) {
        avc_bitstream_put_ue(bs, slice_param->disable_deblocking_filter_idc);           /* disable_deblocking_filter_idc: 0 */

        if (slice_param->disable_deblocking_filter_idc != 1) {
            avc_bitstream_put_se(bs, slice_param->slice_alpha_c0_offset_div2);          /* slice_alpha_c0_offset_div2: 2 */
            avc_bitstream_put_se(bs, slice_param->slice_beta_offset_div2);              /* slice_beta_offset_div2: 2 */
        }
    }

    if (pic_param->pic_fields.bits.entropy_coding_mode_flag) {
        avc_bitstream_byte_aligning(bs, 1);
    }
}

int 
build_avc_slice_header(VAEncSequenceParameterBufferH264 *sps_param,
                       VAEncPictureParameterBufferH264 *pic_param,
                       VAEncSliceParameterBufferH264 *slice_param,
                       unsigned char **slice_header_buffer)
{
    avc_bitstream bs;
    int is_idr = !!pic_param->pic_fields.bits.idr_pic_flag;
    int is_ref = !!pic_param->pic_fields.bits.reference_pic_flag;

    avc_bitstream_start(&bs);
    nal_start_code_prefix(&bs);

    if (IS_I_SLICE(slice_param->slice_type)) {
        nal_header(&bs, NAL_REF_IDC_HIGH, is_idr ? NAL_IDR : NAL_NON_IDR);
    } else if (IS_P_SLICE(slice_param->slice_type)) {
        assert(!is_idr);
        nal_header(&bs, NAL_REF_IDC_MEDIUM, NAL_NON_IDR);
    } else {
        assert(IS_B_SLICE(slice_param->slice_type));
        assert(!is_idr);
        nal_header(&bs, is_ref ? NAL_REF_IDC_LOW : NAL_REF_IDC_NONE, NAL_NON_IDR);
    }

    slice_header(&bs, sps_param, pic_param, slice_param);

    avc_bitstream_end(&bs);
    *slice_header_buffer = (unsigned char *)bs.buffer;

    return bs.bit_offset;
}

int 
build_avc_sei_buffering_period(int cpb_removal_length,
                               unsigned int init_cpb_removal_delay, 
                               unsigned int init_cpb_removal_delay_offset,
                               unsigned char **sei_buffer) 
{
    unsigned char *byte_buf;
    int byte_size, i;

    avc_bitstream nal_bs;
    avc_bitstream sei_bs;

    avc_bitstream_start(&sei_bs);
    avc_bitstream_put_ue(&sei_bs, 0);       /*seq_parameter_set_id*/
    avc_bitstream_put_ui(&sei_bs, init_cpb_removal_delay, cpb_removal_length); 
    avc_bitstream_put_ui(&sei_bs, init_cpb_removal_delay_offset, cpb_removal_length); 
    if ( sei_bs.bit_offset & 0x7) {
        avc_bitstream_put_ui(&sei_bs, 1, 1);
    }
    avc_bitstream_end(&sei_bs);
    byte_size = (sei_bs.bit_offset + 7) / 8;
    
    avc_bitstream_start(&nal_bs);
    nal_start_code_prefix(&nal_bs);
    nal_header(&nal_bs, NAL_REF_IDC_NONE, NAL_SEI);
    
    avc_bitstream_put_ui(&nal_bs, 0, 8);
    avc_bitstream_put_ui(&nal_bs, byte_size, 8);
    
    byte_buf = (unsigned char *)sei_bs.buffer;
    for(i = 0; i < byte_size; i++) {
        avc_bitstream_put_ui(&nal_bs, byte_buf[i], 8);
    }
    free(byte_buf);

    avc_rbsp_trailing_bits(&nal_bs);
    avc_bitstream_end(&nal_bs);

    *sei_buffer = (unsigned char *)nal_bs.buffer; 
   
    return nal_bs.bit_offset;
}

int 
build_avc_sei_pic_timing(unsigned int cpb_removal_length, unsigned int cpb_removal_delay, 
                         unsigned int dpb_output_length, unsigned int dpb_output_delay,
                         unsigned char **sei_buffer)
{
    unsigned char *byte_buf;
    int byte_size, i;

    avc_bitstream nal_bs;
    avc_bitstream sei_bs;

    avc_bitstream_start(&sei_bs);
    avc_bitstream_put_ui(&sei_bs, cpb_removal_delay, cpb_removal_length); 
    avc_bitstream_put_ui(&sei_bs, dpb_output_delay, dpb_output_length); 
    if ( sei_bs.bit_offset & 0x7) {
        avc_bitstream_put_ui(&sei_bs, 1, 1);
    }
    avc_bitstream_end(&sei_bs);
    byte_size = (sei_bs.bit_offset + 7) / 8;
    
    avc_bitstream_start(&nal_bs);
    nal_start_code_prefix(&nal_bs);
    nal_header(&nal_bs, NAL_REF_IDC_NONE, NAL_SEI);
    
    avc_bitstream_put_ui(&nal_bs, 0x01, 8);
    avc_bitstream_put_ui(&nal_bs, byte_size, 8);
    
    byte_buf = (unsigned char *)sei_bs.buffer;
    for(i = 0; i < byte_size; i++) {
        avc_bitstream_put_ui(&nal_bs, byte_buf[i], 8);
    }
    free(byte_buf);

    avc_rbsp_trailing_bits(&nal_bs);
    avc_bitstream_end(&nal_bs);

    *sei_buffer = (unsigned char *)nal_bs.buffer; 
   
    return nal_bs.bit_offset;
}


int 
build_avc_sei_buffer_timing(unsigned int init_cpb_removal_length,
				unsigned int init_cpb_removal_delay,
				unsigned int init_cpb_removal_delay_offset,
				unsigned int cpb_removal_length,
				unsigned int cpb_removal_delay,
				unsigned int dpb_output_length,
				unsigned int dpb_output_delay,
				unsigned char **sei_buffer)
{
    unsigned char *byte_buf;
    int bp_byte_size, i, pic_byte_size;

    avc_bitstream nal_bs;
    avc_bitstream sei_bp_bs, sei_pic_bs;

    avc_bitstream_start(&sei_bp_bs);
    avc_bitstream_put_ue(&sei_bp_bs, 0);       /*seq_parameter_set_id*/
    avc_bitstream_put_ui(&sei_bp_bs, init_cpb_removal_delay, cpb_removal_length); 
    avc_bitstream_put_ui(&sei_bp_bs, init_cpb_removal_delay_offset, cpb_removal_length); 
    if ( sei_bp_bs.bit_offset & 0x7) {
        avc_bitstream_put_ui(&sei_bp_bs, 1, 1);
    }
    avc_bitstream_end(&sei_bp_bs);
    bp_byte_size = (sei_bp_bs.bit_offset + 7) / 8;
    
    avc_bitstream_start(&sei_pic_bs);
    avc_bitstream_put_ui(&sei_pic_bs, cpb_removal_delay, cpb_removal_length); 
    avc_bitstream_put_ui(&sei_pic_bs, dpb_output_delay, dpb_output_length); 
    if ( sei_pic_bs.bit_offset & 0x7) {
        avc_bitstream_put_ui(&sei_pic_bs, 1, 1);
    }
    avc_bitstream_end(&sei_pic_bs);
    pic_byte_size = (sei_pic_bs.bit_offset + 7) / 8;
    
    avc_bitstream_start(&nal_bs);
    nal_start_code_prefix(&nal_bs);
    nal_header(&nal_bs, NAL_REF_IDC_NONE, NAL_SEI);

	/* Write the SEI buffer period data */    
    avc_bitstream_put_ui(&nal_bs, 0, 8);
    avc_bitstream_put_ui(&nal_bs, bp_byte_size, 8);
    
    byte_buf = (unsigned char *)sei_bp_bs.buffer;
    for(i = 0; i < bp_byte_size; i++) {
        avc_bitstream_put_ui(&nal_bs, byte_buf[i], 8);
    }
    free(byte_buf);
	/* write the SEI timing data */
    avc_bitstream_put_ui(&nal_bs, 0x01, 8);
    avc_bitstream_put_ui(&nal_bs, pic_byte_size, 8);
    
    byte_buf = (unsigned char *)sei_pic_bs.buffer;
    for(i = 0; i < pic_byte_size; i++) {
        avc_bitstream_put_ui(&nal_bs, byte_buf[i], 8);
    }
    free(byte_buf);

    avc_rbsp_trailing_bits(&nal_bs);
    avc_bitstream_end(&nal_bs);

    *sei_buffer = (unsigned char *)nal_bs.buffer; 
   
    return nal_bs.bit_offset;
}

int 
build_mpeg2_slice_header(VAEncSequenceParameterBufferMPEG2 *sps_param,
                         VAEncPictureParameterBufferMPEG2 *pic_param,
                         VAEncSliceParameterBufferMPEG2 *slice_param,
                         unsigned char **slice_header_buffer)
{
    avc_bitstream bs;

    avc_bitstream_start(&bs);
    avc_bitstream_end(&bs);
    *slice_header_buffer = (unsigned char *)bs.buffer;

    return bs.bit_offset;
}

static void binarize_qindex_delta(avc_bitstream *bs, int qindex_delta)
{
    if (qindex_delta == 0)
        avc_bitstream_put_ui(bs, 0, 1);
    else {
       avc_bitstream_put_ui(bs, 1, 1);
       avc_bitstream_put_ui(bs, abs(qindex_delta), 4);

       if (qindex_delta < 0)
           avc_bitstream_put_ui(bs, 1, 1);
       else
           avc_bitstream_put_ui(bs, 0, 1);
    }
}

void binarize_vp8_frame_header(VAEncSequenceParameterBufferVP8 *seq_param,
                           VAEncPictureParameterBufferVP8 *pic_param,
                           VAQMatrixBufferVP8 *q_matrix,
                           struct gen6_mfc_context *mfc_context,
                           struct intel_encoder_context *encoder_context)
{
    avc_bitstream bs;
    int i, j;
    int is_intra_frame = !pic_param->pic_flags.bits.frame_type;
    int log2num = pic_param->pic_flags.bits.num_token_partitions;

    /* modify picture paramters */
    pic_param->pic_flags.bits.loop_filter_adj_enable = 1;
    pic_param->pic_flags.bits.mb_no_coeff_skip = 1;
    pic_param->pic_flags.bits.forced_lf_adjustment = 1;
    pic_param->pic_flags.bits.refresh_entropy_probs = 1;
    pic_param->pic_flags.bits.segmentation_enabled = 0;

    pic_param->pic_flags.bits.loop_filter_type = pic_param->pic_flags.bits.version / 2;
    if (pic_param->pic_flags.bits.version > 1)
        pic_param->loop_filter_level[0] = 0; 

    avc_bitstream_start(&bs);

    if (is_intra_frame) {
       avc_bitstream_put_ui(&bs, 0, 1);
       avc_bitstream_put_ui(&bs, pic_param->pic_flags.bits.clamping_type ,1);
    }

    avc_bitstream_put_ui(&bs, pic_param->pic_flags.bits.segmentation_enabled, 1);
    
    if (pic_param->pic_flags.bits.segmentation_enabled) {
        avc_bitstream_put_ui(&bs, pic_param->pic_flags.bits.update_mb_segmentation_map, 1);
        avc_bitstream_put_ui(&bs, pic_param->pic_flags.bits.update_segment_feature_data, 1);
        if (pic_param->pic_flags.bits.update_segment_feature_data) {
            /*add it later*/
            assert(0);
        }
        if (pic_param->pic_flags.bits.update_mb_segmentation_map) {
           for (i = 0; i < 3; i++) {
              if (mfc_context->vp8_state.mb_segment_tree_probs[i] == 255)
                  avc_bitstream_put_ui(&bs, 0, 1);
              else {
                  avc_bitstream_put_ui(&bs, 1, 1);
                  avc_bitstream_put_ui(&bs, mfc_context->vp8_state.mb_segment_tree_probs[i], 8);
              }
           }
        }
    }

    avc_bitstream_put_ui(&bs, pic_param->pic_flags.bits.loop_filter_type, 1);
    avc_bitstream_put_ui(&bs, pic_param->loop_filter_level[0], 6);
    avc_bitstream_put_ui(&bs, pic_param->sharpness_level, 3);
    
    mfc_context->vp8_state.frame_header_lf_update_pos = bs.bit_offset;
    
    if (pic_param->pic_flags.bits.forced_lf_adjustment) {
        avc_bitstream_put_ui(&bs, 1, 1);//mode_ref_lf_delta_enable = 1
        avc_bitstream_put_ui(&bs, 1, 1);//mode_ref_lf_delta_update = 1

        for (i =0; i < 4; i++) {
            avc_bitstream_put_ui(&bs, 1, 1);
            if (pic_param->ref_lf_delta[i] > 0) {
                avc_bitstream_put_ui(&bs, (abs(pic_param->ref_lf_delta[i]) & 0x3F), 6);
                avc_bitstream_put_ui(&bs, 0, 1);
            } else {
                avc_bitstream_put_ui(&bs, (abs(pic_param->ref_lf_delta[i]) & 0x3F), 6);
                avc_bitstream_put_ui(&bs, 1, 1);
            }
        }

        for (i =0; i < 4; i++) {
            avc_bitstream_put_ui(&bs, 1, 1);
            if (pic_param->mode_lf_delta[i] > 0) {
                avc_bitstream_put_ui(&bs, (abs(pic_param->mode_lf_delta[i]) & 0x3F), 6);
                avc_bitstream_put_ui(&bs, 0, 1);
            } else {
                avc_bitstream_put_ui(&bs, (abs(pic_param->mode_lf_delta[i]) & 0x3F), 6);
                avc_bitstream_put_ui(&bs, 1, 1);
            }
        }

    } else {
        avc_bitstream_put_ui(&bs, 0, 1);//mode_ref_lf_delta_enable = 0
    }

    avc_bitstream_put_ui(&bs, log2num, 2);
    
    mfc_context->vp8_state.frame_header_qindex_update_pos = bs.bit_offset;

    avc_bitstream_put_ui(&bs, q_matrix->quantization_index[0], 7);
   
    for (i = 0; i < 5; i++) 
        binarize_qindex_delta(&bs, q_matrix->quantization_index_delta[i]);

    if (!is_intra_frame) {
        avc_bitstream_put_ui(&bs, pic_param->pic_flags.bits.refresh_golden_frame, 1); 
        avc_bitstream_put_ui(&bs, pic_param->pic_flags.bits.refresh_alternate_frame, 1);

        if (!pic_param->pic_flags.bits.refresh_golden_frame)
            avc_bitstream_put_ui(&bs, pic_param->pic_flags.bits.copy_buffer_to_golden, 2);

        if (!pic_param->pic_flags.bits.refresh_alternate_frame)
            avc_bitstream_put_ui(&bs, pic_param->pic_flags.bits.copy_buffer_to_alternate, 2);
       
        avc_bitstream_put_ui(&bs, pic_param->pic_flags.bits.sign_bias_golden, 1);
        avc_bitstream_put_ui(&bs, pic_param->pic_flags.bits.sign_bias_alternate, 1);
    }
   
    avc_bitstream_put_ui(&bs, pic_param->pic_flags.bits.refresh_entropy_probs, 1);

    if (!is_intra_frame)
        avc_bitstream_put_ui(&bs, pic_param->pic_flags.bits.refresh_last, 1);

    mfc_context->vp8_state.frame_header_token_update_pos = bs.bit_offset;

    for (i = 0; i < 4 * 8 * 3 * 11; i++)
        avc_bitstream_put_ui(&bs, 0, 1); //don't update coeff_probs

    avc_bitstream_put_ui(&bs, pic_param->pic_flags.bits.mb_no_coeff_skip, 1);
    if (pic_param->pic_flags.bits.mb_no_coeff_skip)
        avc_bitstream_put_ui(&bs, mfc_context->vp8_state.prob_skip_false, 8);

    if (!is_intra_frame) {
        avc_bitstream_put_ui(&bs, mfc_context->vp8_state.prob_intra, 8);
        avc_bitstream_put_ui(&bs, mfc_context->vp8_state.prob_last, 8);
        avc_bitstream_put_ui(&bs, mfc_context->vp8_state.prob_gf, 8);
 
        avc_bitstream_put_ui(&bs, 1, 1); //y_mode_update_flag = 1
        for (i = 0; i < 4; i++) {
            avc_bitstream_put_ui(&bs, mfc_context->vp8_state.y_mode_probs[i], 8);
        } 

        avc_bitstream_put_ui(&bs, 1, 1); //uv_mode_update_flag = 1
        for (i = 0; i < 3; i++) {
            avc_bitstream_put_ui(&bs, mfc_context->vp8_state.uv_mode_probs[i], 8);
        } 

        mfc_context->vp8_state.frame_header_bin_mv_upate_pos = bs.bit_offset;
        
        for (i = 0; i < 2 ; i++) {
            for (j = 0; j < 19; j++) {
                avc_bitstream_put_ui(&bs, 0, 1);
                //avc_bitstream_put_ui(&bs, mfc_context->vp8_state.mv_probs[i][j], 7);
            }
        } 
    }

    avc_bitstream_end(&bs);

    mfc_context->vp8_state.vp8_frame_header = (unsigned char *)bs.buffer;
    mfc_context->vp8_state.frame_header_bit_count = bs.bit_offset;
}

/* HEVC to do for internal header generated*/

void nal_header_hevc(avc_bitstream *bs, int nal_unit_type, int temporalid)
{
    /* forbidden_zero_bit: 0 */
    avc_bitstream_put_ui(bs, 0, 1);
    /* nal unit_type */
    avc_bitstream_put_ui(bs, nal_unit_type, 6);
    /* layer_id. currently it is zero */
    avc_bitstream_put_ui(bs, 0, 6);
    /* teporalid + 1 .*/
    avc_bitstream_put_ui(bs, temporalid + 1, 3);
}

int build_hevc_sei_buffering_period(int init_cpb_removal_delay_length,
                                unsigned int init_cpb_removal_delay,
                                unsigned int init_cpb_removal_delay_offset,
                                unsigned char **sei_buffer)
{
    unsigned char *byte_buf;
    int bp_byte_size, i;
    //unsigned int cpb_removal_delay;

    avc_bitstream nal_bs;
    avc_bitstream sei_bp_bs;

    avc_bitstream_start(&sei_bp_bs);
    avc_bitstream_put_ue(&sei_bp_bs, 0);       /*seq_parameter_set_id*/
    /* SEI buffer period info */
    /* NALHrdBpPresentFlag == 1 */
    avc_bitstream_put_ui(&sei_bp_bs, init_cpb_removal_delay,init_cpb_removal_delay_length);
    avc_bitstream_put_ui(&sei_bp_bs, init_cpb_removal_delay_offset,init_cpb_removal_delay_length);
    if ( sei_bp_bs.bit_offset & 0x7) {
        avc_bitstream_put_ui(&sei_bp_bs, 1, 1);
    }
    avc_bitstream_end(&sei_bp_bs);
    bp_byte_size = (sei_bp_bs.bit_offset + 7) / 8;

    avc_bitstream_start(&nal_bs);
    nal_start_code_prefix(&nal_bs);
    nal_header_hevc(&nal_bs, PREFIX_SEI_NUT ,0);

    /* Write the SEI buffer period data */
    avc_bitstream_put_ui(&nal_bs, 0, 8);
    avc_bitstream_put_ui(&nal_bs, bp_byte_size, 8);

    byte_buf = (unsigned char *)sei_bp_bs.buffer;
    for(i = 0; i < bp_byte_size; i++) {
        avc_bitstream_put_ui(&nal_bs, byte_buf[i], 8);
    }
    free(byte_buf);

    avc_rbsp_trailing_bits(&nal_bs);
    avc_bitstream_end(&nal_bs);

    *sei_buffer = (unsigned char *)nal_bs.buffer;

    return nal_bs.bit_offset;
}

int build_hevc_idr_sei_buffer_timing(unsigned int init_cpb_removal_delay_length,
                                 unsigned int init_cpb_removal_delay,
                                 unsigned int init_cpb_removal_delay_offset,
                                 unsigned int cpb_removal_length,
                                 unsigned int cpb_removal_delay,
                                 unsigned int dpb_output_length,
                                 unsigned int dpb_output_delay,
                                 unsigned char **sei_buffer)
{
    unsigned char *byte_buf;
    int bp_byte_size, i, pic_byte_size;
    //unsigned int cpb_removal_delay;

    avc_bitstream nal_bs;
    avc_bitstream sei_bp_bs, sei_pic_bs;

    avc_bitstream_start(&sei_bp_bs);
    avc_bitstream_put_ue(&sei_bp_bs, 0);       /*seq_parameter_set_id*/
    /* SEI buffer period info */
    /* NALHrdBpPresentFlag == 1 */
    avc_bitstream_put_ui(&sei_bp_bs, init_cpb_removal_delay,init_cpb_removal_delay_length);
    avc_bitstream_put_ui(&sei_bp_bs, init_cpb_removal_delay_offset,init_cpb_removal_delay_length);
    if ( sei_bp_bs.bit_offset & 0x7) {
        avc_bitstream_put_ui(&sei_bp_bs, 1, 1);
    }
    avc_bitstream_end(&sei_bp_bs);
    bp_byte_size = (sei_bp_bs.bit_offset + 7) / 8;

    /* SEI pic timing info */
    avc_bitstream_start(&sei_pic_bs);
    /* The info of CPB and DPB delay is controlled by CpbDpbDelaysPresentFlag,
    * which is derived as 1 if one of the following conditions is true:
    * nal_hrd_parameters_present_flag is present in the avc_bitstream and is equal to 1,
    * vcl_hrd_parameters_present_flag is present in the avc_bitstream and is equal to 1,
    */
    //cpb_removal_delay = (hevc_context.current_cpb_removal - hevc_context.prev_idr_cpb_removal);
    avc_bitstream_put_ui(&sei_pic_bs, cpb_removal_delay, cpb_removal_length);
    avc_bitstream_put_ui(&sei_pic_bs, dpb_output_delay,dpb_output_length);
    if ( sei_pic_bs.bit_offset & 0x7) {
        avc_bitstream_put_ui(&sei_pic_bs, 1, 1);
    }
    /* The pic_structure_present_flag determines whether the pic_structure
    * info is written into the SEI pic timing info.
    * Currently it is set to zero.
    */
    avc_bitstream_end(&sei_pic_bs);
    pic_byte_size = (sei_pic_bs.bit_offset + 7) / 8;

    avc_bitstream_start(&nal_bs);
    nal_start_code_prefix(&nal_bs);
    nal_header_hevc(&nal_bs, PREFIX_SEI_NUT ,0);

    /* Write the SEI buffer period data */
    avc_bitstream_put_ui(&nal_bs, 0, 8);
    avc_bitstream_put_ui(&nal_bs, bp_byte_size, 8);

    byte_buf = (unsigned char *)sei_bp_bs.buffer;
    for(i = 0; i < bp_byte_size; i++) {
        avc_bitstream_put_ui(&nal_bs, byte_buf[i], 8);
    }
    free(byte_buf);
    /* write the SEI pic timing data */
    avc_bitstream_put_ui(&nal_bs, 0x01, 8);
    avc_bitstream_put_ui(&nal_bs, pic_byte_size, 8);

    byte_buf = (unsigned char *)sei_pic_bs.buffer;
    for(i = 0; i < pic_byte_size; i++) {
        avc_bitstream_put_ui(&nal_bs, byte_buf[i], 8);
    }
    free(byte_buf);

    avc_rbsp_trailing_bits(&nal_bs);
    avc_bitstream_end(&nal_bs);

    *sei_buffer = (unsigned char *)nal_bs.buffer;

    return nal_bs.bit_offset;
}

int build_hevc_sei_pic_timing(unsigned int cpb_removal_length, unsigned int cpb_removal_delay,
                         unsigned int dpb_output_length, unsigned int dpb_output_delay,
                         unsigned char **sei_buffer)
{
    unsigned char *byte_buf;
    int i, pic_byte_size;
    //unsigned int cpb_removal_delay;

    avc_bitstream nal_bs;
    avc_bitstream sei_pic_bs;

    avc_bitstream_start(&sei_pic_bs);
    /* The info of CPB and DPB delay is controlled by CpbDpbDelaysPresentFlag,
    * which is derived as 1 if one of the following conditions is true:
    * nal_hrd_parameters_present_flag is present in the avc_bitstream and is equal to 1,
    * vcl_hrd_parameters_present_flag is present in the avc_bitstream and is equal to 1,
    */
    //cpb_removal_delay = (hevc_context.current_cpb_removal - hevc_context.current_idr_cpb_removal);
    avc_bitstream_put_ui(&sei_pic_bs, cpb_removal_delay, cpb_removal_length);
    avc_bitstream_put_ui(&sei_pic_bs, dpb_output_delay,	 dpb_output_length);
    if ( sei_pic_bs.bit_offset & 0x7) {
        avc_bitstream_put_ui(&sei_pic_bs, 1, 1);
    }

    /* The pic_structure_present_flag determines whether the pic_structure
    * info is written into the SEI pic timing info.
    * Currently it is set to zero.
    */
    avc_bitstream_end(&sei_pic_bs);
    pic_byte_size = (sei_pic_bs.bit_offset + 7) / 8;

    avc_bitstream_start(&nal_bs);
    nal_start_code_prefix(&nal_bs);
    nal_header_hevc(&nal_bs, PREFIX_SEI_NUT ,0);

    /* write the SEI Pic timing data */
    avc_bitstream_put_ui(&nal_bs, 0x01, 8);
    avc_bitstream_put_ui(&nal_bs, pic_byte_size, 8);

    byte_buf = (unsigned char *)sei_pic_bs.buffer;
    for(i = 0; i < pic_byte_size; i++) {
        avc_bitstream_put_ui(&nal_bs, byte_buf[i], 8);
    }
    free(byte_buf);

    avc_rbsp_trailing_bits(&nal_bs);
    avc_bitstream_end(&nal_bs);

    *sei_buffer = (unsigned char *)nal_bs.buffer;

    return nal_bs.bit_offset;
}

typedef struct _RefPicSet
{
    unsigned char    num_negative_pics;
    unsigned char    num_positive_pics;
    unsigned char    delta_poc_s0_minus1[8];
    unsigned char    used_by_curr_pic_s0_flag[8];
    unsigned char    delta_poc_s1_minus1[8];
    unsigned char    used_by_curr_pic_s1_flag[8];
    unsigned int     inter_ref_pic_set_prediction_flag;
}hevcRefPicSet;

void hevc_short_term_ref_pic_set(avc_bitstream *bs,VAEncSliceParameterBufferHEVC *slice_param,int curPicOrderCnt)
{
    hevcRefPicSet hevc_rps;
    int rps_idx = 1, ref_idx = 0;
    int i = 0;

    hevc_rps.inter_ref_pic_set_prediction_flag = 0;
    /* s0: between I and P/B; s1 : between P and B */
    hevc_rps.num_negative_pics               = (slice_param->slice_type!=HEVC_SLICE_I) ? 1 : 0;
    hevc_rps.num_positive_pics               = (slice_param->slice_type==HEVC_SLICE_B) ? 1 : 0;
    hevc_rps.delta_poc_s0_minus1[0]          = 0;
    hevc_rps.used_by_curr_pic_s0_flag[0]     = 0;
    hevc_rps.delta_poc_s1_minus1[0]          = 0;
    hevc_rps.used_by_curr_pic_s1_flag[0]     = 0;
    if(slice_param->num_ref_idx_l0_active_minus1==0 )
    {
        hevc_rps.delta_poc_s0_minus1[0]          = (slice_param->slice_type==HEVC_SLICE_I) ? 0 : ( curPicOrderCnt - slice_param->ref_pic_list0[0].pic_order_cnt-1); //0;
        hevc_rps.used_by_curr_pic_s0_flag[0]     = 1;
    }
    if(slice_param->num_ref_idx_l1_active_minus1==0 )
    {
        hevc_rps.delta_poc_s1_minus1[0]          = (slice_param->slice_type==HEVC_SLICE_I) ? 0 : ( slice_param->ref_pic_list1[0].pic_order_cnt -curPicOrderCnt -1);
        hevc_rps.used_by_curr_pic_s1_flag[0]     = 1;
    }

    if (rps_idx)
        avc_bitstream_put_ui(bs, hevc_rps.inter_ref_pic_set_prediction_flag, 1);

    if (hevc_rps.inter_ref_pic_set_prediction_flag)
    {
        /* not support */
        /* to do */
    } else {
        avc_bitstream_put_ue(bs, hevc_rps.num_negative_pics);
        avc_bitstream_put_ue(bs, hevc_rps.num_positive_pics);

        for (i = 0; i < hevc_rps.num_negative_pics; i++)
        {
            avc_bitstream_put_ue(bs, hevc_rps.delta_poc_s0_minus1[ref_idx]);
            avc_bitstream_put_ui(bs, hevc_rps.used_by_curr_pic_s0_flag[ref_idx], 1);
        }
        for (i = 0; i < hevc_rps.num_positive_pics; i++)
        {
            avc_bitstream_put_ue(bs, hevc_rps.delta_poc_s1_minus1[ref_idx]);
            avc_bitstream_put_ui(bs, hevc_rps.used_by_curr_pic_s1_flag[ref_idx], 1);
        }
    }

    return;
}

static void slice_rbsp(avc_bitstream *bs,
                       int slice_index,
                       VAEncSequenceParameterBufferHEVC *seq_param,
                       VAEncPictureParameterBufferHEVC *pic_param,
                       VAEncSliceParameterBufferHEVC *slice_param)
{
    int log2_cu_size = seq_param->log2_min_luma_coding_block_size_minus3 + 3;
    int log2_ctb_size = seq_param->log2_diff_max_min_luma_coding_block_size + log2_cu_size;
    int ctb_size = 1 << log2_ctb_size;

    int picture_width_in_ctb = (seq_param->pic_width_in_luma_samples + ctb_size - 1) / ctb_size;
    int picture_height_in_ctb = (seq_param->pic_height_in_luma_samples + ctb_size - 1) / ctb_size;

    /* first_slice_segment_in_pic_flag */
    if (slice_index == 0)
    {
        avc_bitstream_put_ui(bs, 1, 1);
    }
    else
    {
        avc_bitstream_put_ui(bs, 0, 1);
    }

    /* no_output_of_prior_pics_flag */
    if (pic_param->pic_fields.bits.idr_pic_flag)
        avc_bitstream_put_ui(bs, 1, 1);

    /* slice_pic_parameter_set_id */
    avc_bitstream_put_ue(bs, 0);

    /* not the first slice */
    if (slice_index)
    {
        /* TBD */
        int bit_size;

        float num_ctus;

        num_ctus = picture_width_in_ctb * picture_height_in_ctb;
        bit_size = ceilf(log2f(num_ctus));

        if (pic_param->pic_fields.bits.dependent_slice_segments_enabled_flag)
        {
            avc_bitstream_put_ui(bs,
                slice_param->slice_fields.bits.dependent_slice_segment_flag, 1);
        }
        /* slice_segment_address is based on Ceil(log2(PictureSizeinCtbs)) */
        avc_bitstream_put_ui(bs, slice_param->slice_segment_address, bit_size);
    }
    if (!slice_param->slice_fields.bits.dependent_slice_segment_flag)
    {
        /* slice_reserved_flag */

        /* slice_type */
        avc_bitstream_put_ue(bs, slice_param->slice_type);
        /* use the inferred the value of pic_output_flag */

        /* colour_plane_id */
        if (seq_param->seq_fields.bits.separate_colour_plane_flag)
        {
            avc_bitstream_put_ui(bs, slice_param->slice_fields.bits.colour_plane_id, 1);
        }

        if (!pic_param->pic_fields.bits.idr_pic_flag)
        {
            int Log2MaxPicOrderCntLsb = 8;
            avc_bitstream_put_ui(bs, pic_param->decoded_curr_pic.pic_order_cnt, Log2MaxPicOrderCntLsb);

            //if (!slice_param->short_term_ref_pic_set_sps_flag)
            {
                /* short_term_ref_pic_set_sps_flag.
                * Use zero and then pass the RPS from slice_header
                */
                avc_bitstream_put_ui(bs, 0, 1);
                /* TBD
                * Add the short_term reference picture set
                */
                hevc_short_term_ref_pic_set(bs,slice_param,pic_param->decoded_curr_pic.pic_order_cnt);
            }
            /* long term reference present flag. unpresent */
            /* TBD */

            /* sps temporal MVP*/
            if (seq_param->seq_fields.bits.sps_temporal_mvp_enabled_flag)
            {
                avc_bitstream_put_ui(bs,
                    slice_param->slice_fields.bits.slice_temporal_mvp_enabled_flag, 1);
            }
        }

        /* long term reference present flag. unpresent */

        /* sample adaptive offset enabled flag */
        if (seq_param->seq_fields.bits.sample_adaptive_offset_enabled_flag)
        {
            avc_bitstream_put_ui(bs, slice_param->slice_fields.bits.slice_sao_luma_flag, 1);
            avc_bitstream_put_ui(bs, slice_param->slice_fields.bits.slice_sao_chroma_flag, 1);
        }

        if (slice_param->slice_type != HEVC_SLICE_I)
        {
            /* num_ref_idx_active_override_flag. 0 */
            avc_bitstream_put_ui(bs, 0, 1);
            /* lists_modification_flag is unpresent NumPocTotalCurr > 1 ,here it is 1*/

            /* No reference picture set modification */

            /* MVD_l1_zero_flag */
            if (slice_param->slice_type == HEVC_SLICE_B)
                avc_bitstream_put_ui(bs, slice_param->slice_fields.bits.mvd_l1_zero_flag, 1);

            /* cabac_init_present_flag. 0 */

            /* slice_temporal_mvp_enabled_flag. */
            if (slice_param->slice_fields.bits.slice_temporal_mvp_enabled_flag)
            {
                if (slice_param->slice_type == HEVC_SLICE_B)
                    avc_bitstream_put_ui(bs, slice_param->slice_fields.bits.collocated_from_l0_flag, 1);
                /*
                * TBD: Add the collocated_ref_idx.
                */
            }
            if (((pic_param->pic_fields.bits.weighted_pred_flag) &&
                (slice_param->slice_type == HEVC_SLICE_P)) ||
                ((pic_param->pic_fields.bits.weighted_bipred_flag) &&
                (slice_param->slice_type == HEVC_SLICE_B)))
            {
                /* TBD:
                * add the weighted table
                */
            }
            avc_bitstream_put_ue(bs, 5 - slice_param->max_num_merge_cand);
        }
        /* slice_qp_delta */
        avc_bitstream_put_ue(bs, slice_param->slice_qp_delta);

        /* slice_cb/cr_qp_offset is controlled by pps_slice_chroma_qp_offsets_present_flag
        * The present flag is set to 1.
        */
        avc_bitstream_put_ue(bs, slice_param->slice_cb_qp_offset);
        avc_bitstream_put_ue(bs, slice_param->slice_cr_qp_offset);

        /*
        * deblocking_filter_override_flag is controlled by
        * deblocking_filter_override_enabled_flag.
        * The override_enabled_flag is zero.
        * deblocking_filter_override_flag is zero. then
        * slice_deblocking_filter_disabled_flag is also zero
        * (It is inferred to be equal to pps_deblocking_filter_disabled_flag.
        */

        /* slice_loop_filter_across_slices_enabled_flag is controlled
        * by pps_loop_filter_across_slices_enabled_flag &&
        * (slice_sao_luma_flag | | slice_sao_chroma_flag | |
        *  !slice_deblocking_filter_disabled_flag ))
        *
        */
    }

    if (pic_param->pic_fields.bits.tiles_enabled_flag ||
        pic_param->pic_fields.bits.entropy_coding_sync_enabled_flag)
    {
        /* TBD.
        * Add the Entry-points && tile definition.
        */
    }

    /* slice_segment_header_extension_present_flag. Not present */

    /* byte_alignment */
    avc_rbsp_trailing_bits(bs);
}

int get_hevc_slice_nalu_type (VAEncPictureParameterBufferHEVC *pic_param)
{
    if (pic_param->pic_fields.bits.idr_pic_flag)
      return IDR_WRADL_NUT;
    else if (pic_param->pic_fields.bits.reference_pic_flag)
      return SLICE_TRAIL_R_NUT;
    else
      return SLICE_TRAIL_N_NUT;
}

int build_hevc_slice_header(VAEncSequenceParameterBufferHEVC *seq_param,
                       VAEncPictureParameterBufferHEVC *pic_param,
                       VAEncSliceParameterBufferHEVC *slice_param,
                       unsigned char **header_buffer,
                       int slice_index)
{
    avc_bitstream bs;

    avc_bitstream_start(&bs);
    nal_start_code_prefix(&bs);
    nal_header_hevc(&bs, get_hevc_slice_nalu_type(pic_param), 0);
    slice_rbsp(&bs, slice_index, seq_param,pic_param,slice_param);
    avc_bitstream_end(&bs);

    *header_buffer = (unsigned char *)bs.buffer;
    return bs.bit_offset;
}
