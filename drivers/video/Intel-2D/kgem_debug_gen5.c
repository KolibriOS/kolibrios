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
 *    Chris Wilson <chris@chris-wilson.co.uk>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>

#include "sna.h"
#include "sna_reg.h"

#include "gen5_render.h"

#include "kgem_debug.h"

static struct state {
	struct vertex_buffer {
		int handle;
		void *base;
		int size;
		const char *ptr;
		int pitch;

		struct kgem_bo *current;
	} vb[17];
	struct vertex_elements {
		int buffer;
		int offset;
		bool valid;
		uint32_t type;
		uint8_t swizzle[4];
	} ve[17];
	int num_ve;

	struct dynamic_state {
		struct kgem_bo *current;
		void *base, *ptr;
	} dynamic_state;
} state;

static void gen5_update_vertex_buffer(struct kgem *kgem, const uint32_t *data)
{
	struct drm_i915_gem_relocation_entry *reloc;
	struct kgem_bo *bo = NULL;
	void *base, *ptr;
	int i, size;

	reloc = kgem_debug_get_reloc_entry(kgem, &data[1] - kgem->batch);
	if (reloc->target_handle == -1) {
		base = kgem->batch;
		size = kgem->nbatch * sizeof(uint32_t);
	} else {
		bo = kgem_debug_get_bo_for_reloc_entry(kgem, reloc);
		base = kgem_bo_map__debug(kgem, bo);
		size = kgem_bo_size(bo);
	}
	ptr = (char *)base + reloc->delta;

	i = data[0] >> 27;

	state.vb[i].handle = reloc->target_handle;
	state.vb[i].current = bo;
	state.vb[i].base = base;
	state.vb[i].ptr = ptr;
	state.vb[i].pitch = data[0] & 0x7ff;
	state.vb[i].size = size;
}

static uint32_t
get_ve_component(uint32_t data, int component)
{
	return (data >> (16 + (3 - component) * 4)) & 0x7;
}

static void gen5_update_vertex_elements(struct kgem *kgem, int id, const uint32_t *data)
{
	state.ve[id].buffer = data[0] >> 27;
	state.ve[id].valid = !!(data[0] & (1 << 26));
	state.ve[id].type = (data[0] >> 16) & 0x1ff;
	state.ve[id].offset = data[0] & 0x7ff;
	state.ve[id].swizzle[0] = get_ve_component(data[1], 0);
	state.ve[id].swizzle[1] = get_ve_component(data[1], 1);
	state.ve[id].swizzle[2] = get_ve_component(data[1], 2);
	state.ve[id].swizzle[3] = get_ve_component(data[1], 3);
}

static void vertices_sint16_out(const struct vertex_elements *ve, const int16_t *v, int max)
{
	int c, o;

	ErrorF("(");
	for (c = o = 0; c < 4 && o < max; c++) {
		switch (ve->swizzle[c]) {
		case 0: ErrorF("#"); break;
		case 1: ErrorF("%d", v[o++]); break;
		case 2: ErrorF("0.0"); break;
		case 3: ErrorF("1.0"); break;
		case 4: ErrorF("0x1"); break;
		case 5: break;
		default: ErrorF("?");
		}
		if (o < max)
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
		if (o < max)
			ErrorF(", ");
	}
	ErrorF(")");
}

static void ve_out(const struct vertex_elements *ve, const void *ptr)
{
	switch (ve->type) {
	case GEN5_SURFACEFORMAT_R32_FLOAT:
		vertices_float_out(ve, ptr, 1);
		break;
	case GEN5_SURFACEFORMAT_R32G32_FLOAT:
		vertices_float_out(ve, ptr, 2);
		break;
	case GEN5_SURFACEFORMAT_R32G32B32_FLOAT:
		vertices_float_out(ve, ptr, 3);
		break;
	case GEN5_SURFACEFORMAT_R32G32B32A32_FLOAT:
		vertices_float_out(ve, ptr, 4);
		break;
	case GEN5_SURFACEFORMAT_R16_SINT:
		vertices_sint16_out(ve, ptr, 1);
		break;
	case GEN5_SURFACEFORMAT_R16G16_SINT:
		vertices_sint16_out(ve, ptr, 2);
		break;
	case GEN5_SURFACEFORMAT_R16G16B16A16_SINT:
		vertices_sint16_out(ve, ptr, 4);
		break;
	case GEN5_SURFACEFORMAT_R16_SSCALED:
		vertices_sint16_out(ve, ptr, 1);
		break;
	case GEN5_SURFACEFORMAT_R16G16_SSCALED:
		vertices_sint16_out(ve, ptr, 2);
		break;
	case GEN5_SURFACEFORMAT_R16G16B16A16_SSCALED:
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

		assert(vb->pitch);
		assert(ve->offset + v*vb->pitch < vb->size);

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

#if 0
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
			base = kgem_bo_map(kgem, bo, PROT_READ);
			r->bo = bo;
			r->base = base;
		}
	}

	return (char *)base + delta;
}
#endif

