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
 *    Xiang Haihao <haihao.xiang@intel.com>
 *    Zhao Yakui <yakui.zhao@intel.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "intel_batchbuffer.h"
#include "i965_defines.h"
#include "i965_structs.h"
#include "i965_drv_video.h"
#include "i965_encoder.h"
#include "i965_encoder_utils.h"
#include "gen6_mfc.h"
#include "gen6_vme.h"
#include "gen9_mfc.h"
#include "intel_media.h"

#ifndef HAVE_LOG2F
#define log2f(x) (logf(x)/(float)M_LN2)
#endif

int intel_avc_enc_slice_type_fixup(int slice_type)
{
    if (slice_type == SLICE_TYPE_SP ||
        slice_type == SLICE_TYPE_P)
        slice_type = SLICE_TYPE_P;
    else if (slice_type == SLICE_TYPE_SI ||
             slice_type == SLICE_TYPE_I)
        slice_type = SLICE_TYPE_I;
    else {
        if (slice_type != SLICE_TYPE_B)
            WARN_ONCE("Invalid slice type for H.264 encoding!\n");

        slice_type = SLICE_TYPE_B;
    }

    return slice_type;
}

static void
intel_mfc_bit_rate_control_context_init(struct encode_state *encode_state, 
                                        struct gen6_mfc_context *mfc_context)
{
    VAEncSequenceParameterBufferH264 *pSequenceParameter = (VAEncSequenceParameterBufferH264 *)encode_state->seq_param_ext->buffer;
    int width_in_mbs = pSequenceParameter->picture_width_in_mbs;
    int height_in_mbs = pSequenceParameter->picture_height_in_mbs;
    float fps =  pSequenceParameter->time_scale * 0.5 / pSequenceParameter->num_units_in_tick ;
    int inter_mb_size = pSequenceParameter->bits_per_second * 1.0 / (fps+4.0) / width_in_mbs / height_in_mbs;
    int intra_mb_size = inter_mb_size * 5.0;
    int i;

    mfc_context->bit_rate_control_context[SLICE_TYPE_I].target_mb_size = intra_mb_size;
    mfc_context->bit_rate_control_context[SLICE_TYPE_I].target_frame_size = intra_mb_size * width_in_mbs * height_in_mbs;
    mfc_context->bit_rate_control_context[SLICE_TYPE_P].target_mb_size = inter_mb_size;
    mfc_context->bit_rate_control_context[SLICE_TYPE_P].target_frame_size = inter_mb_size * width_in_mbs * height_in_mbs;
    mfc_context->bit_rate_control_context[SLICE_TYPE_B].target_mb_size = inter_mb_size;
    mfc_context->bit_rate_control_context[SLICE_TYPE_B].target_frame_size = inter_mb_size * width_in_mbs * height_in_mbs;

    for(i = 0 ; i < 3; i++) {
        mfc_context->bit_rate_control_context[i].QpPrimeY = 26;
        mfc_context->bit_rate_control_context[i].MaxQpNegModifier = 6;
        mfc_context->bit_rate_control_context[i].MaxQpPosModifier = 6;
        mfc_context->bit_rate_control_context[i].GrowInit = 6;
        mfc_context->bit_rate_control_context[i].GrowResistance = 4;
        mfc_context->bit_rate_control_context[i].ShrinkInit = 6;
        mfc_context->bit_rate_control_context[i].ShrinkResistance = 4;
        
        mfc_context->bit_rate_control_context[i].Correct[0] = 8;
        mfc_context->bit_rate_control_context[i].Correct[1] = 4;
        mfc_context->bit_rate_control_context[i].Correct[2] = 2;
        mfc_context->bit_rate_control_context[i].Correct[3] = 2;
        mfc_context->bit_rate_control_context[i].Correct[4] = 4;
        mfc_context->bit_rate_control_context[i].Correct[5] = 8;
    }
    
    mfc_context->bit_rate_control_context[SLICE_TYPE_I].TargetSizeInWord = (intra_mb_size + 16)/ 16;
    mfc_context->bit_rate_control_context[SLICE_TYPE_P].TargetSizeInWord = (inter_mb_size + 16)/ 16;
    mfc_context->bit_rate_control_context[SLICE_TYPE_B].TargetSizeInWord = (inter_mb_size + 16)/ 16;

    mfc_context->bit_rate_control_context[SLICE_TYPE_I].MaxSizeInWord = mfc_context->bit_rate_control_context[SLICE_TYPE_I].TargetSizeInWord * 1.5;
    mfc_context->bit_rate_control_context[SLICE_TYPE_P].MaxSizeInWord = mfc_context->bit_rate_control_context[SLICE_TYPE_P].TargetSizeInWord * 1.5;
    mfc_context->bit_rate_control_context[SLICE_TYPE_B].MaxSizeInWord = mfc_context->bit_rate_control_context[SLICE_TYPE_B].TargetSizeInWord * 1.5;
}

static void intel_mfc_brc_init(struct encode_state *encode_state,
                               struct intel_encoder_context* encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    VAEncSequenceParameterBufferH264 *pSequenceParameter = (VAEncSequenceParameterBufferH264 *)encode_state->seq_param_ext->buffer;
    VAEncMiscParameterBuffer* pMiscParamHRD = NULL;
    VAEncMiscParameterHRD* pParameterHRD = NULL;
    double bitrate = pSequenceParameter->bits_per_second;
    double framerate = (double)pSequenceParameter->time_scale /(2 * (double)pSequenceParameter->num_units_in_tick);
    int inum = 1, pnum = 0, bnum = 0; /* Gop structure: number of I, P, B frames in the Gop. */
    int intra_period = pSequenceParameter->intra_period;
    int ip_period = pSequenceParameter->ip_period;
    double qp1_size = 0.1 * 8 * 3 * (pSequenceParameter->picture_width_in_mbs<<4) * (pSequenceParameter->picture_height_in_mbs<<4)/2;
    double qp51_size = 0.001 * 8 * 3 * (pSequenceParameter->picture_width_in_mbs<<4) * (pSequenceParameter->picture_height_in_mbs<<4)/2;
    double bpf;

    if (!encode_state->misc_param[VAEncMiscParameterTypeHRD] || !encode_state->misc_param[VAEncMiscParameterTypeHRD]->buffer)
        return;

    pMiscParamHRD = (VAEncMiscParameterBuffer*)encode_state->misc_param[VAEncMiscParameterTypeHRD]->buffer;
    pParameterHRD = (VAEncMiscParameterHRD*)pMiscParamHRD->data;

    if (pSequenceParameter->ip_period) {
        pnum = (intra_period + ip_period - 1)/ip_period - 1;
        bnum = intra_period - inum - pnum;
    }

    mfc_context->brc.mode = encoder_context->rate_control_mode;

    mfc_context->brc.target_frame_size[SLICE_TYPE_I] = (int)((double)((bitrate * intra_period)/framerate) /
                                                             (double)(inum + BRC_PWEIGHT * pnum + BRC_BWEIGHT * bnum));
    mfc_context->brc.target_frame_size[SLICE_TYPE_P] = BRC_PWEIGHT * mfc_context->brc.target_frame_size[SLICE_TYPE_I];
    mfc_context->brc.target_frame_size[SLICE_TYPE_B] = BRC_BWEIGHT * mfc_context->brc.target_frame_size[SLICE_TYPE_I];

    mfc_context->brc.gop_nums[SLICE_TYPE_I] = inum;
    mfc_context->brc.gop_nums[SLICE_TYPE_P] = pnum;
    mfc_context->brc.gop_nums[SLICE_TYPE_B] = bnum;

    bpf = mfc_context->brc.bits_per_frame = bitrate/framerate;

    mfc_context->hrd.buffer_size = (double)pParameterHRD->buffer_size;
    mfc_context->hrd.current_buffer_fullness =
        (double)(pParameterHRD->initial_buffer_fullness < mfc_context->hrd.buffer_size)?
        pParameterHRD->initial_buffer_fullness: mfc_context->hrd.buffer_size/2.;
    mfc_context->hrd.target_buffer_fullness = (double)mfc_context->hrd.buffer_size/2.;
    mfc_context->hrd.buffer_capacity = (double)mfc_context->hrd.buffer_size/qp1_size;
    mfc_context->hrd.violation_noted = 0;

    if ((bpf > qp51_size) && (bpf < qp1_size)) {
        mfc_context->bit_rate_control_context[SLICE_TYPE_P].QpPrimeY = 51 - 50*(bpf - qp51_size)/(qp1_size - qp51_size);
    }
    else if (bpf >= qp1_size)
        mfc_context->bit_rate_control_context[SLICE_TYPE_P].QpPrimeY = 1;
    else if (bpf <= qp51_size)
        mfc_context->bit_rate_control_context[SLICE_TYPE_P].QpPrimeY = 51;

    mfc_context->bit_rate_control_context[SLICE_TYPE_I].QpPrimeY = mfc_context->bit_rate_control_context[SLICE_TYPE_P].QpPrimeY;
    mfc_context->bit_rate_control_context[SLICE_TYPE_B].QpPrimeY = mfc_context->bit_rate_control_context[SLICE_TYPE_I].QpPrimeY;

    BRC_CLIP(mfc_context->bit_rate_control_context[SLICE_TYPE_I].QpPrimeY, 1, 51);
    BRC_CLIP(mfc_context->bit_rate_control_context[SLICE_TYPE_P].QpPrimeY, 1, 51);
    BRC_CLIP(mfc_context->bit_rate_control_context[SLICE_TYPE_B].QpPrimeY, 1, 51);
}

