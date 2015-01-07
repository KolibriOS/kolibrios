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

#include "sysdeps.h"

#include <va/va_dricommon.h>

#include "i965_drv_video.h"
#include "i965_output_dri.h"
#include "dso_utils.h"

#define LIBVA_X11_NAME "libva-x11.so.1"

typedef struct dri_drawable *(*dri_get_drawable_func)(
    VADriverContextP ctx, XID drawable);
typedef union dri_buffer *(*dri_get_rendering_buffer_func)(
    VADriverContextP ctx, struct dri_drawable *d);
typedef void (*dri_swap_buffer_func)(
    VADriverContextP ctx, struct dri_drawable *d);

struct dri_vtable {
    dri_get_drawable_func               get_drawable;
    dri_get_rendering_buffer_func       get_rendering_buffer;
    dri_swap_buffer_func                swap_buffer;
};

struct va_dri_output {
    struct dso_handle  *handle;
    struct dri_vtable   vtable;
};

bool
i965_output_dri_init(VADriverContextP ctx)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx); 
    struct dso_handle *dso_handle;
    struct dri_vtable *dri_vtable;

    static const struct dso_symbol symbols[] = {
        { "dri_get_drawable",
          offsetof(struct dri_vtable, get_drawable) },
        { "dri_get_rendering_buffer",
          offsetof(struct dri_vtable, get_rendering_buffer) },
        { "dri_swap_buffer",
          offsetof(struct dri_vtable, swap_buffer) },
        { NULL, }
    };

    i965->dri_output = calloc(1, sizeof(struct va_dri_output));
    if (!i965->dri_output)
        goto error;

    i965->dri_output->handle = dso_open(LIBVA_X11_NAME);
    if (!i965->dri_output->handle)
        goto error;

    dso_handle = i965->dri_output->handle;
    dri_vtable = &i965->dri_output->vtable;
    if (!dso_get_symbols(dso_handle, dri_vtable, sizeof(*dri_vtable), symbols))
        goto error;
    return true;

error:
    i965_output_dri_terminate(ctx);
    return false;
}

void
i965_output_dri_terminate(VADriverContextP ctx)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx); 
    struct va_dri_output * const dri_output = i965->dri_output;

    if (!dri_output)
        return;

    if (dri_output->handle) {
        dso_close(dri_output->handle);
        dri_output->handle = NULL;
    }

    free(dri_output);
    i965->dri_output = NULL;
}

VAStatus
i965_put_surface_dri(
    VADriverContextP    ctx,
    VASurfaceID         surface,
    void               *draw,
    const VARectangle  *src_rect,
    const VARectangle  *dst_rect,
    const VARectangle  *cliprects,
    unsigned int        num_cliprects,
    unsigned int        flags
)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx); 
    struct dri_vtable * const dri_vtable = &i965->dri_output->vtable;
    struct i965_render_state * const render_state = &i965->render_state;
    struct dri_drawable *dri_drawable;
    union dri_buffer *buffer;
    struct intel_region *dest_region;
    struct object_surface *obj_surface; 
    unsigned int pp_flag = 0;
    bool new_region = false;
    uint32_t name;
    int i, ret;
    unsigned int color_flag = 0;

    /* Currently don't support DRI1 */
    if (!VA_CHECK_DRM_AUTH_TYPE(ctx, VA_DRM_AUTH_DRI2))
        return VA_STATUS_ERROR_UNKNOWN;

    /* Some broken sources such as H.264 conformance case FM2_SVA_C
     * will get here
     */
    obj_surface = SURFACE(surface);
    ASSERT_RET(obj_surface && obj_surface->bo, VA_STATUS_SUCCESS);

    _i965LockMutex(&i965->render_mutex);

    dri_drawable = dri_vtable->get_drawable(ctx, (Drawable)draw);
    assert(dri_drawable);

    buffer = dri_vtable->get_rendering_buffer(ctx, dri_drawable);
    assert(buffer);
    
    dest_region = render_state->draw_region;

    if (dest_region) {
        assert(dest_region->bo);
        dri_bo_flink(dest_region->bo, &name);
        
        if (buffer->dri2.name != name) {
            new_region = True;
            dri_bo_unreference(dest_region->bo);
        }
    } else {
        dest_region = (struct intel_region *)calloc(1, sizeof(*dest_region));
        assert(dest_region);
        render_state->draw_region = dest_region;
        new_region = True;
    }

    if (new_region) {
        dest_region->x = dri_drawable->x;
        dest_region->y = dri_drawable->y;
        dest_region->width = dri_drawable->width;
        dest_region->height = dri_drawable->height;
        dest_region->cpp = buffer->dri2.cpp;
        dest_region->pitch = buffer->dri2.pitch;

        dest_region->bo = intel_bo_gem_create_from_name(i965->intel.bufmgr, "rendering buffer", buffer->dri2.name);
        assert(dest_region->bo);

        ret = dri_bo_get_tiling(dest_region->bo, &(dest_region->tiling), &(dest_region->swizzle));
        assert(ret == 0);
    }

    color_flag = flags & VA_SRC_COLOR_MASK;
    if (color_flag == 0)
        color_flag = VA_SRC_BT601;

    pp_flag = color_flag;

    if ((flags & VA_FILTER_SCALING_MASK) == VA_FILTER_SCALING_NL_ANAMORPHIC)
        pp_flag |= I965_PP_FLAG_AVS;

    if (flags & VA_TOP_FIELD)
        pp_flag |= I965_PP_FLAG_TOP_FIELD;
    else if (flags & VA_BOTTOM_FIELD)
        pp_flag |= I965_PP_FLAG_BOTTOM_FIELD;

    intel_render_put_surface(ctx, obj_surface, src_rect, dst_rect, pp_flag);

    for (i = 0; i < I965_MAX_SUBPIC_SUM; i++) {
        if (obj_surface->obj_subpic[i] != NULL) {
            assert(obj_surface->subpic[i] != VA_INVALID_ID);
            obj_surface->subpic_render_idx = i;
            intel_render_put_subpicture(ctx, obj_surface, src_rect, dst_rect);
        }
    }

    if (!(g_intel_debug_option_flags & VA_INTEL_DEBUG_OPTION_BENCH))
        dri_vtable->swap_buffer(ctx, dri_drawable);

    _i965UnlockMutex(&i965->render_mutex);

    return VA_STATUS_SUCCESS;
}
