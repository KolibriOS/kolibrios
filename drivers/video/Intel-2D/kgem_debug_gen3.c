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

#include "gen3_render.h"

#include "kgem_debug.h"

enum type {
	T_FLOAT32,
	T_FLOAT16,
};

static struct state {
	struct vertex_buffer {
		int handle;
		void *base;
		const char *ptr;
		int pitch;

		struct kgem_bo *current;
	} vb;
	struct vertex_elements {
		int offset;
		bool valid;
		enum type type;
		int size;
		uint8_t swizzle[4];
	} ve[33];
	int num_ve;
} state;

static float int_as_float(int i)
{
	union {
		float f;
		int i;
	} x;
	x.i = i;
	return x.f;
}

static void gen3_update_vertex_buffer_addr(struct kgem *kgem,
					   uint32_t offset)
{
	uint32_t handle;
	struct kgem_bo *bo = NULL;
	void *base, *ptr;
	int i;

	offset *= sizeof(uint32_t);

	for (i = 0; i < kgem->nreloc; i++)
		if (kgem->reloc[i].offset == offset)
			break;
	assert(i < kgem->nreloc);
	handle = kgem->reloc[i].target_handle;

	if (handle == 0) {
		base = kgem->batch;
	} else {
		list_for_each_entry(bo, &kgem->next_request->buffers, request)
			if (bo->handle == handle)
				break;
		assert(&bo->request != &kgem->next_request->buffers);
		base = kgem_bo_map__debug(kgem, bo);
	}
	ptr = (char *)base + kgem->reloc[i].delta;

	state.vb.current = bo;
	state.vb.base = base;
	state.vb.ptr = ptr;
}

static void gen3_update_vertex_buffer_pitch(struct kgem *kgem,
					   uint32_t offset)
{
	state.vb.pitch = kgem->batch[offset] >> 16 & 0x3f;
	state.vb.pitch *= sizeof(uint32_t);
}

static void gen3_update_vertex_elements(struct kgem *kgem, uint32_t data)
{
	state.ve[1].valid = 1;

	switch ((data >> 6) & 7) {
	case 1:
		state.ve[1].type = T_FLOAT32;
		state.ve[1].size = 3;
		state.ve[1].swizzle[0] = 1;
		state.ve[1].swizzle[1] = 1;
		state.ve[1].swizzle[2] = 1;
		state.ve[1].swizzle[3] = 3;
		break;
	case 2:
		state.ve[1].type = T_FLOAT32;
		state.ve[1].size = 4;
		state.ve[1].swizzle[0] = 1;
		state.ve[1].swizzle[1] = 1;
		state.ve[1].swizzle[2] = 1;
		state.ve[1].swizzle[3] = 1;
		break;
	case 3:
		state.ve[1].type = T_FLOAT32;
		state.ve[1].size = 2;
		state.ve[1].swizzle[0] = 1;
		state.ve[1].swizzle[1] = 1;
		state.ve[1].swizzle[2] = 2;
		state.ve[1].swizzle[3] = 3;
		break;
	case 4:
		state.ve[1].type = T_FLOAT32;
		state.ve[1].size = 3;
		state.ve[1].swizzle[0] = 1;
		state.ve[1].swizzle[1] = 1;
		state.ve[1].swizzle[2] = 3;
		state.ve[1].swizzle[3] = 1;
		break;
	}

	state.ve[2].valid = 0;
	state.ve[3].valid = 0;
}

static void gen3_update_vertex_texcoords(struct kgem *kgem, uint32_t data)
{
	int id;
	for (id = 0; id < 8; id++) {
		uint32_t fmt = (data >> (id*4)) & 0xf;
		int width;

		state.ve[id+4].valid = fmt != 0xf;

		width = 0;
		switch (fmt) {
		case 0:
			state.ve[id+4].type = T_FLOAT32;
			width = state.ve[id+4].size = 2;
			break;
		case 1:
			state.ve[id+4].type = T_FLOAT32;
			width = state.ve[id+4].size = 3;
			break;
		case 2:
			state.ve[id+4].type = T_FLOAT32;
			width = state.ve[id+4].size = 4;
			break;
		case 3:
			state.ve[id+4].type = T_FLOAT32;
			width = state.ve[id+4].size = 1;
			break;
		case 4:
			state.ve[id+4].type = T_FLOAT16;
			width = state.ve[id+4].size = 2;
			break;
		case 5:
			state.ve[id+4].type = T_FLOAT16;
			width = state.ve[id+4].size = 4;
			break;
		}

		state.ve[id+4].swizzle[0] = width > 0 ? 1 : 2;
		state.ve[id+4].swizzle[1] = width > 1 ? 1 : 2;
		state.ve[id+4].swizzle[2] = width > 2 ? 1 : 2;
		state.ve[id+4].swizzle[3] = width > 3 ? 1 : 2;
	}
}

static void gen3_update_vertex_elements_offsets(struct kgem *kgem)
{
	int i, offset;

	for (i = offset = 0; i < ARRAY_SIZE(state.ve); i++) {
		if (!state.ve[i].valid)
			continue;

		state.ve[i].offset = offset;
		offset += 4 * state.ve[i].size;
		state.num_ve = i;
	}
}

static void vertices_float32_out(const struct vertex_elements *ve, const float *f, int max)
{
	int c;

	ErrorF("(");
	for (c = 0; c < max; c++) {
		switch (ve->swizzle[c]) {
		case 0: ErrorF("#"); break;
		case 1: ErrorF("%f", f[c]); break;
		case 2: ErrorF("0.0"); break;
		case 3: ErrorF("1.0"); break;
		case 4: ErrorF("0x1"); break;
		case 5: break;
		default: ErrorF("?");
		}
		if (c < max-1)
			ErrorF(", ");
	}
	ErrorF(")");
}

static void ve_out(const struct vertex_elements *ve, const void *ptr)
{
	switch (ve->type) {
	case T_FLOAT32:
		vertices_float32_out(ve, ptr, ve->size);
		break;
	case T_FLOAT16:
		//vertices_float16_out(ve, ptr, ve->size);
		break;
	}
}

