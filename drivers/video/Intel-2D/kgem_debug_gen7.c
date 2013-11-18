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

#include <assert.h>

#include "sna.h"
#include "sna_reg.h"
#include "gen7_render.h"

#include "kgem_debug.h"

static struct state {
	struct vertex_buffer {
		int handle;
		void *base;
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

static void gen7_update_vertex_buffer(struct kgem *kgem, const uint32_t *data)
{
	uint32_t reloc = sizeof(uint32_t) * (&data[1] - kgem->batch);
	struct kgem_bo *bo = NULL;
	void *base, *ptr;
	int i;

	for (i = 0; i < kgem->nreloc; i++)
		if (kgem->reloc[i].offset == reloc)
			break;
	assert(i < kgem->nreloc);
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
	ptr = (char *)base + kgem->reloc[i].delta;

	i = data[0] >> 26;

	state.vb[i].current = bo;
	state.vb[i].base = base;
	state.vb[i].ptr = ptr;
	state.vb[i].pitch = data[0] & 0x7ff;
}

static void gen7_update_dynamic_buffer(struct kgem *kgem, const uint32_t offset)
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

static void gen7_update_vertex_elements(struct kgem *kgem, int id, const uint32_t *data)
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

static void gen7_update_sf_state(struct kgem *kgem, uint32_t *data)
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
	case GEN7_SURFACEFORMAT_R32_FLOAT:
		vertices_float_out(ve, ptr, 1);
		break;
	case GEN7_SURFACEFORMAT_R32G32_FLOAT:
		vertices_float_out(ve, ptr, 2);
		break;
	case GEN7_SURFACEFORMAT_R32G32B32_FLOAT:
		vertices_float_out(ve, ptr, 3);
		break;
	case GEN7_SURFACEFORMAT_R32G32B32A32_FLOAT:
		vertices_float_out(ve, ptr, 4);
		break;
	case GEN7_SURFACEFORMAT_R16_SINT:
		vertices_sint16_out(ve, ptr, 1);
		break;
	case GEN7_SURFACEFORMAT_R16G16_SINT:
		vertices_sint16_out(ve, ptr, 2);
		break;
	case GEN7_SURFACEFORMAT_R16G16B16A16_SINT:
		vertices_sint16_out(ve, ptr, 4);
		break;
	case GEN7_SURFACEFORMAT_R16_SSCALED:
		vertices_sint16_out(ve, ptr, 1);
		break;
	case GEN7_SURFACEFORMAT_R16G16_SSCALED:
		vertices_sint16_out(ve, ptr, 2);
		break;
	case GEN7_SURFACEFORMAT_R16G16B16A16_SSCALED:
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

		if (!ve->valid)
			continue;

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