int intel_mfc_update_hrd(struct encode_state *encode_state,
                         struct gen6_mfc_context *mfc_context,
                         int frame_bits)
{
    double prev_bf = mfc_context->hrd.current_buffer_fullness;

    mfc_context->hrd.current_buffer_fullness -= frame_bits;

    if (mfc_context->hrd.buffer_size > 0 && mfc_context->hrd.current_buffer_fullness <= 0.) {
        mfc_context->hrd.current_buffer_fullness = prev_bf;
        return BRC_UNDERFLOW;
    }
    
    mfc_context->hrd.current_buffer_fullness += mfc_context->brc.bits_per_frame;
    if (mfc_context->hrd.buffer_size > 0 && mfc_context->hrd.current_buffer_fullness > mfc_context->hrd.buffer_size) {
        if (mfc_context->brc.mode == VA_RC_VBR)
            mfc_context->hrd.current_buffer_fullness = mfc_context->hrd.buffer_size;
        else {
            mfc_context->hrd.current_buffer_fullness = prev_bf;
            return BRC_OVERFLOW;
        }
    }
    return BRC_NO_HRD_VIOLATION;
}

int intel_mfc_brc_postpack(struct encode_state *encode_state,
                           struct gen6_mfc_context *mfc_context,
                           int frame_bits)
{
    gen6_brc_status sts = BRC_NO_HRD_VIOLATION;
    VAEncSliceParameterBufferH264 *pSliceParameter = (VAEncSliceParameterBufferH264 *)encode_state->slice_params_ext[0]->buffer; 
    int slicetype = intel_avc_enc_slice_type_fixup(pSliceParameter->slice_type);
    int qpi = mfc_context->bit_rate_control_context[SLICE_TYPE_I].QpPrimeY;
    int qpp = mfc_context->bit_rate_control_context[SLICE_TYPE_P].QpPrimeY;
    int qpb = mfc_context->bit_rate_control_context[SLICE_TYPE_B].QpPrimeY;
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
    BRC_CLIP(qpn, 1, 51);

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
    BRC_CLIP(qpn, 1, 51);

    if (sts == BRC_NO_HRD_VIOLATION) { // no HRD violation
        /* correcting QPs of slices of other types */
        if (slicetype == SLICE_TYPE_P) {
            if (abs(qpn + BRC_P_B_QP_DIFF - qpb) > 2)
                mfc_context->bit_rate_control_context[SLICE_TYPE_B].QpPrimeY += (qpn + BRC_P_B_QP_DIFF - qpb) >> 1;
            if (abs(qpn - BRC_I_P_QP_DIFF - qpi) > 2)
                mfc_context->bit_rate_control_context[SLICE_TYPE_I].QpPrimeY += (qpn - BRC_I_P_QP_DIFF - qpi) >> 1;
        } else if (slicetype == SLICE_TYPE_I) {
            if (abs(qpn + BRC_I_B_QP_DIFF - qpb) > 4)
                mfc_context->bit_rate_control_context[SLICE_TYPE_B].QpPrimeY += (qpn + BRC_I_B_QP_DIFF - qpb) >> 2;
            if (abs(qpn + BRC_I_P_QP_DIFF - qpp) > 2)
                mfc_context->bit_rate_control_context[SLICE_TYPE_P].QpPrimeY += (qpn + BRC_I_P_QP_DIFF - qpp) >> 2;
        } else { // SLICE_TYPE_B
            if (abs(qpn - BRC_P_B_QP_DIFF - qpp) > 2)
                mfc_context->bit_rate_control_context[SLICE_TYPE_P].QpPrimeY += (qpn - BRC_P_B_QP_DIFF - qpp) >> 1;
            if (abs(qpn - BRC_I_B_QP_DIFF - qpi) > 4)
                mfc_context->bit_rate_control_context[SLICE_TYPE_I].QpPrimeY += (qpn - BRC_I_B_QP_DIFF - qpi) >> 2;
        }
        BRC_CLIP(mfc_context->bit_rate_control_context[SLICE_TYPE_I].QpPrimeY, 1, 51);
        BRC_CLIP(mfc_context->bit_rate_control_context[SLICE_TYPE_P].QpPrimeY, 1, 51);
        BRC_CLIP(mfc_context->bit_rate_control_context[SLICE_TYPE_B].QpPrimeY, 1, 51);
    } else if (sts == BRC_UNDERFLOW) { // underflow
        if (qpn <= qp) qpn = qp + 1;
        if (qpn > 51) {
            qpn = 51;
            sts = BRC_UNDERFLOW_WITH_MAX_QP; //underflow with maxQP
        }
    } else if (sts == BRC_OVERFLOW) {
        if (qpn >= qp) qpn = qp - 1;
        if (qpn < 1) { // < 0 (?) overflow with minQP
            qpn = 1;
            sts = BRC_OVERFLOW_WITH_MIN_QP; // bit stuffing to be done
        }
    }

    mfc_context->bit_rate_control_context[slicetype].QpPrimeY = qpn;

    return sts;
}

static void intel_mfc_hrd_context_init(struct encode_state *encode_state,
                                       struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    VAEncSequenceParameterBufferH264 *pSequenceParameter = (VAEncSequenceParameterBufferH264 *)encode_state->seq_param_ext->buffer;
    unsigned int rate_control_mode = encoder_context->rate_control_mode;
    int target_bit_rate = pSequenceParameter->bits_per_second;
    
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

void 
intel_mfc_hrd_context_update(struct encode_state *encode_state, 
                             struct gen6_mfc_context *mfc_context)
{
    mfc_context->vui_hrd.i_frame_number++;
}

int intel_mfc_interlace_check(VADriverContextP ctx,
                              struct encode_state *encode_state,
                              struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    VAEncSliceParameterBufferH264 *pSliceParameter;
    int i;
    int mbCount = 0;
    int width_in_mbs = (mfc_context->surface_state.width + 15) / 16;
    int height_in_mbs = (mfc_context->surface_state.height + 15) / 16;
  
    for (i = 0; i < encode_state->num_slice_params_ext; i++) {
        pSliceParameter = (VAEncSliceParameterBufferH264 *)encode_state->slice_params_ext[i]->buffer; 
        mbCount += pSliceParameter->num_macroblocks; 
    }
    
    if ( mbCount == ( width_in_mbs * height_in_mbs ) )
        return 0;

    return 1;
}

/*
 * Check whether the parameters related with CBR are updated and decide whether
 * it needs to reinitialize the configuration related with CBR.
 * Currently it will check the following parameters:
 *      bits_per_second
 *      frame_rate
 *      gop_configuration(intra_period, ip_period, intra_idr_period)
 */
static bool intel_mfc_brc_updated_check(struct encode_state *encode_state,
                           struct intel_encoder_context *encoder_context)
{
    unsigned int rate_control_mode = encoder_context->rate_control_mode;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    double cur_fps, cur_bitrate;
    VAEncSequenceParameterBufferH264 *pSequenceParameter;


    if (rate_control_mode != VA_RC_CBR) {
        return false;
    }

    pSequenceParameter = (VAEncSequenceParameterBufferH264 *)encode_state->seq_param_ext->buffer;

    cur_bitrate = pSequenceParameter->bits_per_second;
    cur_fps = (double)pSequenceParameter->time_scale /
                (2 * (double)pSequenceParameter->num_units_in_tick);

    if ((cur_bitrate == mfc_context->brc.saved_bps) &&
        (cur_fps == mfc_context->brc.saved_fps) &&
        (pSequenceParameter->intra_period == mfc_context->brc.saved_intra_period) &&
        (pSequenceParameter->intra_idr_period == mfc_context->brc.saved_idr_period) &&
        (pSequenceParameter->intra_period == mfc_context->brc.saved_intra_period)) {
        /* the parameters related with CBR are not updaetd */
        return false;
    }

    mfc_context->brc.saved_ip_period = pSequenceParameter->ip_period;
    mfc_context->brc.saved_intra_period = pSequenceParameter->intra_period;
    mfc_context->brc.saved_idr_period = pSequenceParameter->intra_idr_period;
    mfc_context->brc.saved_fps = cur_fps;
    mfc_context->brc.saved_bps = cur_bitrate;
    return true;
}

void intel_mfc_brc_prepare(struct encode_state *encode_state,
                           struct intel_encoder_context *encoder_context)
{
    unsigned int rate_control_mode = encoder_context->rate_control_mode;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;

    if (rate_control_mode == VA_RC_CBR) {
        bool brc_updated;
        assert(encoder_context->codec != CODEC_MPEG2);

        brc_updated = intel_mfc_brc_updated_check(encode_state, encoder_context);

        /*Programing bit rate control */
        if ((mfc_context->bit_rate_control_context[SLICE_TYPE_I].MaxSizeInWord == 0) ||
             brc_updated) {
            intel_mfc_bit_rate_control_context_init(encode_state, mfc_context);
            intel_mfc_brc_init(encode_state, encoder_context);
        }

        /*Programing HRD control */
        if ((mfc_context->vui_hrd.i_cpb_size_value == 0) || brc_updated )
            intel_mfc_hrd_context_init(encode_state, encoder_context);    
    }
}

static int intel_avc_find_skipemulcnt(unsigned char *buf, int bits_length)
{
    int i, found;
    int leading_zero_cnt, byte_length, zero_byte;
    int nal_unit_type;
    int skip_cnt = 0;

#define NAL_UNIT_TYPE_MASK 0x1f
#define HW_MAX_SKIP_LENGTH 15

    byte_length = ALIGN(bits_length, 32) >> 3;


    leading_zero_cnt = 0;
    found = 0;
    for(i = 0; i < byte_length - 4; i++) {
        if (((buf[i] == 0) && (buf[i + 1] == 0) && (buf[i + 2] == 1)) ||
            ((buf[i] == 0) && (buf[i + 1] == 0) && (buf[i + 2] == 0) && (buf[i + 3] == 1))) {
                found = 1;
                break;
            }
        leading_zero_cnt++;
    }
    if (!found) {
        /* warning message is complained. But anyway it will be inserted. */
        WARN_ONCE("Invalid packed header data. "
                   "Can't find the 000001 start_prefix code\n");
        return 0;
    }
    i = leading_zero_cnt;

    zero_byte = 0;
    if (!((buf[i] == 0) && (buf[i + 1] == 0) && (buf[i + 2] == 1)))
        zero_byte = 1;

    skip_cnt = leading_zero_cnt + zero_byte + 3;

    /* the unit header byte is accounted */
    nal_unit_type = (buf[skip_cnt]) & NAL_UNIT_TYPE_MASK;
    skip_cnt += 1;

    if (nal_unit_type == 14 || nal_unit_type == 20 || nal_unit_type == 21) {
        /* more unit header bytes are accounted for MVC/SVC */
        skip_cnt += 3;
    }
    if (skip_cnt > HW_MAX_SKIP_LENGTH) {
        WARN_ONCE("Too many leading zeros are padded for packed data. "
                   "It is beyond the HW range.!!!\n");
    }
    return skip_cnt;
}

void intel_mfc_avc_pipeline_header_programing(VADriverContextP ctx,
                                              struct encode_state *encode_state,
                                              struct intel_encoder_context *encoder_context,
                                              struct intel_batchbuffer *slice_batch)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    int idx = va_enc_packed_type_to_idx(VAEncPackedHeaderH264_SPS);
    unsigned int rate_control_mode = encoder_context->rate_control_mode;
    unsigned int skip_emul_byte_cnt;

    if (encode_state->packed_header_data[idx]) {
        VAEncPackedHeaderParameterBuffer *param = NULL;
        unsigned int *header_data = (unsigned int *)encode_state->packed_header_data[idx]->buffer;
        unsigned int length_in_bits;

        assert(encode_state->packed_header_param[idx]);
        param = (VAEncPackedHeaderParameterBuffer *)encode_state->packed_header_param[idx]->buffer;
        length_in_bits = param->bit_length;

        skip_emul_byte_cnt = intel_avc_find_skipemulcnt((unsigned char *)header_data, length_in_bits);
        mfc_context->insert_object(ctx,
                                   encoder_context,
                                   header_data,
                                   ALIGN(length_in_bits, 32) >> 5,
                                   length_in_bits & 0x1f,
                                   skip_emul_byte_cnt,
                                   0,
                                   0,
                                   !param->has_emulation_bytes,
                                   slice_batch);
    }

    idx = va_enc_packed_type_to_idx(VAEncPackedHeaderH264_PPS);

    if (encode_state->packed_header_data[idx]) {
        VAEncPackedHeaderParameterBuffer *param = NULL;
        unsigned int *header_data = (unsigned int *)encode_state->packed_header_data[idx]->buffer;
        unsigned int length_in_bits;

        assert(encode_state->packed_header_param[idx]);
        param = (VAEncPackedHeaderParameterBuffer *)encode_state->packed_header_param[idx]->buffer;
        length_in_bits = param->bit_length;

        skip_emul_byte_cnt = intel_avc_find_skipemulcnt((unsigned char *)header_data, length_in_bits);

        mfc_context->insert_object(ctx,
                                   encoder_context,
                                   header_data,
                                   ALIGN(length_in_bits, 32) >> 5,
                                   length_in_bits & 0x1f,
                                   skip_emul_byte_cnt,
                                   0,
                                   0,
                                   !param->has_emulation_bytes,
                                   slice_batch);
    }
    
    idx = va_enc_packed_type_to_idx(VAEncPackedHeaderH264_SEI);

    if (encode_state->packed_header_data[idx]) {
        VAEncPackedHeaderParameterBuffer *param = NULL;
        unsigned int *header_data = (unsigned int *)encode_state->packed_header_data[idx]->buffer;
        unsigned int length_in_bits;

        assert(encode_state->packed_header_param[idx]);
        param = (VAEncPackedHeaderParameterBuffer *)encode_state->packed_header_param[idx]->buffer;
        length_in_bits = param->bit_length;

        skip_emul_byte_cnt = intel_avc_find_skipemulcnt((unsigned char *)header_data, length_in_bits);
        mfc_context->insert_object(ctx,
                                   encoder_context,
                                   header_data,
                                   ALIGN(length_in_bits, 32) >> 5,
                                   length_in_bits & 0x1f,
                                   skip_emul_byte_cnt,
                                   0,
                                   0,
                                   !param->has_emulation_bytes,
                                   slice_batch);
    } else if (rate_control_mode == VA_RC_CBR) {
        // this is frist AU
        struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;

        unsigned char *sei_data = NULL;
    
        int length_in_bits = build_avc_sei_buffer_timing(
            mfc_context->vui_hrd.i_initial_cpb_removal_delay_length,
            mfc_context->vui_hrd.i_initial_cpb_removal_delay,
            0,
            mfc_context->vui_hrd.i_cpb_removal_delay_length,                                                       mfc_context->vui_hrd.i_cpb_removal_delay * mfc_context->vui_hrd.i_frame_number,
            mfc_context->vui_hrd.i_dpb_output_delay_length,
            0,
            &sei_data);
        mfc_context->insert_object(ctx,
                                   encoder_context,
                                   (unsigned int *)sei_data,
                                   ALIGN(length_in_bits, 32) >> 5,
                                   length_in_bits & 0x1f,
                                   5,
                                   0,   
                                   0,   
                                   1,
                                   slice_batch);  
        free(sei_data);
    }
}

