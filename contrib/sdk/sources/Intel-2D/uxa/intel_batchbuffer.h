/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
Copyright © 2002 David Dawes

All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

#ifndef _INTEL_BATCHBUFFER_H
#define _INTEL_BATCHBUFFER_H

#define BATCH_RESERVED		16


void intel_batch_init();
void intel_batch_teardown();
void intel_batch_emit_flush();
void intel_batch_submit();

static inline int intel_batch_space(intel_screen_private *intel)
{
	return (intel->batch_bo->size - BATCH_RESERVED) - (4*intel->batch_used);
}

static inline int intel_vertex_space(intel_screen_private *intel)
{
	return intel->vertex_bo ? intel->vertex_bo->size - (4*intel->vertex_used) : 0;
}

static inline void
intel_batch_require_space(intel_screen_private *intel, int sz)
{
	assert(sz < intel->batch_bo->size - 8);
	if (intel_batch_space(intel) < sz)
		intel_batch_submit();
}

static inline void intel_batch_start_atomic(int sz)
{
	intel_screen_private *intel = intel_get_screen_private();

	assert(!intel->in_batch_atomic);

	if (intel->current_batch != RENDER_BATCH) {
		if (intel->current_batch && intel->context_switch)
			intel->context_switch(intel, RENDER_BATCH);
	}

	intel_batch_require_space(intel, sz * 4);
	intel->current_batch = RENDER_BATCH;

	intel->in_batch_atomic = TRUE;
	intel->batch_atomic_limit = intel->batch_used + sz;
}

static inline void intel_batch_end_atomic()
{
	intel_screen_private *intel = intel_get_screen_private();

	assert(intel->in_batch_atomic);
	assert(intel->batch_used <= intel->batch_atomic_limit);
	intel->in_batch_atomic = FALSE;
}

static inline void intel_batch_emit_dword(intel_screen_private *intel, uint32_t dword)
{
	intel->batch_ptr[intel->batch_used++] = dword;
}

static inline void intel_batch_align(intel_screen_private *intel, uint32_t align)
{
	uint32_t delta;

	align /= 4;
	assert(align);

	if ((delta = intel->batch_used & (align - 1))) {
		delta = align - delta;
		memset (intel->batch_ptr + intel->batch_used, 0, 4*delta);
		intel->batch_used += delta;
	}
}

static inline void
intel_batch_emit_reloc(intel_screen_private *intel,
		       dri_bo * bo,
		       uint32_t read_domains,
		       uint32_t write_domains, uint32_t delta, int needs_fence)
{
	if (needs_fence)
		drm_intel_bo_emit_reloc_fence(intel->batch_bo,
					      intel->batch_used * 4,
					      bo, delta,
					      read_domains, write_domains);
	else
		drm_intel_bo_emit_reloc(intel->batch_bo, intel->batch_used * 4,
					bo, delta,
					read_domains, write_domains);

	intel_batch_emit_dword(intel, bo->offset + delta);
}

static inline void
intel_batch_mark_pixmap_domains(intel_screen_private *intel,
				struct intel_pixmap *priv,
				uint32_t read_domains, uint32_t write_domain)
{
	assert (read_domains);
	assert (write_domain == 0 || write_domain == read_domains);

	if (list_is_empty(&priv->batch))
		list_add(&priv->batch, &intel->batch_pixmaps);

	priv->dirty |= write_domain != 0;
	priv->busy = 1;

	intel->needs_flush |= write_domain != 0;
}

static inline void
intel_batch_emit_reloc_pixmap(intel_screen_private *intel, PixmapPtr pixmap,
			      uint32_t read_domains, uint32_t write_domain,
			      uint32_t delta, int needs_fence)
{
	struct intel_pixmap *priv = pixmap->private;

	intel_batch_mark_pixmap_domains(intel, priv, read_domains, write_domain);

	intel_batch_emit_reloc(intel, priv->bo,
			       read_domains, write_domain,
			       delta, needs_fence);
}

#define ALIGN_BATCH(align) intel_batch_align(intel, align);
#define OUT_BATCH(dword) intel_batch_emit_dword(intel, dword)

#define OUT_RELOC(bo, read_domains, write_domains, delta) \
	intel_batch_emit_reloc(intel, bo, read_domains, write_domains, delta, 0)

#define OUT_RELOC_FENCED(bo, read_domains, write_domains, delta) \
	intel_batch_emit_reloc(intel, bo, read_domains, write_domains, delta, 1)

#define OUT_RELOC_PIXMAP(pixmap, reads, write, delta)	\
	intel_batch_emit_reloc_pixmap(intel, pixmap, reads, write, delta, 0)

#define OUT_RELOC_PIXMAP_FENCED(pixmap, reads, write, delta)	\
	intel_batch_emit_reloc_pixmap(intel, pixmap, reads, write, delta, 1)

union intfloat {
	float f;
	unsigned int ui;
};

#define OUT_BATCH_F(x) do {			\
	union intfloat tmp;			\
	tmp.f = (float)(x);			\
	OUT_BATCH(tmp.ui);			\
} while(0)

#define __BEGIN_BATCH(n,batch_idx)					\
do {									\
	if (intel->batch_emitting != 0)					\
		printf("%s: BEGIN_BATCH called without closing "	\
			   "ADVANCE_BATCH\n", __FUNCTION__);		\
	assert(!intel->in_batch_atomic);				\
	if (intel->current_batch != batch_idx) {			\
		if (intel->current_batch && intel->context_switch)	\
			intel->context_switch(intel, batch_idx);	\
	}								\
	intel_batch_require_space(intel, (n) * 4);		\
	intel->current_batch = batch_idx;				\
	intel->batch_emitting = (n);					\
	intel->batch_emit_start = intel->batch_used;			\
} while (0)

#define BEGIN_BATCH(n)	__BEGIN_BATCH(n,RENDER_BATCH)
#define BEGIN_BATCH_BLT(n)	__BEGIN_BATCH(n,BLT_BATCH)

#define ADVANCE_BATCH() do {						\
	if (intel->batch_emitting == 0)					\
		printf("%s: ADVANCE_BATCH called with no matching "	\
			   "BEGIN_BATCH\n", __FUNCTION__);		\
	if (intel->batch_used >						\
	    intel->batch_emit_start + intel->batch_emitting)		\
		printf("%s: ADVANCE_BATCH: exceeded allocation %d/%d\n ", \
			   __FUNCTION__,				\
			   intel->batch_used - intel->batch_emit_start,	\
			   intel->batch_emitting);			\
	if (intel->batch_used < intel->batch_emit_start +		\
	    intel->batch_emitting)					\
		printf("%s: ADVANCE_BATCH: under-used allocation %d/%d\n ", \
			   __FUNCTION__,				\
			   intel->batch_used - intel->batch_emit_start,	\
			   intel->batch_emitting);			\
	intel->batch_emitting = 0;					\
} while (0)

void intel_next_vertex(intel_screen_private *intel);
static inline void intel_vertex_emit(intel_screen_private *intel, float v)
{
	intel->vertex_ptr[intel->vertex_used++] = v;
}
#define OUT_VERTEX(v) intel_vertex_emit(intel, v)

#endif /* _INTEL_BATCHBUFFER_H */
