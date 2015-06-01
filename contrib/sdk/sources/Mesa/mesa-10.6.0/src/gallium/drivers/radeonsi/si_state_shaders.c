/*
 * Copyright 2012 Advanced Micro Devices, Inc.
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
 *      Christian König <christian.koenig@amd.com>
 *      Marek Olšák <maraeo@gmail.com>
 */

#include "si_pipe.h"
#include "si_shader.h"
#include "sid.h"

#include "tgsi/tgsi_parse.h"
#include "util/u_memory.h"
#include "util/u_simple_shaders.h"

static void si_shader_es(struct si_shader *shader)
{
	struct si_pm4_state *pm4;
	unsigned num_sgprs, num_user_sgprs;
	unsigned vgpr_comp_cnt;
	uint64_t va;

	pm4 = shader->pm4 = CALLOC_STRUCT(si_pm4_state);

	if (pm4 == NULL)
		return;

	va = shader->bo->gpu_address;
	si_pm4_add_bo(pm4, shader->bo, RADEON_USAGE_READ, RADEON_PRIO_SHADER_DATA);

	vgpr_comp_cnt = shader->uses_instanceid ? 3 : 0;

	num_user_sgprs = SI_VS_NUM_USER_SGPR;
	num_sgprs = shader->num_sgprs;
	/* One SGPR after user SGPRs is pre-loaded with es2gs_offset */
	if ((num_user_sgprs + 1) > num_sgprs) {
		/* Last 2 reserved SGPRs are used for VCC */
		num_sgprs = num_user_sgprs + 1 + 2;
	}
	assert(num_sgprs <= 104);

	si_pm4_set_reg(pm4, R_00B320_SPI_SHADER_PGM_LO_ES, va >> 8);
	si_pm4_set_reg(pm4, R_00B324_SPI_SHADER_PGM_HI_ES, va >> 40);
	si_pm4_set_reg(pm4, R_00B328_SPI_SHADER_PGM_RSRC1_ES,
		       S_00B328_VGPRS((shader->num_vgprs - 1) / 4) |
		       S_00B328_SGPRS((num_sgprs - 1) / 8) |
		       S_00B328_VGPR_COMP_CNT(vgpr_comp_cnt) |
		       S_00B328_DX10_CLAMP(shader->dx10_clamp_mode));
	si_pm4_set_reg(pm4, R_00B32C_SPI_SHADER_PGM_RSRC2_ES,
		       S_00B32C_USER_SGPR(num_user_sgprs) |
		       S_00B32C_SCRATCH_EN(shader->scratch_bytes_per_wave > 0));
}

static void si_shader_gs(struct si_shader *shader)
{
	unsigned gs_vert_itemsize = shader->selector->info.num_outputs * (16 >> 2);
	unsigned gs_max_vert_out = shader->selector->gs_max_out_vertices;
	unsigned gsvs_itemsize = gs_vert_itemsize * gs_max_vert_out;
	unsigned cut_mode;
	struct si_pm4_state *pm4;
	unsigned num_sgprs, num_user_sgprs;
	uint64_t va;

	/* The GSVS_RING_ITEMSIZE register takes 15 bits */
	assert(gsvs_itemsize < (1 << 15));

	pm4 = shader->pm4 = CALLOC_STRUCT(si_pm4_state);

	if (pm4 == NULL)
		return;

	if (gs_max_vert_out <= 128) {
		cut_mode = V_028A40_GS_CUT_128;
	} else if (gs_max_vert_out <= 256) {
		cut_mode = V_028A40_GS_CUT_256;
	} else if (gs_max_vert_out <= 512) {
		cut_mode = V_028A40_GS_CUT_512;
	} else {
		assert(gs_max_vert_out <= 1024);
		cut_mode = V_028A40_GS_CUT_1024;
	}

	si_pm4_set_reg(pm4, R_028A40_VGT_GS_MODE,
		       S_028A40_MODE(V_028A40_GS_SCENARIO_G) |
		       S_028A40_CUT_MODE(cut_mode)|
		       S_028A40_ES_WRITE_OPTIMIZE(1) |
		       S_028A40_GS_WRITE_OPTIMIZE(1));

	si_pm4_set_reg(pm4, R_028A60_VGT_GSVS_RING_OFFSET_1, gsvs_itemsize);
	si_pm4_set_reg(pm4, R_028A64_VGT_GSVS_RING_OFFSET_2, gsvs_itemsize);
	si_pm4_set_reg(pm4, R_028A68_VGT_GSVS_RING_OFFSET_3, gsvs_itemsize);

	si_pm4_set_reg(pm4, R_028AAC_VGT_ESGS_RING_ITEMSIZE,
		       util_bitcount64(shader->selector->gs_used_inputs) * (16 >> 2));
	si_pm4_set_reg(pm4, R_028AB0_VGT_GSVS_RING_ITEMSIZE, gsvs_itemsize);

	si_pm4_set_reg(pm4, R_028B38_VGT_GS_MAX_VERT_OUT, gs_max_vert_out);

	si_pm4_set_reg(pm4, R_028B5C_VGT_GS_VERT_ITEMSIZE, gs_vert_itemsize);

	va = shader->bo->gpu_address;
	si_pm4_add_bo(pm4, shader->bo, RADEON_USAGE_READ, RADEON_PRIO_SHADER_DATA);
	si_pm4_set_reg(pm4, R_00B220_SPI_SHADER_PGM_LO_GS, va >> 8);
	si_pm4_set_reg(pm4, R_00B224_SPI_SHADER_PGM_HI_GS, va >> 40);

	num_user_sgprs = SI_GS_NUM_USER_SGPR;
	num_sgprs = shader->num_sgprs;
	/* Two SGPRs after user SGPRs are pre-loaded with gs2vs_offset, gs_wave_id */
	if ((num_user_sgprs + 2) > num_sgprs) {
		/* Last 2 reserved SGPRs are used for VCC */
		num_sgprs = num_user_sgprs + 2 + 2;
	}
	assert(num_sgprs <= 104);

	si_pm4_set_reg(pm4, R_00B228_SPI_SHADER_PGM_RSRC1_GS,
		       S_00B228_VGPRS((shader->num_vgprs - 1) / 4) |
		       S_00B228_SGPRS((num_sgprs - 1) / 8) |
		       S_00B228_DX10_CLAMP(shader->dx10_clamp_mode));
	si_pm4_set_reg(pm4, R_00B22C_SPI_SHADER_PGM_RSRC2_GS,
		       S_00B22C_USER_SGPR(num_user_sgprs) |
		       S_00B22C_SCRATCH_EN(shader->scratch_bytes_per_wave > 0));
}