VAStatus intel_mfc_avc_prepare(VADriverContextP ctx, 
                               struct encode_state *encode_state,
                               struct intel_encoder_context *encoder_context)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct object_surface *obj_surface;	
    struct object_buffer *obj_buffer;
    GenAvcSurface *gen6_avc_surface;
    dri_bo *bo;
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    int i, j, enable_avc_ildb = 0;
    VAEncSliceParameterBufferH264 *slice_param;
    struct i965_coded_buffer_segment *coded_buffer_segment;
    VAEncSequenceParameterBufferH264 *pSequenceParameter = (VAEncSequenceParameterBufferH264 *)encode_state->seq_param_ext->buffer;
    int width_in_mbs = pSequenceParameter->picture_width_in_mbs;
    int height_in_mbs = pSequenceParameter->picture_height_in_mbs;

    if (IS_GEN6(i965->intel.device_info)) {
        /* On the SNB it should be fixed to 128 for the DMV buffer */
        width_in_mbs = 128;
    }

    for (j = 0; j < encode_state->num_slice_params_ext && enable_avc_ildb == 0; j++) {
        assert(encode_state->slice_params_ext && encode_state->slice_params_ext[j]->buffer);
        slice_param = (VAEncSliceParameterBufferH264 *)encode_state->slice_params_ext[j]->buffer;

        for (i = 0; i < encode_state->slice_params_ext[j]->num_elements; i++) {
            assert((slice_param->slice_type == SLICE_TYPE_I) ||
                   (slice_param->slice_type == SLICE_TYPE_SI) ||
                   (slice_param->slice_type == SLICE_TYPE_P) ||
                   (slice_param->slice_type == SLICE_TYPE_SP) ||
                   (slice_param->slice_type == SLICE_TYPE_B));

            if (slice_param->disable_deblocking_filter_idc != 1) {
                enable_avc_ildb = 1;
                break;
            }

            slice_param++;
        }
    }

    /*Setup all the input&output object*/

    /* Setup current frame and current direct mv buffer*/
    obj_surface = encode_state->reconstructed_object;
    i965_check_alloc_surface_bo(ctx, obj_surface, 1, VA_FOURCC_NV12, SUBSAMPLE_YUV420);

    if ( obj_surface->private_data == NULL) {
        gen6_avc_surface = calloc(sizeof(GenAvcSurface), 1);
        assert(gen6_avc_surface);
        gen6_avc_surface->dmv_top = 
            dri_bo_alloc(i965->intel.bufmgr,
                         "Buffer",
                         68 * width_in_mbs * height_in_mbs, 
                         64);
        gen6_avc_surface->dmv_bottom = 
            dri_bo_alloc(i965->intel.bufmgr,
                         "Buffer",
                         68 * width_in_mbs * height_in_mbs, 
                         64);
        assert(gen6_avc_surface->dmv_top);
        assert(gen6_avc_surface->dmv_bottom);
        obj_surface->private_data = (void *)gen6_avc_surface;
        obj_surface->free_private_data = (void *)gen_free_avc_surface; 
    }
    gen6_avc_surface = (GenAvcSurface *) obj_surface->private_data;
    mfc_context->direct_mv_buffers[NUM_MFC_DMV_BUFFERS - 2].bo = gen6_avc_surface->dmv_top;
    mfc_context->direct_mv_buffers[NUM_MFC_DMV_BUFFERS - 1].bo = gen6_avc_surface->dmv_bottom;
    dri_bo_reference(gen6_avc_surface->dmv_top);
    dri_bo_reference(gen6_avc_surface->dmv_bottom);

    if (enable_avc_ildb) {
        mfc_context->post_deblocking_output.bo = obj_surface->bo;
        dri_bo_reference(mfc_context->post_deblocking_output.bo);
    } else {
        mfc_context->pre_deblocking_output.bo = obj_surface->bo;
        dri_bo_reference(mfc_context->pre_deblocking_output.bo);
    }

    mfc_context->surface_state.width = obj_surface->orig_width;
    mfc_context->surface_state.height = obj_surface->orig_height;
    mfc_context->surface_state.w_pitch = obj_surface->width;
    mfc_context->surface_state.h_pitch = obj_surface->height;
    
    /* Setup reference frames and direct mv buffers*/
    for(i = 0; i < MAX_MFC_REFERENCE_SURFACES; i++) {
        obj_surface = encode_state->reference_objects[i];
        
        if (obj_surface && obj_surface->bo) {
            mfc_context->reference_surfaces[i].bo = obj_surface->bo;
            dri_bo_reference(obj_surface->bo);

            /* Check DMV buffer */
            if ( obj_surface->private_data == NULL) {
                
                gen6_avc_surface = calloc(sizeof(GenAvcSurface), 1);
                assert(gen6_avc_surface);
                gen6_avc_surface->dmv_top = 
                    dri_bo_alloc(i965->intel.bufmgr,
                                 "Buffer",
                                 68 * width_in_mbs * height_in_mbs, 
                                 64);
                gen6_avc_surface->dmv_bottom = 
                    dri_bo_alloc(i965->intel.bufmgr,
                                 "Buffer",
                                 68 * width_in_mbs * height_in_mbs, 
                                 64);
                assert(gen6_avc_surface->dmv_top);
                assert(gen6_avc_surface->dmv_bottom);
                obj_surface->private_data = gen6_avc_surface;
                obj_surface->free_private_data = gen_free_avc_surface; 
            }
    
            gen6_avc_surface = (GenAvcSurface *) obj_surface->private_data;
            /* Setup DMV buffer */
            mfc_context->direct_mv_buffers[i*2].bo = gen6_avc_surface->dmv_top;
            mfc_context->direct_mv_buffers[i*2+1].bo = gen6_avc_surface->dmv_bottom; 
            dri_bo_reference(gen6_avc_surface->dmv_top);
            dri_bo_reference(gen6_avc_surface->dmv_bottom);
        } else {
            break;
        }
    }

    mfc_context->uncompressed_picture_source.bo = encode_state->input_yuv_object->bo;
    dri_bo_reference(mfc_context->uncompressed_picture_source.bo);

    obj_buffer = encode_state->coded_buf_object;
    bo = obj_buffer->buffer_store->bo;
    mfc_context->mfc_indirect_pak_bse_object.bo = bo;
    mfc_context->mfc_indirect_pak_bse_object.offset = I965_CODEDBUFFER_HEADER_SIZE;
    mfc_context->mfc_indirect_pak_bse_object.end_offset = ALIGN(obj_buffer->size_element - 0x1000, 0x1000);
    dri_bo_reference(mfc_context->mfc_indirect_pak_bse_object.bo);
    
    dri_bo_map(bo, 1);
    coded_buffer_segment = (struct i965_coded_buffer_segment *)bo->virtual;
    coded_buffer_segment->mapped = 0;
    coded_buffer_segment->codec = encoder_context->codec;
    dri_bo_unmap(bo);

    return vaStatus;
}
/*
 * The LUT uses the pair of 4-bit units: (shift, base) structure.
 * 2^K * X = value . 
 * So it is necessary to convert one cost into the nearest LUT format.
 * The derivation is:
 * 2^K *x = 2^n * (1 + deltaX)
 *    k + log2(x) = n + log2(1 + deltaX)
 *    log2(x) = n - k + log2(1 + deltaX)
 *    As X is in the range of [1, 15]
 *      4 > n - k + log2(1 + deltaX) >= 0 
 *      =>    n + log2(1 + deltaX)  >= k > n - 4  + log2(1 + deltaX)
 *    Then we can derive the corresponding K and get the nearest LUT format.
 */
