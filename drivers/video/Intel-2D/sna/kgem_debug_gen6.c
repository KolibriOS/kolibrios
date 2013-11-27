/*
 * Copyright © 2007-2011 Intel Corporation
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
 *    Eric Anholt <eric@anholt.net>
 *    Chris Wilson <chris"chris-wilson.co.uk>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#include <sys/mman.h>
#include <assert.h>

#include "sna.h"
#include "sna_reg.h"
#include "gen6_render.h"

#include "kgem_debug.h"

static struct state {
	struct vertex_buffer {
		int handle;
		const char *ptr;
		int pitch;

		struct kgem_bo *current;
	} vb[33];
	struct vertex_elements {
		int buffer;
		int offset;
		bool valid;
		uint32_t type;
		uint8_t swizzle[4];
	} ve[33];
	int num_ve;

	struct dynamic_state {
		struct kgem_bo *current;
		void *base, *ptr;
	} dynamic_state;
} state;

static void gen6_update_vertex_buffer(struct kgem *kgem, const uint32_t *data)
{
	uint32_t reloc = sizeof(uint32_t) * (&data[1] - kgem->batch);
	struct kgem_bo *bo = NULL;
	void *base;
	int i;

	for (i = 0; i < kgem->nreloc; i++)
		if (kgem->reloc[i].offset == reloc)
			break;
	assert(i < kgem->nreloc);
	reloc = kgem->reloc[i].target_handle;

	if (reloc == -1) {
		base = kgem->batch;
	} else {
		list_for_each_entry(bo, &kgem->next_request->buffers, request)
			if (bo->target_handle == reloc)
				break;
		assert(&bo->request != &kgem->next_request->buffers);
		base = kgem_bo_map__debug(kgem, bo);
	}

	base = (char *)base + kgem->reloc[i].delta;
	i = data[0] >> 26;

	state.vb[i].current = bo;
	state.vb[i].ptr = base;
	state.vb[i].pitch = data[0] & 0x7ff;
}

static void gen6_update_dynamic_buffer(struct kgem *kgem, const uint32_t offset)
{
	uint32_t reloc = sizeof(uint32_t) * offset;
	struct kgem_bo *bo = NULL;
	void *base, *ptr;
	int i;

	if ((kgem->batch[offset] & 1) == 0)
		return;

	for (i = 0; i < kgem->nreloc; i++)
		if (kgem->reloc[i].offset == reloc)
			break;
	if(i < kgem->nreloc) {
		reloc = kgem->reloc[i].target_handle;

		if (reloc == 0) {
			base = kgem->batch;
		} else {
			list_for_each_entry(bo, &kgem->next_request->buffers, request)
				if (bo->handle == reloc)
					break;
			assert(&bo->request != &kgem->next_request->buffers);
			base = kgem_bo_map__debug(kgem, bo);
		}
		ptr = (char *)base + (kgem->reloc[i].delta & ~1);
	} else {
		bo = NULL;
		base = NULL;
		ptr = NULL;
	}

	state.dynamic_state.current = bo;
	state.dynamic_state.base = base;
	state.dynamic_state.ptr = ptr;
}

static uint32_t
get_ve_component(uint32_t data, int component)
{
	return (data >> (16 + (3 - component) * 4)) & 0x7;
}

static void gen6_update_vertex_elements(struct kgem *kgem, int id, const uint32_t *data)
{
	state.ve[id].buffer = data[0] >> 26;
	state.ve[id].valid = !!(data[0] & (1 << 25));
	state.ve[id].type = (data[0] >> 16) & 0x1ff;
	state.ve[id].offset = data[0] & 0x7ff;
	state.ve[id].swizzle[0] = get_ve_component(data[1], 0);
	state.ve[id].swizzle[1] = get_ve_component(data[1], 1);
	state.ve[id].swizzle[2] = get_ve_component(data[1], 2);
	state.ve[id].swizzle[3] = get_ve_component(data[1], 3);
}

static void gen6_update_sf_state(struct kgem *kgem, uint32_t *data)
{
	state.num_ve = 1 + ((data[1] >> 22) & 0x3f);
}

static void vertices_sint16_out(const struct vertex_elements *ve, const int16_t *v, int max)
{
	int c;

	ErrorF("(");
	for (c = 0; c < max; c++) {
		switch (ve->swizzle[c]) {
		case 0: ErrorF("#"); break;
		case 1: ErrorF("%d", v[c]); break;
		case 2: ErrorF("0.0"); break;
		case 3: ErrorF("1.0"); break;
		case 4: ErrorF("0x1"); break;
		case 5: break;
		default: ErrorF("?");
		}
		if (c < 3)
			ErrorF(", ");
	}
	for (; c < 4; c++) {
		switch (ve->swizzle[c]) {
		case 0: ErrorF("#"); break;
		case 1: ErrorF("1.0"); break;
		case 2: ErrorF("0.0"); break;
		case 3: ErrorF("1.0"); break;
		case 4: ErrorF("0x1"); break;
		case 5: break;
		default: ErrorF("?");
		}
		if (c < 3)
			ErrorF(", ");
	}
	ErrorF(")");
}

static void vertices_float_out(const struct vertex_elements *ve, const float *f, int max)
{
	int c, o;

	ErrorF("(");
	for (c = o = 0; c < 4 && o < max; c++) {
		switch (ve->swizzle[c]) {
		case 0: ErrorF("#"); break;
		case 1: ErrorF("%f", f[o++]); break;
		case 2: ErrorF("0.0"); break;
		case 3: ErrorF("1.0"); break;
		case 4: ErrorF("0x1"); break;
		case 5: break;
		default: ErrorF("?");
		}
		if (c < 3)
			ErrorF(", ");
	}
	for (; c < 4; c++) {
		switch (ve->swizzle[c]) {
		case 0: ErrorF("#"); break;
		case 1: ErrorF("1.0"); break;
		case 2: ErrorF("0.0"); break;
		case 3: ErrorF("1.0"); break;
		case 4: ErrorF("0x1"); break;
		case 5: break;
		default: ErrorF("?");
		}
		if (c < 3)
			ErrorF(", ");
	}
	ErrorF(")");
}

static void ve_out(const struct vertex_elements *ve, const void *ptr)
{
	switch (ve->type) {
	case GEN6_SURFACEFORMAT_R32_FLOAT:
		vertices_float_out(ve, ptr, 1);
		break;
	case GEN6_SURFACEFORMAT_R32G32_FLOAT:
		vertices_float_out(ve, ptr, 2);
		break;
	case GEN6_SURFACEFORMAT_R32G32B32_FLOAT:
		vertices_float_out(ve, ptr, 3);
		break;
	case GEN6_SURFACEFORMAT_R32G32B32A32_FLOAT:
		vertices_float_out(ve, ptr, 4);
		break;
	case GEN6_SURFACEFORMAT_R16_SINT:
		vertices_sint16_out(ve, ptr, 1);
		break;
	case GEN6_SURFACEFORMAT_R16G16_SINT:
		vertices_sint16_out(ve, ptr, 2);
		break;
	case GEN6_SURFACEFORMAT_R16G16B16A16_SINT:
		vertices_sint16_out(ve, ptr, 4);
		break;
	case GEN6_SURFACEFORMAT_R16_SSCALED:
		vertices_sint16_out(ve, ptr, 1);
		break;
	case GEN6_SURFACEFORMAT_R16G16_SSCALED:
		vertices_sint16_out(ve, ptr, 2);
		break;
	case GEN6_SURFACEFORMAT_R16G16B16A16_SSCALED:
		vertices_sint16_out(ve, ptr, 4);
		break;
	}
}

static void indirect_vertex_out(struct kgem *kgem, uint32_t v)
{
	int i = 1;

	do {
		const struct vertex_elements *ve = &state.ve[i];
		const struct vertex_buffer *vb = &state.vb[ve->buffer];
		const void *ptr = vb->ptr + v * vb->pitch + ve->offset;

		if (ve->valid)
			ve_out(ve, ptr);

		while (++i <= state.num_ve && !state.ve[i].valid)
			;

		if (i <= state.num_ve)
			ErrorF(", ");
	} while (i <= state.num_ve);
}

static void primitive_out(struct kgem *kgem, uint32_t *data)
{
	int n;

	assert((data[0] & (1<<15)) == 0); /* XXX index buffers */

	for (n = 0; n < data[1]; n++) {
		int v = data[2] + n;
		ErrorF("	[%d:%d] = ", n, v);
		indirect_vertex_out(kgem, v);
		ErrorF("\n");
	}
}