static void si_shader_vs(struct si_shader *shader)
{
	struct tgsi_shader_info *info = &shader->selector->info;
	struct si_pm4_state *pm4;
	unsigned num_sgprs, num_user_sgprs;
	unsigned nparams, i, vgpr_comp_cnt;
	uint64_t va;
	unsigned window_space =
	   shader->selector->info.properties[TGSI_PROPERTY_VS_WINDOW_SPACE_POSITION];

	pm4 = shader->pm4 = CALLOC_STRUCT(si_pm4_state);

	if (pm4 == NULL)
		return;

	va = shader->bo->gpu_address;
	si_pm4_add_bo(pm4, shader->bo, RADEON_USAGE_READ, RADEON_PRIO_SHADER_DATA);

	if (shader->is_gs_copy_shader) {
		vgpr_comp_cnt = 0; /* only VertexID is needed for GS-COPY. */
		num_user_sgprs = SI_GSCOPY_NUM_USER_SGPR;
	} else if (shader->selector->type == PIPE_SHADER_VERTEX) {
		vgpr_comp_cnt = shader->uses_instanceid ? 3 : 0;
		num_user_sgprs = SI_VS_NUM_USER_SGPR;
	} else
		assert(0);

	num_sgprs = shader->num_sgprs;
	if (num_user_sgprs > num_sgprs) {
		/* Last 2 reserved SGPRs are used for VCC */
		num_sgprs = num_user_sgprs + 2;
	}
	assert(num_sgprs <= 104);

	/* Certain attributes (position, psize, etc.) don't count as params.
	 * VS is required to export at least one param and r600_shader_from_tgsi()
	 * takes care of adding a dummy export.
	 */
	for (nparams = 0, i = 0 ; i < info->num_outputs; i++) {
		switch (info->output_semantic_name[i]) {
		case TGSI_SEMANTIC_CLIPVERTEX:
		case TGSI_SEMANTIC_POSITION:
		case TGSI_SEMANTIC_PSIZE:
			break;
		default:
			nparams++;
		}
	}
	if (nparams < 1)
		nparams = 1;

	si_pm4_set_reg(pm4, R_0286C4_SPI_VS_OUT_CONFIG,
		       S_0286C4_VS_EXPORT_COUNT(nparams - 1));

	si_pm4_set_reg(pm4, R_02870C_SPI_SHADER_POS_FORMAT,
		       S_02870C_POS0_EXPORT_FORMAT(V_02870C_SPI_SHADER_4COMP) |
		       S_02870C_POS1_EXPORT_FORMAT(shader->nr_pos_exports > 1 ?
						   V_02870C_SPI_SHADER_4COMP :
						   V_02870C_SPI_SHADER_NONE) |
		       S_02870C_POS2_EXPORT_FORMAT(shader->nr_pos_exports > 2 ?
						   V_02870C_SPI_SHADER_4COMP :
						   V_02870C_SPI_SHADER_NONE) |
		       S_02870C_POS3_EXPORT_FORMAT(shader->nr_pos_exports > 3 ?
						   V_02870C_SPI_SHADER_4COMP :
						   V_02870C_SPI_SHADER_NONE));

	si_pm4_set_reg(pm4, R_00B120_SPI_SHADER_PGM_LO_VS, va >> 8);
	si_pm4_set_reg(pm4, R_00B124_SPI_SHADER_PGM_HI_VS, va >> 40);
	si_pm4_set_reg(pm4, R_00B128_SPI_SHADER_PGM_RSRC1_VS,
		       S_00B128_VGPRS((shader->num_vgprs - 1) / 4) |
		       S_00B128_SGPRS((num_sgprs - 1) / 8) |
		       S_00B128_VGPR_COMP_CNT(vgpr_comp_cnt) |
		       S_00B128_DX10_CLAMP(shader->dx10_clamp_mode));
	si_pm4_set_reg(pm4, R_00B12C_SPI_SHADER_PGM_RSRC2_VS,
		       S_00B12C_USER_SGPR(num_user_sgprs) |
		       S_00B12C_SO_BASE0_EN(!!shader->selector->so.stride[0]) |
		       S_00B12C_SO_BASE1_EN(!!shader->selector->so.stride[1]) |
		       S_00B12C_SO_BASE2_EN(!!shader->selector->so.stride[2]) |
		       S_00B12C_SO_BASE3_EN(!!shader->selector->so.stride[3]) |
		       S_00B12C_SO_EN(!!shader->selector->so.num_outputs) |
		       S_00B12C_SCRATCH_EN(shader->scratch_bytes_per_wave > 0));
	if (window_space)
		si_pm4_set_reg(pm4, R_028818_PA_CL_VTE_CNTL,
			       S_028818_VTX_XY_FMT(1) | S_028818_VTX_Z_FMT(1));
	else
		si_pm4_set_reg(pm4, R_028818_PA_CL_VTE_CNTL,
			       S_028818_VTX_W0_FMT(1) |
			       S_028818_VPORT_X_SCALE_ENA(1) | S_028818_VPORT_X_OFFSET_ENA(1) |
			       S_028818_VPORT_Y_SCALE_ENA(1) | S_028818_VPORT_Y_OFFSET_ENA(1) |
			       S_028818_VPORT_Z_SCALE_ENA(1) | S_028818_VPORT_Z_OFFSET_ENA(1));
}

