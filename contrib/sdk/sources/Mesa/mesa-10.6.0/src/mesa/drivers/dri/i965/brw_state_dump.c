/*
 * Copyright © 2007-2015 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

#include "main/mtypes.h"
#include "intel_batchbuffer.h"

#include "brw_context.h"
#include "brw_defines.h"
#include "brw_eu.h"
#include "brw_state.h"

static const char *sampler_mip_filter[] = {
   "NONE",
   "NEAREST",
   "RSVD",
   "LINEAR"
};

static const char *sampler_mag_filter[] = {
   "NEAREST",
   "LINEAR",
   "ANISOTROPIC",
   "FLEXIBLE (GEN8+)",
   "RSVD", "RSVD",
   "MONO",
   "RSVD"
};

static const char *sampler_addr_mode[] = {
   "WRAP",
   "MIRROR",
   "CLAMP",
   "CUBE",
   "CLAMP_BORDER",
   "MIRROR_ONCE",
   "HALF_BORDER"
};

static const char *surface_tiling[] = {
   "LINEAR",
   "W-tiled",
   "X-tiled",
   "Y-tiled"
};

static void
batch_out(struct brw_context *brw, const char *name, uint32_t offset,
	  int index, char *fmt, ...) PRINTFLIKE(5, 6);

static void
batch_out(struct brw_context *brw, const char *name, uint32_t offset,
	  int index, char *fmt, ...)
{
   uint32_t *data = brw->batch.bo->virtual + offset;
   va_list va;

   fprintf(stderr, "0x%08x:      0x%08x: %8s: ",
	   offset + index * 4, data[index], name);
   va_start(va, fmt);
   vfprintf(stderr, fmt, va);
   va_end(va);
}

static void
batch_out64(struct brw_context *brw, const char *name, uint32_t offset,
            int index, char *fmt, ...)
{
   uint32_t *tmp = brw->batch.bo->virtual + offset;

   /* Swap the dwords since we want to handle this as a 64b value, but the data
    * is typically emitted as dwords.
    */
   uint64_t data = ((uint64_t)tmp[index + 1]) << 32 | tmp[index];
   va_list va;

   fprintf(stderr, "0x%08x:      0x%016" PRIx64 ": %8s: ",
          offset + index * 4, data, name);
   va_start(va, fmt);
   vfprintf(stderr, fmt, va);
   va_end(va);
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

static void dump_vs_state(struct brw_context *brw, uint32_t offset)
{
   const char *name = "VS_STATE";
   struct brw_vs_unit_state *vs = brw->batch.bo->virtual + offset;

   batch_out(brw, name, offset, 0, "thread0\n");
   batch_out(brw, name, offset, 1, "thread1\n");
   batch_out(brw, name, offset, 2, "thread2\n");
   batch_out(brw, name, offset, 3, "thread3\n");
   batch_out(brw, name, offset, 4, "thread4: %d threads\n",
	     vs->thread4.max_threads + 1);
   batch_out(brw, name, offset, 5, "vs5\n");
   batch_out(brw, name, offset, 6, "vs6\n");
}

static void dump_gs_state(struct brw_context *brw, uint32_t offset)
{
   const char *name = "GS_STATE";
   struct brw_gs_unit_state *gs = brw->batch.bo->virtual + offset;

   batch_out(brw, name, offset, 0, "thread0\n");
   batch_out(brw, name, offset, 1, "thread1\n");
   batch_out(brw, name, offset, 2, "thread2\n");
   batch_out(brw, name, offset, 3, "thread3\n");
   batch_out(brw, name, offset, 4, "thread4: %d threads\n",
	     gs->thread4.max_threads + 1);
   batch_out(brw, name, offset, 5, "vs5\n");
   batch_out(brw, name, offset, 6, "vs6\n");
}

static void dump_clip_state(struct brw_context *brw, uint32_t offset)
{
   const char *name = "CLIP_STATE";
   struct brw_clip_unit_state *clip = brw->batch.bo->virtual + offset;

   batch_out(brw, name, offset, 0, "thread0\n");
   batch_out(brw, name, offset, 1, "thread1\n");
   batch_out(brw, name, offset, 2, "thread2\n");
   batch_out(brw, name, offset, 3, "thread3\n");
   batch_out(brw, name, offset, 4, "thread4: %d threads\n",
	     clip->thread4.max_threads + 1);
   batch_out(brw, name, offset, 5, "clip5\n");
   batch_out(brw, name, offset, 6, "clip6\n");
   batch_out(brw, name, offset, 7, "vp xmin %f\n", clip->viewport_xmin);
   batch_out(brw, name, offset, 8, "vp xmax %f\n", clip->viewport_xmax);
   batch_out(brw, name, offset, 9, "vp ymin %f\n", clip->viewport_ymin);
   batch_out(brw, name, offset, 10, "vp ymax %f\n", clip->viewport_ymax);
}

