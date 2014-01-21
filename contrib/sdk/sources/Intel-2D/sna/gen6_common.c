/*
 * Copyright © 2011-2013 Intel Corporation
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
 *    Chris Wilson <chris@chris-wilson.co.uk>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gen6_common.h"
#include "gen4_vertex.h"

void
gen6_render_context_switch(struct kgem *kgem,
			   int new_mode)
{
	if (kgem->nbatch) {
		DBG(("%s: from %d to %d, submit batch\n", __FUNCTION__, kgem->mode, new_mode));
		_kgem_submit(kgem);
	}

	if (kgem->nexec) {
		DBG(("%s: from %d to %d, reset incomplete batch\n", __FUNCTION__, kgem->mode, new_mode));
		kgem_reset(kgem);
	}

	assert(kgem->nbatch == 0);
	assert(kgem->nreloc == 0);
	assert(kgem->nexec == 0);

	kgem->ring = new_mode;
}

void gen6_render_retire(struct kgem *kgem)
{
	struct sna *sna;

	if (kgem->ring && (kgem->has_semaphores || !kgem->need_retire))
		kgem->ring = kgem->mode;

	sna = container_of(kgem, struct sna, kgem);
	if (sna->render.nvertex_reloc == 0 &&
	    sna->render.vbo &&
	    !kgem_bo_is_busy(sna->render.vbo)) {
		DBG(("%s: resetting idle vbo\n", __FUNCTION__));
		sna->render.vertex_used = 0;
		sna->render.vertex_index = 0;
	}
}
