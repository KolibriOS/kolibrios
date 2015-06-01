/*
 * Copyright 2010 Jerome Glisse <glisse@freedesktop.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *      Jerome Glisse
 */
#ifndef SI_PIPE_H
#define SI_PIPE_H

#include "si_state.h"

#include <llvm-c/TargetMachine.h>

#ifdef PIPE_ARCH_BIG_ENDIAN
#define SI_BIG_ENDIAN 1
#else
#define SI_BIG_ENDIAN 0
#endif

/* The base vertex and primitive restart can be any number, but we must pick
 * one which will mean "unknown" for the purpose of state tracking and
 * the number shouldn't be a commonly-used one. */
#define SI_BASE_VERTEX_UNKNOWN INT_MIN
#define SI_RESTART_INDEX_UNKNOWN INT_MIN
#define SI_NUM_SMOOTH_AA_SAMPLES 8

#define SI_TRACE_CS 0
#define SI_TRACE_CS_DWORDS		6

#define SI_MAX_DRAW_CS_DWORDS \
	(/*scratch:*/ 3 + /*derived prim state:*/ 3 + \
	 /*draw regs:*/ 16 + /*draw packets:*/ 31)

/* Instruction cache. */
#define SI_CONTEXT_INV_ICACHE		(R600_CONTEXT_PRIVATE_FLAG << 0)
/* Cache used by scalar memory (SMEM) instructions. They also use TC
 * as a second level cache, which isn't flushed by this.
 * Other names: constant cache, data cache, DCACHE */
#define SI_CONTEXT_INV_KCACHE		(R600_CONTEXT_PRIVATE_FLAG << 1)
/* Caches used by vector memory (VMEM) instructions.
 * L1 can optionally be bypassed (GLC=1) and can only be used by shaders.
 * L2 is used by shaders and can be used by other blocks (CP, sDMA). */
#define SI_CONTEXT_INV_TC_L1		(R600_CONTEXT_PRIVATE_FLAG << 2)
#define SI_CONTEXT_INV_TC_L2		(R600_CONTEXT_PRIVATE_FLAG << 3)
/* Framebuffer caches. */
#define SI_CONTEXT_FLUSH_AND_INV_CB_META (R600_CONTEXT_PRIVATE_FLAG << 4)
#define SI_CONTEXT_FLUSH_AND_INV_DB_META (R600_CONTEXT_PRIVATE_FLAG << 5)
#define SI_CONTEXT_FLUSH_AND_INV_DB	(R600_CONTEXT_PRIVATE_FLAG << 6)
#define SI_CONTEXT_FLUSH_AND_INV_CB	(R600_CONTEXT_PRIVATE_FLAG << 7)
/* Engine synchronization. */
#define SI_CONTEXT_VS_PARTIAL_FLUSH	(R600_CONTEXT_PRIVATE_FLAG << 8)
#define SI_CONTEXT_PS_PARTIAL_FLUSH	(R600_CONTEXT_PRIVATE_FLAG << 9)
#define SI_CONTEXT_CS_PARTIAL_FLUSH	(R600_CONTEXT_PRIVATE_FLAG << 10)
#define SI_CONTEXT_VGT_FLUSH		(R600_CONTEXT_PRIVATE_FLAG << 11)
#define SI_CONTEXT_VGT_STREAMOUT_SYNC	(R600_CONTEXT_PRIVATE_FLAG << 12)
/* Compute only. */
#define SI_CONTEXT_FLUSH_WITH_INV_L2	(R600_CONTEXT_PRIVATE_FLAG << 13) /* TODO: merge with TC? */
#define SI_CONTEXT_FLAG_COMPUTE		(R600_CONTEXT_PRIVATE_FLAG << 14)

#define SI_CONTEXT_FLUSH_AND_INV_FRAMEBUFFER (SI_CONTEXT_FLUSH_AND_INV_CB | \
					      SI_CONTEXT_FLUSH_AND_INV_CB_META | \
					      SI_CONTEXT_FLUSH_AND_INV_DB | \
					      SI_CONTEXT_FLUSH_AND_INV_DB_META)