static void dump_sf_state(struct brw_context *brw, uint32_t offset)
{
   const char *name = "SF_STATE";
   struct brw_sf_unit_state *sf = brw->batch.bo->virtual + offset;

   batch_out(brw, name, offset, 0, "thread0\n");
   batch_out(brw, name, offset, 1, "thread1\n");
   batch_out(brw, name, offset, 2, "thread2\n");
   batch_out(brw, name, offset, 3, "thread3\n");
   batch_out(brw, name, offset, 4, "thread4: %d threads\n",
	     sf->thread4.max_threads + 1);
   batch_out(brw, name, offset, 5, "sf5: viewport offset\n");
   batch_out(brw, name, offset, 6, "sf6\n");
   batch_out(brw, name, offset, 7, "sf7\n");
}

static void dump_wm_state(struct brw_context *brw, uint32_t offset)
{
   const char *name = "WM_STATE";
   struct brw_wm_unit_state *wm = brw->batch.bo->virtual + offset;

   batch_out(brw, name, offset, 0, "thread0\n");
   batch_out(brw, name, offset, 1, "thread1\n");
   batch_out(brw, name, offset, 2, "thread2\n");
   batch_out(brw, name, offset, 3, "thread3\n");
   batch_out(brw, name, offset, 4, "wm4\n");
   batch_out(brw, name, offset, 5, "wm5: %s%s%s%s%s%s, %d threads\n",
	     wm->wm5.enable_8_pix ? "8pix" : "",
	     wm->wm5.enable_16_pix ? "16pix" : "",
	     wm->wm5.program_uses_depth ? ", uses depth" : "",
	     wm->wm5.program_computes_depth ? ", computes depth" : "",
	     wm->wm5.program_uses_killpixel ? ", kills" : "",
	     wm->wm5.thread_dispatch_enable ? "" : ", no dispatch",
	     wm->wm5.max_threads + 1);
   batch_out(brw, name, offset, 6, "depth offset constant %f\n",
	     wm->global_depth_offset_constant);
   batch_out(brw, name, offset, 7, "depth offset scale %f\n",
	     wm->global_depth_offset_scale);
   batch_out(brw, name, offset, 8, "wm8: kernel 1 (gen5+)\n");
   batch_out(brw, name, offset, 9, "wm9: kernel 2 (gen5+)\n");
   batch_out(brw, name, offset, 10, "wm10: kernel 3 (gen5+)\n");
}

static void dump_surface_state(struct brw_context *brw, uint32_t offset)
{
   const char *name = "SURF";
   uint32_t *surf = brw->batch.bo->virtual + offset;

   batch_out(brw, name, offset, 0, "%s %s\n",
	     get_965_surfacetype(GET_FIELD(surf[0], BRW_SURFACE_TYPE)),
             brw_surface_format_name(GET_FIELD(surf[0], BRW_SURFACE_FORMAT)));
   batch_out(brw, name, offset, 1, "offset\n");
   batch_out(brw, name, offset, 2, "%dx%d size, %d mips\n",
	     GET_FIELD(surf[2], BRW_SURFACE_WIDTH) + 1,
	     GET_FIELD(surf[2], BRW_SURFACE_HEIGHT) + 1,
	     GET_FIELD(surf[2], BRW_SURFACE_LOD));
   batch_out(brw, name, offset, 3, "pitch %d, %s tiled\n",
	     GET_FIELD(surf[3], BRW_SURFACE_PITCH) + 1,
	     (surf[3] & BRW_SURFACE_TILED) ?
	     ((surf[3] & BRW_SURFACE_TILED_Y) ? "Y" : "X") : "not");
   batch_out(brw, name, offset, 4, "mip base %d\n",
	     GET_FIELD(surf[4], BRW_SURFACE_MIN_LOD));
   batch_out(brw, name, offset, 5, "x,y offset: %d,%d\n",
	     GET_FIELD(surf[5], BRW_SURFACE_X_OFFSET),
	     GET_FIELD(surf[5], BRW_SURFACE_Y_OFFSET));
}

