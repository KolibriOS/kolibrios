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
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#include <sys/mman.h>
#include <assert.h>

#include "sna.h"
#include "sna_reg.h"

#include "kgem_debug.h"

#include <kos32sys.h>

/*
void
ErrorF(const char *f, ...)
{
    va_list args;

    va_start(args, f);
    VErrorF(f, args);
    va_end(args);
}
*/

#define ErrorF printf

struct drm_i915_gem_relocation_entry *
kgem_debug_get_reloc_entry(struct kgem *kgem, uint32_t offset)
{
	int i;

	offset *= sizeof(uint32_t);

	for (i = 0; i < kgem->nreloc; i++)
		if (kgem->reloc[i].offset == offset)
			return kgem->reloc+i;

	assert(!"valid relocation entry, unknown batch offset");
	return NULL;
}

struct kgem_bo *
kgem_debug_get_bo_for_reloc_entry(struct kgem *kgem,
				  struct drm_i915_gem_relocation_entry *reloc)
{
	struct kgem_bo *bo;

	if (reloc == NULL)
		return NULL;

	list_for_each_entry(bo, &kgem->next_request->buffers, request)
		if (bo->target_handle == reloc->target_handle && bo->proxy == NULL)
			break;

	assert(&bo->request != &kgem->next_request->buffers);

	return bo;
}

static int kgem_debug_handle_is_fenced(struct kgem *kgem, uint32_t handle)
{
	int i;

	if (kgem->has_handle_lut)
		return kgem->exec[handle].flags & EXEC_OBJECT_NEEDS_FENCE;

	for (i = 0; i < kgem->nexec; i++)
		if (kgem->exec[i].handle == handle)
			return kgem->exec[i].flags & EXEC_OBJECT_NEEDS_FENCE;

	return 0;
}

static int kgem_debug_handle_tiling(struct kgem *kgem, uint32_t handle)
{
	struct kgem_bo *bo;

	list_for_each_entry(bo, &kgem->next_request->buffers, request)
		if (bo->target_handle == handle)
			return bo->tiling;

	return 0;
}

void
kgem_debug_print(const uint32_t *data,
		 uint32_t offset, unsigned int index,
		 const char *fmt, ...)
{
	va_list va;
	char buf[240];
	int len;

	len = snprintf(buf, sizeof(buf),
		       "0x%08x: 0x%08x: %s",
		       (offset + index) * 4,
		       data[index],
		       index == 0 ? "" : "   ");

	va_start(va, fmt);
	vsnprintf(buf + len, sizeof(buf) - len, fmt, va);
	va_end(va);

	ErrorF("%s", buf);
    delay(1);
}

static int
decode_nop(struct kgem *kgem, uint32_t offset)
{
	uint32_t *data = kgem->batch + offset;
	kgem_debug_print(data, offset, 0, "UNKNOWN\n");
	assert(0);
	return 1;
}

