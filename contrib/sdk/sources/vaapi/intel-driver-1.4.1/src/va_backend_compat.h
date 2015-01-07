/*
 * Copyright (C) 2012 Intel Corporation. All Rights Reserved.
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

#ifndef VA_BACKEND_COMPAT_H
#define VA_BACKEND_COMPAT_H

#include <va/va_backend.h>

#if VA_CHECK_VERSION(0,33,0)
# include <va/va_drmcommon.h>

# define VA_CHECK_DRM_AUTH_TYPE(ctx, type) \
    (((struct drm_state *)(ctx)->drm_state)->auth_type == (type))

#else
# include <va/va_dricommon.h>

# define VA_CHECK_DRM_AUTH_TYPE(ctx, type) \
    (((struct dri_state *)(ctx)->dri_state)->driConnectedFlag == (type))

# define drm_state              dri_state
# define VA_DRM_AUTH_DRI1       VA_DRI1
# define VA_DRM_AUTH_DRI2       VA_DRI2
# define VA_DRM_AUTH_CUSTOM     VA_DUMMY
#endif

#if !VA_CHECK_VERSION(0,35,2)
# define VAProfileH264MultiviewHigh     15
# define VAProfileH264StereoHigh        16
#endif

#endif /* VA_BACKEND_COMPAT_H */