static void si_shader_ps(struct si_shader *shader)
{
	struct tgsi_shader_info *info = &shader->selector->info;
	struct si_pm4_state *pm4;
	unsigned i, spi_ps_in_control;
	unsigned num_sgprs, num_user_sgprs;
	unsigned spi_baryc_cntl = 0, spi_ps_input_ena;
	uint64_t va;

	pm4 = shader->pm4 = CALLOC_STRUCT(si_pm4_state);

	if (pm4 == NULL)
		return;

	for (i = 0; i < info->num_inputs; i++) {
		switch (info->input_semantic_name[i]) {
		case TGSI_SEMANTIC_POSITION:
			/* SPI_BARYC_CNTL.POS_FLOAT_LOCATION
			 * Possible vaules:
			 * 0 -> Position = pixel center (default)
			 * 1 -> Position = pixel centroid
			 * 2 -> Position = at sample position
			 */
			switch (info->input_interpolate_loc[i]) {
			case TGSI_INTERPOLATE_LOC_CENTROID:
				spi_baryc_cntl |= S_0286E0_POS_FLOAT_LOCATION(1);
				break;
			case TGSI_INTERPOLATE_LOC_SAMPLE:
				spi_baryc_cntl |= S_0286E0_POS_FLOAT_LOCATION(2);
				break;
			}

			if (info->properties[TGSI_PROPERTY_FS_COORD_PIXEL_CENTER] ==
			    TGSI_FS_COORD_PIXEL_CENTER_INTEGER)
				spi_baryc_cntl |= S_0286E0_POS_FLOAT_ULC(1);
			break;
		}
	}

	spi_ps_in_control = S_0286D8_NUM_INTERP(shader->nparam) |
		S_0286D8_BC_OPTIMIZE_DISABLE(1);

	si_pm4_set_reg(pm4, R_0286E0_SPI_BARYC_CNTL, spi_baryc_cntl);
	spi_ps_input_ena = shader->spi_ps_input_ena;
	/* we need to enable at least one of them, otherwise we hang the GPU */
	assert(G_0286CC_PERSP_SAMPLE_ENA(spi_ps_input_ena) ||
	    G_0286CC_PERSP_CENTER_ENA(spi_ps_input_ena) ||
	    G_0286CC_PERSP_CENTROID_ENA(spi_ps_input_ena) ||
	    G_0286CC_PERSP_PULL_MODEL_ENA(spi_ps_input_ena) ||
	    G_0286CC_LINEAR_SAMPLE_ENA(spi_ps_input_ena) ||
	    G_0286CC_LINEAR_CENTER_ENA(spi_ps_input_ena) ||
	    G_0286CC_LINEAR_CENTROID_ENA(spi_ps_input_ena) ||
	    G_0286CC_LINE_STIPPLE_TEX_ENA(spi_ps_input_ena));

	si_pm4_set_reg(pm4, R_0286CC_SPI_PS_INPUT_ENA, spi_ps_input_ena);
	si_pm4_set_reg(pm4, R_0286D0_SPI_PS_INPUT_ADDR, spi_ps_input_ena);
	si_pm4_set_reg(pm4, R_0286D8_SPI_PS_IN_CONTROL, spi_ps_in_control);

	si_pm4_set_reg(pm4, R_028710_SPI_SHADER_Z_FORMAT, shader->spi_shader_z_format);
	si_pm4_set_reg(pm4, R_028714_SPI_SHADER_COL_FORMAT,
		       shader->spi_shader_col_format);
	si_pm4_set_reg(pm4, R_02823C_CB_SHADER_MASK, shader->cb_shader_mask);

	va = shader->bo->gpu_address;
	si_pm4_add_bo(pm4, shader->bo, RADEON_USAGE_READ, RADEON_PRIO_SHADER_DATA);
	si_pm4_set_reg(pm4, R_00B020_SPI_SHADER_PGM_LO_PS, va >> 8);
	si_pm4_set_reg(pm4, R_00B024_SPI_SHADER_PGM_HI_PS, va >> 40);

	num_user_sgprs = SI_PS_NUM_USER_SGPR;
	num_sgprs = shader->num_sgprs;
	/* One SGPR after user SGPRs is pre-loaded with {prim_mask, lds_offset} */
	if ((num_user_sgprs + 1) > num_sgprs) {
		/* Last 2 reserved SGPRs are used for VCC */
		num_sgprs = num_user_sgprs + 1 + 2;
	}
	assert(num_sgprs <= 104);

	si_pm4_set_reg(pm4, R_00B028_SPI_SHADER_PGM_RSRC1_PS,
		       S_00B028_VGPRS((shader->num_vgprs - 1) / 4) |
		       S_00B028_SGPRS((num_sgprs - 1) / 8) |
		       S_00B028_DX10_CLAMP(shader->dx10_clamp_mode));
	si_pm4_set_reg(pm4, R_00B02C_SPI_SHADER_PGM_RSRC2_PS,
		       S_00B02C_EXTRA_LDS_SIZE(shader->lds_size) |
		       S_00B02C_USER_SGPR(num_user_sgprs) |
		       S_00B32C_SCRATCH_EN(shader->scratch_bytes_per_wave > 0));
}