static int
decode_mi(struct kgem *kgem, uint32_t offset)
{
	static const struct {
		uint32_t opcode;
		int len_mask;
		int min_len;
		int max_len;
		const char *name;
	} opcodes[] = {
		{ 0x08, 0, 1, 1, "MI_ARB_ON_OFF" },
		{ 0x0a, 0, 1, 1, "MI_BATCH_BUFFER_END" },
		{ 0x30, 0x3f, 3, 3, "MI_BATCH_BUFFER" },
		{ 0x31, 0x3f, 2, 2, "MI_BATCH_BUFFER_START" },
		{ 0x14, 0x3f, 3, 3, "MI_DISPLAY_BUFFER_INFO" },
		{ 0x04, 0, 1, 1, "MI_FLUSH" },
		{ 0x22, 0x1f, 3, 3, "MI_LOAD_REGISTER_IMM" },
		{ 0x13, 0x3f, 2, 2, "MI_LOAD_SCAN_LINES_EXCL" },
		{ 0x12, 0x3f, 2, 2, "MI_LOAD_SCAN_LINES_INCL" },
		{ 0x00, 0, 1, 1, "MI_NOOP" },
		{ 0x11, 0x3f, 2, 2, "MI_OVERLAY_FLIP" },
		{ 0x07, 0, 1, 1, "MI_REPORT_HEAD" },
		{ 0x18, 0x3f, 2, 2, "MI_SET_CONTEXT" },
		{ 0x20, 0x3f, 3, 4, "MI_STORE_DATA_IMM" },
		{ 0x21, 0x3f, 3, 4, "MI_STORE_DATA_INDEX" },
		{ 0x24, 0x3f, 3, 3, "MI_STORE_REGISTER_MEM" },
		{ 0x02, 0, 1, 1, "MI_USER_INTERRUPT" },
		{ 0x03, 0, 1, 1, "MI_WAIT_FOR_EVENT" },
		{ 0x16, 0x7f, 3, 3, "MI_SEMAPHORE_MBOX" },
		{ 0x26, 0x1f, 3, 4, "MI_FLUSH_DW" },
		{ 0x0b, 0, 1, 1, "MI_SUSPEND_FLUSH" },
	};
	uint32_t *data = kgem->batch + offset;
	int op;

	for (op = 0; op < ARRAY_SIZE(opcodes); op++) {
		if ((data[0] & 0x1f800000) >> 23 == opcodes[op].opcode) {
			unsigned int len = 1, i;

			kgem_debug_print(data, offset, 0, "%s\n", opcodes[op].name);
			if (opcodes[op].max_len > 1) {
				len = (data[0] & opcodes[op].len_mask) + 2;
				if (len < opcodes[op].min_len ||
				    len > opcodes[op].max_len)
				{
					ErrorF("Bad length (%d) in %s, [%d, %d]\n",
					       len, opcodes[op].name,
					       opcodes[op].min_len,
					       opcodes[op].max_len);
					assert(0);
				}
			}

			for (i = 1; i < len; i++)
				kgem_debug_print(data, offset, i, "dword %d\n", i);

			return len;
		}
	}

	kgem_debug_print(data, offset, 0, "MI UNKNOWN\n");
	assert(0);
	return 1;
}