static void dump_gen7_surface_state(struct brw_context *brw, uint32_t offset)
{
   const char *name = "SURF";
   uint32_t *surf = brw->batch.bo->virtual + offset;

   batch_out(brw, name, offset, 0, "%s %s %s\n",
             get_965_surfacetype(GET_FIELD(surf[0], BRW_SURFACE_TYPE)),
             brw_surface_format_name(GET_FIELD(surf[0], BRW_SURFACE_FORMAT)),
             (surf[0] & GEN7_SURFACE_IS_ARRAY) ? "array" : "");
   batch_out(brw, name, offset, 1, "offset\n");
   batch_out(brw, name, offset, 2, "%dx%d size, %d mips, %d slices\n",
             GET_FIELD(surf[2], GEN7_SURFACE_WIDTH) + 1,
             GET_FIELD(surf[2], GEN7_SURFACE_HEIGHT) + 1,
             surf[5] & INTEL_MASK(3, 0),
             GET_FIELD(surf[3], BRW_SURFACE_DEPTH) + 1);
   batch_out(brw, name, offset, 3, "pitch %d, %stiled\n",
	     (surf[3] & INTEL_MASK(17, 0)) + 1,
             (surf[0] & (1 << 14)) ? "" : "not ");
   batch_out(brw, name, offset, 4, "min array element %d, array extent %d\n",
             GET_FIELD(surf[4], GEN7_SURFACE_MIN_ARRAY_ELEMENT),
             GET_FIELD(surf[4], GEN7_SURFACE_RENDER_TARGET_VIEW_EXTENT) + 1);
   batch_out(brw, name, offset, 5, "mip base %d\n",
             GET_FIELD(surf[5], GEN7_SURFACE_MIN_LOD));
   batch_out(brw, name, offset, 6, "x,y offset: %d,%d\n",
             GET_FIELD(surf[5], BRW_SURFACE_X_OFFSET),
             GET_FIELD(surf[5], BRW_SURFACE_Y_OFFSET));
   batch_out(brw, name, offset, 7, "\n");
}

static float q_to_float(uint32_t data, int integer_end, int integer_start,
                        int fractional_end, int fractional_start)
{
   /* Convert the number to floating point. */
   float n = GET_BITS(data, integer_start, fractional_end);

   /* Multiply by 2^-n */
   return n * exp2(-(fractional_end - fractional_start + 1));
}

static void
dump_gen8_surface_state(struct brw_context *brw, uint32_t offset, int index)
{
   uint32_t *surf = brw->batch.bo->virtual + offset;
   int aux_mode = surf[6] & INTEL_MASK(2, 0);
   const char *aux_str;
   char *name;

   if (brw->gen >= 9 && (aux_mode == 1 || aux_mode == 5)) {
      bool msrt = GET_BITS(surf[4], 5, 3) > 0;
      bool compression = GET_FIELD(surf[7], GEN9_SURFACE_RT_COMPRESSION) == 1;
      aux_str = ralloc_asprintf(NULL, "AUX_CCS_%c (%s, MULTISAMPLE_COUNT%c1)",
                                (aux_mode == 1) ? 'D' : 'E',
                                compression ? "Compressed RT" : "Uncompressed",
                                msrt ? '>' : '=');
   } else {
      static const char *surface_aux_mode[] = { "AUX_NONE", "AUX_MCS",
                                                "AUX_APPEND", "AUX_HIZ",
                                                "RSVD", "RSVD"};
      aux_str = ralloc_asprintf(NULL, "%s", surface_aux_mode[aux_mode]);
   }

   name = ralloc_asprintf(NULL, "SURF%03d", index);
   batch_out(brw, name, offset, 0, "%s %s %s VALIGN%d HALIGN%d %s\n",
             get_965_surfacetype(GET_FIELD(surf[0], BRW_SURFACE_TYPE)),
             brw_surface_format_name(GET_FIELD(surf[0], BRW_SURFACE_FORMAT)),
             (surf[0] & GEN7_SURFACE_IS_ARRAY) ? "array" : "",
             1 << (GET_BITS(surf[0], 17, 16) + 1), /* VALIGN */
             1 << (GET_BITS(surf[0], 15, 14) + 1), /* HALIGN */
             surface_tiling[GET_BITS(surf[0], 13, 12)]);
   batch_out(brw, name, offset, 1, "MOCS: 0x%x Base MIP: %.1f (%u mips) Surface QPitch: %d\n",
             GET_FIELD(surf[1], GEN8_SURFACE_MOCS),
             q_to_float(surf[1], 23, 20, 19, 19),
             surf[5] & INTEL_MASK(3, 0),
             GET_FIELD(surf[1], GEN8_SURFACE_QPITCH) << 2);
   batch_out(brw, name, offset, 2, "%dx%d [%s]\n",
             GET_FIELD(surf[2], GEN7_SURFACE_WIDTH) + 1,
             GET_FIELD(surf[2], GEN7_SURFACE_HEIGHT) + 1,
             aux_str);
   batch_out(brw, name, offset, 3, "%d slices (depth), pitch: %d\n",
             GET_FIELD(surf[3], BRW_SURFACE_DEPTH) + 1,
             (surf[3] & INTEL_MASK(17, 0)) + 1);
   batch_out(brw, name, offset, 4, "min array element: %d, array extent %d, MULTISAMPLE_%d\n",
             GET_FIELD(surf[4], GEN7_SURFACE_MIN_ARRAY_ELEMENT),
             GET_FIELD(surf[4], GEN7_SURFACE_RENDER_TARGET_VIEW_EXTENT) + 1,
             1 << GET_BITS(surf[4], 5, 3));
   batch_out(brw, name, offset, 5, "x,y offset: %d,%d, min LOD: %d\n",
             GET_FIELD(surf[5], BRW_SURFACE_X_OFFSET),
             GET_FIELD(surf[5], BRW_SURFACE_Y_OFFSET),
             GET_FIELD(surf[5], GEN7_SURFACE_MIN_LOD));
   batch_out(brw, name, offset, 6, "AUX pitch: %d qpitch: %d\n",
             GET_FIELD(surf[6], GEN8_SURFACE_AUX_QPITCH) << 2,
             GET_FIELD(surf[6], GEN8_SURFACE_AUX_PITCH) << 2);
   if (brw->gen >= 9) {
      batch_out(brw, name, offset, 7, "Clear color: R(%x)G(%x)B(%x)A(%x)\n",
                surf[12], surf[13], surf[14], surf[15]);
   } else {
      batch_out(brw, name, offset, 7, "Clear color: %c%c%c%c\n",
                GET_BITS(surf[7], 31, 31) ? 'R' : '-',
                GET_BITS(surf[7], 30, 30) ? 'G' : '-',
                GET_BITS(surf[7], 29, 29) ? 'B' : '-',
                GET_BITS(surf[7], 28, 28) ? 'A' : '-');
   }

   for (int i = 8; i < 12; i++)
      batch_out(brw, name, offset, i, "0x%08x\n", surf[i]);

   ralloc_free((void *)aux_str);
   ralloc_free(name);
}