static void si_shader_init_pm4_state(struct si_shader *shader)
{

	if (shader->pm4)
		si_pm4_free_state_simple(shader->pm4);

	switch (shader->selector->type) {
	case PIPE_SHADER_VERTEX:
		if (shader->key.vs.as_es)
			si_shader_es(shader);
		else
			si_shader_vs(shader);
		break;
	case PIPE_SHADER_GEOMETRY:
		si_shader_gs(shader);
		si_shader_vs(shader->gs_copy_shader);
		break;
	case PIPE_SHADER_FRAGMENT:
		si_shader_ps(shader);
		break;
	default:
		assert(0);
	}
}

/* Compute the key for the hw shader variant */
static INLINE void si_shader_selector_key(struct pipe_context *ctx,
					  struct si_shader_selector *sel,
					  union si_shader_key *key)
{
	struct si_context *sctx = (struct si_context *)ctx;
	memset(key, 0, sizeof(*key));

	if (sel->type == PIPE_SHADER_VERTEX) {
		unsigned i;
		if (!sctx->vertex_elements)
			return;

		for (i = 0; i < sctx->vertex_elements->count; ++i)
			key->vs.instance_divisors[i] = sctx->vertex_elements->elements[i].instance_divisor;

		if (sctx->gs_shader) {
			key->vs.as_es = 1;
			key->vs.gs_used_inputs = sctx->gs_shader->gs_used_inputs;
		}
	} else if (sel->type == PIPE_SHADER_FRAGMENT) {
		struct si_state_rasterizer *rs = sctx->queued.named.rasterizer;

		if (sel->info.properties[TGSI_PROPERTY_FS_COLOR0_WRITES_ALL_CBUFS])
			key->ps.last_cbuf = MAX2(sctx->framebuffer.state.nr_cbufs, 1) - 1;
		key->ps.export_16bpc = sctx->framebuffer.export_16bpc;

		if (rs) {
			bool is_poly = (sctx->current_rast_prim >= PIPE_PRIM_TRIANGLES &&
					sctx->current_rast_prim <= PIPE_PRIM_POLYGON) ||
				       sctx->current_rast_prim >= PIPE_PRIM_TRIANGLES_ADJACENCY;
			bool is_line = !is_poly && sctx->current_rast_prim != PIPE_PRIM_POINTS;

			key->ps.color_two_side = rs->two_side;

			if (sctx->queued.named.blend) {
				key->ps.alpha_to_one = sctx->queued.named.blend->alpha_to_one &&
						       rs->multisample_enable &&
						       !sctx->framebuffer.cb0_is_integer;
			}

			key->ps.poly_stipple = rs->poly_stipple_enable && is_poly;
			key->ps.poly_line_smoothing = ((is_poly && rs->poly_smooth) ||
						       (is_line && rs->line_smooth)) &&
						      sctx->framebuffer.nr_samples <= 1;
		}

		key->ps.alpha_func = PIPE_FUNC_ALWAYS;

		/* Alpha-test should be disabled if colorbuffer 0 is integer. */
		if (sctx->queued.named.dsa &&
		    !sctx->framebuffer.cb0_is_integer)
			key->ps.alpha_func = sctx->queued.named.dsa->alpha_func;
	}
}

/* Select the hw shader variant depending on the current state. */
static int si_shader_select(struct pipe_context *ctx,
			    struct si_shader_selector *sel)
{
	struct si_context *sctx = (struct si_context *)ctx;
	union si_shader_key key;
	struct si_shader * shader = NULL;
	int r;

	si_shader_selector_key(ctx, sel, &key);

	/* Check if we don't need to change anything.
	 * This path is also used for most shaders that don't need multiple
	 * variants, it will cost just a computation of the key and this
	 * test. */
	if (likely(sel->current && memcmp(&sel->current->key, &key, sizeof(key)) == 0)) {
		return 0;
	}

