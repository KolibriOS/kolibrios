/* -*- mode: C; c-file-style: "k&r"; tab-width 4; indent-tabs-mode: t; -*- */

/*
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include "util/u_inlines.h"

#include "freedreno_fence.h"
#include "freedreno_context.h"
#include "freedreno_util.h"

struct pipe_fence_handle {
	struct pipe_reference reference;
	struct fd_context *ctx;
	struct fd_screen *screen;
	uint32_t timestamp;
};

void
fd_screen_fence_ref(struct pipe_screen *pscreen,
		struct pipe_fence_handle **ptr,
		struct pipe_fence_handle *pfence)
{
	if (pipe_reference(&(*ptr)->reference, &pfence->reference))
		FREE(*ptr);

	*ptr = pfence;
}

/* TODO we need to spiff out libdrm_freedreno a bit to allow passing
 * the timeout.. and maybe a better way to check if fence has been
 * signaled.  The current implementation is a bit lame for now to
 * avoid bumping libdrm version requirement.
 */

boolean fd_screen_fence_signalled(struct pipe_screen *screen,
		struct pipe_fence_handle *fence)
{
	uint32_t timestamp = fd_ringbuffer_timestamp(fence->ctx->ring);

	/* TODO util helper for compare w/ rollover? */
	return timestamp >= fence->timestamp;
}

boolean fd_screen_fence_finish(struct pipe_screen *screen,
		struct pipe_fence_handle *fence,
		uint64_t timeout)
{
	if (fd_pipe_wait(fence->screen->pipe, fence->timestamp))
		return false;

	return true;
}

struct pipe_fence_handle * fd_fence_create(struct pipe_context *pctx)
{
	struct pipe_fence_handle *fence;
	struct fd_context *ctx = fd_context(pctx);

	fence = CALLOC_STRUCT(pipe_fence_handle);
	if (!fence)
		return NULL;

	pipe_reference_init(&fence->reference, 1);

	fence->ctx = ctx;
	fence->screen = ctx->screen;
	fence->timestamp = fd_ringbuffer_timestamp(ctx->ring);

	return fence;
}