static void indirect_vertex_out(struct kgem *kgem, uint32_t v)
{
	const struct vertex_buffer *vb = &state.vb;
	int i = 1;

	do {
		const struct vertex_elements *ve = &state.ve[i];
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

static int inline_vertex_out(struct kgem *kgem, void *base)
{
	const struct vertex_buffer *vb = &state.vb;
	int i = 1;

	do {
		const struct vertex_elements *ve = &state.ve[i];
		const void *ptr = (char *)base + ve->offset;

		if (!ve->valid)
			continue;

		ve_out(ve, ptr);

		while (++i <= state.num_ve && !state.ve[i].valid)
			;

		if (i <= state.num_ve)
			ErrorF(", ");
	} while (i <= state.num_ve);

	return vb->pitch;
}

static int
gen3_decode_3d_1c(struct kgem *kgem, uint32_t offset)
{
	uint32_t *data = kgem->batch + offset;
	uint32_t opcode;

	opcode = (data[0] & 0x00f80000) >> 19;

	switch (opcode) {
	case 0x11:
		kgem_debug_print(data, offset, 0, "3DSTATE_DEPTH_SUBRECTANGLE_DISABLE\n");
		return 1;
	case 0x10:
		kgem_debug_print(data, offset, 0, "3DSTATE_SCISSOR_ENABLE %s\n",
			  data[0]&1?"enabled":"disabled");
		return 1;
	case 0x01:
		kgem_debug_print(data, offset, 0, "3DSTATE_MAP_COORD_SET_I830\n");
		return 1;
	case 0x0a:
		kgem_debug_print(data, offset, 0, "3DSTATE_MAP_CUBE_I830\n");
		return 1;
	case 0x05:
		kgem_debug_print(data, offset, 0, "3DSTATE_MAP_TEX_STREAM_I830\n");
		return 1;
	}

	kgem_debug_print(data, offset, 0, "3D UNKNOWN: 3d_1c opcode = 0x%x\n",
		  opcode);
	assert(0);
	return 1;
}

/** Sets the string dstname to describe the destination of the PS instruction */
static void
gen3_get_instruction_dst(uint32_t *data, int i, char *dstname, int do_mask)
{
    uint32_t a0 = data[i];
    int dst_nr = (a0 >> 14) & 0xf;
    char dstmask[8];
    const char *sat;

    if (do_mask) {
	if (((a0 >> 10) & 0xf) == 0xf) {
	    dstmask[0] = 0;
	} else {
	    int dstmask_index = 0;

	    dstmask[dstmask_index++] = '.';
	    if (a0 & (1 << 10))
		dstmask[dstmask_index++] = 'x';
	    if (a0 & (1 << 11))
		dstmask[dstmask_index++] = 'y';
	    if (a0 & (1 << 12))
		dstmask[dstmask_index++] = 'z';
	    if (a0 & (1 << 13))
		dstmask[dstmask_index++] = 'w';
	    dstmask[dstmask_index++] = 0;
	}

	if (a0 & (1 << 22))
	    sat = ".sat";
	else
	    sat = "";
    } else {
	dstmask[0] = 0;
	sat = "";
    }

    switch ((a0 >> 19) & 0x7) {
    case 0:
	    assert(dst_nr <= 15);
	sprintf(dstname, "R%d%s%s", dst_nr, dstmask, sat);
	break;
    case 4:
	assert(dst_nr == 0);
	sprintf(dstname, "oC%s%s", dstmask, sat);
	break;
    case 5:
	assert(dst_nr == 0);
	sprintf(dstname, "oD%s%s",  dstmask, sat);
	break;
    case 6:
	assert(dst_nr <= 3);
	sprintf(dstname, "U%d%s%s", dst_nr, dstmask, sat);
	break;
    default:
	sprintf(dstname, "RESERVED");
	break;
    }
}

static const char *
gen3_get_channel_swizzle(uint32_t select)
{
    switch (select & 0x7) {
    case 0:
	return (select & 8) ? "-x" : "x";
    case 1:
	return (select & 8) ? "-y" : "y";
    case 2:
	return (select & 8) ? "-z" : "z";
    case 3:
	return (select & 8) ? "-w" : "w";
    case 4:
	return (select & 8) ? "-0" : "0";
    case 5:
	return (select & 8) ? "-1" : "1";
    default:
	return (select & 8) ? "-bad" : "bad";
    }
}

static void
gen3_get_instruction_src_name(uint32_t src_type, uint32_t src_nr, char *name)
{
	switch (src_type) {
	case 0:
		sprintf(name, "R%d", src_nr);
		assert(src_nr <= 15);
		break;
	case 1:
		if (src_nr < 8)
			sprintf(name, "T%d", src_nr);
		else if (src_nr == 8)
			sprintf(name, "DIFFUSE");
		else if (src_nr == 9)
			sprintf(name, "SPECULAR");
		else if (src_nr == 10)
			sprintf(name, "FOG");
		else {
			assert(0);
			sprintf(name, "RESERVED");
		}
		break;
	case 2:
		sprintf(name, "C%d", src_nr);
		assert(src_nr <= 31);
		break;
	case 4:
		sprintf(name, "oC");
		assert(src_nr == 0);
		break;
	case 5:
		sprintf(name, "oD");
		assert(src_nr == 0);
		break;
	case 6:
		sprintf(name, "U%d", src_nr);
		assert(src_nr <= 3);
		break;
	default:
		sprintf(name, "RESERVED");
		assert(0);
		break;
	}
}

static void
gen3_get_instruction_src0(uint32_t *data, int i, char *srcname)
{
    uint32_t a0 = data[i];
    uint32_t a1 = data[i + 1];
    int src_nr = (a0 >> 2) & 0x1f;
    const char *swizzle_x = gen3_get_channel_swizzle((a1 >> 28) & 0xf);
    const char *swizzle_y = gen3_get_channel_swizzle((a1 >> 24) & 0xf);
    const char *swizzle_z = gen3_get_channel_swizzle((a1 >> 20) & 0xf);
    const char *swizzle_w = gen3_get_channel_swizzle((a1 >> 16) & 0xf);
    char swizzle[100];

    gen3_get_instruction_src_name((a0 >> 7) & 0x7, src_nr, srcname);
    sprintf(swizzle, ".%s%s%s%s", swizzle_x, swizzle_y, swizzle_z, swizzle_w);
    if (strcmp(swizzle, ".xyzw") != 0)
	strcat(srcname, swizzle);
}

static void
gen3_get_instruction_src1(uint32_t *data, int i, char *srcname)
{
    uint32_t a1 = data[i + 1];
    uint32_t a2 = data[i + 2];
    int src_nr = (a1 >> 8) & 0x1f;
    const char *swizzle_x = gen3_get_channel_swizzle((a1 >> 4) & 0xf);
    const char *swizzle_y = gen3_get_channel_swizzle((a1 >> 0) & 0xf);
    const char *swizzle_z = gen3_get_channel_swizzle((a2 >> 28) & 0xf);
    const char *swizzle_w = gen3_get_channel_swizzle((a2 >> 24) & 0xf);
    char swizzle[100];

    gen3_get_instruction_src_name((a1 >> 13) & 0x7, src_nr, srcname);
    sprintf(swizzle, ".%s%s%s%s", swizzle_x, swizzle_y, swizzle_z, swizzle_w);
    if (strcmp(swizzle, ".xyzw") != 0)
	strcat(srcname, swizzle);
}

static void
gen3_get_instruction_src2(uint32_t *data, int i, char *srcname)
{
    uint32_t a2 = data[i + 2];
    int src_nr = (a2 >> 16) & 0x1f;
    const char *swizzle_x = gen3_get_channel_swizzle((a2 >> 12) & 0xf);
    const char *swizzle_y = gen3_get_channel_swizzle((a2 >> 8) & 0xf);
    const char *swizzle_z = gen3_get_channel_swizzle((a2 >> 4) & 0xf);
    const char *swizzle_w = gen3_get_channel_swizzle((a2 >> 0) & 0xf);
    char swizzle[100];

    gen3_get_instruction_src_name((a2 >> 21) & 0x7, src_nr, srcname);
    sprintf(swizzle, ".%s%s%s%s", swizzle_x, swizzle_y, swizzle_z, swizzle_w);
    if (strcmp(swizzle, ".xyzw") != 0)
	strcat(srcname, swizzle);
}

static void
gen3_get_instruction_addr(uint32_t src_type, uint32_t src_nr, char *name)
{
	switch (src_type) {
	case 0:
		sprintf(name, "R%d", src_nr);
		assert(src_nr <= 15);
		break;
	case 1:
		if (src_nr < 8)
			sprintf(name, "T%d", src_nr);
		else if (src_nr == 8)
			sprintf(name, "DIFFUSE");
		else if (src_nr == 9)
			sprintf(name, "SPECULAR");
		else if (src_nr == 10)
			sprintf(name, "FOG");
		else {
			assert(0);
			sprintf(name, "RESERVED");
		}
		break;
	case 4:
		sprintf(name, "oC");
		assert(src_nr == 0);
		break;
	case 5:
		sprintf(name, "oD");
		assert(src_nr == 0);
		break;
	default:
		assert(0);
		sprintf(name, "RESERVED");
		break;
	}
}

static void
gen3_decode_alu1(uint32_t *data, uint32_t offset,
		 int i, char *instr_prefix, const char *op_name)
{
    char dst[100], src0[100];

    gen3_get_instruction_dst(data, i, dst, 1);
    gen3_get_instruction_src0(data, i, src0);

    kgem_debug_print(data, offset, i++, "%s: %s %s, %s\n", instr_prefix,
	      op_name, dst, src0);
    kgem_debug_print(data, offset, i++, "%s\n", instr_prefix);
    kgem_debug_print(data, offset, i++, "%s\n", instr_prefix);
}

static void
gen3_decode_alu2(uint32_t *data, uint32_t offset,
		 int i, char *instr_prefix, const char *op_name)
{
    char dst[100], src0[100], src1[100];

    gen3_get_instruction_dst(data, i, dst, 1);
    gen3_get_instruction_src0(data, i, src0);
    gen3_get_instruction_src1(data, i, src1);

    kgem_debug_print(data, offset, i++, "%s: %s %s, %s, %s\n", instr_prefix,
	      op_name, dst, src0, src1);
    kgem_debug_print(data, offset, i++, "%s\n", instr_prefix);
    kgem_debug_print(data, offset, i++, "%s\n", instr_prefix);
}

static void
gen3_decode_alu3(uint32_t *data, uint32_t offset,
		 int i, char *instr_prefix, const char *op_name)
{
    char dst[100], src0[100], src1[100], src2[100];

    gen3_get_instruction_dst(data, i, dst, 1);
    gen3_get_instruction_src0(data, i, src0);
    gen3_get_instruction_src1(data, i, src1);
    gen3_get_instruction_src2(data, i, src2);

    kgem_debug_print(data, offset, i++, "%s: %s %s, %s, %s, %s\n", instr_prefix,
	      op_name, dst, src0, src1, src2);
    kgem_debug_print(data, offset, i++, "%s\n", instr_prefix);
    kgem_debug_print(data, offset, i++, "%s\n", instr_prefix);
}

static void
gen3_decode_tex(uint32_t *data, uint32_t offset, int i, char *instr_prefix,
		const char *tex_name)
{
    uint32_t t0 = data[i];
    uint32_t t1 = data[i + 1];
    char dst_name[100];
    char addr_name[100];
    int sampler_nr;

    gen3_get_instruction_dst(data, i, dst_name, 0);
    gen3_get_instruction_addr((t1 >> 24) & 0x7,
			      (t1 >> 17) & 0xf,
			      addr_name);
    sampler_nr = t0 & 0xf;

    kgem_debug_print(data, offset, i++, "%s: %s %s, S%d, %s\n", instr_prefix,
	      tex_name, dst_name, sampler_nr, addr_name);
    kgem_debug_print(data, offset, i++, "%s\n", instr_prefix);
    kgem_debug_print(data, offset, i++, "%s\n", instr_prefix);
}

static void
gen3_decode_dcl(uint32_t *data, uint32_t offset, int i, char *instr_prefix)
{
	uint32_t d0 = data[i];
	const char *sampletype;
	int dcl_nr = (d0 >> 14) & 0xf;
	const char *dcl_x = d0 & (1 << 10) ? "x" : "";
	const char *dcl_y = d0 & (1 << 11) ? "y" : "";
	const char *dcl_z = d0 & (1 << 12) ? "z" : "";
	const char *dcl_w = d0 & (1 << 13) ? "w" : "";
	char dcl_mask[10];

	switch ((d0 >> 19) & 0x3) {
	case 1:
		sprintf(dcl_mask, ".%s%s%s%s", dcl_x, dcl_y, dcl_z, dcl_w);
		assert (strcmp(dcl_mask, "."));

		assert(dcl_nr <= 10);
		if (dcl_nr < 8) {
			if (strcmp(dcl_mask, ".x") != 0 &&
			    strcmp(dcl_mask, ".xy") != 0 &&
			    strcmp(dcl_mask, ".xz") != 0 &&
			    strcmp(dcl_mask, ".w") != 0 &&
			    strcmp(dcl_mask, ".xyzw") != 0) {
				assert(0);
			}
			kgem_debug_print(data, offset, i++, "%s: DCL T%d%s\n", instr_prefix,
				  dcl_nr, dcl_mask);
		} else {
			if (strcmp(dcl_mask, ".xz") == 0)
				assert(0);
			else if (strcmp(dcl_mask, ".xw") == 0)
				assert(0);
			else if (strcmp(dcl_mask, ".xzw") == 0)
				assert(0);

			if (dcl_nr == 8) {
				kgem_debug_print(data, offset, i++, "%s: DCL DIFFUSE%s\n", instr_prefix,
					  dcl_mask);
			} else if (dcl_nr == 9) {
				kgem_debug_print(data, offset, i++, "%s: DCL SPECULAR%s\n", instr_prefix,
					  dcl_mask);
			} else if (dcl_nr == 10) {
				kgem_debug_print(data, offset, i++, "%s: DCL FOG%s\n", instr_prefix,
					  dcl_mask);
			}
		}
		kgem_debug_print(data, offset, i++, "%s\n", instr_prefix);
		kgem_debug_print(data, offset, i++, "%s\n", instr_prefix);
		break;
	case 3:
		switch ((d0 >> 22) & 0x3) {
		case 0:
			sampletype = "2D";
			break;
		case 1:
			sampletype = "CUBE";
			break;
		case 2:
			sampletype = "3D";
			break;
		default:
			sampletype = "RESERVED";
			break;
		}
		assert(dcl_nr <= 15);
		kgem_debug_print(data, offset, i++, "%s: DCL S%d %s\n", instr_prefix,
			  dcl_nr, sampletype);
		kgem_debug_print(data, offset, i++, "%s\n", instr_prefix);
		kgem_debug_print(data, offset, i++, "%s\n", instr_prefix);
		break;
	default:
		kgem_debug_print(data, offset, i++, "%s: DCL RESERVED%d\n", instr_prefix, dcl_nr);
		kgem_debug_print(data, offset, i++, "%s\n", instr_prefix);
		kgem_debug_print(data, offset, i++, "%s\n", instr_prefix);
	}
}

static void
gen3_decode_instruction(uint32_t *data, uint32_t offset,
			int i, char *instr_prefix)
{
    switch ((data[i] >> 24) & 0x1f) {
    case 0x0:
	kgem_debug_print(data, offset, i++, "%s: NOP\n", instr_prefix);
	kgem_debug_print(data, offset, i++, "%s\n", instr_prefix);
	kgem_debug_print(data, offset, i++, "%s\n", instr_prefix);
	break;
    case 0x01:
	gen3_decode_alu2(data, offset, i, instr_prefix, "ADD");
	break;
    case 0x02:
	gen3_decode_alu1(data, offset, i, instr_prefix, "MOV");
	break;
    case 0x03:
	gen3_decode_alu2(data, offset, i, instr_prefix, "MUL");
	break;
    case 0x04:
	gen3_decode_alu3(data, offset, i, instr_prefix, "MAD");
	break;
    case 0x05:
	gen3_decode_alu3(data, offset, i, instr_prefix, "DP2ADD");
	break;
    case 0x06:
	gen3_decode_alu2(data, offset, i, instr_prefix, "DP3");
	break;
    case 0x07:
	gen3_decode_alu2(data, offset, i, instr_prefix, "DP4");
	break;
    case 0x08:
	gen3_decode_alu1(data, offset, i, instr_prefix, "FRC");
	break;
    case 0x09:
	gen3_decode_alu1(data, offset, i, instr_prefix, "RCP");
	break;
    case 0x0a:
	gen3_decode_alu1(data, offset, i, instr_prefix, "RSQ");
	break;
    case 0x0b:
	gen3_decode_alu1(data, offset, i, instr_prefix, "EXP");
	break;
    case 0x0c:
	gen3_decode_alu1(data, offset, i, instr_prefix, "LOG");
	break;
    case 0x0d:
	gen3_decode_alu2(data, offset, i, instr_prefix, "CMP");
	break;
    case 0x0e:
	gen3_decode_alu2(data, offset, i, instr_prefix, "MIN");
	break;
    case 0x0f:
	gen3_decode_alu2(data, offset, i, instr_prefix, "MAX");
	break;
    case 0x10:
	gen3_decode_alu1(data, offset, i, instr_prefix, "FLR");
	break;
    case 0x11:
	gen3_decode_alu1(data, offset, i, instr_prefix, "MOD");
	break;
    case 0x12:
	gen3_decode_alu1(data, offset, i, instr_prefix, "TRC");
	break;
    case 0x13:
	gen3_decode_alu2(data, offset, i, instr_prefix, "SGE");
	break;
    case 0x14:
	gen3_decode_alu2(data, offset, i, instr_prefix, "SLT");
	break;
    case 0x15:
	gen3_decode_tex(data, offset, i, instr_prefix, "TEXLD");
	break;
    case 0x16:
	gen3_decode_tex(data, offset, i, instr_prefix, "TEXLDP");
	break;
    case 0x17:
	gen3_decode_tex(data, offset, i, instr_prefix, "TEXLDB");
	break;
    case 0x19:
	gen3_decode_dcl(data, offset, i, instr_prefix);
	break;
    default:
	kgem_debug_print(data, offset, i++, "%s: unknown\n", instr_prefix);
	kgem_debug_print(data, offset, i++, "%s\n", instr_prefix);
	kgem_debug_print(data, offset, i++, "%s\n", instr_prefix);
	break;
    }
}

static const char *
gen3_decode_compare_func(uint32_t op)
{
	switch (op&0x7) {
	case 0: return "always";
	case 1: return "never";
	case 2: return "less";
	case 3: return "equal";
	case 4: return "lequal";
	case 5: return "greater";
	case 6: return "notequal";
	case 7: return "gequal";
	}
	return "";
}

static const char *
gen3_decode_stencil_op(uint32_t op)
{
	switch (op&0x7) {
	case 0: return "keep";
	case 1: return "zero";
	case 2: return "replace";
	case 3: return "incr_sat";
	case 4: return "decr_sat";
	case 5: return "greater";
	case 6: return "incr";
	case 7: return "decr";
	}
	return "";
}

#if 0
/* part of MODES_4 */
static const char *
gen3_decode_logic_op(uint32_t op)
{
	switch (op&0xf) {
	case 0: return "clear";
	case 1: return "nor";
	case 2: return "and_inv";
	case 3: return "copy_inv";
	case 4: return "and_rvrse";
	case 5: return "inv";
	case 6: return "xor";
	case 7: return "nand";
	case 8: return "and";
	case 9: return "equiv";
	case 10: return "noop";
	case 11: return "or_inv";
	case 12: return "copy";
	case 13: return "or_rvrse";
	case 14: return "or";
	case 15: return "set";
	}
	return "";
}
#endif

static const char *
gen3_decode_blend_fact(uint32_t op)
{
	switch (op&0xf) {
	case 1: return "zero";
	case 2: return "one";
	case 3: return "src_colr";
	case 4: return "inv_src_colr";
	case 5: return "src_alpha";
	case 6: return "inv_src_alpha";
	case 7: return "dst_alpha";
	case 8: return "inv_dst_alpha";
	case 9: return "dst_colr";
	case 10: return "inv_dst_colr";
	case 11: return "src_alpha_sat";
	case 12: return "cnst_colr";
	case 13: return "inv_cnst_colr";
	case 14: return "cnst_alpha";
	case 15: return "inv_const_alpha";
	}
	return "";
}

static const char *
decode_tex_coord_mode(uint32_t mode)
{
    switch (mode&0x7) {
    case 0: return "wrap";
    case 1: return "mirror";
    case 2: return "clamp_edge";
    case 3: return "cube";
    case 4: return "clamp_border";
    case 5: return "mirror_once";
    }
    return "";
}

static const char *
gen3_decode_sample_filter(uint32_t mode)
{
	switch (mode&0x7) {
	case 0: return "nearest";
	case 1: return "linear";
	case 2: return "anisotropic";
	case 3: return "4x4_1";
	case 4: return "4x4_2";
	case 5: return "4x4_flat";
	case 6: return "6x5_mono";
	}
	return "";
}

static int
gen3_decode_load_state_immediate_1(struct kgem *kgem, uint32_t offset)
{
	const uint32_t *data = kgem->batch + offset;
	int len, i, word;

	kgem_debug_print(data, offset, 0, "3DSTATE_LOAD_STATE_IMMEDIATE_1\n");
	len = (data[0] & 0x0000000f) + 2;
	i = 1;
	for (word = 0; word <= 8; word++) {
		if (data[0] & (1 << (4 + word))) {
			switch (word) {
			case 0:
				kgem_debug_print(data, offset, i, "S0: vbo offset: 0x%08x%s\n",
					  data[i]&(~1),data[i]&1?", auto cache invalidate disabled":"");
				gen3_update_vertex_buffer_addr(kgem, offset + i);
				break;
			case 1:
				kgem_debug_print(data, offset, i, "S1: vertex width: %i, vertex pitch: %i\n",
					  (data[i]>>24)&0x3f,(data[i]>>16)&0x3f);
				gen3_update_vertex_buffer_pitch(kgem, offset + i);
				break;
			case 2:
				{
					char buf[200];
					int len = 0;
					int tex_num;
					for (tex_num = 0; tex_num < 8; tex_num++) {
						switch((data[i]>>tex_num*4)&0xf) {
						case 0: len += sprintf(buf + len, "%i=2D ", tex_num); break;
						case 1: len += sprintf(buf + len, "%i=3D ", tex_num); break;
						case 2: len += sprintf(buf + len, "%i=4D ", tex_num); break;
						case 3: len += sprintf(buf + len, "%i=1D ", tex_num); break;
						case 4: len += sprintf(buf + len, "%i=2D_16 ", tex_num); break;
						case 5: len += sprintf(buf + len, "%i=4D_16 ", tex_num); break;
						case 0xf: len += sprintf(buf + len, "%i=NP ", tex_num); break;
						}
					}
					kgem_debug_print(data, offset, i, "S2: texcoord formats: %s\n", buf);
					gen3_update_vertex_texcoords(kgem, data[i]);
				}

				break;
			case 3:
				kgem_debug_print(data, offset, i, "S3: not documented\n");
				break;
			case 4:
				{
					const char *cullmode = "";
					const char *vfmt_xyzw = "";
					switch((data[i]>>13)&0x3) {
					case 0: cullmode = "both"; break;
					case 1: cullmode = "none"; break;
					case 2: cullmode = "cw"; break;
					case 3: cullmode = "ccw"; break;
					}
					switch(data[i] & (7<<6 | 1<<2)) {
					case 1<<6: vfmt_xyzw = "XYZ,"; break;
					case 2<<6: vfmt_xyzw = "XYZW,"; break;
					case 3<<6: vfmt_xyzw = "XY,"; break;
					case 4<<6: vfmt_xyzw = "XYW,"; break;
					case 1<<6 | 1<<2: vfmt_xyzw = "XYZF,"; break;
					case 2<<6 | 1<<2: vfmt_xyzw = "XYZWF,"; break;
					case 3<<6 | 1<<2: vfmt_xyzw = "XYF,"; break;
					case 4<<6 | 1<<2: vfmt_xyzw = "XYWF,"; break;
					}
					kgem_debug_print(data, offset, i, "S4: point_width=%i, line_width=%.1f,"
						  "%s%s%s%s%s cullmode=%s, vfmt=%s%s%s%s%s%s%s%s "
						  "%s%s%s\n",
						  (data[i]>>23)&0x1ff,
						  ((data[i]>>19)&0xf) / 2.0,
						  data[i]&(0xf<<15)?" flatshade=":"",
						  data[i]&(1<<18)?"Alpha,":"",
						  data[i]&(1<<17)?"Fog,":"",
						  data[i]&(1<<16)?"Specular,":"",
						  data[i]&(1<<15)?"Color,":"",
						  cullmode,
						  data[i]&(1<<12)?"PointWidth,":"",
						  data[i]&(1<<11)?"SpecFog,":"",
						  data[i]&(1<<10)?"Color,":"",
						  data[i]&(1<<9)?"DepthOfs,":"",
						  vfmt_xyzw,
						  data[i]&(1<<9)?"FogParam,":"",
						  data[i]&(1<<5)?"force default diffuse, ":"",
						  data[i]&(1<<4)?"force default specular, ":"",
						  data[i]&(1<<3)?"local depth ofs enable, ":"",
						  data[i]&(1<<1)?"point sprite enable, ":"",
						  data[i]&(1<<0)?"line AA enable, ":"");
					gen3_update_vertex_elements(kgem, data[i]);
					break;
				}
			case 5:
				{
					kgem_debug_print(data, offset, i, "S5:%s%s%s%s%s"
						  "%s%s%s%s stencil_ref=0x%x, stencil_test=%s, "
						  "stencil_fail=%s, stencil_pass_z_fail=%s, "
						  "stencil_pass_z_pass=%s, %s%s%s%s\n",
						  data[i]&(0xf<<28)?" write_disable=":"",
						  data[i]&(1<<31)?"Alpha,":"",
						  data[i]&(1<<30)?"Red,":"",
						  data[i]&(1<<29)?"Green,":"",
						  data[i]&(1<<28)?"Blue,":"",
						  data[i]&(1<<27)?" force default point size,":"",
						  data[i]&(1<<26)?" last pixel enable,":"",
						  data[i]&(1<<25)?" global depth ofs enable,":"",
						  data[i]&(1<<24)?" fog enable,":"",
						  (data[i]>>16)&0xff,
						  gen3_decode_compare_func(data[i]>>13),
						  gen3_decode_stencil_op(data[i]>>10),
						  gen3_decode_stencil_op(data[i]>>7),
						  gen3_decode_stencil_op(data[i]>>4),
						  data[i]&(1<<3)?"stencil write enable, ":"",
						  data[i]&(1<<2)?"stencil test enable, ":"",
						  data[i]&(1<<1)?"color dither enable, ":"",
						  data[i]&(1<<0)?"logicop enable, ":"");
				}
				break;
			case 6:
				kgem_debug_print(data, offset, i, "S6: %salpha_test=%s, alpha_ref=0x%x, "
					  "depth_test=%s, %ssrc_blnd_fct=%s, dst_blnd_fct=%s, "
					  "%s%stristrip_provoking_vertex=%i\n",
					  data[i]&(1<<31)?"alpha test enable, ":"",
					  gen3_decode_compare_func(data[i]>>28),
					  data[i]&(0xff<<20),
					  gen3_decode_compare_func(data[i]>>16),
					  data[i]&(1<<15)?"cbuf blend enable, ":"",
					  gen3_decode_blend_fact(data[i]>>8),
					  gen3_decode_blend_fact(data[i]>>4),
					  data[i]&(1<<3)?"depth write enable, ":"",
					  data[i]&(1<<2)?"cbuf write enable, ":"",
					  data[i]&(0x3));
				break;
			case 7:
				kgem_debug_print(data, offset, i, "S7: depth offset constant: 0x%08x\n", data[i]);
				break;
			}
			i++;
		}
	}

	assert(len == i);
	return len;
}

static int
gen3_decode_3d_1d(struct kgem *kgem, uint32_t offset)
{
	uint32_t *data = kgem->batch + offset;
	unsigned int len, i, c, idx, word, map, sampler, instr;
	const char *format, *zformat, *type;
	uint32_t opcode;
	static const struct {
		uint32_t opcode;
		int min_len;
		int max_len;
		const char *name;
	} opcodes_3d_1d[] = {
		{ 0x86, 4, 4, "3DSTATE_CHROMA_KEY" },
		{ 0x88, 2, 2, "3DSTATE_CONSTANT_BLEND_COLOR" },
		{ 0x99, 2, 2, "3DSTATE_DEFAULT_DIFFUSE" },
		{ 0x9a, 2, 2, "3DSTATE_DEFAULT_SPECULAR" },
		{ 0x98, 2, 2, "3DSTATE_DEFAULT_Z" },
		{ 0x97, 2, 2, "3DSTATE_DEPTH_OFFSET_SCALE" },
		{ 0x9d, 65, 65, "3DSTATE_FILTER_COEFFICIENTS_4X4" },
		{ 0x9e, 4, 4, "3DSTATE_MONO_FILTER" },
		{ 0x89, 4, 4, "3DSTATE_FOG_MODE" },
		{ 0x8f, 2, 16, "3DSTATE_MAP_PALLETE_LOAD_32" },
		{ 0x83, 2, 2, "3DSTATE_SPAN_STIPPLE" },
	}, *opcode_3d_1d;

	opcode = (data[0] & 0x00ff0000) >> 16;

	switch (opcode) {
	case 0x07:
		/* This instruction is unusual.  A 0 length means just 1 DWORD instead of
		 * 2.  The 0 length is specified in one place to be unsupported, but
		 * stated to be required in another, and 0 length LOAD_INDIRECTs appear
		 * to cause no harm at least.
		 */
		kgem_debug_print(data, offset, 0, "3DSTATE_LOAD_INDIRECT\n");
		len = (data[0] & 0x000000ff) + 1;
		i = 1;
		if (data[0] & (0x01 << 8)) {
			kgem_debug_print(data, offset, i++, "SIS.0\n");
			kgem_debug_print(data, offset, i++, "SIS.1\n");
		}
		if (data[0] & (0x02 << 8)) {
			kgem_debug_print(data, offset, i++, "DIS.0\n");
		}
		if (data[0] & (0x04 << 8)) {
			kgem_debug_print(data, offset, i++, "SSB.0\n");
			kgem_debug_print(data, offset, i++, "SSB.1\n");
		}
		if (data[0] & (0x08 << 8)) {
			kgem_debug_print(data, offset, i++, "MSB.0\n");
			kgem_debug_print(data, offset, i++, "MSB.1\n");
		}
		if (data[0] & (0x10 << 8)) {
			kgem_debug_print(data, offset, i++, "PSP.0\n");
			kgem_debug_print(data, offset, i++, "PSP.1\n");
		}
		if (data[0] & (0x20 << 8)) {
			kgem_debug_print(data, offset, i++, "PSC.0\n");
			kgem_debug_print(data, offset, i++, "PSC.1\n");
		}
		assert(len == i);
		return len;
	case 0x04:
		return gen3_decode_load_state_immediate_1(kgem, offset);
	case 0x03:
		kgem_debug_print(data, offset, 0, "3DSTATE_LOAD_STATE_IMMEDIATE_2\n");
		len = (data[0] & 0x0000000f) + 2;
		i = 1;
		for (word = 6; word <= 14; word++) {
			if (data[0] & (1 << word)) {
				if (word == 6)
					kgem_debug_print(data, offset, i++, "TBCF\n");
				else if (word >= 7 && word <= 10) {
					kgem_debug_print(data, offset, i++, "TB%dC\n", word - 7);
					kgem_debug_print(data, offset, i++, "TB%dA\n", word - 7);
				} else if (word >= 11 && word <= 14) {
					kgem_debug_print(data, offset, i, "TM%dS0: offset=0x%08x, %s\n",
						  word - 11,
						  data[i]&0xfffffffe,
						  data[i]&1?"use fence":"");
					i++;
					kgem_debug_print(data, offset, i, "TM%dS1: height=%i, width=%i, %s\n",
						  word - 11,
						  data[i]>>21, (data[i]>>10)&0x3ff,
						  data[i]&2?(data[i]&1?"y-tiled":"x-tiled"):"");
					i++;
					kgem_debug_print(data, offset, i, "TM%dS2: pitch=%i, \n",
						  word - 11,
						  ((data[i]>>21) + 1)*4);
					i++;
					kgem_debug_print(data, offset, i++, "TM%dS3\n", word - 11);
					kgem_debug_print(data, offset, i++, "TM%dS4: dflt color\n", word - 11);
				}
			}
		}
		assert(len == i);
		return len;
	case 0x00:
		kgem_debug_print(data, offset, 0, "3DSTATE_MAP_STATE\n");
		len = (data[0] & 0x0000003f) + 2;
		kgem_debug_print(data, offset, 1, "mask\n");

		i = 2;
		for (map = 0; map <= 15; map++) {
			if (data[1] & (1 << map)) {
				int width, height, pitch, dword;
				struct drm_i915_gem_relocation_entry *reloc;
				const char *tiling;

				reloc = kgem_debug_get_reloc_entry(kgem, &data[i] - kgem->batch);
				assert(reloc->target_handle);

				dword = data[i];
				kgem_debug_print(data, offset, i++, "map %d MS2 %s%s%s, handle=%d\n", map,
					  dword&(1<<31)?"untrusted surface, ":"",
					  dword&(1<<1)?"vertical line stride enable, ":"",
					  dword&(1<<0)?"vertical ofs enable, ":"",
					  reloc->target_handle);

				dword = data[i];
				width = ((dword >> 10) & ((1 << 11) - 1))+1;
				height = ((dword >> 21) & ((1 << 11) - 1))+1;

				tiling = "none";
				if (dword & (1 << 2))
					tiling = "fenced";
				else if (dword & (1 << 1))
					tiling = dword & (1 << 0) ? "Y" : "X";
				type = " BAD";
				format = " (invalid)";
				switch ((dword>>7) & 0x7) {
				case 1:
					type = "8";
					switch ((dword>>3) & 0xf) {
					case 0: format = "I"; break;
					case 1: format = "L"; break;
					case 4: format = "A"; break;
					case 5: format = " mono"; break;
					}
					break;
				case 2:
					type = "16";
					switch ((dword>>3) & 0xf) {
					case 0: format = " rgb565"; break;
					case 1: format = " argb1555"; break;
					case 2: format = " argb4444"; break;
					case 3: format = " ay88"; break;
					case 5: format = " 88dvdu"; break;
					case 6: format = " bump655"; break;
					case 7: format = "I"; break;
					case 8: format = "L"; break;
					case 9: format = "A"; break;
					}
					break;
				case 3:
					type = "32";
					switch ((dword>>3) & 0xf) {
					case 0: format = " argb8888"; break;
					case 1: format = " abgr8888"; break;
					case 2: format = " xrgb8888"; break;
					case 3: format = " xbgr8888"; break;
					case 4: format = " qwvu8888"; break;
					case 5: format = " axvu8888"; break;
					case 6: format = " lxvu8888"; break;
					case 7: format = " xlvu8888"; break;
					case 8: format = " argb2101010"; break;
					case 9: format = " abgr2101010"; break;
					case 10: format = " awvu2101010"; break;
					case 11: format = " gr1616"; break;
					case 12: format = " vu1616"; break;
					case 13: format = " xI824"; break;
					case 14: format = " xA824"; break;
					case 15: format = " xL824"; break;
					}
					break;
				case 5:
					type = "422";
					switch ((dword>>3) & 0xf) {
					case 0: format = " yuv_swapy"; break;
					case 1: format = " yuv"; break;
					case 2: format = " yuv_swapuv"; break;
					case 3: format = " yuv_swapuvy"; break;
					}
					break;
				case 6:
					type = "compressed";
					switch ((dword>>3) & 0x7) {
					case 0: format = " dxt1"; break;
					case 1: format = " dxt2_3"; break;
					case 2: format = " dxt4_5"; break;
					case 3: format = " fxt1"; break;
					case 4: format = " dxt1_rb"; break;
					}
					break;
				case 7:
					type = "4b indexed";
					switch ((dword>>3) & 0xf) {
					case 7: format = " argb8888"; break;
					}
					break;
				default:
					format = "BAD";
					break;
				}
				dword = data[i];
				kgem_debug_print(data, offset, i++, "map %d MS3 [width=%d, height=%d, format=%s%s, tiling=%s%s]\n",
					  map, width, height, type, format, tiling,
					  dword&(1<<9)?" palette select":"");

				dword = data[i];
				pitch = 4*(((dword >> 21) & ((1 << 11) - 1))+1);
				kgem_debug_print(data, offset, i++, "map %d MS4 [pitch=%d, max_lod=%i, vol_depth=%i, cube_face_ena=%x, %s]\n",
					  map, pitch,
					  (dword>>9)&0x3f, dword&0xff, (dword>>15)&0x3f,
					  dword&(1<<8)?"miplayout legacy":"miplayout right");
			}
		}
		assert(len == i);
		return len;
	case 0x06:
		kgem_debug_print(data, offset, 0, "3DSTATE_PIXEL_SHADER_CONSTANTS\n");
		len = (data[0] & 0x000000ff) + 2;

		i = 2;
		for (c = 0; c <= 31; c++) {
			if (data[1] & (1 << c)) {
				kgem_debug_print(data, offset, i, "C%d.X = %f\n",
					  c, int_as_float(data[i]));
				i++;
				kgem_debug_print(data, offset, i, "C%d.Y = %f\n",
					  c, int_as_float(data[i]));
				i++;
				kgem_debug_print(data, offset, i, "C%d.Z = %f\n",
					  c, int_as_float(data[i]));
				i++;
				kgem_debug_print(data, offset, i, "C%d.W = %f\n",
					  c, int_as_float(data[i]));
				i++;
			}
		}
		assert(len == i);
		return len;
	case 0x05:
		kgem_debug_print(data, offset, 0, "3DSTATE_PIXEL_SHADER_PROGRAM\n");
		len = (data[0] & 0x000000ff) + 2;
		assert(((len-1) % 3) == 0);
		assert(len <= 370);
		i = 1;
		for (instr = 0; instr < (len - 1) / 3; instr++) {
			char instr_prefix[10];

			sprintf(instr_prefix, "PS%03d", instr);
			gen3_decode_instruction(data, offset, i, instr_prefix);
			i += 3;
		}
		return len;
	case 0x01:
		kgem_debug_print(data, offset, 0, "3DSTATE_SAMPLER_STATE\n");
		kgem_debug_print(data, offset, 1, "mask\n");
		len = (data[0] & 0x0000003f) + 2;
		i = 2;
		for (sampler = 0; sampler <= 15; sampler++) {
			if (data[1] & (1 << sampler)) {
				uint32_t dword;
				const char *mip_filter = "";
				dword = data[i];
				switch ((dword>>20)&0x3) {
				case 0: mip_filter = "none"; break;
				case 1: mip_filter = "nearest"; break;
				case 3: mip_filter = "linear"; break;
				}
				kgem_debug_print(data, offset, i++, "sampler %d SS2:%s%s%s "
					  "base_mip_level=%i, mip_filter=%s, mag_filter=%s, min_filter=%s "
					  "lod_bias=%.2f,%s max_aniso=%i, shadow_func=%s\n", sampler,
					  dword&(1<<31)?" reverse gamma,":"",
					  dword&(1<<30)?" packed2planar,":"",
					  dword&(1<<29)?" colorspace conversion,":"",
					  (dword>>22)&0x1f,
					  mip_filter,
					  gen3_decode_sample_filter(dword>>17),
					  gen3_decode_sample_filter(dword>>14),
					  ((dword>>5)&0x1ff)/(0x10*1.0),
					  dword&(1<<4)?" shadow,":"",
					  dword&(1<<3)?4:2,
					  gen3_decode_compare_func(dword));
				dword = data[i];
				kgem_debug_print(data, offset, i++, "sampler %d SS3: min_lod=%.2f,%s "
					  "tcmode_x=%s, tcmode_y=%s, tcmode_z=%s,%s texmap_idx=%i,%s\n",
					  sampler, ((dword>>24)&0xff)/(0x10*1.0),
					  dword&(1<<17)?" kill pixel enable,":"",
					  decode_tex_coord_mode(dword>>12),
					  decode_tex_coord_mode(dword>>9),
					  decode_tex_coord_mode(dword>>6),
					  dword&(1<<5)?" normalized coords,":"",
					  (dword>>1)&0xf,
					  dword&(1<<0)?" deinterlacer,":"");
				kgem_debug_print(data, offset, i++, "sampler %d SS4: border color\n",
					  sampler);
			}
		}
		assert(len == i);
		return len;
	case 0x85:
		len = (data[0] & 0x0000000f) + 2;
		assert(len == 2);

		kgem_debug_print(data, offset, 0,
			  "3DSTATE_DEST_BUFFER_VARIABLES\n");

		switch ((data[1] >> 8) & 0xf) {
		case 0x0: format = "g8"; break;
		case 0x1: format = "x1r5g5b5"; break;
		case 0x2: format = "r5g6b5"; break;
		case 0x3: format = "a8r8g8b8"; break;
		case 0x4: format = "ycrcb_swapy"; break;
		case 0x5: format = "ycrcb_normal"; break;
		case 0x6: format = "ycrcb_swapuv"; break;
		case 0x7: format = "ycrcb_swapuvy"; break;
		case 0x8: format = "a4r4g4b4"; break;
		case 0x9: format = "a1r5g5b5"; break;
		case 0xa: format = "a2r10g10b10"; break;
		default: format = "BAD"; break;
		}
		switch ((data[1] >> 2) & 0x3) {
		case 0x0: zformat = "u16"; break;
		case 0x1: zformat = "f16"; break;
		case 0x2: zformat = "u24x8"; break;
		default: zformat = "BAD"; break;
		}
		kgem_debug_print(data, offset, 1, "%s format, %s depth format, early Z %sabled\n",
			  format, zformat,
			  (data[1] & (1 << 31)) ? "en" : "dis");
		return len;

	case 0x8e:
		{
			const char *name, *tiling;

			len = (data[0] & 0x0000000f) + 2;
			assert(len == 3);

			switch((data[1] >> 24) & 0x7) {
			case 0x3: name = "color"; break;
			case 0x7: name = "depth"; break;
			default: name = "unknown"; break;
			}

			tiling = "none";
			if (data[1] & (1 << 23))
				tiling = "fenced";
			else if (data[1] & (1 << 22))
				tiling = data[1] & (1 << 21) ? "Y" : "X";

			kgem_debug_print(data, offset, 0, "3DSTATE_BUFFER_INFO\n");
			kgem_debug_print(data, offset, 1, "%s, tiling = %s, pitch=%d\n", name, tiling, data[1]&0xffff);

			kgem_debug_print(data, offset, 2, "address\n");
			return len;
		}
	case 0x81:
		len = (data[0] & 0x0000000f) + 2;
		assert(len == 3);

		kgem_debug_print(data, offset, 0,
			  "3DSTATE_SCISSOR_RECTANGLE\n");
		kgem_debug_print(data, offset, 1, "(%d,%d)\n",
			  data[1] & 0xffff, data[1] >> 16);
		kgem_debug_print(data, offset, 2, "(%d,%d)\n",
			  data[2] & 0xffff, data[2] >> 16);

		return len;
	case 0x80:
		len = (data[0] & 0x0000000f) + 2;
		assert(len == 5);

		kgem_debug_print(data, offset, 0,
			  "3DSTATE_DRAWING_RECTANGLE\n");
		kgem_debug_print(data, offset, 1, "%s\n",
			  data[1]&(1<<30)?"depth ofs disabled ":"");
		kgem_debug_print(data, offset, 2, "(%d,%d)\n",
			  data[2] & 0xffff, data[2] >> 16);
		kgem_debug_print(data, offset, 3, "(%d,%d)\n",
			  data[3] & 0xffff, data[3] >> 16);
		kgem_debug_print(data, offset, 4, "(%d,%d)\n",
			  (int16_t)(data[4] & 0xffff),
			  (int16_t)(data[4] >> 16));

		return len;
	case 0x9c:
		len = (data[0] & 0x0000000f) + 2;
		assert(len == 7);

		kgem_debug_print(data, offset, 0,
			  "3DSTATE_CLEAR_PARAMETERS\n");
		kgem_debug_print(data, offset, 1, "prim_type=%s, clear=%s%s%s\n",
			  data[1]&(1<<16)?"CLEAR_RECT":"ZONE_INIT",
			  data[1]&(1<<2)?"color,":"",
			  data[1]&(1<<1)?"depth,":"",
			  data[1]&(1<<0)?"stencil,":"");
		kgem_debug_print(data, offset, 2, "clear color\n");
		kgem_debug_print(data, offset, 3, "clear depth/stencil\n");
		kgem_debug_print(data, offset, 4, "color value (rgba8888)\n");
		kgem_debug_print(data, offset, 5, "depth value %f\n",
			  int_as_float(data[5]));
		kgem_debug_print(data, offset, 6, "clear stencil\n");
		return len;
	}

	for (idx = 0; idx < ARRAY_SIZE(opcodes_3d_1d); idx++) {
		opcode_3d_1d = &opcodes_3d_1d[idx];
		if (((data[0] & 0x00ff0000) >> 16) == opcode_3d_1d->opcode) {
			len = (data[0] & 0xf) + 2;
			kgem_debug_print(data, offset, 0, "%s\n", opcode_3d_1d->name);
			for (i = 1; i < len; i++)
				kgem_debug_print(data, offset, i, "dword %d\n", i);

			return len;
		}
	}

	kgem_debug_print(data, offset, 0, "3D UNKNOWN: 3d_1d opcode = 0x%x\n", opcode);
	assert(0);
	return 1;
}

#define VERTEX_OUT(fmt, ...) do {					\
	kgem_debug_print(data, offset, i, " V%d."fmt"\n", vertex, __VA_ARGS__); \
	i++;								\
} while (0)

static int
gen3_decode_3d_primitive(struct kgem *kgem, uint32_t offset)
{
	uint32_t *data = kgem->batch + offset;
	char immediate = (data[0] & (1 << 23)) == 0;
	unsigned int len, i, ret;
	const char *primtype;
	unsigned int vertex = 0;

	switch ((data[0] >> 18) & 0xf) {
	case 0x0: primtype = "TRILIST"; break;
	case 0x1: primtype = "TRISTRIP"; break;
	case 0x2: primtype = "TRISTRIP_REVERSE"; break;
	case 0x3: primtype = "TRIFAN"; break;
	case 0x4: primtype = "POLYGON"; break;
	case 0x5: primtype = "LINELIST"; break;
	case 0x6: primtype = "LINESTRIP"; break;
	case 0x7: primtype = "RECTLIST"; break;
	case 0x8: primtype = "POINTLIST"; break;
	case 0x9: primtype = "DIB"; break;
	case 0xa: primtype = "CLEAR_RECT"; assert(0); break;
	default: primtype = "unknown"; break;
	}

	gen3_update_vertex_elements_offsets(kgem);

	/* XXX: 3DPRIM_DIB not supported */
	if (immediate) {
		len = (data[0] & 0x0003ffff) + 2;
		kgem_debug_print(data, offset, 0, "3DPRIMITIVE inline %s\n", primtype);
		for (i = 1; i < len; ) {
			ErrorF("    [%d]: ", vertex);
			i += inline_vertex_out(kgem, data + i) / sizeof(uint32_t);
			ErrorF("\n");
			vertex++;
		}

		ret = len;
	} else {
		/* indirect vertices */
		len = data[0] & 0x0000ffff; /* index count */
		if (data[0] & (1 << 17)) {
			/* random vertex access */
			kgem_debug_print(data, offset, 0,
				  "3DPRIMITIVE random indirect %s (%d)\n", primtype, len);
			assert(0);
			if (len == 0) {
				/* vertex indices continue until 0xffff is found */
			} else {
				/* fixed size vertex index buffer */
			}
			ret = (len + 1) / 2 + 1;
			goto out;
		} else {
			/* sequential vertex access */
			vertex = data[1] & 0xffff;
			kgem_debug_print(data, offset, 0,
				  "3DPRIMITIVE sequential indirect %s, %d starting from "
				  "%d\n", primtype, len, vertex);
			kgem_debug_print(data, offset, 1, "  start\n");
			for (i = 0; i < len; i++) {
				ErrorF("    [%d]: ", vertex);
				indirect_vertex_out(kgem, vertex++);
				ErrorF("\n");
			}
			ret = 2;
			goto out;
		}
	}

out:
	return ret;
}

int kgem_gen3_decode_3d(struct kgem *kgem, uint32_t offset)
{
    static const struct {
	uint32_t opcode;
	int min_len;
	int max_len;
	const char *name;
    } opcodes[] = {
	{ 0x06, 1, 1, "3DSTATE_ANTI_ALIASING" },
	{ 0x08, 1, 1, "3DSTATE_BACKFACE_STENCIL_OPS" },
	{ 0x09, 1, 1, "3DSTATE_BACKFACE_STENCIL_MASKS" },
	{ 0x16, 1, 1, "3DSTATE_COORD_SET_BINDINGS" },
	{ 0x15, 1, 1, "3DSTATE_FOG_COLOR" },
	{ 0x0b, 1, 1, "3DSTATE_INDEPENDENT_ALPHA_BLEND" },
	{ 0x0d, 1, 1, "3DSTATE_MODES_4" },
	{ 0x0c, 1, 1, "3DSTATE_MODES_5" },
	{ 0x07, 1, 1, "3DSTATE_RASTERIZATION_RULES" },
    };
    uint32_t *data = kgem->batch + offset;
    uint32_t opcode;
    unsigned int idx;

    opcode = (data[0] & 0x1f000000) >> 24;

    switch (opcode) {
    case 0x1f:
	return gen3_decode_3d_primitive(kgem, offset);
    case 0x1d:
	return gen3_decode_3d_1d(kgem, offset);
    case 0x1c:
	return gen3_decode_3d_1c(kgem, offset);
    }

    for (idx = 0; idx < ARRAY_SIZE(opcodes); idx++) {
	if (opcode == opcodes[idx].opcode) {
	    unsigned int len = 1, i;

	    kgem_debug_print(data, offset, 0, "%s\n", opcodes[idx].name);
	    if (opcodes[idx].max_len > 1) {
		len = (data[0] & 0xff) + 2;
		assert(len >= opcodes[idx].min_len ||
		       len <= opcodes[idx].max_len);
	    }

	    for (i = 1; i < len; i++)
		kgem_debug_print(data, offset, i, "dword %d\n", i);
	    return len;
	}
    }

    kgem_debug_print(data, offset, 0, "3D UNKNOWN: 3d opcode = 0x%x\n", opcode);
    return 1;
}


void kgem_gen3_finish_state(struct kgem *kgem)
{
	memset(&state, 0, sizeof(state));
}
