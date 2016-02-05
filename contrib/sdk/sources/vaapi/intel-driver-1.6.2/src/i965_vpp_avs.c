/*
 * i965_vpp_avs.c - Adaptive Video Scaler (AVS) block
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

#include "sysdeps.h"
#include <math.h>
#include <va/va.h>
#include "i965_vpp_avs.h"

typedef void (*AVSGenCoeffsFunc)(float *coeffs, int num_coeffs, int phase,
    int num_phases, float f);

/* Initializes all coefficients to zero */
static void
avs_init_coeffs(float *coeffs, int num_coeffs)
{
#if defined(__STDC_IEC_559__) && (__STDC_IEC_559__ > 0)
    memset(coeffs, 0, num_coeffs * sizeof(*coeffs));
#else
    int i;

    for (i = 0; i < num_coeffs; i++)
        coeffs[i] = 0.0f;
#endif
}

/* Computes the sinc(x) function */
static float
avs_sinc(float x)
{
    if (x == 0.0f)
        return 1.0f;
    return sin(x * M_PI) / (x * M_PI);
}

/* Convolution kernel for linear interpolation */
static float
avs_kernel_linear(float x)
{
    const float abs_x = fabsf(x);

    return abs_x < 1.0f ? 1 - abs_x : 0.0f;
}

/* Convolution kernel for Lanczos-based interpolation */
static float
avs_kernel_lanczos(float x, float a)
{
    const float abs_x = fabsf(x);

    return abs_x < a ? avs_sinc(x) * avs_sinc(x / a) : 0.0f;
}

/* Truncates floating-point value towards an epsilon factor */
static inline float
avs_trunc_coeff(float x, float epsilon)
{
    return rintf(x / epsilon) * epsilon;
}

/* Normalize coefficients for one sample/direction */
static void
avs_normalize_coeffs_1(float *coeffs, int num_coeffs, float epsilon)
{
    float s, sum = 0.0;
    int i, c, r, r1;

    for (i = 0; i < num_coeffs; i++)
        sum += coeffs[i];

    if (sum < epsilon)
        return;

    s = 0.0;
    for (i = 0; i < num_coeffs; i++)
        s += (coeffs[i] = avs_trunc_coeff(coeffs[i] / sum, epsilon));

    /* Distribute the remaining bits, while allocating more to the center */
    c = num_coeffs/2;
    c = c - (coeffs[c - 1] > coeffs[c]);

    r = (1.0f - s) / epsilon;
    r1 = r / 4;
    if (coeffs[c + 1] == 0.0f)
        coeffs[c] += r * epsilon;
    else {
        coeffs[c] += (r - 2*r1) * epsilon;
        coeffs[c - 1] += r1 * epsilon;
        coeffs[c + 1] += r1 * epsilon;
    }
}

/* Normalize all coefficients so that their sum yields 1.0f */
static void
avs_normalize_coeffs(AVSCoeffs *coeffs, const AVSConfig *config)
{
    avs_normalize_coeffs_1(coeffs->y_k_h, config->num_luma_coeffs,
        config->coeff_epsilon);
    avs_normalize_coeffs_1(coeffs->y_k_v, config->num_luma_coeffs,
        config->coeff_epsilon);
    avs_normalize_coeffs_1(coeffs->uv_k_h, config->num_chroma_coeffs,
        config->coeff_epsilon);
    avs_normalize_coeffs_1(coeffs->uv_k_v, config->num_chroma_coeffs,
        config->coeff_epsilon);
}

/* Validate coefficients for one sample/direction */
static bool
avs_validate_coeffs_1(float *coeffs, int num_coeffs, const float *min_coeffs,
    const float *max_coeffs)
{
    int i;

    for (i = 0; i < num_coeffs; i++) {
        if (coeffs[i] < min_coeffs[i] || coeffs[i] > max_coeffs[i])
            return false;
    }
    return true;
}

