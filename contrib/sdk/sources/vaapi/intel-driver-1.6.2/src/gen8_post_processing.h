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
 */

#ifndef _GEN8_POST_PROCESSING_H_
#define _GEN8_POST_PROCESSING_H_

VAStatus pp_null_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                            const struct i965_surface *src_surface,
                            const VARectangle *src_rect,
                            struct i965_surface *dst_surface,
                            const VARectangle *dst_rect,
                            void *filter_param);

VAStatus
gen8_pp_plx_avs_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                           const struct i965_surface *src_surface,
                           const VARectangle *src_rect,
                           struct i965_surface *dst_surface,
                           const VARectangle *dst_rect,
                           void *filter_param);

VAStatus
gen8_pp_nv12_blending_initialize(VADriverContextP ctx, struct i965_post_processing_context *pp_context,
                                 const struct i965_surface *src_surface,
                                 const VARectangle *src_rect,
                                 struct i965_surface *dst_surface,
                                 const VARectangle *dst_rect,
                                 void *filter_param);

void
gen8_pp_vfe_state(VADriverContextP ctx,
                  struct i965_post_processing_context *pp_context);

void
gen8_interface_descriptor_load(VADriverContextP ctx,
                               struct i965_post_processing_context *pp_context);

void
gen8_pp_curbe_load(VADriverContextP ctx,
                   struct i965_post_processing_context *pp_context);

void
gen8_pp_object_walker(VADriverContextP ctx,
                      struct i965_post_processing_context *pp_context);

void
gen8_pp_states_setup(VADriverContextP ctx,
                     struct i965_post_processing_context *pp_context);

VAStatus
gen8_pp_initialize(VADriverContextP ctx,
                   struct i965_post_processing_context *pp_context,
                   const struct i965_surface *src_surface,
                   const VARectangle *src_rect,
                   struct i965_surface *dst_surface,
                   const VARectangle *dst_rect,
                   int pp_index,
                   void *filter_param);

void
gen8_post_processing_context_common_init(VADriverContextP ctx,
                                         void *data,
                                         struct pp_module *pp_modules,
                                         int num_pp_modules,
                                         struct intel_batchbuffer *batch);
#endif