int kgem_gen5_decode_3d(struct kgem *kgem, uint32_t offset)
{
	static const struct {
		uint32_t opcode;
		int min_len;
		int max_len;
		const char *name;
	} opcodes[] = {
		{ 0x6000, 3, 3, "URB_FENCE" },
		{ 0x6001, 2, 2, "CS_URB_FENCE" },
		{ 0x6002, 2, 2, "CONSTANT_BUFFER" },
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
		{ 0x7805, 3, 3, "3DSTATE_URB" },
		{ 0x7815, 5, 5, "3DSTATE_CONSTANT_VS_STATE" },
		{ 0x7816, 5, 5, "3DSTATE_CONSTANT_GS_STATE" },
		{ 0x7817, 5, 5, "3DSTATE_CONSTANT_PS_STATE" },
		{ 0x7818, 2, 2, "3DSTATE_SAMPLE_MASK" },
	};
	uint32_t *data = kgem->batch + offset;
	uint32_t op;
	unsigned int len;
	int i;
	const char *desc1 = NULL;

	len = (data[0] & 0xff) + 2;
	op = (data[0] & 0xffff0000) >> 16;
	switch (op) {
	case 0x6000:
		assert(len == 3);

		kgem_debug_print(data, offset, 0, "URB_FENCE: %s%s%s%s%s%s\n",
			  (data[0] >> 13) & 1 ? "cs " : "",
			  (data[0] >> 12) & 1 ? "vfe " : "",
			  (data[0] >> 11) & 1 ? "sf " : "",
			  (data[0] >> 10) & 1 ? "clip " : "",
			  (data[0] >> 9)  & 1 ? "gs " : "",
			  (data[0] >> 8)  & 1 ? "vs " : "");
		kgem_debug_print(data, offset, 1,
			  "vs fence: %d, gs_fence: %d, clip_fence: %d\n",
			  data[1] & 0x3ff,
			  (data[1] >> 10) & 0x3ff,
			  (data[1] >> 20) & 0x3ff);
		kgem_debug_print(data, offset, 2,
			  "sf fence: %d, vfe_fence: %d, cs_fence: %d\n",
			   data[2] & 0x3ff,
			   (data[2] >> 10) & 0x3ff,
			   (data[2] >> 20) & 0x7ff);
		return len;

	case 0x6001:
		kgem_debug_print(data, offset, 0, "CS_URB_STATE\n");
		kgem_debug_print(data, offset, 1, "entry_size: %d [%d bytes], n_entries: %d\n",
			  (data[1] >> 4) & 0x1f,
			  (((data[1] >> 4) & 0x1f) + 1) * 64,
			  data[1] & 0x7);
		return len;
	case 0x6002:
		kgem_debug_print(data, offset, 0, "CONSTANT_BUFFER: %s\n",
			  (data[0] >> 8) & 1 ? "valid" : "invalid");
		kgem_debug_print(data, offset, 1, "offset: 0x%08x, length: %d bytes\n",
			  data[1] & ~0x3f, ((data[1] & 0x3f) + 1) * 64);
		return len;
	case 0x6101:
		i = 0;
		kgem_debug_print(data, offset, i++, "STATE_BASE_ADDRESS\n");
		assert(len == 8);

		state_base_out(data, offset, i++, "general");
		state_base_out(data, offset, i++, "surface");
		state_base_out(data, offset, i++, "media");
		state_base_out(data, offset, i++, "instruction");

		state_max_out(data, offset, i++, "general");
		state_max_out(data, offset, i++, "media");
		state_max_out(data, offset, i++, "instruction");

		return len;

	case 0x7801:
		assert(len == 6);

		kgem_debug_print(data, offset, 0,
			  "3DSTATE_BINDING_TABLE_POINTERS\n");
		kgem_debug_print(data, offset, 1, "VS binding table\n");
		kgem_debug_print(data, offset, 2, "GS binding table\n");
		kgem_debug_print(data, offset, 3, "CLIP binding table\n");
		kgem_debug_print(data, offset, 4, "SF binding table\n");
		kgem_debug_print(data, offset, 5, "WM binding table\n");

		return len;

	case 0x7808:
		assert((len - 1) % 4 == 0);
		kgem_debug_print(data, offset, 0, "3DSTATE_VERTEX_BUFFERS\n");

		for (i = 1; i < len;) {
			gen5_update_vertex_buffer(kgem, data + i);

			kgem_debug_print(data, offset, i, "buffer %d: %s, pitch %db\n",
				  data[i] >> 27,
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

		memset(state.ve, 0, sizeof(state.ve)); /* XXX? */
		for (i = 1; i < len;) {
			gen5_update_vertex_elements(kgem, (i - 1)/2, data + i);

			kgem_debug_print(data, offset, i,
					 "buffer %d: %svalid, type 0x%04x, "
					 "src offset 0x%04x bytes\n",
					 data[i] >> 27,
					 data[i] & (1 << 26) ? "" : "in",
					 (data[i] >> 16) & 0x1ff,
					 data[i] & 0x07ff);
			i++;
			kgem_debug_print(data, offset, i, "(%s, %s, %s, %s)\n",
				  get_965_element_component(data[i], 0),
				  get_965_element_component(data[i], 1),
				  get_965_element_component(data[i], 2),
				  get_965_element_component(data[i], 3));
			i++;
		}
		state.num_ve = (len - 1) / 2; /* XXX? */
		return len;

	case 0x780a:
		assert(len == 3);
		kgem_debug_print(data, offset, 0, "3DSTATE_INDEX_BUFFER\n");
		kgem_debug_print(data, offset, 1, "beginning buffer address\n");
		kgem_debug_print(data, offset, 2, "ending buffer address\n");
		return len;

	case 0x7900:
		assert(len == 4);
		kgem_debug_print(data, offset, 0,
			  "3DSTATE_DRAWING_RECTANGLE\n");
		kgem_debug_print(data, offset, 1, "top left: %d,%d\n",
			  data[1] & 0xffff,
			  (data[1] >> 16) & 0xffff);
		kgem_debug_print(data, offset, 2, "bottom right: %d,%d\n",
			  data[2] & 0xffff,
			  (data[2] >> 16) & 0xffff);
		kgem_debug_print(data, offset, 3, "origin: %d,%d\n",
			  (int)data[3] & 0xffff,
			  ((int)data[3] >> 16) & 0xffff);
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

void kgem_gen5_finish_state(struct kgem *kgem)
{
	memset(&state, 0, sizeof(state));
}