int intel_format_lutvalue(int value, int max)
{
    int ret;
    int logvalue, temp1, temp2;

    if (value <= 0)
        return 0;

    logvalue = (int)(log2f((float)value));
    if (logvalue < 4) {
        ret = value;
    } else {
        int error, temp_value, base, j, temp_err;
        error = value;
        j = logvalue - 4 + 1;
        ret = -1;
        for(; j <= logvalue; j++) {
            if (j == 0) {
                base = value >> j;
            } else {
                base = (value + (1 << (j - 1)) - 1) >> j;
            }
            if (base >= 16)
                continue;

            temp_value = base << j;
            temp_err = abs(value - temp_value);
            if (temp_err < error) {
                error = temp_err;
                ret = (j << 4) | base;
                if (temp_err == 0)
                    break;
            }
        }
    }
    temp1 = (ret & 0xf) << ((ret & 0xf0) >> 4);
    temp2 = (max & 0xf) << ((max & 0xf0) >> 4);
    if (temp1 > temp2)
        ret = max;
    return ret;

}


#define		QP_MAX			52
#define		VP8_QP_MAX	        128


static float intel_lambda_qp(int qp)
{
    float value, lambdaf;
    value = qp;
    value = value / 6 - 2;
    if (value < 0)
        value = 0;
    lambdaf = roundf(powf(2, value));
    return lambdaf;
}


void intel_vme_update_mbmv_cost(VADriverContextP ctx,
                                struct encode_state *encode_state,
                                struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    VAEncPictureParameterBufferH264 *pic_param = (VAEncPictureParameterBufferH264 *)encode_state->pic_param_ext->buffer;
    VAEncSliceParameterBufferH264 *slice_param = (VAEncSliceParameterBufferH264 *)encode_state->slice_params_ext[0]->buffer;
    int qp, m_cost, j, mv_count;
    uint8_t *vme_state_message = (uint8_t *)(vme_context->vme_state_message);
    float   lambda, m_costf;

    int slice_type = intel_avc_enc_slice_type_fixup(slice_param->slice_type);

    
    if (encoder_context->rate_control_mode == VA_RC_CQP)
        qp = pic_param->pic_init_qp + slice_param->slice_qp_delta;
    else
        qp = mfc_context->bit_rate_control_context[slice_type].QpPrimeY;

    if (vme_state_message == NULL)
        return;

    assert(qp <= QP_MAX); 
    lambda = intel_lambda_qp(qp);
    if (slice_type == SLICE_TYPE_I) {
        vme_state_message[MODE_INTRA_16X16] = 0;
        m_cost = lambda * 4;
        vme_state_message[MODE_INTRA_8X8] = intel_format_lutvalue(m_cost, 0x8f);
        m_cost = lambda * 16; 
        vme_state_message[MODE_INTRA_4X4] = intel_format_lutvalue(m_cost, 0x8f);
        m_cost = lambda * 3;
        vme_state_message[MODE_INTRA_NONPRED] = intel_format_lutvalue(m_cost, 0x6f);
    } else {
        m_cost = 0;
        vme_state_message[MODE_INTER_MV0] = intel_format_lutvalue(m_cost, 0x6f);
        for (j = 1; j < 3; j++) {
            m_costf = (log2f((float)(j + 1)) + 1.718f) * lambda;
            m_cost = (int)m_costf;
            vme_state_message[MODE_INTER_MV0 + j] = intel_format_lutvalue(m_cost, 0x6f);
        }
        mv_count = 3;
        for (j = 4; j <= 64; j *= 2) {
            m_costf = (log2f((float)(j + 1)) + 1.718f) * lambda;
            m_cost = (int)m_costf;
            vme_state_message[MODE_INTER_MV0 + mv_count] = intel_format_lutvalue(m_cost, 0x6f);
            mv_count++;
        }

        if (qp <= 25) {
            vme_state_message[MODE_INTRA_16X16] = 0x4a;
            vme_state_message[MODE_INTRA_8X8] = 0x4a;
            vme_state_message[MODE_INTRA_4X4] = 0x4a;
            vme_state_message[MODE_INTRA_NONPRED] = 0x4a;
            vme_state_message[MODE_INTER_16X16] = 0x4a;
            vme_state_message[MODE_INTER_16X8] = 0x4a;
            vme_state_message[MODE_INTER_8X8] = 0x4a;
            vme_state_message[MODE_INTER_8X4] = 0x4a;
            vme_state_message[MODE_INTER_4X4] = 0x4a;
            vme_state_message[MODE_INTER_BWD] = 0x2a;
            return;
        }
        m_costf = lambda * 10;
        vme_state_message[MODE_INTRA_16X16] = intel_format_lutvalue(m_cost, 0x8f);
        m_cost = lambda * 14;
        vme_state_message[MODE_INTRA_8X8] = intel_format_lutvalue(m_cost, 0x8f);
        m_cost = lambda * 24; 
        vme_state_message[MODE_INTRA_4X4] = intel_format_lutvalue(m_cost, 0x8f);
        m_costf = lambda * 3.5;
        m_cost = m_costf;
        vme_state_message[MODE_INTRA_NONPRED] = intel_format_lutvalue(m_cost, 0x6f);
        if (slice_type == SLICE_TYPE_P) {
            m_costf = lambda * 2.5;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_16X16] = intel_format_lutvalue(m_cost, 0x8f);
            m_costf = lambda * 4;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_16X8] = intel_format_lutvalue(m_cost, 0x8f);
            m_costf = lambda * 1.5;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_8X8] = intel_format_lutvalue(m_cost, 0x6f);
            m_costf = lambda * 3;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_8X4] = intel_format_lutvalue(m_cost, 0x6f);
            m_costf = lambda * 5;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_4X4] = intel_format_lutvalue(m_cost, 0x6f);
            /* BWD is not used in P-frame */
            vme_state_message[MODE_INTER_BWD] = 0;
        } else {
            m_costf = lambda * 2.5;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_16X16] = intel_format_lutvalue(m_cost, 0x8f);
            m_costf = lambda * 5.5;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_16X8] = intel_format_lutvalue(m_cost, 0x8f);
            m_costf = lambda * 3.5;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_8X8] = intel_format_lutvalue(m_cost, 0x6f);
            m_costf = lambda * 5.0;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_8X4] = intel_format_lutvalue(m_cost, 0x6f);
            m_costf = lambda * 6.5;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_4X4] = intel_format_lutvalue(m_cost, 0x6f);
            m_costf = lambda * 1.5;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_BWD] = intel_format_lutvalue(m_cost, 0x6f);
        }
    }
}

void intel_vme_vp8_update_mbmv_cost(VADriverContextP ctx,
                                struct encode_state *encode_state,
                                struct intel_encoder_context *encoder_context)
{
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    VAEncPictureParameterBufferVP8 *pic_param = (VAEncPictureParameterBufferVP8 *)encode_state->pic_param_ext->buffer;
    VAQMatrixBufferVP8 *q_matrix = (VAQMatrixBufferVP8 *)encode_state->q_matrix->buffer;
    int qp, m_cost, j, mv_count;
    uint8_t *vme_state_message = (uint8_t *)(vme_context->vme_state_message);
    float   lambda, m_costf;

    int is_key_frame = !pic_param->pic_flags.bits.frame_type;
    int slice_type = (is_key_frame ? SLICE_TYPE_I : SLICE_TYPE_P);
  