static void finish_state(struct kgem *kgem)
{
	memset(&state, 0, sizeof(state));
}

static void
state_base_out(uint32_t *data, uint32_t offset, unsigned int index,
	       const char *name)
{
    if (data[index] & 1)
	kgem_debug_print(data, offset, index,
		  "%s state base address 0x%08x\n",
		  name, data[index] & ~1);
    else
	kgem_debug_print(data, offset, index,
		  "%s state base not updated\n",
		  name);
}

static void
state_max_out(uint32_t *data, uint32_t offset, unsigned int index,
	      const char *name)
{
	if (data[index] == 1)
		kgem_debug_print(data, offset, index,
			  "%s state upper bound disabled\n", name);
	else if (data[index] & 1)
		kgem_debug_print(data, offset, index,
			  "%s state upper bound 0x%08x\n",
			  name, data[index] & ~1);
	else
		kgem_debug_print(data, offset, index,
			  "%s state upper bound not updated\n",
			  name);
}

static const char *
get_965_surfacetype(unsigned int surfacetype)
{
	switch (surfacetype) {
	case 0: return "1D";
	case 1: return "2D";
	case 2: return "3D";
	case 3: return "CUBE";
	case 4: return "BUFFER";
	case 7: return "NULL";
	default: return "unknown";
	}
}

static const char *
get_965_depthformat(unsigned int depthformat)
{
	switch (depthformat) {
	case 0: return "s8_z24float";
	case 1: return "z32float";
	case 2: return "z24s8";
	case 5: return "z16";
	default: return "unknown";
	}
}

static const char *
get_965_element_component(uint32_t data, int component)
{
	uint32_t component_control = (data >> (16 + (3 - component) * 4)) & 0x7;

	switch (component_control) {
	case 0:
		return "nostore";
	case 1:
		switch (component) {
		case 0: return "X";
		case 1: return "Y";
		case 2: return "Z";
		case 3: return "W";
		default: return "fail";
		}
	case 2:
		return "0.0";
	case 3:
		return "1.0";
	case 4:
		return "0x1";
	case 5:
		return "VID";
	default:
		return "fail";
	}
}