	/* lookup if we have other variants in the list */
	if (sel->num_shaders > 1) {
		struct si_shader *p = sel->current, *c = p->next_variant;

		while (c && memcmp(&c->key, &key, sizeof(key)) != 0) {
			p = c;
			c = c->next_variant;
		}

		if (c) {
			p->next_variant = c->next_variant;
			shader = c;
		}
	}

	if (shader) {
		shader->next_variant = sel->current;
		sel->current = shader;
	} else {
		shader = CALLOC(1, sizeof(struct si_shader));
		shader->selector = sel;
		shader->key = key;

		shader->next_variant = sel->current;
		sel->current = shader;
		r = si_shader_create((struct si_screen*)ctx->screen, sctx->tm,
				     shader);
		if (unlikely(r)) {
			R600_ERR("Failed to build shader variant (type=%u) %d\n",
				 sel->type, r);
			sel->current = NULL;
			FREE(shader);
			return r;
		}
		si_shader_init_pm4_state(shader);
		sel->num_shaders++;
	}

	return 0;
}

static void *si_create_shader_state(struct pipe_context *ctx,
				    const struct pipe_shader_state *state,
				    unsigned pipe_shader_type)
{
	struct si_screen *sscreen = (struct si_screen *)ctx->screen;
	struct si_shader_selector *sel = CALLOC_STRUCT(si_shader_selector);
	int i;

	sel->type = pipe_shader_type;
	sel->tokens = tgsi_dup_tokens(state->tokens);
	sel->so = state->stream_output;
	tgsi_scan_shader(state->tokens, &sel->info);

	switch (pipe_shader_type) {
	case PIPE_SHADER_GEOMETRY:
		sel->gs_output_prim =
			sel->info.properties[TGSI_PROPERTY_GS_OUTPUT_PRIM];
		sel->gs_max_out_vertices =
			sel->info.properties[TGSI_PROPERTY_GS_MAX_OUTPUT_VERTICES];

		for (i = 0; i < sel->info.num_inputs; i++) {
			unsigned name = sel->info.input_semantic_name[i];
			unsigned index = sel->info.input_semantic_index[i];

			switch (name) {
			case TGSI_SEMANTIC_PRIMID:
				break;
			default:
				sel->gs_used_inputs |=
					1llu << si_shader_io_get_unique_index(name, index);
			}
		}
	}

	if (sscreen->b.debug_flags & DBG_PRECOMPILE)
		si_shader_select(ctx, sel);

	return sel;
}

static void *si_create_fs_state(struct pipe_context *ctx,
				const struct pipe_shader_state *state)
{
	return si_create_shader_state(ctx, state, PIPE_SHADER_FRAGMENT);
}

static void *si_create_gs_state(struct pipe_context *ctx,
				const struct pipe_shader_state *state)
{
	return si_create_shader_state(ctx, state, PIPE_SHADER_GEOMETRY);
}

static void *si_create_vs_state(struct pipe_context *ctx,
				const struct pipe_shader_state *state)
{
	return si_create_shader_state(ctx, state, PIPE_SHADER_VERTEX);
}

static void si_bind_vs_shader(struct pipe_context *ctx, void *state)
{
	struct si_context *sctx = (struct si_context *)ctx;
	struct si_shader_selector *sel = state;

	if (sctx->vs_shader == sel || !sel)
		return;

	sctx->vs_shader = sel;
	sctx->clip_regs.dirty = true;
}

static void si_bind_gs_shader(struct pipe_context *ctx, void *state)
{
	struct si_context *sctx = (struct si_context *)ctx;
	struct si_shader_selector *sel = state;

	if (sctx->gs_shader == sel)
		return;

	sctx->gs_shader = sel;
	sctx->clip_regs.dirty = true;
	sctx->last_rast_prim = -1; /* reset this so that it gets updated */
}

static void si_make_dummy_ps(struct si_context *sctx)
{
	if (!sctx->dummy_pixel_shader) {
		sctx->dummy_pixel_shader =
			util_make_fragment_cloneinput_shader(&sctx->b.b, 0,
							     TGSI_SEMANTIC_GENERIC,
							     TGSI_INTERPOLATE_CONSTANT);
	}
}

static void si_bind_ps_shader(struct pipe_context *ctx, void *state)
{
	struct si_context *sctx = (struct si_context *)ctx;
	struct si_shader_selector *sel = state;

	/* skip if supplied shader is one already in use */
	if (sctx->ps_shader == sel)
		return;

	/* use a dummy shader if binding a NULL shader */
	if (!sel) {
		si_make_dummy_ps(sctx);
		sel = sctx->dummy_pixel_shader;
	}

	sctx->ps_shader = sel;
}

static void si_delete_shader_selector(struct pipe_context *ctx,
				      struct si_shader_selector *sel)
{
	struct si_context *sctx = (struct si_context *)ctx;
	struct si_shader *p = sel->current, *c;

	while (p) {
		c = p->next_variant;
		if (sel->type == PIPE_SHADER_GEOMETRY) {
			si_pm4_delete_state(sctx, gs, p->pm4);
			si_pm4_delete_state(sctx, vs, p->gs_copy_shader->pm4);
		} else if (sel->type == PIPE_SHADER_FRAGMENT)
			si_pm4_delete_state(sctx, ps, p->pm4);
		else if (p->key.vs.as_es)
			si_pm4_delete_state(sctx, es, p->pm4);
		else
			si_pm4_delete_state(sctx, vs, p->pm4);
		si_shader_destroy(ctx, p);
		free(p);
		p = c;
	}

