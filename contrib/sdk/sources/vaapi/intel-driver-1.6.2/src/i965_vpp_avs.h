/*
 * i965_vpp_avs.h - Adaptive Video Scaler (AVS) block
 *
 * Copyright (C) 2014 Intel Corporation
 *   Author: Gwenole Beauchesne <gwenole.beauchesne@intel.com>
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
 */

#ifndef I965_VPP_AVS_H
#define I965_VPP_AVS_H

#include <stdint.h>
#include <stdbool.h>

/** Maximum number of phases for the sharp filter */
#define AVS_MAX_PHASES 32

/** Maximum number of coefficients for luma samples */
#define AVS_MAX_LUMA_COEFFS 8

/** Maximum number of coefficients for chroma samples */
#define AVS_MAX_CHROMA_COEFFS 4

typedef struct avs_coeffs               AVSCoeffs;
typedef struct avs_coeffs_range         AVSCoeffsRange;
typedef struct avs_config               AVSConfig;
typedef struct avs_state                AVSState;

/** AVS coefficients for one phase */
struct avs_coeffs {
    /** Coefficients for luma samples on the X-axis (horizontal) */
    float y_k_h[AVS_MAX_LUMA_COEFFS];
    /** Coefficients for luma samples on the Y-axis (vertical) */
    float y_k_v[AVS_MAX_LUMA_COEFFS];
    /** Coefficients for chroma samples on the X-axis (horizontal) */
    float uv_k_h[AVS_MAX_CHROMA_COEFFS];
    /** Coefficients for chroma samples on the Y-axis (vertical) */
    float uv_k_v[AVS_MAX_CHROMA_COEFFS];
};

/** AVS coefficients range used for validation */
struct avs_coeffs_range {
    /** Lower bound for all coefficients */
    AVSCoeffs lower_bound;
    /** Upper bound for all coefficients */
    AVSCoeffs upper_bound;
};

/** Static configuration (per-generation) */
struct avs_config {
    /** Number of bits used for the fractional part of a coefficient */
    int coeff_frac_bits;
    /** The smallest float that could be represented as a coefficient */
    float coeff_epsilon;
    /** Coefficients range */
    AVSCoeffsRange coeff_range;
    /** Number of phases for the sharp filter */
    int num_phases;
    /** Number of coefficients for luma samples */
    int num_luma_coeffs;
    /** Number of coefficients for chroma samples */
    int num_chroma_coeffs;
};

/** AVS block state */
struct avs_state {
    /** Per-generation configuration parameters */
    const AVSConfig *config;
    /** Scaling flags */
    uint32_t flags;
    /** Scaling factor on the X-axis (horizontal) */
    float scale_x;
    /** Scaling factor on the Y-axis (vertical) */
    float scale_y;
    /** Coefficients for the polyphase scaler */
    AVSCoeffs coeffs[AVS_MAX_PHASES + 1];
};

/** Initializes AVS state with the supplied configuration */
void
avs_init_state(AVSState *avs, const AVSConfig *config);

/** Updates AVS coefficients for the supplied factors and quality level */
bool
avs_update_coefficients(AVSState *avs, float sx, float sy, uint32_t flags);

/** Checks whether AVS is needed, e.g. if high-quality scaling is requested */
static inline bool
avs_is_needed(uint32_t flags)
{
    return ((flags & VA_FILTER_SCALING_MASK) >= VA_FILTER_SCALING_HQ);
}

#endif /* I965_VPP_AVS_H */
