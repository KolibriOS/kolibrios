#ifndef SNA_RENDER_INLINE_H
#define SNA_RENDER_INLINE_H

static inline bool need_tiling(struct sna *sna, int16_t width, int16_t height)
{
	/* Is the damage area too large to fit in 3D pipeline,
	 * and so do we need to split the operation up into tiles?
	 */
	return (width > sna->render.max_3d_size ||
		height > sna->render.max_3d_size);
}

static inline bool need_redirect(struct sna *sna, PixmapPtr dst)
{
	/* Is the pixmap too large to render to? */
	return (dst->drawable.width > sna->render.max_3d_size ||
		dst->drawable.height > sna->render.max_3d_size);
}

static inline float pack_2s(int16_t x, int16_t y)
{
	union {
		struct sna_coordinate p;
		float f;
	} u;
	u.p.x = x;
	u.p.y = y;
	return u.f;
}

static inline int vertex_space(struct sna *sna)
{
	return sna->render.vertex_size - sna->render.vertex_used;
}
static inline void vertex_emit(struct sna *sna, float v)
{
	assert(sna->render.vertex_used < sna->render.vertex_size);
	sna->render.vertices[sna->render.vertex_used++] = v;
}
static inline void vertex_emit_2s(struct sna *sna, int16_t x, int16_t y)
{
	vertex_emit(sna, pack_2s(x, y));
}

static inline int batch_space(struct sna *sna)
{
	assert(sna->kgem.nbatch <= KGEM_BATCH_SIZE(&sna->kgem));
	assert(sna->kgem.nbatch + KGEM_BATCH_RESERVED <= sna->kgem.surface);
	return sna->kgem.surface - sna->kgem.nbatch - KGEM_BATCH_RESERVED;
}

static inline void batch_emit(struct sna *sna, uint32_t dword)
{
	assert(sna->kgem.mode != KGEM_NONE);
	assert(sna->kgem.nbatch + KGEM_BATCH_RESERVED < sna->kgem.surface);
	sna->kgem.batch[sna->kgem.nbatch++] = dword;
}

static inline void batch_emit_float(struct sna *sna, float f)
{
	union {
		uint32_t dw;
		float f;
	} u;
	u.f = f;
	batch_emit(sna, u.dw);
}



#endif /* SNA_RENDER_INLINE_H */