    if (vme_state_message == NULL)
	return;
 
    if (encoder_context->rate_control_mode == VA_RC_CQP)
        qp = q_matrix->quantization_index[0];
    else
        qp = mfc_context->bit_rate_control_context[slice_type].QpPrimeY;

    lambda = intel_lambda_qp(qp * QP_MAX / VP8_QP_MAX);
    if (is_key_frame) {
	vme_state_message[MODE_INTRA_16X16] = 0;
	m_cost = lambda * 16; 
	vme_state_message[MODE_INTRA_4X4] = intel_format_lutvalue(m_cost, 0x8f);
    } else {
    	m_cost = 0;
	vme_state_message[MODE_INTER_MV0] = intel_format_lutvalue(m_cost, 0x6f);
	for (j = 1; j < 3; j++) {
            m_costf = (log2f((float)(j + 1)) + 1.718f) * lambda;
            m_cost = (int)m_costf;
            vme_state_message[MODE_INTER_MV0 + j] = intel_format_lutvalue(m_cost, 0x6f);
   	}
    	mv_count = 3;
    	for (j = 4; j <= 64; j *= 2) {
            m_costf = (log2f((float)(j + 1)) + 1.718f) * lambda;
            m_cost = (int)m_costf;
            vme_state_message[MODE_INTER_MV0 + mv_count] = intel_format_lutvalue(m_cost, 0x6f);
            mv_count++;
	}

	if (qp < 92 ) {
            vme_state_message[MODE_INTRA_16X16] = 0x4a;
            vme_state_message[MODE_INTRA_4X4] = 0x4a;
            vme_state_message[MODE_INTRA_NONPRED] = 0x4a;
            vme_state_message[MODE_INTER_16X16] = 0x4a;
            vme_state_message[MODE_INTER_16X8] = 0x4a;
            vme_state_message[MODE_INTER_8X8] = 0x4a;
            vme_state_message[MODE_INTER_4X4] = 0x4a;
            return;
	}
	m_costf = lambda * 10;
	vme_state_message[MODE_INTRA_16X16] = intel_format_lutvalue(m_cost, 0x8f);
	m_cost = lambda * 24; 
	vme_state_message[MODE_INTRA_4X4] = intel_format_lutvalue(m_cost, 0x8f);
            
        m_costf = lambda * 2.5;
        m_cost = m_costf;
        vme_state_message[MODE_INTER_16X16] = intel_format_lutvalue(m_cost, 0x8f);
        m_costf = lambda * 4;
        m_cost = m_costf;
        vme_state_message[MODE_INTER_16X8] = intel_format_lutvalue(m_cost, 0x8f);
        m_costf = lambda * 1.5;
        m_cost = m_costf;
        vme_state_message[MODE_INTER_8X8] = intel_format_lutvalue(m_cost, 0x6f);
        m_costf = lambda * 5;
        m_cost = m_costf;
        vme_state_message[MODE_INTER_4X4] = intel_format_lutvalue(m_cost, 0x6f);
        /* BWD is not used in P-frame */
        vme_state_message[MODE_INTER_BWD] = 0;
    }
}

#define		MB_SCOREBOARD_A		(1 << 0)
#define		MB_SCOREBOARD_B		(1 << 1)
#define		MB_SCOREBOARD_C		(1 << 2)
void 
gen7_vme_scoreboard_init(VADriverContextP ctx, struct gen6_vme_context *vme_context)
{
    vme_context->gpe_context.vfe_desc5.scoreboard0.enable = 1;
    vme_context->gpe_context.vfe_desc5.scoreboard0.type = SCOREBOARD_STALLING;
    vme_context->gpe_context.vfe_desc5.scoreboard0.mask = (MB_SCOREBOARD_A |
                                                           MB_SCOREBOARD_B |
                                                           MB_SCOREBOARD_C);

    /* In VME prediction the current mb depends on the neighbour 
     * A/B/C macroblock. So the left/up/up-right dependency should
     * be considered.
     */
    vme_context->gpe_context.vfe_desc6.scoreboard1.delta_x0 = -1;
    vme_context->gpe_context.vfe_desc6.scoreboard1.delta_y0 = 0;
    vme_context->gpe_context.vfe_desc6.scoreboard1.delta_x1 = 0;
    vme_context->gpe_context.vfe_desc6.scoreboard1.delta_y1 = -1;
    vme_context->gpe_context.vfe_desc6.scoreboard1.delta_x2 = 1;
    vme_context->gpe_context.vfe_desc6.scoreboard1.delta_y2 = -1;

    vme_context->gpe_context.vfe_desc7.dword = 0;
    return;
}

/* check whether the mb of (x_index, y_index) is out of bound */
static inline int loop_in_bounds(int x_index, int y_index, int first_mb, int num_mb, int mb_width, int mb_height)
{
    int mb_index;
    if (x_index < 0 || x_index >= mb_width)
        return -1;
    if (y_index < 0 || y_index >= mb_height)
        return -1;

    mb_index = y_index * mb_width + x_index;
    if (mb_index < first_mb || mb_index > (first_mb + num_mb))
        return -1;
    return 0;
}

void
gen7_vme_walker_fill_vme_batchbuffer(VADriverContextP ctx, 
                                     struct encode_state *encode_state,
                                     int mb_width, int mb_height,
                                     int kernel,
                                     int transform_8x8_mode_flag,
                                     struct intel_encoder_context *encoder_context)
{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    int mb_row;
    int s;
    unsigned int *command_ptr;

#define		USE_SCOREBOARD		(1 << 21)
 
    dri_bo_map(vme_context->vme_batchbuffer.bo, 1);
    command_ptr = vme_context->vme_batchbuffer.bo->virtual;

    for (s = 0; s < encode_state->num_slice_params_ext; s++) {
        VAEncSliceParameterBufferH264 *pSliceParameter = (VAEncSliceParameterBufferH264 *)encode_state->slice_params_ext[s]->buffer;
        int first_mb = pSliceParameter->macroblock_address;
        int num_mb = pSliceParameter->num_macroblocks;
        unsigned int mb_intra_ub, score_dep;
        int x_outer, y_outer, x_inner, y_inner;
        int xtemp_outer = 0;

        x_outer = first_mb % mb_width;
        y_outer = first_mb / mb_width;
        mb_row = y_outer;

        for (; x_outer < (mb_width -2 ) && !loop_in_bounds(x_outer, y_outer, first_mb, num_mb, mb_width, mb_height); ) {
            x_inner = x_outer;
            y_inner = y_outer;
            for (; !loop_in_bounds(x_inner, y_inner, first_mb, num_mb, mb_width, mb_height);) {
                mb_intra_ub = 0;
                score_dep = 0;
                if (x_inner != 0) {
                    mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_AE;
                    score_dep |= MB_SCOREBOARD_A; 
                }
                if (y_inner != mb_row) {
                    mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_B;
                    score_dep |= MB_SCOREBOARD_B;
                    if (x_inner != 0)
                        mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_D;
                    if (x_inner != (mb_width -1)) {
                        mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_C;
                        score_dep |= MB_SCOREBOARD_C;
                    }
                }

                *command_ptr++ = (CMD_MEDIA_OBJECT | (8 - 2));
                *command_ptr++ = kernel;
                *command_ptr++ = USE_SCOREBOARD;
                /* Indirect data */
                *command_ptr++ = 0;
                /* the (X, Y) term of scoreboard */
                *command_ptr++ = ((y_inner << 16) | x_inner);
                *command_ptr++ = score_dep;
                /*inline data */
                *command_ptr++ = (mb_width << 16 | y_inner << 8 | x_inner);
                *command_ptr++ = ((1 << 18) | (1 << 16) | transform_8x8_mode_flag | (mb_intra_ub << 8));
                x_inner -= 2;
                y_inner += 1;
            }
            x_outer += 1;
        }

        xtemp_outer = mb_width - 2;
        if (xtemp_outer < 0)
            xtemp_outer = 0;
        x_outer = xtemp_outer;
        y_outer = first_mb / mb_width;
        for (;!loop_in_bounds(x_outer, y_outer, first_mb, num_mb, mb_width, mb_height); ) { 
            y_inner = y_outer;
            x_inner = x_outer;
            for (; !loop_in_bounds(x_inner, y_inner, first_mb, num_mb, mb_width, mb_height);) {
                mb_intra_ub = 0;
                score_dep = 0;
                if (x_inner != 0) {
                    mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_AE;
                    score_dep |= MB_SCOREBOARD_A; 
                }
                if (y_inner != mb_row) {
                    mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_B;
                    score_dep |= MB_SCOREBOARD_B;
                    if (x_inner != 0)
                        mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_D;

                    if (x_inner != (mb_width -1)) {
                        mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_C;
                        score_dep |= MB_SCOREBOARD_C;
                    }
                }

                *command_ptr++ = (CMD_MEDIA_OBJECT | (8 - 2));
                *command_ptr++ = kernel;
                *command_ptr++ = USE_SCOREBOARD;
                /* Indirect data */
                *command_ptr++ = 0;
                /* the (X, Y) term of scoreboard */
                *command_ptr++ = ((y_inner << 16) | x_inner);
                *command_ptr++ = score_dep;
                /*inline data */
                *command_ptr++ = (mb_width << 16 | y_inner << 8 | x_inner);
                *command_ptr++ = ((1 << 18) | (1 << 16) | transform_8x8_mode_flag | (mb_intra_ub << 8));

                x_inner -= 2;
                y_inner += 1;
            }
            x_outer++;
            if (x_outer >= mb_width) {
                y_outer += 1;
                x_outer = xtemp_outer;
            }		
        }
    }

    *command_ptr++ = 0;
    *command_ptr++ = MI_BATCH_BUFFER_END;

    dri_bo_unmap(vme_context->vme_batchbuffer.bo);
}

