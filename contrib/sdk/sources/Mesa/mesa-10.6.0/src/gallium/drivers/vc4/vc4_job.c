/*
 * Copyright © 2014-2015 Broadcom
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
 */

/** @file vc4_job.c
 *
 * Functions for submitting VC4 render jobs to the kernel.
 */

#include <xf86drm.h>
#include "vc4_context.h"

void
vc4_job_init(struct vc4_context *vc4)
{
        vc4_init_cl(vc4, &vc4->bcl);
        vc4_init_cl(vc4, &vc4->rcl);
        vc4_init_cl(vc4, &vc4->shader_rec);
        vc4_init_cl(vc4, &vc4->uniforms);
        vc4_init_cl(vc4, &vc4->bo_handles);
        vc4_init_cl(vc4, &vc4->bo_pointers);
        vc4_job_reset(vc4);
}

void
vc4_job_reset(struct vc4_context *vc4)
{
        struct vc4_bo **referenced_bos = vc4->bo_pointers.base;
        for (int i = 0; i < (vc4->bo_handles.next -
                             vc4->bo_handles.base) / 4; i++) {
                vc4_bo_unreference(&referenced_bos[i]);
        }
        vc4_reset_cl(&vc4->bcl);
        vc4_reset_cl(&vc4->rcl);
        vc4_reset_cl(&vc4->shader_rec);
        vc4_reset_cl(&vc4->uniforms);
        vc4_reset_cl(&vc4->bo_handles);
        vc4_reset_cl(&vc4->bo_pointers);
        vc4->shader_rec_count = 0;

        vc4->needs_flush = false;
        vc4->draw_call_queued = false;

        /* We have no hardware context saved between our draw calls, so we
         * need to flag the next draw as needing all state emitted.  Emitting
         * all state at the start of our draws is also what ensures that we
         * return to the state we need after a previous tile has finished.
         */
        vc4->dirty = ~0;
        vc4->resolve = 0;
        vc4->cleared = 0;

        vc4->draw_min_x = ~0;
        vc4->draw_min_y = ~0;
        vc4->draw_max_x = 0;
        vc4->draw_max_y = 0;
}

/**
 * Submits the job to the kernel and then reinitializes it.
 */
void
vc4_job_submit(struct vc4_context *vc4)
{
        if (vc4_debug & VC4_DEBUG_CL) {
                fprintf(stderr, "BCL:\n");
                vc4_dump_cl(vc4->bcl.base, vc4->bcl.next - vc4->bcl.base, false);
                fprintf(stderr, "RCL:\n");
                vc4_dump_cl(vc4->rcl.base, vc4->rcl.next - vc4->rcl.base, true);
        }

        struct drm_vc4_submit_cl submit;
        memset(&submit, 0, sizeof(submit));

        submit.bo_handles = (uintptr_t)vc4->bo_handles.base;
        submit.bo_handle_count = (vc4->bo_handles.next -
                                  vc4->bo_handles.base) / 4;
        submit.bin_cl = (uintptr_t)vc4->bcl.base;
        submit.bin_cl_size = vc4->bcl.next - vc4->bcl.base;
        submit.render_cl = (uintptr_t)vc4->rcl.base;
        submit.render_cl_size = vc4->rcl.next - vc4->rcl.base;
        submit.shader_rec = (uintptr_t)vc4->shader_rec.base;
        submit.shader_rec_size = vc4->shader_rec.next - vc4->shader_rec.base;
        submit.shader_rec_count = vc4->shader_rec_count;
        submit.uniforms = (uintptr_t)vc4->uniforms.base;
        submit.uniforms_size = vc4->uniforms.next - vc4->uniforms.base;

        if (!(vc4_debug & VC4_DEBUG_NORAST)) {
                int ret;

#ifndef USE_VC4_SIMULATOR
                ret = drmIoctl(vc4->fd, DRM_IOCTL_VC4_SUBMIT_CL, &submit);
#else
                ret = vc4_simulator_flush(vc4, &submit);
#endif
                if (ret) {
                        fprintf(stderr, "VC4 submit failed\n");
                        abort();
                }
        }

        vc4->last_emit_seqno = submit.seqno;

        if (vc4_debug & VC4_DEBUG_ALWAYS_SYNC) {
                if (!vc4_wait_seqno(vc4->screen, vc4->last_emit_seqno,
                                    PIPE_TIMEOUT_INFINITE)) {
                        fprintf(stderr, "Wait failed.\n");
                        abort();
                }
        }

        vc4_job_reset(vc4);
}