static void
dump_sdc(struct brw_context *brw, uint32_t offset)
{
   const char *name = "SDC";

   if (brw->gen >= 5 && brw->gen <= 6) {
      struct gen5_sampler_default_color *sdc = (brw->batch.bo->virtual +
                                                offset);
      batch_out(brw, name, offset, 0, "unorm rgba\n");
      batch_out(brw, name, offset, 1, "r %f\n", sdc->f[0]);
      batch_out(brw, name, offset, 2, "b %f\n", sdc->f[1]);
      batch_out(brw, name, offset, 3, "g %f\n", sdc->f[2]);
      batch_out(brw, name, offset, 4, "a %f\n", sdc->f[3]);
      batch_out(brw, name, offset, 5, "half float rg\n");
      batch_out(brw, name, offset, 6, "half float ba\n");
      batch_out(brw, name, offset, 7, "u16 rg\n");
      batch_out(brw, name, offset, 8, "u16 ba\n");
      batch_out(brw, name, offset, 9, "s16 rg\n");
      batch_out(brw, name, offset, 10, "s16 ba\n");
      batch_out(brw, name, offset, 11, "s8 rgba\n");
   } else {
      float *sdc = brw->batch.bo->virtual + offset;
      batch_out(brw, name, offset, 0, "r %f\n", sdc[0]);
      batch_out(brw, name, offset, 1, "g %f\n", sdc[1]);
      batch_out(brw, name, offset, 2, "b %f\n", sdc[2]);
      batch_out(brw, name, offset, 3, "a %f\n", sdc[3]);
   }
}

static void dump_sampler_state(struct brw_context *brw,
			       uint32_t offset, uint32_t size)
{
   int i;
   uint32_t *samp = brw->batch.bo->virtual + offset;

   for (i = 0; i < size / 16; i++) {
      char name[20];

      sprintf(name, "WM SAMP%d", i);
      batch_out(brw, name, offset, 0, "filtering\n");
      batch_out(brw, name, offset, 1, "wrapping, lod\n");
      batch_out(brw, name, offset, 2, "default color pointer\n");
      batch_out(brw, name, offset, 3, "chroma key, aniso\n");

      samp += 4;
      offset += 4 * sizeof(uint32_t);
   }
}

static void gen7_dump_sampler_state(struct brw_context *brw,
                                    uint32_t offset, uint32_t size)
{
   const uint32_t *samp = brw->batch.bo->virtual + offset;
   char name[20];

   for (int i = 0; i < size / 16; i++) {
      sprintf(name, "SAMPLER_STATE %d", i);
      batch_out(brw, name, offset, i,
                "Disabled = %s, Base Mip: %u.%u, Mip/Mag/Min Filter: %s/%s/%s, LOD Bias: %d.%d\n",
                GET_BITS(samp[0], 31, 31) ? "yes" : "no",
                GET_BITS(samp[0], 26, 23),
                GET_BITS(samp[0], 22, 22),
                sampler_mip_filter[GET_FIELD(samp[0], BRW_SAMPLER_MIP_FILTER)],
                sampler_mag_filter[GET_FIELD(samp[0], BRW_SAMPLER_MAG_FILTER)],
                /* min filter defs are the same as mag */
                sampler_mag_filter[GET_FIELD(samp[0], BRW_SAMPLER_MIN_FILTER)],
                GET_BITS(samp[0], 13, 10),
                GET_BITS(samp[0], 9, 1)
               );
      batch_out(brw, name, offset, i+1, "Min LOD: %u.%u, Max LOD: %u.%u\n",
                GET_BITS(samp[1], 31, 28),
                GET_BITS(samp[1], 27, 20),
                GET_BITS(samp[1], 19, 16),
                GET_BITS(samp[1], 15, 8)
               );
      batch_out(brw, name, offset, i+2, "Border Color\n"); /* FINISHME: gen8+ */
      batch_out(brw, name, offset, i+3, "Max aniso: RATIO %d:1, TC[XYZ] Address Control: %s|%s|%s\n",
                (GET_FIELD(samp[3], BRW_SAMPLER_MAX_ANISOTROPY) + 1) * 2,
                sampler_addr_mode[GET_FIELD(samp[3], BRW_SAMPLER_TCX_WRAP_MODE)],
                sampler_addr_mode[GET_FIELD(samp[3], BRW_SAMPLER_TCY_WRAP_MODE)],
                sampler_addr_mode[GET_FIELD(samp[3], BRW_SAMPLER_TCZ_WRAP_MODE)]
               );

      samp += 4;
      offset += 4 * sizeof(uint32_t);
   }
}