	free(sel->tokens);
	free(sel);
}

static void si_delete_vs_shader(struct pipe_context *ctx, void *state)
{
	struct si_context *sctx = (struct si_context *)ctx;
	struct si_shader_selector *sel = (struct si_shader_selector *)state;

	if (sctx->vs_shader == sel) {
		sctx->vs_shader = NULL;
	}

	si_delete_shader_selector(ctx, sel);
}

static void si_delete_gs_shader(struct pipe_context *ctx, void *state)
{
	struct si_context *sctx = (struct si_context *)ctx;
	struct si_shader_selector *sel = (struct si_shader_selector *)state;

	if (sctx->gs_shader == sel) {
		sctx->gs_shader = NULL;
	}

	si_delete_shader_selector(ctx, sel);
}

static void si_delete_ps_shader(struct pipe_context *ctx, void *state)
{
	struct si_context *sctx = (struct si_context *)ctx;
	struct si_shader_selector *sel = (struct si_shader_selector *)state;

	if (sctx->ps_shader == sel) {
		sctx->ps_shader = NULL;
	}

	si_delete_shader_selector(ctx, sel);
}

static void si_update_spi_map(struct si_context *sctx)
{
	struct si_shader *ps = sctx->ps_shader->current;
	struct si_shader *vs = si_get_vs_state(sctx);
	struct tgsi_shader_info *psinfo = &ps->selector->info;
	struct tgsi_shader_info *vsinfo = &vs->selector->info;
	struct si_pm4_state *pm4 = CALLOC_STRUCT(si_pm4_state);
	unsigned i, j, tmp;

	for (i = 0; i < psinfo->num_inputs; i++) {
		unsigned name = psinfo->input_semantic_name[i];
		unsigned index = psinfo->input_semantic_index[i];
		unsigned interpolate = psinfo->input_interpolate[i];
		unsigned param_offset = ps->ps_input_param_offset[i];

		if (name == TGSI_SEMANTIC_POSITION ||
		    name == TGSI_SEMANTIC_FACE)
			/* Read from preloaded VGPRs, not parameters */
			continue;

bcolor:
		tmp = 0;

		if (interpolate == TGSI_INTERPOLATE_CONSTANT ||
		    (interpolate == TGSI_INTERPOLATE_COLOR && sctx->flatshade))
			tmp |= S_028644_FLAT_SHADE(1);

		if (name == TGSI_SEMANTIC_GENERIC &&
		    sctx->sprite_coord_enable & (1 << index)) {
			tmp |= S_028644_PT_SPRITE_TEX(1);
		}

		for (j = 0; j < vsinfo->num_outputs; j++) {
			if (name == vsinfo->output_semantic_name[j] &&
			    index == vsinfo->output_semantic_index[j]) {
				tmp |= S_028644_OFFSET(vs->vs_output_param_offset[j]);
				break;
			}
		}

		if (j == vsinfo->num_outputs && !G_028644_PT_SPRITE_TEX(tmp)) {
			/* No corresponding output found, load defaults into input.
			 * Don't set any other bits.
			 * (FLAT_SHADE=1 completely changes behavior) */
			tmp = S_028644_OFFSET(0x20);
		}

		si_pm4_set_reg(pm4,
			       R_028644_SPI_PS_INPUT_CNTL_0 + param_offset * 4,
			       tmp);

		if (name == TGSI_SEMANTIC_COLOR &&
		    ps->key.ps.color_two_side) {
			name = TGSI_SEMANTIC_BCOLOR;
			param_offset++;
			goto bcolor;
		}
	}

	si_pm4_set_state(sctx, spi, pm4);
}

/* Initialize state related to ESGS / GSVS ring buffers */
static void si_init_gs_rings(struct si_context *sctx)
{
	unsigned esgs_ring_size = 128 * 1024;
	unsigned gsvs_ring_size = 64 * 1024 * 1024;

	assert(!sctx->gs_rings);
	sctx->gs_rings = CALLOC_STRUCT(si_pm4_state);

	sctx->esgs_ring = pipe_buffer_create(sctx->b.b.screen, PIPE_BIND_CUSTOM,
				       PIPE_USAGE_DEFAULT, esgs_ring_size);

	sctx->gsvs_ring = pipe_buffer_create(sctx->b.b.screen, PIPE_BIND_CUSTOM,
					     PIPE_USAGE_DEFAULT, gsvs_ring_size);

	if (sctx->b.chip_class >= CIK) {
		si_pm4_set_reg(sctx->gs_rings, R_030900_VGT_ESGS_RING_SIZE,
			       esgs_ring_size / 256);
		si_pm4_set_reg(sctx->gs_rings, R_030904_VGT_GSVS_RING_SIZE,
			       gsvs_ring_size / 256);
	} else {
		si_pm4_set_reg(sctx->gs_rings, R_0088C8_VGT_ESGS_RING_SIZE,
			       esgs_ring_size / 256);
		si_pm4_set_reg(sctx->gs_rings, R_0088CC_VGT_GSVS_RING_SIZE,
			       gsvs_ring_size / 256);
	}

	si_set_ring_buffer(&sctx->b.b, PIPE_SHADER_VERTEX, SI_RING_ESGS,
			   sctx->esgs_ring, 0, esgs_ring_size,
			   true, true, 4, 64);
	si_set_ring_buffer(&sctx->b.b, PIPE_SHADER_GEOMETRY, SI_RING_ESGS,
			   sctx->esgs_ring, 0, esgs_ring_size,
			   false, false, 0, 0);
	si_set_ring_buffer(&sctx->b.b, PIPE_SHADER_VERTEX, SI_RING_GSVS,
			   sctx->gsvs_ring, 0, gsvs_ring_size,
			   false, false, 0, 0);
}