struct si_compute;

struct si_screen {
	struct r600_common_screen	b;
};

struct si_sampler_view {
	struct pipe_sampler_view	base;
	struct list_head		list;
	struct r600_resource		*resource;
        /* [0..7] = image descriptor
         * [4..7] = buffer descriptor */
	uint32_t			state[8];
	uint32_t			fmask_state[8];
};

struct si_sampler_state {
	uint32_t			val[4];
	uint32_t			border_color[4];
};

struct si_cs_shader_state {
	struct si_compute		*program;
};

struct si_textures_info {
	struct si_sampler_views		views;
	struct si_sampler_states	states;
	uint32_t			depth_texture_mask; /* which textures are depth */
	uint32_t			compressed_colortex_mask;
};

struct si_framebuffer {
	struct r600_atom		atom;
	struct pipe_framebuffer_state	state;
	unsigned			nr_samples;
	unsigned			log_samples;
	unsigned			cb0_is_integer;
	unsigned			compressed_cb_mask;
	unsigned			export_16bpc;
};

#define SI_NUM_ATOMS(sctx) (sizeof((sctx)->atoms)/sizeof((sctx)->atoms.array[0]))

#define SI_NUM_SHADERS (PIPE_SHADER_GEOMETRY+1)

struct si_context {
	struct r600_common_context	b;
	struct blitter_context		*blitter;
	void				*custom_dsa_flush;
	void				*custom_blend_resolve;
	void				*custom_blend_decompress;
	void				*custom_blend_fastclear;
	void				*pstipple_sampler_state;
	struct si_screen		*screen;
	struct si_pm4_state		*init_config;

	union {
		struct {
			/* The order matters. */
			struct r600_atom *vertex_buffers;
			struct r600_atom *const_buffers[SI_NUM_SHADERS];
			struct r600_atom *rw_buffers[SI_NUM_SHADERS];
			struct r600_atom *sampler_views[SI_NUM_SHADERS];
			struct r600_atom *sampler_states[SI_NUM_SHADERS];
			/* Caches must be flushed after resource descriptors are
			 * updated in memory. */
			struct r600_atom *cache_flush;
			struct r600_atom *streamout_begin;
			struct r600_atom *streamout_enable; /* must be after streamout_begin */
			struct r600_atom *framebuffer;
			struct r600_atom *msaa_sample_locs;
			struct r600_atom *db_render_state;
			struct r600_atom *msaa_config;
			struct r600_atom *clip_regs;
		} s;
		struct r600_atom *array[0];
	} atoms;

	struct si_framebuffer		framebuffer;
	struct si_vertex_element	*vertex_elements;
	/* for saving when using blitter */
	struct pipe_stencil_ref		stencil_ref;
	/* shaders */
	struct si_shader_selector	*ps_shader;
	struct si_shader_selector	*gs_shader;
	struct si_shader_selector	*vs_shader;
	struct si_cs_shader_state	cs_shader_state;
	/* shader information */
	unsigned			sprite_coord_enable;
	bool				flatshade;
	struct si_descriptors		vertex_buffers;
	struct si_buffer_resources	const_buffers[SI_NUM_SHADERS];
	struct si_buffer_resources	rw_buffers[SI_NUM_SHADERS];
	struct si_textures_info		samplers[SI_NUM_SHADERS];
	struct r600_resource		*scratch_buffer;
	struct r600_resource		*border_color_table;
	unsigned			border_color_offset;

	struct r600_atom		clip_regs;
	struct r600_atom		msaa_sample_locs;
	struct r600_atom		msaa_config;
	int				ps_iter_samples;
	bool				smoothing_enabled;

	/* Vertex and index buffers. */
	bool			vertex_buffers_dirty;
	struct pipe_index_buffer index_buffer;
	struct pipe_vertex_buffer vertex_buffer[SI_NUM_VERTEX_BUFFERS];