static uint8_t
intel_get_ref_idx_state_1(VAPictureH264 *va_pic, unsigned int frame_store_id)
{
    unsigned int is_long_term =
        !!(va_pic->flags & VA_PICTURE_H264_LONG_TERM_REFERENCE);
    unsigned int is_top_field =
        !!(va_pic->flags & VA_PICTURE_H264_TOP_FIELD);
    unsigned int is_bottom_field =
        !!(va_pic->flags & VA_PICTURE_H264_BOTTOM_FIELD);

    return ((is_long_term                         << 6) |
            ((is_top_field ^ is_bottom_field ^ 1) << 5) |
            (frame_store_id                       << 1) |
            ((is_top_field ^ 1) & is_bottom_field));
}

void
intel_mfc_avc_ref_idx_state(VADriverContextP ctx,
                            struct encode_state *encode_state,
                            struct intel_encoder_context *encoder_context)
{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    struct intel_batchbuffer *batch = encoder_context->base.batch;
    int slice_type;
    struct object_surface *obj_surface;
    unsigned int fref_entry, bref_entry;
    int frame_index, i;
    VAEncSliceParameterBufferH264 *slice_param = (VAEncSliceParameterBufferH264 *)encode_state->slice_params_ext[0]->buffer;

    fref_entry = 0x80808080;
    bref_entry = 0x80808080;
    slice_type = intel_avc_enc_slice_type_fixup(slice_param->slice_type);

    if (slice_type == SLICE_TYPE_P || slice_type == SLICE_TYPE_B) {
        int ref_idx_l0 = (vme_context->ref_index_in_mb[0] & 0xff);

        if (ref_idx_l0 > 3) {
            WARN_ONCE("ref_idx_l0 is out of range\n");
            ref_idx_l0 = 0;
        }

        obj_surface = vme_context->used_reference_objects[0];
        frame_index = -1;
        for (i = 0; i < 16; i++) {
            if (obj_surface &&
                obj_surface == encode_state->reference_objects[i]) {
                frame_index = i;
                break;
            }
        }
        if (frame_index == -1) {
            WARN_ONCE("RefPicList0 is not found in DPB!\n");
        } else {
            int ref_idx_l0_shift = ref_idx_l0 * 8;
            fref_entry &= ~(0xFF << ref_idx_l0_shift);
            fref_entry += (intel_get_ref_idx_state_1(vme_context->used_references[0], frame_index) << ref_idx_l0_shift);
        }
        if(frame_index == 1){
            WARN_ONCE("Input ref list is Wrong !\n");
        }
    }

    if (slice_type == SLICE_TYPE_B) {
        int ref_idx_l1 = (vme_context->ref_index_in_mb[1] & 0xff);

        if (ref_idx_l1 > 3) {
            WARN_ONCE("ref_idx_l1 is out of range\n");
            ref_idx_l1 = 0;
        }

        obj_surface = vme_context->used_reference_objects[1];
        frame_index = -1;
        for (i = 0; i < 16; i++) {
            if (obj_surface &&
                obj_surface == encode_state->reference_objects[i]) {
                frame_index = i;
                break;
            }
        }
        if (frame_index == -1) {
            WARN_ONCE("RefPicList1 is not found in DPB!\n");
        } else {
            int ref_idx_l1_shift = ref_idx_l1 * 8;
            bref_entry &= ~(0xFF << ref_idx_l1_shift);
            bref_entry += (intel_get_ref_idx_state_1(vme_context->used_references[1], frame_index) << ref_idx_l1_shift);
        }
    }

    BEGIN_BCS_BATCH(batch, 10);
    OUT_BCS_BATCH(batch, MFX_AVC_REF_IDX_STATE | 8);
    OUT_BCS_BATCH(batch, 0);                  //Select L0
    OUT_BCS_BATCH(batch, fref_entry);         //Only 1 reference
    for(i = 0; i < 7; i++) {
        OUT_BCS_BATCH(batch, 0x80808080);
    }
    ADVANCE_BCS_BATCH(batch);

    BEGIN_BCS_BATCH(batch, 10);
    OUT_BCS_BATCH(batch, MFX_AVC_REF_IDX_STATE | 8);
    OUT_BCS_BATCH(batch, 1);                  //Select L1
    OUT_BCS_BATCH(batch, bref_entry);         //Only 1 reference
    for(i = 0; i < 7; i++) {
        OUT_BCS_BATCH(batch, 0x80808080);
    }
    ADVANCE_BCS_BATCH(batch);
}


void intel_vme_mpeg2_state_setup(VADriverContextP ctx,
                                 struct encode_state *encode_state,
                                 struct intel_encoder_context *encoder_context)
{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    uint32_t *vme_state_message = (uint32_t *)(vme_context->vme_state_message);
    VAEncSequenceParameterBufferMPEG2 *seq_param = (VAEncSequenceParameterBufferMPEG2 *)encode_state->seq_param_ext->buffer;
    int width_in_mbs = ALIGN(seq_param->picture_width, 16) / 16;
    int height_in_mbs = ALIGN(seq_param->picture_height, 16) / 16;
    uint32_t mv_x, mv_y;
    VAEncSliceParameterBufferMPEG2 *slice_param = NULL;
    VAEncPictureParameterBufferMPEG2 *pic_param = NULL;
    slice_param = (VAEncSliceParameterBufferMPEG2 *)encode_state->slice_params_ext[0]->buffer;

    if (vme_context->mpeg2_level == MPEG2_LEVEL_LOW) {
        mv_x = 512;
        mv_y = 64;
    } else if (vme_context->mpeg2_level == MPEG2_LEVEL_MAIN) {
        mv_x = 1024;
        mv_y = 128;
    } else if (vme_context->mpeg2_level == MPEG2_LEVEL_HIGH) {
        mv_x = 2048;
        mv_y = 128;
    } else {
        WARN_ONCE("Incorrect Mpeg2 level setting!\n");
        mv_x = 512;
        mv_y = 64;
    }

    pic_param = (VAEncPictureParameterBufferMPEG2 *)encode_state->pic_param_ext->buffer;
    if (pic_param->picture_type != VAEncPictureTypeIntra) {
        int qp, m_cost, j, mv_count;
        float   lambda, m_costf;
        slice_param = (VAEncSliceParameterBufferMPEG2 *)
            encode_state->slice_params_ext[0]->buffer;
        qp = slice_param->quantiser_scale_code;
        lambda = intel_lambda_qp(qp);
        /* No Intra prediction. So it is zero */
        vme_state_message[MODE_INTRA_8X8] = 0;
        vme_state_message[MODE_INTRA_4X4] = 0;
        vme_state_message[MODE_INTER_MV0] = 0;
        for (j = 1; j < 3; j++) {
            m_costf = (log2f((float)(j + 1)) + 1.718f) * lambda;
            m_cost = (int)m_costf;
            vme_state_message[MODE_INTER_MV0 + j] = intel_format_lutvalue(m_cost, 0x6f);
        }
        mv_count = 3;
        for (j = 4; j <= 64; j *= 2) {
            m_costf = (log2f((float)(j + 1)) + 1.718f) * lambda;
            m_cost = (int)m_costf;
            vme_state_message[MODE_INTER_MV0 + mv_count] =
                intel_format_lutvalue(m_cost, 0x6f);
            mv_count++;
        }
        m_cost = lambda;
        /* It can only perform the 16x16 search. So mode cost can be ignored for
         * the other mode. for example: 16x8/8x8
         */
        vme_state_message[MODE_INTRA_16X16] = intel_format_lutvalue(m_cost, 0x8f);
        vme_state_message[MODE_INTER_16X16] = intel_format_lutvalue(m_cost, 0x8f);

        vme_state_message[MODE_INTER_16X8] = 0;
        vme_state_message[MODE_INTER_8X8] = 0;
        vme_state_message[MODE_INTER_8X4] = 0;
        vme_state_message[MODE_INTER_4X4] = 0;
        vme_state_message[MODE_INTER_BWD] = intel_format_lutvalue(m_cost, 0x6f);

    }
    vme_state_message[MPEG2_MV_RANGE] = (mv_y << 16) | (mv_x);

    vme_state_message[MPEG2_PIC_WIDTH_HEIGHT] = (height_in_mbs << 16) |
        width_in_mbs;
}

void
gen7_vme_mpeg2_walker_fill_vme_batchbuffer(VADriverContextP ctx, 
                                           struct encode_state *encode_state,
                                           int mb_width, int mb_height,
                                           int kernel,
                                           struct intel_encoder_context *encoder_context)
{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    unsigned int *command_ptr;

#define		MPEG2_SCOREBOARD		(1 << 21)

    dri_bo_map(vme_context->vme_batchbuffer.bo, 1);
    command_ptr = vme_context->vme_batchbuffer.bo->virtual;