/**
 * @returns 1 if \p sel has been updated to use a new scratch buffer and 0
 *          otherwise.
 */
static unsigned si_update_scratch_buffer(struct si_context *sctx,
				    struct si_shader_selector *sel)
{
	struct si_shader *shader;
	uint64_t scratch_va = sctx->scratch_buffer->gpu_address;
	unsigned char *ptr;

	if (!sel)
		return 0;

	shader = sel->current;

	/* This shader doesn't need a scratch buffer */
	if (shader->scratch_bytes_per_wave == 0)
		return 0;

	/* This shader is already configured to use the current
	 * scratch buffer. */
	if (shader->scratch_bo == sctx->scratch_buffer)
		return 0;

	assert(sctx->scratch_buffer);

	si_shader_apply_scratch_relocs(sctx, shader, scratch_va);

	/* Replace the shader bo with a new bo that has the relocs applied. */
	r600_resource_reference(&shader->bo, NULL);
	shader->bo = si_resource_create_custom(&sctx->screen->b.b, PIPE_USAGE_IMMUTABLE,
					       shader->binary.code_size);
	ptr = sctx->screen->b.ws->buffer_map(shader->bo->cs_buf, NULL, PIPE_TRANSFER_WRITE);
	util_memcpy_cpu_to_le32(ptr, shader->binary.code, shader->binary.code_size);
	sctx->screen->b.ws->buffer_unmap(shader->bo->cs_buf);

	/* Update the shader state to use the new shader bo. */
	si_shader_init_pm4_state(shader);

	r600_resource_reference(&shader->scratch_bo, sctx->scratch_buffer);

	return 1;
}

static unsigned si_get_current_scratch_buffer_size(struct si_context *sctx)
{
	if (!sctx->scratch_buffer)
		return 0;

	return sctx->scratch_buffer->b.b.width0;
}

static unsigned si_get_scratch_buffer_bytes_per_wave(struct si_context *sctx,
					struct si_shader_selector *sel)
{
	if (!sel)
		return 0;

	return sel->current->scratch_bytes_per_wave;
}

static unsigned si_get_max_scratch_bytes_per_wave(struct si_context *sctx)
{

	return MAX3(si_get_scratch_buffer_bytes_per_wave(sctx, sctx->ps_shader),
			si_get_scratch_buffer_bytes_per_wave(sctx, sctx->gs_shader),
			si_get_scratch_buffer_bytes_per_wave(sctx, sctx->vs_shader));
}

static void si_update_spi_tmpring_size(struct si_context *sctx)
{
	unsigned current_scratch_buffer_size =
		si_get_current_scratch_buffer_size(sctx);
	unsigned scratch_bytes_per_wave =
		si_get_max_scratch_bytes_per_wave(sctx);
	unsigned scratch_needed_size = scratch_bytes_per_wave *
		sctx->scratch_waves;

	if (scratch_needed_size > 0) {

		if (scratch_needed_size > current_scratch_buffer_size) {
			/* Create a bigger scratch buffer */
			pipe_resource_reference(
					(struct pipe_resource**)&sctx->scratch_buffer,
					NULL);

			sctx->scratch_buffer =
					si_resource_create_custom(&sctx->screen->b.b,
	                                PIPE_USAGE_DEFAULT, scratch_needed_size);
		}

		/* Update the shaders, so they are using the latest scratch.  The
		 * scratch buffer may have been changed since these shaders were
		 * last used, so we still need to try to update them, even if
		 * they require scratch buffers smaller than the current size.
		 */
		if (si_update_scratch_buffer(sctx, sctx->ps_shader))
			si_pm4_bind_state(sctx, ps, sctx->ps_shader->current->pm4);
		if (si_update_scratch_buffer(sctx, sctx->gs_shader))
			si_pm4_bind_state(sctx, gs, sctx->gs_shader->current->pm4);

		/* VS can be bound as ES or VS. */
		if (sctx->gs_shader) {
			if (si_update_scratch_buffer(sctx, sctx->vs_shader))
				si_pm4_bind_state(sctx, es, sctx->vs_shader->current->pm4);
		} else {
			if (si_update_scratch_buffer(sctx, sctx->vs_shader))
				si_pm4_bind_state(sctx, vs, sctx->vs_shader->current->pm4);
		}
	}

	/* The LLVM shader backend should be reporting aligned scratch_sizes. */
	assert((scratch_needed_size & ~0x3FF) == scratch_needed_size &&
		"scratch size should already be aligned correctly.");

	sctx->spi_tmpring_size = S_0286E8_WAVES(sctx->scratch_waves) |
				S_0286E8_WAVESIZE(scratch_bytes_per_wave >> 10);
}