static const char *
get_965_prim_type(uint32_t data)
{
	uint32_t primtype = (data >> 10) & 0x1f;

	switch (primtype) {
	case 0x01: return "point list";
	case 0x02: return "line list";
	case 0x03: return "line strip";
	case 0x04: return "tri list";
	case 0x05: return "tri strip";
	case 0x06: return "tri fan";
	case 0x07: return "quad list";
	case 0x08: return "quad strip";
	case 0x09: return "line list adj";
	case 0x0a: return "line strip adj";
	case 0x0b: return "tri list adj";
	case 0x0c: return "tri strip adj";
	case 0x0d: return "tri strip reverse";
	case 0x0e: return "polygon";
	case 0x0f: return "rect list";
	case 0x10: return "line loop";
	case 0x11: return "point list bf";
	case 0x12: return "line strip cont";
	case 0x13: return "line strip bf";
	case 0x14: return "line strip cont bf";
	case 0x15: return "tri fan no stipple";
	default: return "fail";
	}
}

struct reloc {
	struct kgem_bo *bo;
	void *base;
};

static void *
get_reloc(struct kgem *kgem,
	  void *base, const uint32_t *reloc,
	  struct reloc *r)
{
	uint32_t delta = *reloc;

	memset(r, 0, sizeof(*r));

	if (base == 0) {
		uint32_t handle = sizeof(uint32_t) * (reloc - kgem->batch);
		struct kgem_bo *bo = NULL;
		int i;

		for (i = 0; i < kgem->nreloc; i++)
			if (kgem->reloc[i].offset == handle)
				break;
		assert(i < kgem->nreloc);
		handle = kgem->reloc[i].target_handle;
		delta = kgem->reloc[i].delta;

		if (handle == 0) {
			base = kgem->batch;
		} else {
			list_for_each_entry(bo, &kgem->next_request->buffers, request)
				if (bo->handle == handle)
					break;
			assert(&bo->request != &kgem->next_request->buffers);
			base = kgem_bo_map__debug(kgem, bo);
			r->bo = bo;
			r->base = base;
		}
	}

	return (char *)base + (delta & ~3);
}

static const char *
gen6_filter_to_string(uint32_t filter)
{
	switch (filter) {
	default:
	case GEN6_MAPFILTER_NEAREST: return "nearest";
	case GEN6_MAPFILTER_LINEAR: return "linear";
	}
}

static const char *
gen6_repeat_to_string(uint32_t repeat)
{
	switch (repeat) {
	default:
	case GEN6_TEXCOORDMODE_CLAMP_BORDER: return "border";
	case GEN6_TEXCOORDMODE_WRAP: return "wrap";
	case GEN6_TEXCOORDMODE_CLAMP: return "clamp";
	case GEN6_TEXCOORDMODE_MIRROR: return "mirror";
	}
}

static void
gen6_decode_sampler_state(struct kgem *kgem, const uint32_t *reloc)
{
	const struct gen6_sampler_state *ss;
	struct reloc r;
	const char *min, *mag;
	const char *s_wrap, *t_wrap, *r_wrap;

	ss = get_reloc(kgem, state.dynamic_state.ptr, reloc, &r);

	min = gen6_filter_to_string(ss->ss0.min_filter);
	mag = gen6_filter_to_string(ss->ss0.mag_filter);

	s_wrap = gen6_repeat_to_string(ss->ss1.s_wrap_mode);
	t_wrap = gen6_repeat_to_string(ss->ss1.t_wrap_mode);
	r_wrap = gen6_repeat_to_string(ss->ss1.r_wrap_mode);

	ErrorF("  Sampler 0:\n");
	ErrorF("    filter: min=%s, mag=%s\n", min, mag);
	ErrorF("    wrap: s=%s, t=%s, r=%s\n", s_wrap, t_wrap, r_wrap);

	ss++;
	min = gen6_filter_to_string(ss->ss0.min_filter);
	mag = gen6_filter_to_string(ss->ss0.mag_filter);

	s_wrap = gen6_repeat_to_string(ss->ss1.s_wrap_mode);
	t_wrap = gen6_repeat_to_string(ss->ss1.t_wrap_mode);
	r_wrap = gen6_repeat_to_string(ss->ss1.r_wrap_mode);

	ErrorF("  Sampler 1:\n");
	ErrorF("    filter: min=%s, mag=%s\n", min, mag);
	ErrorF("    wrap: s=%s, t=%s, r=%s\n", s_wrap, t_wrap, r_wrap);
}

static const char *
gen6_blend_factor_to_string(uint32_t v)
{
	switch (v) {
#define C(x) case GEN6_BLENDFACTOR_##x: return #x;
		C(ONE);
		C(SRC_COLOR);
		C(SRC_ALPHA);
		C(DST_ALPHA);
		C(DST_COLOR);
		C(SRC_ALPHA_SATURATE);
		C(CONST_COLOR);
		C(CONST_ALPHA);
		C(SRC1_COLOR);
		C(SRC1_ALPHA);
		C(ZERO);
		C(INV_SRC_COLOR);
		C(INV_SRC_ALPHA);
		C(INV_DST_ALPHA);
		C(INV_DST_COLOR);
		C(INV_CONST_COLOR);
		C(INV_CONST_ALPHA);
		C(INV_SRC1_COLOR);
		C(INV_SRC1_ALPHA);
#undef C
	default: return "???";
	}
}