	/* With rasterizer discard, there doesn't have to be a pixel shader.
	 * In that case, we bind this one: */
	void			*dummy_pixel_shader;
	struct si_pm4_state	*gs_on;
	struct si_pm4_state	*gs_off;
	struct si_pm4_state	*gs_rings;
	struct r600_atom	cache_flush;
	struct pipe_constant_buffer null_const_buf; /* used for set_constant_buffer(NULL) on CIK */
	struct pipe_resource	*esgs_ring;
	struct pipe_resource	*gsvs_ring;

	LLVMTargetMachineRef		tm;

	/* SI state handling */
	union si_state	queued;
	union si_state	emitted;

	/* DB render state. */
	struct r600_atom	db_render_state;
	bool			dbcb_depth_copy_enabled;
	bool			dbcb_stencil_copy_enabled;
	unsigned		dbcb_copy_sample;
	bool			db_inplace_flush_enabled;
	bool			db_depth_clear;
	bool			db_depth_disable_expclear;
	unsigned		ps_db_shader_control;

	/* Draw state. */
	int			last_base_vertex;
	int			last_start_instance;
	int			last_sh_base_reg;
	int			last_primitive_restart_en;
	int			last_restart_index;
	int			last_gs_out_prim;
	int			last_prim;
	int			last_multi_vgt_param;
	int			last_rast_prim;
	unsigned		last_sc_line_stipple;
	int			current_rast_prim; /* primitive type after TES, GS */

	/* Scratch buffer */
	boolean                 emit_scratch_reloc;
	unsigned		scratch_waves;
	unsigned		spi_tmpring_size;
};

/* si_blit.c */
void si_init_blit_functions(struct si_context *sctx);
void si_flush_depth_textures(struct si_context *sctx,
			     struct si_textures_info *textures);
void si_decompress_color_textures(struct si_context *sctx,
				  struct si_textures_info *textures);
void si_resource_copy_region(struct pipe_context *ctx,
			     struct pipe_resource *dst,
			     unsigned dst_level,
			     unsigned dstx, unsigned dsty, unsigned dstz,
			     struct pipe_resource *src,
			     unsigned src_level,
			     const struct pipe_box *src_box);

/* si_dma.c */
void si_dma_copy(struct pipe_context *ctx,
		 struct pipe_resource *dst,
		 unsigned dst_level,
		 unsigned dstx, unsigned dsty, unsigned dstz,
		 struct pipe_resource *src,
		 unsigned src_level,
		 const struct pipe_box *src_box);

/* si_hw_context.c */
void si_context_gfx_flush(void *context, unsigned flags,
			  struct pipe_fence_handle **fence);
void si_begin_new_cs(struct si_context *ctx);
void si_need_cs_space(struct si_context *ctx, unsigned num_dw, boolean count_draw_in);

#if SI_TRACE_CS
void si_trace_emit(struct si_context *sctx);
#endif

/* si_compute.c */
void si_init_compute_functions(struct si_context *sctx);

/* si_uvd.c */
struct pipe_video_codec *si_uvd_create_decoder(struct pipe_context *context,
					       const struct pipe_video_codec *templ);

struct pipe_video_buffer *si_video_buffer_create(struct pipe_context *pipe,
						 const struct pipe_video_buffer *tmpl);

/*
 * common helpers
 */

static INLINE struct r600_resource *
si_resource_create_custom(struct pipe_screen *screen,
			  unsigned usage, unsigned size)
{
	assert(size);
	return r600_resource(pipe_buffer_create(screen,
		PIPE_BIND_CUSTOM, usage, size));
}

static INLINE void
si_invalidate_draw_sh_constants(struct si_context *sctx)
{
	sctx->last_base_vertex = SI_BASE_VERTEX_UNKNOWN;
	sctx->last_start_instance = -1; /* reset to an unknown value */
	sctx->last_sh_base_reg = -1; /* reset to an unknown value */
}

#endif