void si_update_shaders(struct si_context *sctx)
{
	struct pipe_context *ctx = (struct pipe_context*)sctx;
	struct si_state_rasterizer *rs = sctx->queued.named.rasterizer;

	if (sctx->gs_shader) {
		si_shader_select(ctx, sctx->gs_shader);
		si_pm4_bind_state(sctx, gs, sctx->gs_shader->current->pm4);
		si_pm4_bind_state(sctx, vs, sctx->gs_shader->current->gs_copy_shader->pm4);

		sctx->b.streamout.stride_in_dw = sctx->gs_shader->so.stride;

		si_shader_select(ctx, sctx->vs_shader);
		si_pm4_bind_state(sctx, es, sctx->vs_shader->current->pm4);

		if (!sctx->gs_rings)
			si_init_gs_rings(sctx);
		if (sctx->emitted.named.gs_rings != sctx->gs_rings)
			sctx->b.flags |= SI_CONTEXT_VGT_FLUSH;
		si_pm4_bind_state(sctx, gs_rings, sctx->gs_rings);

		si_set_ring_buffer(ctx, PIPE_SHADER_GEOMETRY, SI_RING_GSVS,
				   sctx->gsvs_ring,
				   sctx->gs_shader->gs_max_out_vertices *
				   sctx->gs_shader->info.num_outputs * 16,
				   64, true, true, 4, 16);

		if (!sctx->gs_on) {
			sctx->gs_on = CALLOC_STRUCT(si_pm4_state);

			si_pm4_set_reg(sctx->gs_on, R_028B54_VGT_SHADER_STAGES_EN,
				       S_028B54_ES_EN(V_028B54_ES_STAGE_REAL) |
				       S_028B54_GS_EN(1) |
				       S_028B54_VS_EN(V_028B54_VS_STAGE_COPY_SHADER));
		}
		si_pm4_bind_state(sctx, gs_onoff, sctx->gs_on);
	} else {
		si_shader_select(ctx, sctx->vs_shader);
		si_pm4_bind_state(sctx, vs, sctx->vs_shader->current->pm4);

		sctx->b.streamout.stride_in_dw = sctx->vs_shader->so.stride;

		if (!sctx->gs_off) {
			sctx->gs_off = CALLOC_STRUCT(si_pm4_state);

			si_pm4_set_reg(sctx->gs_off, R_028A40_VGT_GS_MODE, 0);
			si_pm4_set_reg(sctx->gs_off, R_028B54_VGT_SHADER_STAGES_EN, 0);
		}
		si_pm4_bind_state(sctx, gs_onoff, sctx->gs_off);
		si_pm4_bind_state(sctx, gs_rings, NULL);
		si_pm4_bind_state(sctx, gs, NULL);
		si_pm4_bind_state(sctx, es, NULL);
	}

	si_shader_select(ctx, sctx->ps_shader);

	if (!sctx->ps_shader->current) {
		struct si_shader_selector *sel;

		/* use a dummy shader if compiling the shader (variant) failed */
		si_make_dummy_ps(sctx);
		sel = sctx->dummy_pixel_shader;
		si_shader_select(ctx, sel);
		sctx->ps_shader->current = sel->current;
	}

	si_pm4_bind_state(sctx, ps, sctx->ps_shader->current->pm4);

	if (si_pm4_state_changed(sctx, ps) || si_pm4_state_changed(sctx, vs) ||
	    sctx->sprite_coord_enable != rs->sprite_coord_enable ||
	    sctx->flatshade != rs->flatshade) {
		sctx->sprite_coord_enable = rs->sprite_coord_enable;
		sctx->flatshade = rs->flatshade;
		si_update_spi_map(sctx);
	}

	if (si_pm4_state_changed(sctx, ps) || si_pm4_state_changed(sctx, vs) ||
	    si_pm4_state_changed(sctx, gs)) {
		si_update_spi_tmpring_size(sctx);
	}

	if (sctx->ps_db_shader_control != sctx->ps_shader->current->db_shader_control) {
		sctx->ps_db_shader_control = sctx->ps_shader->current->db_shader_control;
		sctx->db_render_state.dirty = true;
	}

	if (sctx->smoothing_enabled != sctx->ps_shader->current->key.ps.poly_line_smoothing) {
		sctx->smoothing_enabled = sctx->ps_shader->current->key.ps.poly_line_smoothing;
		sctx->msaa_config.dirty = true;

		if (sctx->b.chip_class == SI)
			sctx->db_render_state.dirty = true;
	}
}

void si_init_shader_functions(struct si_context *sctx)
{
	sctx->b.b.create_vs_state = si_create_vs_state;
	sctx->b.b.create_gs_state = si_create_gs_state;
	sctx->b.b.create_fs_state = si_create_fs_state;

	sctx->b.b.bind_vs_state = si_bind_vs_shader;
	sctx->b.b.bind_gs_state = si_bind_gs_shader;
	sctx->b.b.bind_fs_state = si_bind_ps_shader;

	sctx->b.b.delete_vs_state = si_delete_vs_shader;
	sctx->b.b.delete_gs_state = si_delete_gs_shader;
	sctx->b.b.delete_fs_state = si_delete_ps_shader;
}
