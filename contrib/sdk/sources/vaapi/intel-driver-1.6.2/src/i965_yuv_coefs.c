/*
 * Copyright © 2015 Intel Corporation
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

#include "i965_yuv_coefs.h"

static const float yuv_to_rgb_bt601[] = {
    1.164,		0,	1.596,		-0.06275,
    1.164,		-0.392,	-0.813,		-0.50196,
    1.164,		2.017,	0,		-0.50196,
};

static const float yuv_to_rgb_bt709[] = {
    1.164,		0,	1.793,		-0.06275,
    1.164,		-0.213,	-0.533,		-0.50196,
    1.164,		2.112,	0,		-0.50196,
};

static const float yuv_to_rgb_smpte_240[] = {
    1.164,		0,	1.794,		-0.06275,
    1.164,		-0.258,	-0.5425,	-0.50196,
    1.164,		2.078,	0,		-0.50196,
};

VAProcColorStandardType i915_filter_to_color_standard (unsigned int filter) {
    switch (filter & VA_SRC_COLOR_MASK) {
    case VA_SRC_BT601:
        return VAProcColorStandardBT601;
    case VA_SRC_BT709:
        return VAProcColorStandardBT709;
    case VA_SRC_SMPTE_240:
        return VAProcColorStandardSMPTE240M;
    default:
        return VAProcColorStandardBT601;
    }
}

const float *i915_color_standard_to_coefs (VAProcColorStandardType standard, size_t *length) {
    switch (standard) {
    case VAProcColorStandardBT601:
        *length = sizeof(yuv_to_rgb_bt601);
        return yuv_to_rgb_bt601;
    case VAProcColorStandardBT709:
        *length = sizeof(yuv_to_rgb_bt709);
        return yuv_to_rgb_bt709;
    case VAProcColorStandardSMPTE240M:
        *length = sizeof(yuv_to_rgb_smpte_240);
        return yuv_to_rgb_smpte_240;
    default:
        *length = sizeof(yuv_to_rgb_bt601);
        return yuv_to_rgb_bt601;
    }
}