static int
decode_2d(struct kgem *kgem, uint32_t offset)
{
	static const struct {
		uint32_t opcode;
		int min_len;
		int max_len;
		const char *name;
	} opcodes[] = {
		{ 0x40, 5, 5, "COLOR_BLT" },
		{ 0x43, 6, 6, "SRC_COPY_BLT" },
		{ 0x01, 8, 8, "XY_SETUP_BLT" },
		{ 0x11, 9, 9, "XY_SETUP_MONO_PATTERN_SL_BLT" },
		{ 0x03, 3, 3, "XY_SETUP_CLIP_BLT" },
		{ 0x24, 2, 2, "XY_PIXEL_BLT" },
		{ 0x25, 3, 3, "XY_SCANLINES_BLT" },
		{ 0x26, 4, 4, "Y_TEXT_BLT" },
		{ 0x31, 5, 134, "XY_TEXT_IMMEDIATE_BLT" },
		{ 0x50, 6, 6, "XY_COLOR_BLT" },
		{ 0x51, 6, 6, "XY_PAT_BLT" },
		{ 0x76, 8, 8, "XY_PAT_CHROMA_BLT" },
		{ 0x72, 7, 135, "XY_PAT_BLT_IMMEDIATE" },
		{ 0x77, 9, 137, "XY_PAT_CHROMA_BLT_IMMEDIATE" },
		{ 0x52, 9, 9, "XY_MONO_PAT_BLT" },
		{ 0x59, 7, 7, "XY_MONO_PAT_FIXED_BLT" },
		{ 0x53, 8, 8, "XY_SRC_COPY_BLT" },
		{ 0x54, 8, 8, "XY_MONO_SRC_COPY_BLT" },
		{ 0x71, 9, 137, "XY_MONO_SRC_COPY_IMMEDIATE_BLT" },
		{ 0x55, 9, 9, "XY_FULL_BLT" },
		{ 0x55, 9, 137, "XY_FULL_IMMEDIATE_PATTERN_BLT" },
		{ 0x56, 9, 9, "XY_FULL_MONO_SRC_BLT" },
		{ 0x75, 10, 138, "XY_FULL_MONO_SRC_IMMEDIATE_PATTERN_BLT" },
		{ 0x57, 12, 12, "XY_FULL_MONO_PATTERN_BLT" },
		{ 0x58, 12, 12, "XY_FULL_MONO_PATTERN_MONO_SRC_BLT" },
	};

	unsigned int op, len;
	const char *format = NULL;
	uint32_t *data = kgem->batch + offset;
	struct drm_i915_gem_relocation_entry *reloc;

	/* Special case the two most common ops that we detail in full */
	switch ((data[0] & 0x1fc00000) >> 22) {
	case 0x50:
		kgem_debug_print(data, offset, 0,
			  "XY_COLOR_BLT (rgb %sabled, alpha %sabled, dst tile %d)\n",
			  (data[0] & (1 << 20)) ? "en" : "dis",
			  (data[0] & (1 << 21)) ? "en" : "dis",
			  (data[0] >> 11) & 1);

		len = (data[0] & 0x000000ff) + 2;
		assert(len == 6);

		switch ((data[1] >> 24) & 0x3) {
		case 0:
			format="8";
			break;
		case 1:
			format="565";
			break;
		case 2:
			format="1555";
			break;
		case 3:
			format="8888";
			break;
		}

		kgem_debug_print(data, offset, 1, "format %s, rop %x, pitch %d, "
			  "clipping %sabled\n", format,
			  (data[1] >> 16) & 0xff,
			  (short)(data[1] & 0xffff),
			  data[1] & (1 << 30) ? "en" : "dis");
		kgem_debug_print(data, offset, 2, "(%d,%d)\n",
			  data[2] & 0xffff, data[2] >> 16);
		kgem_debug_print(data, offset, 3, "(%d,%d)\n",
			  data[3] & 0xffff, data[3] >> 16);
		reloc = kgem_debug_get_reloc_entry(kgem, offset+4);
		kgem_debug_print(data, offset, 4, "dst offset 0x%08x [handle=%d, delta=%d, read=%x, write=%x (fenced? %d, tiling? %d)]\n",
				 data[4],
				 reloc->target_handle, reloc->delta,
				 reloc->read_domains, reloc->write_domain,
				 kgem_debug_handle_is_fenced(kgem, reloc->target_handle),
				 kgem_debug_handle_tiling(kgem, reloc->target_handle));
		kgem_debug_print(data, offset, 5, "color\n");
		assert(kgem->gen >= 040 ||
		       kgem_debug_handle_is_fenced(kgem, reloc->target_handle));
		return len;

	case 0x53:
		kgem_debug_print(data, offset, 0,
			  "XY_SRC_COPY_BLT (rgb %sabled, alpha %sabled, "
			  "src tile %d, dst tile %d)\n",
			  (data[0] & (1 << 20)) ? "en" : "dis",
			  (data[0] & (1 << 21)) ? "en" : "dis",
			  (data[0] >> 15) & 1,
			  (data[0] >> 11) & 1);

		len = (data[0] & 0x000000ff) + 2;
		assert(len == 8);

		switch ((data[1] >> 24) & 0x3) {
		case 0:
			format="8";
			break;
		case 1:
			format="565";
			break;
		case 2:
			format="1555";
			break;
		case 3:
			format="8888";
			break;
		}

		kgem_debug_print(data, offset, 1, "format %s, rop %x, dst pitch %d, "
				 "clipping %sabled\n", format,
				 (data[1] >> 16) & 0xff,
				 (short)(data[1] & 0xffff),
				 data[1] & (1 << 30) ? "en" : "dis");
		kgem_debug_print(data, offset, 2, "dst (%d,%d)\n",
				 data[2] & 0xffff, data[2] >> 16);
		kgem_debug_print(data, offset, 3, "dst (%d,%d)\n",
				 data[3] & 0xffff, data[3] >> 16);
		reloc = kgem_debug_get_reloc_entry(kgem, offset+4);
		assert(reloc);
		kgem_debug_print(data, offset, 4, "dst offset 0x%08x [handle=%d, delta=%d, read=%x, write=%x, (fenced? %d, tiling? %d)]\n",
				 data[4],
				 reloc->target_handle, reloc->delta,
				 reloc->read_domains, reloc->write_domain,
				 kgem_debug_handle_is_fenced(kgem, reloc->target_handle),
				 kgem_debug_handle_tiling(kgem, reloc->target_handle));
		assert(kgem->gen >= 040 ||
		       kgem_debug_handle_is_fenced(kgem, reloc->target_handle));

		kgem_debug_print(data, offset, 5, "src (%d,%d)\n",
				 data[5] & 0xffff, data[5] >> 16);
		kgem_debug_print(data, offset, 6, "src pitch %d\n",
				 (short)(data[6] & 0xffff));
		reloc = kgem_debug_get_reloc_entry(kgem, offset+7);
		assert(reloc);
		kgem_debug_print(data, offset, 7, "src offset 0x%08x [handle=%d, delta=%d, read=%x, write=%x (fenced? %d, tiling? %d)]\n",
				 data[7],
				 reloc->target_handle, reloc->delta,
				 reloc->read_domains, reloc->write_domain,
				 kgem_debug_handle_is_fenced(kgem, reloc->target_handle),
				 kgem_debug_handle_tiling(kgem, reloc->target_handle));
		assert(kgem->gen >= 040 ||
		       kgem_debug_handle_is_fenced(kgem, reloc->target_handle));

		return len;
	}

	for (op = 0; op < ARRAY_SIZE(opcodes); op++) {
		if ((data[0] & 0x1fc00000) >> 22 == opcodes[op].opcode) {
			unsigned int i;

			len = 1;
			kgem_debug_print(data, offset, 0, "%s\n", opcodes[op].name);
			if (opcodes[op].max_len > 1) {
				len = (data[0] & 0x000000ff) + 2;
				assert(len >= opcodes[op].min_len &&
				       len <= opcodes[op].max_len);
			}

			for (i = 1; i < len; i++)
				kgem_debug_print(data, offset, i, "dword %d\n", i);

			return len;
		}
	}

	kgem_debug_print(data, offset, 0, "2D UNKNOWN\n");
	assert(0);
	return 1;
}