static const char *
gen6_blend_function_to_string(uint32_t v)
{
	switch (v) {
#define C(x) case GEN6_BLENDFUNCTION_##x: return #x;
		C(ADD);
		C(SUBTRACT);
		C(REVERSE_SUBTRACT);
		C(MIN);
		C(MAX);
#undef C
	default: return "???";
	}
}

static float unpack_float(uint32_t dw)
{
	union {
		float f;
		uint32_t dw;
	} u;
	u.dw = dw;
	return u.f;
}

static void
gen6_decode_blend(struct kgem *kgem, const uint32_t *reloc)
{
	const struct gen6_blend_state *blend;
	struct reloc r;
	const char *dst, *src;
	const char *func;

	blend = get_reloc(kgem, state.dynamic_state.ptr, reloc, &r);

	dst = gen6_blend_factor_to_string(blend->blend0.dest_blend_factor);
	src = gen6_blend_factor_to_string(blend->blend0.source_blend_factor);
	func = gen6_blend_function_to_string(blend->blend0.blend_func);

	ErrorF("  Blend (%s): function %s, src=%s, dst=%s\n",
	       blend->blend0.blend_enable ? "enabled" : "disabled",
	       func, src, dst);
}

int kgem_gen6_decode_3d(struct kgem *kgem, uint32_t offset)
{
	static const struct {
		uint32_t opcode;
		int min_len;
		int max_len;
		const char *name;
	} opcodes[] = {
		{ 0x6101, 6, 6, "STATE_BASE_ADDRESS" },
		{ 0x6102, 2, 2 , "STATE_SIP" },
		{ 0x6104, 1, 1, "3DSTATE_PIPELINE_SELECT" },
		{ 0x680b, 1, 1, "3DSTATE_VF_STATISTICS" },
		{ 0x6904, 1, 1, "3DSTATE_PIPELINE_SELECT" },
		{ 0x7800, 7, 7, "3DSTATE_PIPELINED_POINTERS" },
		{ 0x7801, 6, 6, "3DSTATE_BINDING_TABLE_POINTERS" },
		{ 0x7808, 5, 257, "3DSTATE_VERTEX_BUFFERS" },
		{ 0x7809, 3, 256, "3DSTATE_VERTEX_ELEMENTS" },
		{ 0x780a, 3, 3, "3DSTATE_INDEX_BUFFER" },
		{ 0x780b, 1, 1, "3DSTATE_VF_STATISTICS" },
		{ 0x7900, 4, 4, "3DSTATE_DRAWING_RECTANGLE" },
		{ 0x7901, 5, 5, "3DSTATE_CONSTANT_COLOR" },
		{ 0x7905, 5, 7, "3DSTATE_DEPTH_BUFFER" },
		{ 0x7906, 2, 2, "3DSTATE_POLY_STIPPLE_OFFSET" },
		{ 0x7907, 33, 33, "3DSTATE_POLY_STIPPLE_PATTERN" },
		{ 0x7908, 3, 3, "3DSTATE_LINE_STIPPLE" },
		{ 0x7909, 2, 2, "3DSTATE_GLOBAL_DEPTH_OFFSET_CLAMP" },
		{ 0x7909, 2, 2, "3DSTATE_CLEAR_PARAMS" },
		{ 0x790a, 3, 3, "3DSTATE_AA_LINE_PARAMETERS" },
		{ 0x790b, 4, 4, "3DSTATE_GS_SVB_INDEX" },
		{ 0x790d, 3, 3, "3DSTATE_MULTISAMPLE" },
		{ 0x7910, 2, 2, "3DSTATE_CLEAR_PARAMS" },
		{ 0x7b00, 6, 6, "3DPRIMITIVE" },
		{ 0x7802, 4, 4, "3DSTATE_SAMPLER_STATE_POINTERS" },
		{ 0x7805, 3, 3, "3DSTATE_URB" },
		{ 0x780d, 4, 4, "3DSTATE_VIEWPORT_STATE_POINTERS" },
		{ 0x780e, 4, 4, "3DSTATE_CC_STATE_POINTERS" },
		{ 0x780f, 2, 2, "3DSTATE_SCISSOR_STATE_POINTERS" },
		{ 0x7810, 6, 6, "3DSTATE_VS_STATE" },
		{ 0x7811, 7, 7, "3DSTATE_GS_STATE" },
		{ 0x7812, 4, 4, "3DSTATE_CLIP_STATE" },
		{ 0x7813, 20, 20, "3DSTATE_SF_STATE" },
		{ 0x7814, 9, 9, "3DSTATE_WM_STATE" },
		{ 0x7815, 5, 5, "3DSTATE_CONSTANT_VS_STATE" },
		{ 0x7816, 5, 5, "3DSTATE_CONSTANT_GS_STATE" },
		{ 0x7817, 5, 5, "3DSTATE_CONSTANT_WM_STATE" },
		{ 0x7818, 2, 2, "3DSTATE_SAMPLE_MASK" },
	};
	uint32_t *data = kgem->batch + offset;
	uint32_t op;
	unsigned int len;
	int i, j;
	const char *desc1 = NULL;

	len = (data[0] & 0xff) + 2;
	op = (data[0] & 0xffff0000) >> 16;
	switch (op) {
	case 0x6101:
		i = 0;
		kgem_debug_print(data, offset, i++, "STATE_BASE_ADDRESS\n");
		if (kgem->gen >= 060) {
			assert(len == 10);

			state_base_out(data, offset, i++, "general");
			state_base_out(data, offset, i++, "surface");
			state_base_out(data, offset, i++, "dynamic");
			state_base_out(data, offset, i++, "indirect");
			state_base_out(data, offset, i++, "instruction");

			state_max_out(data, offset, i++, "general");
			state_max_out(data, offset, i++, "dynamic");
			state_max_out(data, offset, i++, "indirect");
			state_max_out(data, offset, i++, "instruction");

			gen6_update_dynamic_buffer(kgem, offset + 3);
		} else if (kgem->gen >= 050) {
			assert(len == 8);

			state_base_out(data, offset, i++, "general");
			state_base_out(data, offset, i++, "surface");
			state_base_out(data, offset, i++, "media");
			state_base_out(data, offset, i++, "instruction");

			state_max_out(data, offset, i++, "general");
			state_max_out(data, offset, i++, "media");
			state_max_out(data, offset, i++, "instruction");
		}

		return len;

	case 0x7801:
		if (kgem->gen >= 060) {
			assert(len == 4);

			kgem_debug_print(data, offset, 0,
				  "3DSTATE_BINDING_TABLE_POINTERS: VS mod %d, "
				  "GS mod %d, WM mod %d\n",
				  (data[0] & (1 << 8)) != 0,
				  (data[0] & (1 << 9)) != 0,
				  (data[0] & (1 << 12)) != 0);
			kgem_debug_print(data, offset, 1, "VS binding table\n");
			kgem_debug_print(data, offset, 2, "GS binding table\n");
			kgem_debug_print(data, offset, 3, "WM binding table\n");
		} else if (kgem->gen >= 040) {
			assert(len == 6);

			kgem_debug_print(data, offset, 0,
				  "3DSTATE_BINDING_TABLE_POINTERS\n");
			kgem_debug_print(data, offset, 1, "VS binding table\n");
			kgem_debug_print(data, offset, 2, "GS binding table\n");
			kgem_debug_print(data, offset, 3, "CLIP binding table\n");
			kgem_debug_print(data, offset, 4, "SF binding table\n");
			kgem_debug_print(data, offset, 5, "WM binding table\n");
		}

		return len;

	case 0x7802:
		assert(len == 4);
		kgem_debug_print(data, offset, 0, "3DSTATE_SAMPLER_STATE_POINTERS: VS mod %d, "
			  "GS mod %d, WM mod %d\n",
			  (data[0] & (1 << 8)) != 0,
			  (data[0] & (1 << 9)) != 0,
			  (data[0] & (1 << 12)) != 0);
		kgem_debug_print(data, offset, 1, "VS sampler state\n");
		kgem_debug_print(data, offset, 2, "GS sampler state\n");
		kgem_debug_print(data, offset, 3, "WM sampler state\n");
		gen6_decode_sampler_state(kgem, &data[3]);
		return len;

	case 0x7808:
		assert((len - 1) % 4 == 0);
		kgem_debug_print(data, offset, 0, "3DSTATE_VERTEX_BUFFERS\n");

		for (i = 1; i < len;) {
			gen6_update_vertex_buffer(kgem, data + i);

			kgem_debug_print(data, offset, i, "buffer %d: %s, pitch %db\n",
				  data[i] >> 26,
				  data[i] & (1 << 20) ? "random" : "sequential",
				  data[i] & 0x07ff);
			i++;
			kgem_debug_print(data, offset, i++, "buffer address\n");
			kgem_debug_print(data, offset, i++, "max index\n");
			kgem_debug_print(data, offset, i++, "mbz\n");
		}
		return len;

	case 0x7809:
		assert((len + 1) % 2 == 0);
		kgem_debug_print(data, offset, 0, "3DSTATE_VERTEX_ELEMENTS\n");

		for (i = 1; i < len;) {
			gen6_update_vertex_elements(kgem, (i - 1)/2, data + i);

			kgem_debug_print(data, offset, i, "buffer %d: %svalid, type 0x%04x, "
				  "src offset 0x%04x bytes\n",
				  data[i] >> 26,
				  data[i] & (1 << 25) ? "" : "in",
				  (data[i] >> 16) & 0x1ff,
				  data[i] & 0x07ff);
			i++;
			kgem_debug_print(data, offset, i, "(%s, %s, %s, %s), "
				  "dst offset 0x%02x bytes\n",
				  get_965_element_component(data[i], 0),
				  get_965_element_component(data[i], 1),
				  get_965_element_component(data[i], 2),
				  get_965_element_component(data[i], 3),
				  (data[i] & 0xff) * 4);
			i++;
		}
		return len;

	case 0x780d:
		assert(len == 4);
		kgem_debug_print(data, offset, 0, "3DSTATE_VIEWPORT_STATE_POINTERS\n");
		kgem_debug_print(data, offset, 1, "clip\n");
		kgem_debug_print(data, offset, 2, "sf\n");
		kgem_debug_print(data, offset, 3, "cc\n");
		return len;

	case 0x780a:
		assert(len == 3);
		kgem_debug_print(data, offset, 0, "3DSTATE_INDEX_BUFFER\n");
		kgem_debug_print(data, offset, 1, "beginning buffer address\n");
		kgem_debug_print(data, offset, 2, "ending buffer address\n");
		return len;

	case 0x780e:
		assert(len == 4);
		kgem_debug_print(data, offset, 0, "3DSTATE_CC_STATE_POINTERS\n");
		kgem_debug_print(data, offset, 1, "blend%s\n",
				 data[1] & 1 ? " update" : "");
		if (data[1] & 1)
			gen6_decode_blend(kgem, data+1);
		kgem_debug_print(data, offset, 2, "depth+stencil%s\n",
				 data[2] & 1 ? " update" : "");
		kgem_debug_print(data, offset, 3, "cc%s\n",
				 data[3] & 1 ? " update" : "");
		return len;

	case 0x780f:
		assert(len == 2);
		kgem_debug_print(data, offset, 0, "3DSTATE_SCISSOR_POINTERS\n");
		kgem_debug_print(data, offset, 1, "scissor rect offset\n");
		return len;

	case 0x7810:
		assert(len == 6);
		kgem_debug_print(data, offset, 0, "3DSTATE_VS\n");
		kgem_debug_print(data, offset, 1, "kernel pointer\n");
		kgem_debug_print(data, offset, 2, "SPF=%d, VME=%d, Sampler Count %d, "
			  "Binding table count %d\n",
			  (data[2] >> 31) & 1,
			  (data[2] >> 30) & 1,
			  (data[2] >> 27) & 7,
			  (data[2] >> 18) & 0xff);
		kgem_debug_print(data, offset, 3, "scratch offset\n");
		kgem_debug_print(data, offset, 4, "Dispatch GRF start %d, VUE read length %d, "
			  "VUE read offset %d\n",
			  (data[4] >> 20) & 0x1f,
			  (data[4] >> 11) & 0x3f,
			  (data[4] >> 4) & 0x3f);
		kgem_debug_print(data, offset, 5, "Max Threads %d, Vertex Cache %sable, "
			  "VS func %sable\n",
			  ((data[5] >> 25) & 0x7f) + 1,
			  (data[5] & (1 << 1)) != 0 ? "dis" : "en",
			  (data[5] & 1) != 0 ? "en" : "dis");
		return len;

	case 0x7811:
		assert(len == 7);
		kgem_debug_print(data, offset, 0, "3DSTATE_GS\n");
		kgem_debug_print(data, offset, 1, "kernel pointer\n");
		kgem_debug_print(data, offset, 2, "SPF=%d, VME=%d, Sampler Count %d, "
			  "Binding table count %d\n",
			  (data[2] >> 31) & 1,
			  (data[2] >> 30) & 1,
			  (data[2] >> 27) & 7,
			  (data[2] >> 18) & 0xff);
		kgem_debug_print(data, offset, 3, "scratch offset\n");
		kgem_debug_print(data, offset, 4, "Dispatch GRF start %d, VUE read length %d, "
			  "VUE read offset %d\n",
			  (data[4] & 0xf),
			  (data[4] >> 11) & 0x3f,
			  (data[4] >> 4) & 0x3f);
		kgem_debug_print(data, offset, 5, "Max Threads %d, Rendering %sable\n",
			  ((data[5] >> 25) & 0x7f) + 1,
			  (data[5] & (1 << 8)) != 0 ? "en" : "dis");
		kgem_debug_print(data, offset, 6, "Reorder %sable, Discard Adjaceny %sable, "
			  "GS %sable\n",
			  (data[6] & (1 << 30)) != 0 ? "en" : "dis",
			  (data[6] & (1 << 29)) != 0 ? "en" : "dis",
			  (data[6] & (1 << 15)) != 0 ? "en" : "dis");
		return len;

	case 0x7812:
		assert(len == 4);
		kgem_debug_print(data, offset, 0, "3DSTATE_CLIP\n");
		kgem_debug_print(data, offset, 1, "UserClip distance cull test mask 0x%x\n",
			  data[1] & 0xff);
		kgem_debug_print(data, offset, 2, "Clip %sable, API mode %s, Viewport XY test %sable, "
			  "Viewport Z test %sable, Guardband test %sable, Clip mode %d, "
			  "Perspective Divide %sable, Non-Perspective Barycentric %sable, "
			  "Tri Provoking %d, Line Provoking %d, Trifan Provoking %d\n",
			  (data[2] & (1 << 31)) != 0 ? "en" : "dis",
			  (data[2] & (1 << 30)) != 0 ? "D3D" : "OGL",
			  (data[2] & (1 << 28)) != 0 ? "en" : "dis",
			  (data[2] & (1 << 27)) != 0 ? "en" : "dis",
			  (data[2] & (1 << 26)) != 0 ? "en" : "dis",
			  (data[2] >> 13) & 7,
			  (data[2] & (1 << 9)) != 0 ? "dis" : "en",
			  (data[2] & (1 << 8)) != 0 ? "en" : "dis",
			  (data[2] >> 4) & 3,
			  (data[2] >> 2) & 3,
			  (data[2] & 3));
		kgem_debug_print(data, offset, 3, "Min PointWidth %d, Max PointWidth %d, "
			  "Force Zero RTAIndex %sable, Max VPIndex %d\n",
			  (data[3] >> 17) & 0x7ff,
			  (data[3] >> 6) & 0x7ff,
			  (data[3] & (1 << 5)) != 0 ? "en" : "dis",
			  (data[3] & 0xf));
		return len;

	case 0x7813:
		gen6_update_sf_state(kgem, data);
		assert(len == 20);
		kgem_debug_print(data, offset, 0, "3DSTATE_SF\n");
		kgem_debug_print(data, offset, 1, "Attrib Out %d, Attrib Swizzle %sable, VUE read length %d, "
			  "VUE read offset %d\n",
			  (data[1] >> 22) & 0x3f,
			  (data[1] & (1 << 21)) != 0 ? "en" : "dis",
			  (data[1] >> 11) & 0x1f,
			  (data[1] >> 4) & 0x3f);
		kgem_debug_print(data, offset, 2, "Legacy Global DepthBias %sable, FrontFace fill %d, BF fill %d, "
			  "VP transform %sable, FrontWinding_%s\n",
			  (data[2] & (1 << 11)) != 0 ? "en" : "dis",
			  (data[2] >> 5) & 3,
			  (data[2] >> 3) & 3,
			  (data[2] & (1 << 1)) != 0 ? "en" : "dis",
			  (data[2] & 1) != 0 ? "CCW" : "CW");
		kgem_debug_print(data, offset, 3, "AA %sable, CullMode %d, Scissor %sable, Multisample m ode %d\n",
			  (data[3] & (1 << 31)) != 0 ? "en" : "dis",
			  (data[3] >> 29) & 3,
			  (data[3] & (1 << 11)) != 0 ? "en" : "dis",
			  (data[3] >> 8) & 3);
		kgem_debug_print(data, offset, 4, "Last Pixel %sable, SubPixel Precision %d, Use PixelWidth %d\n",
			  (data[4] & (1 << 31)) != 0 ? "en" : "dis",
			  (data[4] & (1 << 12)) != 0 ? 4 : 8,
			  (data[4] & (1 << 11)) != 0);
		kgem_debug_print(data, offset, 5, "Global Depth Offset Constant %f\n", unpack_float(data[5]));
		kgem_debug_print(data, offset, 6, "Global Depth Offset Scale %f\n", unpack_float(data[6]));
		kgem_debug_print(data, offset, 7, "Global Depth Offset Clamp %f\n", unpack_float(data[7]));
		for (i = 0, j = 0; i < 8; i++, j+=2)
			kgem_debug_print(data, offset, i+8, "Attrib %d (Override %s%s%s%s, Const Source %d, Swizzle Select %d, "
				  "Source %d); Attrib %d (Override %s%s%s%s, Const Source %d, Swizzle Select %d, Source %d)\n",
				  j+1,
				  (data[8+i] & (1 << 31)) != 0 ? "W":"",
				  (data[8+i] & (1 << 30)) != 0 ? "Z":"",
				  (data[8+i] & (1 << 29)) != 0 ? "Y":"",
				  (data[8+i] & (1 << 28)) != 0 ? "X":"",
				  (data[8+i] >> 25) & 3, (data[8+i] >> 22) & 3,
				  (data[8+i] >> 16) & 0x1f,
				  j,
				  (data[8+i] & (1 << 15)) != 0 ? "W":"",
				  (data[8+i] & (1 << 14)) != 0 ? "Z":"",
				  (data[8+i] & (1 << 13)) != 0 ? "Y":"",
				  (data[8+i] & (1 << 12)) != 0 ? "X":"",
				  (data[8+i] >> 9) & 3, (data[8+i] >> 6) & 3,
				  (data[8+i] & 0x1f));
		kgem_debug_print(data, offset, 16, "Point Sprite TexCoord Enable\n");
		kgem_debug_print(data, offset, 17, "Const Interp Enable\n");
		kgem_debug_print(data, offset, 18, "Attrib 7-0 WrapShortest Enable\n");
		kgem_debug_print(data, offset, 19, "Attrib 15-8 WrapShortest Enable\n");

		return len;

	case 0x7814:
		assert(len == 9);
		kgem_debug_print(data, offset, 0, "3DSTATE_WM\n");
		kgem_debug_print(data, offset, 1, "kernel start pointer 0\n");
		kgem_debug_print(data, offset, 2, "SPF=%d, VME=%d, Sampler Count %d, "
			  "Binding table count %d\n",
			  (data[2] >> 31) & 1,
			  (data[2] >> 30) & 1,
			  (data[2] >> 27) & 7,
			  (data[2] >> 18) & 0xff);
		kgem_debug_print(data, offset, 3, "scratch offset\n");
		kgem_debug_print(data, offset, 4, "Depth Clear %d, Depth Resolve %d, HiZ Resolve %d, "
			  "Dispatch GRF start[0] %d, start[1] %d, start[2] %d\n",
			  (data[4] & (1 << 30)) != 0,
			  (data[4] & (1 << 28)) != 0,
			  (data[4] & (1 << 27)) != 0,
			  (data[4] >> 16) & 0x7f,
			  (data[4] >> 8) & 0x7f,
			  (data[4] & 0x7f));
		kgem_debug_print(data, offset, 5, "MaxThreads %d, PS KillPixel %d, PS computed Z %d, "
			  "PS use sourceZ %d, Thread Dispatch %d, PS use sourceW %d, Dispatch32 %d, "
			  "Dispatch16 %d, Dispatch8 %d\n",
			  ((data[5] >> 25) & 0x7f) + 1,
			  (data[5] & (1 << 22)) != 0,
			  (data[5] & (1 << 21)) != 0,
			  (data[5] & (1 << 20)) != 0,
			  (data[5] & (1 << 19)) != 0,
			  (data[5] & (1 << 8)) != 0,
			  (data[5] & (1 << 2)) != 0,
			  (data[5] & (1 << 1)) != 0,
			  (data[5] & (1 << 0)) != 0);
		kgem_debug_print(data, offset, 6, "Num SF output %d, Pos XY offset %d, ZW interp mode %d , "
			  "Barycentric interp mode 0x%x, Point raster rule %d, Multisample mode %d, "
			  "Multisample Dispatch mode %d\n",
			  (data[6] >> 20) & 0x3f,
			  (data[6] >> 18) & 3,
			  (data[6] >> 16) & 3,
			  (data[6] >> 10) & 0x3f,
			  (data[6] & (1 << 9)) != 0,
			  (data[6] >> 1) & 3,
			  (data[6] & 1));
		kgem_debug_print(data, offset, 7, "kernel start pointer 1\n");
		kgem_debug_print(data, offset, 8, "kernel start pointer 2\n");

		return len;

	case 0x7900:
		assert(len == 4);
		kgem_debug_print(data, offset, 0,
				 "3DSTATE_DRAWING_RECTANGLE\n");
		kgem_debug_print(data, offset, 1, "top left: %d, %d\n",
				 (uint16_t)(data[1] & 0xffff),
				 (uint16_t)(data[1] >> 16));
		kgem_debug_print(data, offset, 2, "bottom right: %d, %d\n",
				 (uint16_t)(data[2] & 0xffff),
				 (uint16_t)(data[2] >> 16));
		kgem_debug_print(data, offset, 3, "origin: %d, %d\n",
				 (int16_t)(data[3] & 0xffff),
				 (int16_t)(data[3] >> 16));
		return len;

	case 0x7905:
		assert(len == 7);
		kgem_debug_print(data, offset, 0,
			  "3DSTATE_DEPTH_BUFFER\n");
		kgem_debug_print(data, offset, 1, "%s, %s, pitch = %d bytes, %stiled, HiZ %d, Seperate Stencil %d\n",
			  get_965_surfacetype(data[1] >> 29),
			  get_965_depthformat((data[1] >> 18) & 0x7),
			  (data[1] & 0x0001ffff) + 1,
			  data[1] & (1 << 27) ? "" : "not ",
			  (data[1] & (1 << 22)) != 0,
			  (data[1] & (1 << 21)) != 0);
		kgem_debug_print(data, offset, 2, "depth offset\n");
		kgem_debug_print(data, offset, 3, "%dx%d\n",
			  ((data[3] & 0x0007ffc0) >> 6) + 1,
			  ((data[3] & 0xfff80000) >> 19) + 1);
		kgem_debug_print(data, offset, 4, "volume depth\n");
		kgem_debug_print(data, offset, 5, "\n");
		kgem_debug_print(data, offset, 6, "\n");
		return len;

	case 0x7a00:
		assert(len == 4 || len == 5);
		switch ((data[1] >> 14) & 0x3) {
		case 0: desc1 = "no write"; break;
		case 1: desc1 = "qword write"; break;
		case 2: desc1 = "PS_DEPTH_COUNT write"; break;
		case 3: desc1 = "TIMESTAMP write"; break;
		}
		kgem_debug_print(data, offset, 0, "PIPE_CONTROL\n");
		kgem_debug_print(data, offset, 1,
			  "%s, %scs stall, %stlb invalidate, "
			  "%ssync gfdt, %sdepth stall, %sRC write flush, "
			  "%sinst flush, %sTC flush\n",
			  desc1,
			  data[1] & (1 << 20) ? "" : "no ",
			  data[1] & (1 << 18) ? "" : "no ",
			  data[1] & (1 << 17) ? "" : "no ",
			  data[1] & (1 << 13) ? "" : "no ",
			  data[1] & (1 << 12) ? "" : "no ",
			  data[1] & (1 << 11) ? "" : "no ",
			  data[1] & (1 << 10) ? "" : "no ");
		if (len == 5) {
			kgem_debug_print(data, offset, 2, "destination address\n");
			kgem_debug_print(data, offset, 3, "immediate dword low\n");
			kgem_debug_print(data, offset, 4, "immediate dword high\n");
		} else {
			for (i = 2; i < len; i++) {
				kgem_debug_print(data, offset, i, "\n");
			}
		}
		return len;

	case 0x7b00:
		assert(len == 6);
		kgem_debug_print(data, offset, 0,
			  "3DPRIMITIVE: %s %s\n",
			  get_965_prim_type(data[0]),
			  (data[0] & (1 << 15)) ? "random" : "sequential");
		kgem_debug_print(data, offset, 1, "vertex count\n");
		kgem_debug_print(data, offset, 2, "start vertex\n");
		kgem_debug_print(data, offset, 3, "instance count\n");
		kgem_debug_print(data, offset, 4, "start instance\n");
		kgem_debug_print(data, offset, 5, "index bias\n");
		primitive_out(kgem, data);
		return len;
	}

	/* For the rest, just dump the bytes */
	for (i = 0; i < ARRAY_SIZE(opcodes); i++)
		if (op == opcodes[i].opcode)
			break;

	assert(i < ARRAY_SIZE(opcodes));

	len = 1;
	kgem_debug_print(data, offset, 0, "%s\n", opcodes[i].name);
	if (opcodes[i].max_len > 1) {
		len = (data[0] & 0xff) + 2;
		assert(len >= opcodes[i].min_len &&
		       len <= opcodes[i].max_len);
	}

	for (i = 1; i < len; i++)
		kgem_debug_print(data, offset, i, "dword %d\n", i);

	return len;
}

void kgem_gen6_finish_state(struct kgem *kgem)
{
	finish_state(kgem);
}