	for (n = 0; n < data[2]; n++) {
		int v = data[3] + n;
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
get_element_component(uint32_t data, int component)
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
get_prim_type(uint32_t data)
{
	uint32_t primtype = data & 0x1f;

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
gen7_filter_to_string(uint32_t filter)
{
	switch (filter) {
	default:
	case GEN7_MAPFILTER_NEAREST: return "nearest";
	case GEN7_MAPFILTER_LINEAR: return "linear";
	}
}

static const char *
gen7_repeat_to_string(uint32_t repeat)
{
	switch (repeat) {
	default:
	case GEN7_TEXCOORDMODE_CLAMP_BORDER: return "border";
	case GEN7_TEXCOORDMODE_WRAP: return "wrap";
	case GEN7_TEXCOORDMODE_CLAMP: return "clamp";
	case GEN7_TEXCOORDMODE_MIRROR: return "mirror";
	}
}

static void
gen7_decode_sampler_state(struct kgem *kgem, const uint32_t *reloc)
{
	const struct gen7_sampler_state *ss;
	struct reloc r;
	const char *min, *mag;
	const char *s_wrap, *t_wrap, *r_wrap;

	ss = get_reloc(kgem, state.dynamic_state.ptr, reloc, &r);

	min = gen7_filter_to_string(ss->ss0.min_filter);
	mag = gen7_filter_to_string(ss->ss0.mag_filter);

	s_wrap = gen7_repeat_to_string(ss->ss3.s_wrap_mode);
	t_wrap = gen7_repeat_to_string(ss->ss3.t_wrap_mode);
	r_wrap = gen7_repeat_to_string(ss->ss3.r_wrap_mode);

	ErrorF("  Sampler 0:\n");
	ErrorF("    filter: min=%s, mag=%s\n", min, mag);
	ErrorF("    wrap: s=%s, t=%s, r=%s\n", s_wrap, t_wrap, r_wrap);

	ss++;
	min = gen7_filter_to_string(ss->ss0.min_filter);
	mag = gen7_filter_to_string(ss->ss0.mag_filter);

	s_wrap = gen7_repeat_to_string(ss->ss3.s_wrap_mode);
	t_wrap = gen7_repeat_to_string(ss->ss3.t_wrap_mode);
	r_wrap = gen7_repeat_to_string(ss->ss3.r_wrap_mode);

	ErrorF("  Sampler 1:\n");
	ErrorF("    filter: min=%s, mag=%s\n", min, mag);
	ErrorF("    wrap: s=%s, t=%s, r=%s\n", s_wrap, t_wrap, r_wrap);
}

static const char *
gen7_blend_factor_to_string(uint32_t v)
{
	switch (v) {
#define C(x) case GEN7_BLENDFACTOR_##x: return #x;
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
gen7_blend_function_to_string(uint32_t v)
{
	switch (v) {
#define C(x) case GEN7_BLENDFUNCTION_##x: return #x;
		C(ADD);
		C(SUBTRACT);
		C(REVERSE_SUBTRACT);
		C(MIN);
		C(MAX);
#undef C
	default: return "???";
	}
}

static void
gen7_decode_blend(struct kgem *kgem, const uint32_t *reloc)
{
	const struct gen7_blend_state *blend;
	struct reloc r;
	const char *dst, *src;
	const char *func;

	blend = get_reloc(kgem, state.dynamic_state.ptr, reloc, &r);

	dst = gen7_blend_factor_to_string(blend->blend0.dest_blend_factor);
	src = gen7_blend_factor_to_string(blend->blend0.source_blend_factor);
	func = gen7_blend_function_to_string(blend->blend0.blend_func);

	ErrorF("  Blend (%s): function %s, src=%s, dst=%s\n",
	       blend->blend0.blend_enable ? "enabled" : "disabled",
	       func, src, dst);
}

int kgem_gen7_decode_3d(struct kgem *kgem, uint32_t offset)
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
		{ 0x780a, 3, 3, "3DSTATE_INDEX_BUFFER" },
		{ 0x7900, 4, 4, "3DSTATE_DRAWING_RECTANGLE" },
	};
	uint32_t *data = kgem->batch + offset;
	uint32_t op;
	unsigned int len;
	int i;
	const char *name;

	len = (data[0] & 0xff) + 2;
	op = (data[0] & 0xffff0000) >> 16;
	switch (op) {
	case 0x6101:
		i = 0;
		kgem_debug_print(data, offset, i++, "STATE_BASE_ADDRESS\n");
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

		gen7_update_dynamic_buffer(kgem, offset + 3);

		return len;

	case 0x7808:
		assert((len - 1) % 4 == 0);
		kgem_debug_print(data, offset, 0, "3DSTATE_VERTEX_BUFFERS\n");

		for (i = 1; i < len;) {
			gen7_update_vertex_buffer(kgem, data + i);

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
			gen7_update_vertex_elements(kgem, (i - 1)/2, data + i);

			kgem_debug_print(data, offset, i, "buffer %d: %svalid, type 0x%04x, "
				  "src offset 0x%04x bytes\n",
				  data[i] >> 26,
				  data[i] & (1 << 25) ? "" : "in",
				  (data[i] >> 16) & 0x1ff,
				  data[i] & 0x07ff);
			i++;
			kgem_debug_print(data, offset, i, "(%s, %s, %s, %s), "
				  "dst offset 0x%02x bytes\n",
				  get_element_component(data[i], 0),
				  get_element_component(data[i], 1),
				  get_element_component(data[i], 2),
				  get_element_component(data[i], 3),
				  (data[i] & 0xff) * 4);
			i++;
		}
		return len;

	case 0x780a:
		assert(len == 3);
		kgem_debug_print(data, offset, 0, "3DSTATE_INDEX_BUFFER\n");
		kgem_debug_print(data, offset, 1, "beginning buffer address\n");
		kgem_debug_print(data, offset, 2, "ending buffer address\n");
		return len;

	case 0x7b00:
		assert(len == 7);
		kgem_debug_print(data, offset, 0, "3DPRIMITIVE\n");
		kgem_debug_print(data, offset, 1, "type %s, %s\n",
			  get_prim_type(data[1]),
			  (data[1] & (1 << 15)) ? "random" : "sequential");
		kgem_debug_print(data, offset, 2, "vertex count\n");
		kgem_debug_print(data, offset, 3, "start vertex\n");
		kgem_debug_print(data, offset, 4, "instance count\n");
		kgem_debug_print(data, offset, 5, "start instance\n");
		kgem_debug_print(data, offset, 6, "index bias\n");
		primitive_out(kgem, data);
		return len;
	}

	/* For the rest, just dump the bytes */
	name = NULL;
	for (i = 0; i < ARRAY_SIZE(opcodes); i++)
		if (op == opcodes[i].opcode) {
			name = opcodes[i].name;
			break;
		}

	len = (data[0] & 0xff) + 2;
	if (name == NULL) {
		kgem_debug_print(data, offset, 0, "unknown\n");
	} else {
		kgem_debug_print(data, offset, 0, "%s\n", opcodes[i].name);
		if (opcodes[i].max_len > 1) {
			assert(len >= opcodes[i].min_len &&
					len <= opcodes[i].max_len);
		}
	}
	for (i = 1; i < len; i++)
		kgem_debug_print(data, offset, i, "dword %d\n", i);

	return len;
}

void kgem_gen7_finish_state(struct kgem *kgem)
{
	finish_state(kgem);
}