    {
        unsigned int mb_intra_ub, score_dep;
        int x_outer, y_outer, x_inner, y_inner;
        int xtemp_outer = 0;
        int first_mb = 0;
        int num_mb = mb_width * mb_height;

        x_outer = 0;
        y_outer = 0;


        for (; x_outer < (mb_width -2 ) && !loop_in_bounds(x_outer, y_outer, first_mb, num_mb, mb_width, mb_height); ) {
            x_inner = x_outer;
            y_inner = y_outer;
            for (; !loop_in_bounds(x_inner, y_inner, first_mb, num_mb, mb_width, mb_height);) {
                mb_intra_ub = 0;
                score_dep = 0;
                if (x_inner != 0) {
                    mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_AE;
                    score_dep |= MB_SCOREBOARD_A; 
                }
                if (y_inner != 0) {
                    mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_B;
                    score_dep |= MB_SCOREBOARD_B;

                    if (x_inner != 0)
                        mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_D;

                    if (x_inner != (mb_width -1)) {
                        mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_C;
                        score_dep |= MB_SCOREBOARD_C;
                    }
                }

                *command_ptr++ = (CMD_MEDIA_OBJECT | (8 - 2));
                *command_ptr++ = kernel;
                *command_ptr++ = MPEG2_SCOREBOARD;
                /* Indirect data */
                *command_ptr++ = 0;
                /* the (X, Y) term of scoreboard */
                *command_ptr++ = ((y_inner << 16) | x_inner);
                *command_ptr++ = score_dep;
                /*inline data */
                *command_ptr++ = (mb_width << 16 | y_inner << 8 | x_inner);
                *command_ptr++ = ((1 << 18) | (1 << 16) | (mb_intra_ub << 8));
                x_inner -= 2;
                y_inner += 1;
            }
            x_outer += 1;
        }

        xtemp_outer = mb_width - 2;
        if (xtemp_outer < 0)
            xtemp_outer = 0;
        x_outer = xtemp_outer;
        y_outer = 0;
        for (;!loop_in_bounds(x_outer, y_outer, first_mb, num_mb, mb_width, mb_height); ) { 
            y_inner = y_outer;
            x_inner = x_outer;
            for (; !loop_in_bounds(x_inner, y_inner, first_mb, num_mb, mb_width, mb_height);) {
                mb_intra_ub = 0;
                score_dep = 0;
                if (x_inner != 0) {
                    mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_AE;
                    score_dep |= MB_SCOREBOARD_A; 
                }
                if (y_inner != 0) {
                    mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_B;
                    score_dep |= MB_SCOREBOARD_B;

                    if (x_inner != 0)
                        mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_D;

                    if (x_inner != (mb_width -1)) {
                        mb_intra_ub |= INTRA_PRED_AVAIL_FLAG_C;
                        score_dep |= MB_SCOREBOARD_C;
                    }
                }

                *command_ptr++ = (CMD_MEDIA_OBJECT | (8 - 2));
                *command_ptr++ = kernel;
                *command_ptr++ = MPEG2_SCOREBOARD;
                /* Indirect data */
                *command_ptr++ = 0;
                /* the (X, Y) term of scoreboard */
                *command_ptr++ = ((y_inner << 16) | x_inner);
                *command_ptr++ = score_dep;
                /*inline data */
                *command_ptr++ = (mb_width << 16 | y_inner << 8 | x_inner);
                *command_ptr++ = ((1 << 18) | (1 << 16) | (mb_intra_ub << 8));

                x_inner -= 2;
                y_inner += 1;
            }
            x_outer++;
            if (x_outer >= mb_width) {
                y_outer += 1;
                x_outer = xtemp_outer;
            }		
        }
    }

    *command_ptr++ = 0;
    *command_ptr++ = MI_BATCH_BUFFER_END;

    dri_bo_unmap(vme_context->vme_batchbuffer.bo);
    return;
}

static int
avc_temporal_find_surface(VAPictureH264 *curr_pic,
                          VAPictureH264 *ref_list,
                          int num_pictures,
                          int dir)
{
    int i, found = -1, min = 0x7FFFFFFF;

    for (i = 0; i < num_pictures; i++) {
        int tmp;

        if ((ref_list[i].flags & VA_PICTURE_H264_INVALID) ||
            (ref_list[i].picture_id == VA_INVALID_SURFACE))
            break;

        tmp = curr_pic->TopFieldOrderCnt - ref_list[i].TopFieldOrderCnt;

        if (dir)
            tmp = -tmp;

        if (tmp > 0 && tmp < min) {
            min = tmp;
            found = i;
        }
    }

    return found;
}

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
                                  struct intel_encoder_context *encoder_context))
{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    struct object_surface *obj_surface = NULL;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    VASurfaceID ref_surface_id;
    VAEncPictureParameterBufferH264 *pic_param = (VAEncPictureParameterBufferH264 *)encode_state->pic_param_ext->buffer;
    VAEncSliceParameterBufferH264 *slice_param = (VAEncSliceParameterBufferH264 *)encode_state->slice_params_ext[0]->buffer;
    int max_num_references;
    VAPictureH264 *curr_pic;
    VAPictureH264 *ref_list;
    int ref_idx;

    if (list_index == 0) {
        max_num_references = pic_param->num_ref_idx_l0_active_minus1 + 1;
        ref_list = slice_param->RefPicList0;
    } else {
        max_num_references = pic_param->num_ref_idx_l1_active_minus1 + 1;
        ref_list = slice_param->RefPicList1;
    }

    if (max_num_references == 1) {
        if (list_index == 0) {
            ref_surface_id = slice_param->RefPicList0[0].picture_id;
            vme_context->used_references[0] = &slice_param->RefPicList0[0];
        } else {
            ref_surface_id = slice_param->RefPicList1[0].picture_id;
            vme_context->used_references[1] = &slice_param->RefPicList1[0];
        }

        if (ref_surface_id != VA_INVALID_SURFACE)
            obj_surface = SURFACE(ref_surface_id);

        if (!obj_surface ||
            !obj_surface->bo) {
            obj_surface = encode_state->reference_objects[list_index];
            vme_context->used_references[list_index] = &pic_param->ReferenceFrames[list_index];
        }

        ref_idx = 0;
    } else {
        curr_pic = &pic_param->CurrPic;

        /* select the reference frame in temporal space */
        ref_idx = avc_temporal_find_surface(curr_pic, ref_list, max_num_references, list_index == 1);
        ref_surface_id = ref_list[ref_idx].picture_id;

        if (ref_surface_id != VA_INVALID_SURFACE) /* otherwise warning later */
            obj_surface = SURFACE(ref_surface_id);

        vme_context->used_reference_objects[list_index] = obj_surface;
        vme_context->used_references[list_index] = &ref_list[ref_idx];
    }

    if (obj_surface &&
        obj_surface->bo) {
        assert(ref_idx >= 0);
        vme_context->used_reference_objects[list_index] = obj_surface;
        vme_source_surface_state(ctx, surface_index, obj_surface, encoder_context);
        vme_context->ref_index_in_mb[list_index] = (ref_idx << 24 |
                                                    ref_idx << 16 |
                                                    ref_idx <<  8 |
                                                    ref_idx);
    } else {
        vme_context->used_reference_objects[list_index] = NULL;
        vme_context->used_references[list_index] = NULL;
        vme_context->ref_index_in_mb[list_index] = 0;
    }
}

void intel_avc_slice_insert_packed_data(VADriverContextP ctx,
                                        struct encode_state *encode_state,
                                        struct intel_encoder_context *encoder_context,
                                        int slice_index,
                                        struct intel_batchbuffer *slice_batch)
{
    int count, i, start_index;
    unsigned int length_in_bits;
    VAEncPackedHeaderParameterBuffer *param = NULL;
    unsigned int *header_data = NULL;
    struct gen6_mfc_context *mfc_context = encoder_context->mfc_context;
    int slice_header_index;

    if (encode_state->slice_header_index[slice_index] == 0)
        slice_header_index = -1;
    else
        slice_header_index = (encode_state->slice_header_index[slice_index] & SLICE_PACKED_DATA_INDEX_MASK);

    count = encode_state->slice_rawdata_count[slice_index];
    start_index = (encode_state->slice_rawdata_index[slice_index] & SLICE_PACKED_DATA_INDEX_MASK);

    for (i = 0; i < count; i++) {
        unsigned int skip_emul_byte_cnt;

        header_data = (unsigned int *)encode_state->packed_header_data_ext[start_index + i]->buffer;

        param = (VAEncPackedHeaderParameterBuffer *)
                    (encode_state->packed_header_params_ext[start_index + i]->buffer);

        /* skip the slice header packed data type as it is lastly inserted */
        if (param->type == VAEncPackedHeaderSlice)
            continue;

        length_in_bits = param->bit_length;

        skip_emul_byte_cnt = intel_avc_find_skipemulcnt((unsigned char *)header_data, length_in_bits);

        /* as the slice header is still required, the last header flag is set to
         * zero.
         */
        mfc_context->insert_object(ctx,
                                   encoder_context,
                                   header_data,
                                   ALIGN(length_in_bits, 32) >> 5,
                                   length_in_bits & 0x1f,
                                   skip_emul_byte_cnt,
                                   0,
                                   0,
                                   !param->has_emulation_bytes,
                                   slice_batch);
    }

    if (slice_header_index == -1) {
        unsigned char *slice_header = NULL;
        int slice_header_length_in_bits = 0;
        VAEncSequenceParameterBufferH264 *pSequenceParameter = (VAEncSequenceParameterBufferH264 *)encode_state->seq_param_ext->buffer;
        VAEncPictureParameterBufferH264 *pPicParameter = (VAEncPictureParameterBufferH264 *)encode_state->pic_param_ext->buffer;
        VAEncSliceParameterBufferH264 *pSliceParameter = (VAEncSliceParameterBufferH264 *)encode_state->slice_params_ext[slice_index]->buffer;

        /* No slice header data is passed. And the driver needs to generate it */
        /* For the Normal H264 */
        slice_header_length_in_bits = build_avc_slice_header(pSequenceParameter,
                                                             pPicParameter,
                                                             pSliceParameter,
                                                             &slice_header);
        mfc_context->insert_object(ctx, encoder_context,
                                   (unsigned int *)slice_header,
                                   ALIGN(slice_header_length_in_bits, 32) >> 5,
                                   slice_header_length_in_bits & 0x1f,
                                   5,  /* first 5 bytes are start code + nal unit type */
                                   1, 0, 1, slice_batch);

        free(slice_header);
    } else {
        unsigned int skip_emul_byte_cnt;

        header_data = (unsigned int *)encode_state->packed_header_data_ext[slice_header_index]->buffer;

        param = (VAEncPackedHeaderParameterBuffer *)
                    (encode_state->packed_header_params_ext[slice_header_index]->buffer);
        length_in_bits = param->bit_length;

        /* as the slice header is the last header data for one slice,
         * the last header flag is set to one.
         */
        skip_emul_byte_cnt = intel_avc_find_skipemulcnt((unsigned char *)header_data, length_in_bits);

        mfc_context->insert_object(ctx,
                                   encoder_context,
                                   header_data,
                                   ALIGN(length_in_bits, 32) >> 5,
                                   length_in_bits & 0x1f,
                                   skip_emul_byte_cnt,
                                   1,
                                   0,
                                   !param->has_emulation_bytes,
                                   slice_batch);
    }

