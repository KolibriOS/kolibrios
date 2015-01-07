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