static void dump_sf_viewport_state(struct brw_context *brw,
				   uint32_t offset)
{
   const char *name = "SF VP";
   struct brw_sf_viewport *vp = brw->batch.bo->virtual + offset;

   assert(brw->gen < 7);

   batch_out(brw, name, offset, 0, "m00 = %f\n", vp->viewport.m00);
   batch_out(brw, name, offset, 1, "m11 = %f\n", vp->viewport.m11);
   batch_out(brw, name, offset, 2, "m22 = %f\n", vp->viewport.m22);
   batch_out(brw, name, offset, 3, "m30 = %f\n", vp->viewport.m30);
   batch_out(brw, name, offset, 4, "m31 = %f\n", vp->viewport.m31);
   batch_out(brw, name, offset, 5, "m32 = %f\n", vp->viewport.m32);

   batch_out(brw, name, offset, 6, "top left = %d,%d\n",
	     vp->scissor.xmin, vp->scissor.ymin);
   batch_out(brw, name, offset, 7, "bottom right = %d,%d\n",
	     vp->scissor.xmax, vp->scissor.ymax);
}

static void dump_clip_viewport_state(struct brw_context *brw,
				     uint32_t offset)
{
   const char *name = "CLIP VP";
   struct brw_clipper_viewport *vp = brw->batch.bo->virtual + offset;

   assert(brw->gen < 7);

   batch_out(brw, name, offset, 0, "xmin = %f\n", vp->xmin);
   batch_out(brw, name, offset, 1, "xmax = %f\n", vp->xmax);
   batch_out(brw, name, offset, 2, "ymin = %f\n", vp->ymin);
   batch_out(brw, name, offset, 3, "ymax = %f\n", vp->ymax);
}

static void dump_sf_clip_viewport_state(struct brw_context *brw,
					uint32_t offset)
{
   const char *name = "SF_CLIP VP";
   struct gen7_sf_clip_viewport *vp = brw->batch.bo->virtual + offset;

   assert(brw->gen >= 7);

   batch_out(brw, name, offset, 0, "m00 = %f\n", vp->viewport.m00);
   batch_out(brw, name, offset, 1, "m11 = %f\n", vp->viewport.m11);
   batch_out(brw, name, offset, 2, "m22 = %f\n", vp->viewport.m22);
   batch_out(brw, name, offset, 3, "m30 = %f\n", vp->viewport.m30);
   batch_out(brw, name, offset, 4, "m31 = %f\n", vp->viewport.m31);
   batch_out(brw, name, offset, 5, "m32 = %f\n", vp->viewport.m32);
   batch_out(brw, name, offset, 8, "guardband xmin = %f\n", vp->guardband.xmin);
   batch_out(brw, name, offset, 9, "guardband xmax = %f\n", vp->guardband.xmax);
   batch_out(brw, name, offset, 9, "guardband ymin = %f\n", vp->guardband.ymin);
   batch_out(brw, name, offset, 10, "guardband ymax = %f\n", vp->guardband.ymax);
   if (brw->gen >= 8) {
      float *cc_vp = brw->batch.bo->virtual + offset;
      batch_out(brw, name, offset, 12, "Min extents: %.2fx%.2f\n",
                cc_vp[12], cc_vp[14]);
      batch_out(brw, name, offset, 14, "Max extents: %.2fx%.2f\n",
                cc_vp[13], cc_vp[15]);
   }
}


static void dump_cc_viewport_state(struct brw_context *brw, uint32_t offset)
{
   const char *name = "CC VP";
   struct brw_cc_viewport *vp = brw->batch.bo->virtual + offset;

   batch_out(brw, name, offset, 0, "min_depth = %f\n", vp->min_depth);
   batch_out(brw, name, offset, 1, "max_depth = %f\n", vp->max_depth);
}

