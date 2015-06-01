/*
 * Copyright © 2009 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

#include "brw_context.h"
#include "brw_state.h"
#include "brw_defines.h"
#include "intel_batchbuffer.h"
#include "main/fbobject.h"
#include "main/viewport.h"

/* The clip VP defines the guardband region where expensive clipping is skipped
 * and fragments are allowed to be generated and clipped out cheaply by the SF.
 */
static void
gen6_upload_clip_vp(struct brw_context *brw)
{
   struct gl_context *ctx = &brw->ctx;
   struct brw_clipper_viewport *vp;

   vp = brw_state_batch(brw, AUB_TRACE_CLIP_VP_STATE,
                        sizeof(*vp) * ctx->Const.MaxViewports, 32, &brw->clip.vp_offset);

   for (unsigned i = 0; i < ctx->Const.MaxViewports; i++) {
      /* According to the "Vertex X,Y Clamping and Quantization" section of the
       * Strips and Fans documentation, objects must not have a screen-space
       * extents of over 8192 pixels, or they may be mis-rasterized.  The maximum
       * screen space coordinates of a small object may larger, but we have no
       * way to enforce the object size other than through clipping.
       *
       * If you're surprised that we set clip to -gbx to +gbx and it seems like
       * we'll end up with 16384 wide, note that for a 8192-wide render target,
       * we'll end up with a normal (-1, 1) clip volume that just covers the
       * drawable.
       */
      const float maximum_post_clamp_delta = 8192;
      float gbx = maximum_post_clamp_delta / ctx->ViewportArray[i].Width;
      float gby = maximum_post_clamp_delta / ctx->ViewportArray[i].Height;

      vp[i].xmin = -gbx;
      vp[i].xmax = gbx;
      vp[i].ymin = -gby;
      vp[i].ymax = gby;
   }

   brw->ctx.NewDriverState |= BRW_NEW_CLIP_VP;
}

const struct brw_tracked_state gen6_clip_vp = {
   .dirty = {
      .mesa = _NEW_VIEWPORT,
      .brw = BRW_NEW_BATCH,
   },
   .emit = gen6_upload_clip_vp,
};

static void
gen6_upload_sf_vp(struct brw_context *brw)
{
   struct gl_context *ctx = &brw->ctx;
   struct gen6_sf_viewport *sfv;
   GLfloat y_scale, y_bias;
   const bool render_to_fbo = _mesa_is_user_fbo(ctx->DrawBuffer);

   sfv = brw_state_batch(brw, AUB_TRACE_SF_VP_STATE,
                         sizeof(*sfv) * ctx->Const.MaxViewports,
                         32, &brw->sf.vp_offset);
   memset(sfv, 0, sizeof(*sfv) * ctx->Const.MaxViewports);

   /* _NEW_BUFFERS */
   if (render_to_fbo) {
      y_scale = 1.0;
      y_bias = 0;
   } else {
      y_scale = -1.0;
      y_bias = ctx->DrawBuffer->Height;
   }

   for (unsigned i = 0; i < ctx->Const.MaxViewports; i++) {
      double scale[3], translate[3];

      /* _NEW_VIEWPORT */
      _mesa_get_viewport_xform(ctx, i, scale, translate);
      sfv[i].m00 = scale[0];
      sfv[i].m11 = scale[1] * y_scale;
      sfv[i].m22 = scale[2];
      sfv[i].m30 = translate[0];
      sfv[i].m31 = translate[1] * y_scale + y_bias;
      sfv[i].m32 = translate[2];

   }

   brw->ctx.NewDriverState |= BRW_NEW_SF_VP;
}

const struct brw_tracked_state gen6_sf_vp = {
   .dirty = {
      .mesa = _NEW_BUFFERS |
              _NEW_VIEWPORT,
      .brw = BRW_NEW_BATCH,
   },
   .emit = gen6_upload_sf_vp,
};

static void upload_viewport_state_pointers(struct brw_context *brw)
{
   BEGIN_BATCH(4);
   OUT_BATCH(_3DSTATE_VIEWPORT_STATE_POINTERS << 16 | (4 - 2) |
	     GEN6_CC_VIEWPORT_MODIFY |
	     GEN6_SF_VIEWPORT_MODIFY |
	     GEN6_CLIP_VIEWPORT_MODIFY);
   OUT_BATCH(brw->clip.vp_offset);
   OUT_BATCH(brw->sf.vp_offset);
   OUT_BATCH(brw->cc.vp_offset);
   ADVANCE_BATCH();
}

const struct brw_tracked_state gen6_viewport_state = {
   .dirty = {
      .mesa = 0,
      .brw = BRW_NEW_BATCH |
             BRW_NEW_CC_VP |
             BRW_NEW_CLIP_VP |
             BRW_NEW_SF_VP |
             BRW_NEW_STATE_BASE_ADDRESS,
   },
   .emit = upload_viewport_state_pointers,
};