    return;
}

/* HEVC */
static int
hevc_temporal_find_surface(VAPictureHEVC *curr_pic,
                           VAPictureHEVC *ref_list,
                           int num_pictures,
                           int dir)
{
    int i, found = -1, min = 0x7FFFFFFF;

    for (i = 0; i < num_pictures; i++) {
        int tmp;

        if ((ref_list[i].flags & VA_PICTURE_HEVC_INVALID) ||
            (ref_list[i].picture_id == VA_INVALID_SURFACE))
            break;

        tmp = curr_pic->pic_order_cnt - ref_list[i].pic_order_cnt;

        if (dir)
            tmp = -tmp;

        if (tmp > 0 && tmp < min) {
            min = tmp;
            found = i;
        }
    }

    return found;
}
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
                                   struct intel_encoder_context *encoder_context))
{
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    struct object_surface *obj_surface = NULL;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    VASurfaceID ref_surface_id;
    VAEncPictureParameterBufferHEVC *pic_param = (VAEncPictureParameterBufferHEVC *)encode_state->pic_param_ext->buffer;
    VAEncSliceParameterBufferHEVC *slice_param = (VAEncSliceParameterBufferHEVC *)encode_state->slice_params_ext[0]->buffer;
    int max_num_references;
    VAPictureHEVC *curr_pic;
    VAPictureHEVC *ref_list;
    int ref_idx;

    if (list_index == 0) {
        max_num_references = pic_param->num_ref_idx_l0_default_active_minus1 + 1;
        ref_list = slice_param->ref_pic_list0;
    } else {
        max_num_references = pic_param->num_ref_idx_l1_default_active_minus1 + 1;
        ref_list = slice_param->ref_pic_list1;
    }

    if (max_num_references == 1) {
        if (list_index == 0) {
            ref_surface_id = slice_param->ref_pic_list0[0].picture_id;
            vme_context->used_references[0] = &slice_param->ref_pic_list0[0];
        } else {
            ref_surface_id = slice_param->ref_pic_list1[0].picture_id;
            vme_context->used_references[1] = &slice_param->ref_pic_list1[0];
        }

        if (ref_surface_id != VA_INVALID_SURFACE)
            obj_surface = SURFACE(ref_surface_id);

        if (!obj_surface ||
            !obj_surface->bo) {
            obj_surface = encode_state->reference_objects[list_index];
            vme_context->used_references[list_index] = &pic_param->reference_frames[list_index];
        }

        ref_idx = 0;
    } else {
        curr_pic = &pic_param->decoded_curr_pic;

        /* select the reference frame in temporal space */
        ref_idx = hevc_temporal_find_surface(curr_pic, ref_list, max_num_references, list_index == 1);
        ref_surface_id = ref_list[ref_idx].picture_id;

        if (ref_surface_id != VA_INVALID_SURFACE) /* otherwise warning later */
            obj_surface = SURFACE(ref_surface_id);

        vme_context->used_reference_objects[list_index] = obj_surface;
        vme_context->used_references[list_index] = &ref_list[ref_idx];
    }

    if (obj_surface &&
        obj_surface->bo) {
        assert(ref_idx >= 0);
        vme_context->used_reference_objects[list_index] = obj_surface;
        vme_source_surface_state(ctx, surface_index, obj_surface, encoder_context);
        vme_context->ref_index_in_mb[list_index] = (ref_idx << 24 |
                ref_idx << 16 |
                ref_idx <<  8 |
                ref_idx);
    } else {
        vme_context->used_reference_objects[list_index] = NULL;
        vme_context->used_references[list_index] = NULL;
        vme_context->ref_index_in_mb[list_index] = 0;
    }
}

void intel_vme_hevc_update_mbmv_cost(VADriverContextP ctx,
                                     struct encode_state *encode_state,
                                     struct intel_encoder_context *encoder_context)
{
    struct gen9_hcpe_context *mfc_context = encoder_context->mfc_context;
    struct gen6_vme_context *vme_context = encoder_context->vme_context;
    VAEncPictureParameterBufferHEVC *pic_param = (VAEncPictureParameterBufferHEVC *)encode_state->pic_param_ext->buffer;
    VAEncSliceParameterBufferHEVC *slice_param = (VAEncSliceParameterBufferHEVC *)encode_state->slice_params_ext[0]->buffer;
    VAEncSequenceParameterBufferHEVC *pSequenceParameter = (VAEncSequenceParameterBufferHEVC *)encode_state->seq_param_ext->buffer;
    int qp, m_cost, j, mv_count;
    uint8_t *vme_state_message = (uint8_t *)(vme_context->vme_state_message);
    float   lambda, m_costf;

    /* here no SI SP slice for HEVC, do not need slice fixup */
    int slice_type = slice_param->slice_type;


    qp = pic_param->pic_init_qp + slice_param->slice_qp_delta;

    if(encoder_context->rate_control_mode == VA_RC_CBR)
    {
        qp = mfc_context->bit_rate_control_context[slice_type].QpPrimeY;
        if(slice_type == HEVC_SLICE_B) {
            if(pSequenceParameter->ip_period == 1)
            {
                slice_type = HEVC_SLICE_P;
                qp = mfc_context->bit_rate_control_context[HEVC_SLICE_P].QpPrimeY;

            }else if(mfc_context->vui_hrd.i_frame_number % pSequenceParameter->ip_period == 1){
                slice_type = HEVC_SLICE_P;
                qp = mfc_context->bit_rate_control_context[HEVC_SLICE_P].QpPrimeY;
            }
        }

    }

    if (vme_state_message == NULL)
        return;

    assert(qp <= QP_MAX);
    lambda = intel_lambda_qp(qp);
    if (slice_type == HEVC_SLICE_I) {
        vme_state_message[MODE_INTRA_16X16] = 0;
        m_cost = lambda * 4;
        vme_state_message[MODE_INTRA_8X8] = intel_format_lutvalue(m_cost, 0x8f);
        m_cost = lambda * 16;
        vme_state_message[MODE_INTRA_4X4] = intel_format_lutvalue(m_cost, 0x8f);
        m_cost = lambda * 3;
        vme_state_message[MODE_INTRA_NONPRED] = intel_format_lutvalue(m_cost, 0x6f);
    } else {
        m_cost = 0;
        vme_state_message[MODE_INTER_MV0] = intel_format_lutvalue(m_cost, 0x6f);
        for (j = 1; j < 3; j++) {
            m_costf = (log2f((float)(j + 1)) + 1.718f) * lambda;
            m_cost = (int)m_costf;
            vme_state_message[MODE_INTER_MV0 + j] = intel_format_lutvalue(m_cost, 0x6f);
        }
        mv_count = 3;
        for (j = 4; j <= 64; j *= 2) {
            m_costf = (log2f((float)(j + 1)) + 1.718f) * lambda;
            m_cost = (int)m_costf;
            vme_state_message[MODE_INTER_MV0 + mv_count] = intel_format_lutvalue(m_cost, 0x6f);
            mv_count++;
        }

        if (qp <= 25) {
            vme_state_message[MODE_INTRA_16X16] = 0x4a;
            vme_state_message[MODE_INTRA_8X8] = 0x4a;
            vme_state_message[MODE_INTRA_4X4] = 0x4a;
            vme_state_message[MODE_INTRA_NONPRED] = 0x4a;
            vme_state_message[MODE_INTER_16X16] = 0x4a;
            vme_state_message[MODE_INTER_16X8] = 0x4a;
            vme_state_message[MODE_INTER_8X8] = 0x4a;
            vme_state_message[MODE_INTER_8X4] = 0x4a;
            vme_state_message[MODE_INTER_4X4] = 0x4a;
            vme_state_message[MODE_INTER_BWD] = 0x2a;
            return;
        }
        m_costf = lambda * 10;
        vme_state_message[MODE_INTRA_16X16] = intel_format_lutvalue(m_cost, 0x8f);
        m_cost = lambda * 14;
        vme_state_message[MODE_INTRA_8X8] = intel_format_lutvalue(m_cost, 0x8f);
        m_cost = lambda * 24;
        vme_state_message[MODE_INTRA_4X4] = intel_format_lutvalue(m_cost, 0x8f);
        m_costf = lambda * 3.5;
        m_cost = m_costf;
        vme_state_message[MODE_INTRA_NONPRED] = intel_format_lutvalue(m_cost, 0x6f);
        if (slice_type == HEVC_SLICE_P) {
            m_costf = lambda * 2.5;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_16X16] = intel_format_lutvalue(m_cost, 0x8f);
            m_costf = lambda * 4;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_16X8] = intel_format_lutvalue(m_cost, 0x8f);
            m_costf = lambda * 1.5;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_8X8] = intel_format_lutvalue(m_cost, 0x6f);
            m_costf = lambda * 3;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_8X4] = intel_format_lutvalue(m_cost, 0x6f);
            m_costf = lambda * 5;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_4X4] = intel_format_lutvalue(m_cost, 0x6f);
            /* BWD is not used in P-frame */
            vme_state_message[MODE_INTER_BWD] = 0;
        } else {
            m_costf = lambda * 2.5;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_16X16] = intel_format_lutvalue(m_cost, 0x8f);
            m_costf = lambda * 5.5;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_16X8] = intel_format_lutvalue(m_cost, 0x8f);
            m_costf = lambda * 3.5;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_8X8] = intel_format_lutvalue(m_cost, 0x6f);
            m_costf = lambda * 5.0;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_8X4] = intel_format_lutvalue(m_cost, 0x6f);
            m_costf = lambda * 6.5;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_4X4] = intel_format_lutvalue(m_cost, 0x6f);
            m_costf = lambda * 1.5;
            m_cost = m_costf;
            vme_state_message[MODE_INTER_BWD] = intel_format_lutvalue(m_cost, 0x6f);
        }
    }
}