static void dump_depth_stencil_state(struct brw_context *brw, uint32_t offset)
{
   const char *name = "D_S";
   struct gen6_depth_stencil_state *ds = brw->batch.bo->virtual + offset;

   batch_out(brw, name, offset, 0,
	     "stencil %sable, func %d, write %sable\n",
	     ds->ds0.stencil_enable ? "en" : "dis",
	     ds->ds0.stencil_func,
	     ds->ds0.stencil_write_enable ? "en" : "dis");
   batch_out(brw, name, offset, 1,
	     "stencil test mask 0x%x, write mask 0x%x\n",
	     ds->ds1.stencil_test_mask, ds->ds1.stencil_write_mask);
   batch_out(brw, name, offset, 2,
	     "depth test %sable, func %d, write %sable\n",
	     ds->ds2.depth_test_enable ? "en" : "dis",
	     ds->ds2.depth_test_func,
	     ds->ds2.depth_write_enable ? "en" : "dis");
}

static void dump_cc_state_gen4(struct brw_context *brw, uint32_t offset)
{
   const char *name = "CC";

   batch_out(brw, name, offset, 0, "cc0\n");
   batch_out(brw, name, offset, 1, "cc1\n");
   batch_out(brw, name, offset, 2, "cc2\n");
   batch_out(brw, name, offset, 3, "cc3\n");
   batch_out(brw, name, offset, 4, "cc4: viewport offset\n");
   batch_out(brw, name, offset, 5, "cc5\n");
   batch_out(brw, name, offset, 6, "cc6\n");
   batch_out(brw, name, offset, 7, "cc7\n");
}

static void dump_cc_state_gen6(struct brw_context *brw, uint32_t offset)
{
   const char *name = "CC";
   struct gen6_color_calc_state *cc = brw->batch.bo->virtual + offset;

   batch_out(brw, name, offset, 0,
	     "alpha test format %s, round disable %d, stencil ref %d, "
	     "bf stencil ref %d\n",
	     cc->cc0.alpha_test_format ? "FLOAT32" : "UNORM8",
	     cc->cc0.round_disable,
	     cc->cc0.stencil_ref,
	     cc->cc0.bf_stencil_ref);
   batch_out(brw, name, offset, 1, "\n");
   batch_out(brw, name, offset, 2, "constant red %f\n", cc->constant_r);
   batch_out(brw, name, offset, 3, "constant green %f\n", cc->constant_g);
   batch_out(brw, name, offset, 4, "constant blue %f\n", cc->constant_b);
   batch_out(brw, name, offset, 5, "constant alpha %f\n", cc->constant_a);
}

static void dump_blend_state(struct brw_context *brw, uint32_t offset)
{
   const char *name = "BLEND";

   batch_out(brw, name, offset, 0, "\n");
   batch_out(brw, name, offset, 1, "\n");
}