/* Validate coefficients wrt. the supplied range in config */
static bool
avs_validate_coeffs(AVSCoeffs *coeffs, const AVSConfig *config)
{
    const AVSCoeffs * const min_coeffs = &config->coeff_range.lower_bound;
    const AVSCoeffs * const max_coeffs = &config->coeff_range.upper_bound;

    return avs_validate_coeffs_1(coeffs->y_k_h, config->num_luma_coeffs,
            min_coeffs->y_k_h, max_coeffs->y_k_h) &&
        avs_validate_coeffs_1(coeffs->y_k_v, config->num_luma_coeffs,
            min_coeffs->y_k_v, max_coeffs->y_k_v) &&
        avs_validate_coeffs_1(coeffs->uv_k_h, config->num_chroma_coeffs,
            min_coeffs->uv_k_h, max_coeffs->uv_k_h) &&
        avs_validate_coeffs_1(coeffs->uv_k_v, config->num_chroma_coeffs,
            min_coeffs->uv_k_v, max_coeffs->uv_k_v);
}

/* Generate coefficients for default quality (bilinear) */
static void
avs_gen_coeffs_linear(float *coeffs, int num_coeffs, int phase, int num_phases,
    float f)
{
    const int c = num_coeffs/2 - 1;
    const float p = (float)phase / (num_phases*2);

    avs_init_coeffs(coeffs, num_coeffs);
    coeffs[c] = avs_kernel_linear(p);
    coeffs[c + 1] = avs_kernel_linear(p - 1);
}

/* Generate coefficients for high quality (lanczos) */
static void
avs_gen_coeffs_lanczos(float *coeffs, int num_coeffs, int phase, int num_phases,
    float f)
{
    const int c = num_coeffs/2 - 1;
    const int l = num_coeffs > 4 ? 3 : 2;
    const float p = (float)phase / (num_phases*2);
    int i;

    if (f > 1.0f)
        f = 1.0f;
    for (i = 0; i < num_coeffs; i++)
        coeffs[i] = avs_kernel_lanczos((i - (c + p)) * f, l);
}

/* Generate coefficients with the supplied scaler */
static bool
avs_gen_coeffs(AVSState *avs, float sx, float sy, AVSGenCoeffsFunc gen_coeffs)
{
    const AVSConfig * const config = avs->config;
    int i;

    for (i = 0; i <= config->num_phases; i++) {
        AVSCoeffs * const coeffs = &avs->coeffs[i];

        gen_coeffs(coeffs->y_k_h, config->num_luma_coeffs,
            i, config->num_phases, sx);
        gen_coeffs(coeffs->uv_k_h, config->num_chroma_coeffs,
            i, config->num_phases, sx);
        gen_coeffs(coeffs->y_k_v, config->num_luma_coeffs,
            i, config->num_phases, sy);
        gen_coeffs(coeffs->uv_k_v, config->num_chroma_coeffs,
            i, config->num_phases, sy);

        avs_normalize_coeffs(coeffs, config);
        if (!avs_validate_coeffs(coeffs, config))
            return false;
    }
    return true;
}

/* Initializes AVS state with the supplied configuration */
void
avs_init_state(AVSState *avs, const AVSConfig *config)
{
    avs->config = config;
    avs->flags = 0;
    avs->scale_x = 0.0f;
    avs->scale_y = 0.0f;
}

/* Checks whether the AVS scaling parameters changed */
static inline bool
avs_params_changed(AVSState *avs, float sx, float sy, uint32_t flags)
{
    if (avs->flags != flags)
        return true;

    if (flags >= VA_FILTER_SCALING_HQ) {
        if (avs->scale_x != sx || avs->scale_y != sy)
            return true;
    }
    else {
        if (avs->scale_x == 0.0f || avs->scale_y == 0.0f)
            return true;
    }
    return false;
}

/* Updates AVS coefficients for the supplied factors and quality level */
bool
avs_update_coefficients(AVSState *avs, float sx, float sy, uint32_t flags)
{
    AVSGenCoeffsFunc gen_coeffs;

    flags &= VA_FILTER_SCALING_MASK;
    if (!avs_params_changed(avs, sx, sy, flags))
        return true;

    switch (flags) {
    case VA_FILTER_SCALING_HQ:
        gen_coeffs = avs_gen_coeffs_lanczos;
        break;
    default:
        gen_coeffs = avs_gen_coeffs_linear;
        break;
    }
    if (!avs_gen_coeffs(avs, sx, sy, gen_coeffs)) {
        assert(0 && "invalid set of coefficients generated");
        return false;
    }

    avs->flags = flags;
    avs->scale_x = sx;
    avs->scale_y = sy;
    return true;
}