static int (*decode_3d(int gen))(struct kgem*, uint32_t)
{
	return kgem_gen6_decode_3d;
/*
	if (gen >= 0100) {
	} else if (gen >= 070) {
		return kgem_gen7_decode_3d;
	} else if (gen >= 060) {
		return kgem_gen6_decode_3d;
	} else if (gen >= 050) {
		return kgem_gen5_decode_3d;
	} else if (gen >= 040) {
		return kgem_gen4_decode_3d;
	} else if (gen >= 030) {
		return kgem_gen3_decode_3d;
	} else if (gen >= 020) {
		return kgem_gen2_decode_3d;
	}
	assert(0);
*/
}

static void (*finish_state(int gen))(struct kgem*)
{

    return kgem_gen6_finish_state;
/*
	if (gen >= 0100) {
	} else if (gen >= 070) {
		return kgem_gen7_finish_state;
	} else if (gen >= 060) {
		return kgem_gen6_finish_state;
	} else if (gen >= 050) {
		return kgem_gen5_finish_state;
	} else if (gen >= 040) {
		return kgem_gen4_finish_state;
	} else if (gen >= 030) {
		return kgem_gen3_finish_state;
	} else if (gen >= 020) {
		return kgem_gen2_finish_state;
	}
	assert(0);
*/
}

void __kgem_batch_debug(struct kgem *kgem, uint32_t nbatch)
{
	int (*const decode[])(struct kgem *, uint32_t) = {
		decode_mi,
		decode_nop,
		decode_2d,
		decode_3d(kgem->gen),
	};
	uint32_t offset = 0;

	while (offset < nbatch) {
		int class = (kgem->batch[offset] & 0xe0000000) >> 29;
		assert(class < ARRAY_SIZE(decode));
		offset += decode[class](kgem, offset);
	}

	finish_state(kgem->gen)(kgem);
}