static void
gen8_dump_blend_state(struct brw_context *brw, uint32_t offset, uint32_t size)
{
   const uint32_t *blend = brw->batch.bo->virtual + offset;
   const char *logicop[] =
   {
        "LOGICOP_CLEAR (BLACK)",
        "LOGICOP_NOR",
        "LOGICOP_AND_INVERTED",
        "LOGICOP_COPY_INVERTED",
        "LOGICOP_AND_REVERSE",
        "LOGICOP_INVERT",
        "LOGICOP_XOR",
        "LOGICOP_NAND",
        "LOGICOP_AND",
        "LOGICOP_EQUIV",
        "LOGICOP_NOOP",
        "LOGICOP_OR_INVERTED",
        "LOGICOP_COPY",
        "LOGICOP_OR_REVERSE",
        "LOGICOP_OR",
        "LOGICOP_SET (WHITE)"
   };

   const char *blend_function[] =
   { "ADD", "SUBTRACT", "REVERSE_SUBTRACT", "MIN", "MAX};" };

   const char *blend_factor[0x1b] =
   {
      "RSVD",
      "ONE",
      "SRC_COLOR", "SRC_ALPHA",
      "DST_ALPHA", "DST_COLOR",
      "SRC_ALPHA_SATURATE",
      "CONST_COLOR", "CONST_ALPHA",
      "SRC1_COLOR", "SRC1_ALPHA",
      "RSVD", "RSVD", "RSVD", "RSVD", "RSVD", "RSVD",
      "ZERO",
      "INV_SRC_COLOR", "INV_SRC_ALPHA",
      "INV_DST_ALPHA", "INV_DST_COLOR",
      "RSVD",
      "INV_CONST_COLOR", "INV_CONST_ALPHA",
      "INV_SRC1_COLOR", "INV_SRC1_ALPHA"
   };

   batch_out(brw, "BLEND", offset, 0, "Alpha blend/test\n");

   if (((size) % 2) != 0)
      fprintf(stderr, "Invalid blend state size %d\n", size);

   for (int i = 1; i < size / 4; i += 2) {
      char name[sizeof("BLEND_ENTRYXXX")];
      sprintf(name, "BLEND_ENTRY%02d", (i - 1) / 2);
      if (blend[i + 1] & GEN8_BLEND_LOGIC_OP_ENABLE) {
         batch_out(brw, name, offset, i + 1, "%s\n",
                   logicop[GET_FIELD(blend[i + 1],
                                     GEN8_BLEND_LOGIC_OP_FUNCTION)]);
      } else if (blend[i] & GEN8_BLEND_COLOR_BUFFER_BLEND_ENABLE) {
         batch_out64(brw, name, offset, i,
                   "\n\t\t\tColor Buffer Blend factor %s,%s,%s,%s (src,dst,src alpha, dst alpha)"
                   "\n\t\t\tfunction %s,%s (color, alpha), Disables: %c%c%c%c\n",
                   blend_factor[GET_FIELD(blend[i],
                                          GEN8_BLEND_SRC_BLEND_FACTOR)],
                   blend_factor[GET_FIELD(blend[i],
                                          GEN8_BLEND_DST_BLEND_FACTOR)],
                   blend_factor[GET_FIELD(blend[i],
                                          GEN8_BLEND_SRC_ALPHA_BLEND_FACTOR)],
                   blend_factor[GET_FIELD(blend[i],
                                          GEN8_BLEND_DST_ALPHA_BLEND_FACTOR)],
                   blend_function[GET_FIELD(blend[i],
                                            GEN8_BLEND_COLOR_BLEND_FUNCTION)],
                   blend_function[GET_FIELD(blend[i],
                                            GEN8_BLEND_ALPHA_BLEND_FUNCTION)],
                   blend[i] & GEN8_BLEND_WRITE_DISABLE_RED ? 'R' : '-',
                   blend[i] & GEN8_BLEND_WRITE_DISABLE_GREEN ? 'G' : '-',
                   blend[i] & GEN8_BLEND_WRITE_DISABLE_BLUE ? 'B' : '-',
                   blend[i] & GEN8_BLEND_WRITE_DISABLE_ALPHA ? 'A' : '-'
                   );
      } else if (!blend[i] && (blend[i + 1] == 0xb)) {
         batch_out64(brw, name, offset, i, "NOP blend state\n");
      } else {
         batch_out64(brw, name, offset, i, "????\n");
      }
   }
}

static void
dump_scissor(struct brw_context *brw, uint32_t offset)
{
   const char *name = "SCISSOR";
   struct gen6_scissor_rect *scissor = brw->batch.bo->virtual + offset;

   batch_out(brw, name, offset, 0, "xmin %d, ymin %d\n",
	     scissor->xmin, scissor->ymin);
   batch_out(brw, name, offset, 1, "xmax %d, ymax %d\n",
	     scissor->xmax, scissor->ymax);
}

static void
dump_vs_constants(struct brw_context *brw, uint32_t offset, uint32_t size)
{
   const char *name = "VS_CONST";
   uint32_t *as_uint = brw->batch.bo->virtual + offset;
   float *as_float = brw->batch.bo->virtual + offset;
   int i;

   for (i = 0; i < size / 4; i += 4) {
      batch_out(brw, name, offset, i, "%3d: (% f % f % f % f) (0x%08x 0x%08x 0x%08x 0x%08x)\n",
		i / 4,
		as_float[i], as_float[i + 1], as_float[i + 2], as_float[i + 3],
		as_uint[i], as_uint[i + 1], as_uint[i + 2], as_uint[i + 3]);
   }
}

static void
dump_wm_constants(struct brw_context *brw, uint32_t offset, uint32_t size)
{
   const char *name = "WM_CONST";
   uint32_t *as_uint = brw->batch.bo->virtual + offset;
   float *as_float = brw->batch.bo->virtual + offset;
   int i;

   for (i = 0; i < size / 4; i += 4) {
      batch_out(brw, name, offset, i, "%3d: (% f % f % f % f) (0x%08x 0x%08x 0x%08x 0x%08x)\n",
		i / 4,
		as_float[i], as_float[i + 1], as_float[i + 2], as_float[i + 3],
		as_uint[i], as_uint[i + 1], as_uint[i + 2], as_uint[i + 3]);
   }
}

static void dump_binding_table(struct brw_context *brw, uint32_t offset,
			       uint32_t size)
{
   char name[20];
   int i;
   uint32_t *data = brw->batch.bo->virtual + offset;

   for (i = 0; i < size / 4; i++) {
      if (data[i] == 0)
	 continue;

      sprintf(name, "BIND%d", i);
      batch_out(brw, name, offset, i, "surface state address\n");
   }
}

static void
dump_prog_cache(struct brw_context *brw)
{
   struct brw_cache *cache = &brw->cache;
   unsigned int b;

   drm_intel_bo_map(brw->cache.bo, false);

   for (b = 0; b < cache->size; b++) {
      struct brw_cache_item *item;

      for (item = cache->items[b]; item; item = item->next) {
	 const char *name;

	 switch (item->cache_id) {
	 case BRW_CACHE_VS_PROG:
	    name = "VS kernel";
	    break;
	 case BRW_CACHE_FF_GS_PROG:
	    name = "Fixed-function GS kernel";
	    break;
         case BRW_CACHE_GS_PROG:
            name = "GS kernel";
            break;
	 case BRW_CACHE_CLIP_PROG:
	    name = "CLIP kernel";
	    break;
	 case BRW_CACHE_SF_PROG:
	    name = "SF kernel";
	    break;
	 case BRW_CACHE_FS_PROG:
	    name = "FS kernel";
	    break;
         case BRW_CACHE_CS_PROG:
            name = "CS kernel";
            break;
	 default:
	    name = "unknown";
	    break;
	 }

         fprintf(stderr, "%s:\n", name);
         brw_disassemble(brw->intelScreen->devinfo, brw->cache.bo->virtual,
                         item->offset, item->size, stderr);
      }
   }

   drm_intel_bo_unmap(brw->cache.bo);
}

static void
dump_state_batch(struct brw_context *brw)
{
   int i;

   for (i = 0; i < brw->state_batch_count; i++) {
      uint32_t offset = brw->state_batch_list[i].offset;
      uint32_t size = brw->state_batch_list[i].size;

      switch (brw->state_batch_list[i].type) {
      case AUB_TRACE_VS_STATE:
	 dump_vs_state(brw, offset);
	 break;
      case AUB_TRACE_GS_STATE:
	 dump_gs_state(brw, offset);
	 break;
      case AUB_TRACE_CLIP_STATE:
	 dump_clip_state(brw, offset);
	 break;
      case AUB_TRACE_SF_STATE:
	 dump_sf_state(brw, offset);
	 break;
      case AUB_TRACE_WM_STATE:
	 dump_wm_state(brw, offset);
	 break;
      case AUB_TRACE_CLIP_VP_STATE:
	 dump_clip_viewport_state(brw, offset);
	 break;
      case AUB_TRACE_SF_VP_STATE:
	 if (brw->gen >= 7) {
	    dump_sf_clip_viewport_state(brw, offset);
	 } else {
	    dump_sf_viewport_state(brw, offset);
	 }
	 break;
      case AUB_TRACE_CC_VP_STATE:
	 dump_cc_viewport_state(brw, offset);
	 break;
      case AUB_TRACE_DEPTH_STENCIL_STATE:
	 dump_depth_stencil_state(brw, offset);
	 break;
      case AUB_TRACE_CC_STATE:
	 if (brw->gen >= 6)
	    dump_cc_state_gen6(brw, offset);
	 else
	    dump_cc_state_gen4(brw, offset);
	 break;
      case AUB_TRACE_BLEND_STATE:
         if (brw->gen >= 8)
            gen8_dump_blend_state(brw, offset, size);
         else
            dump_blend_state(brw, offset);
	 break;
      case AUB_TRACE_BINDING_TABLE:
	 dump_binding_table(brw, offset, size);
	 break;
      case AUB_TRACE_SURFACE_STATE:
         if (brw->gen >= 8) {
            dump_gen8_surface_state(brw, offset,
                                    brw->state_batch_list[i].index);
         } else if (brw->gen >= 7) {
	    dump_gen7_surface_state(brw, offset);
         } else {
            dump_surface_state(brw, offset);
         }
	 break;
      case AUB_TRACE_SAMPLER_STATE:
         if (brw->gen >= 7)
            gen7_dump_sampler_state(brw, offset, size);
         else
            dump_sampler_state(brw, offset, size);
	 break;
      case AUB_TRACE_SAMPLER_DEFAULT_COLOR:
	 dump_sdc(brw, offset);
	 break;
      case AUB_TRACE_SCISSOR_STATE:
	 dump_scissor(brw, offset);
	 break;
      case AUB_TRACE_VS_CONSTANTS:
	 dump_vs_constants(brw, offset, size);
	 break;
      case AUB_TRACE_WM_CONSTANTS:
	 dump_wm_constants(brw, offset, size);
	 break;
      default:
	 break;
      }
   }
}

/**
 * Print additional debug information associated with the batchbuffer
 * when DEBUG_BATCH is set.
 *
 * For 965, this means mapping the state buffers that would have been referenced
 * by the batchbuffer and dumping them.
 *
 * The buffer offsets printed rely on the buffer containing the last offset
 * it was validated at.
 */
void brw_debug_batch(struct brw_context *brw)
{
   drm_intel_bo_map(brw->batch.bo, false);
   dump_state_batch(brw);
   drm_intel_bo_unmap(brw->batch.bo);

   if (0)
      dump_prog_cache(brw);
}
