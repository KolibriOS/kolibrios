/*
 * Copyright © 2012 Intel Corporation
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
 *    Chris Wilson <chris@chris-wilson.co.uk>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sna.h"
#include "sna_render.h"
#include "sna_render_inline.h"
#include "gen4_vertex.h"

void gen4_vertex_flush(struct sna *sna)
{
    DBG(("%s[%x] = %d\n", __FUNCTION__,
         4*sna->render.vertex_offset,
         sna->render.vertex_index - sna->render.vertex_start));

    assert(sna->render.vertex_offset);
    assert(sna->render.vertex_index > sna->render.vertex_start);

    sna->kgem.batch[sna->render.vertex_offset] =
        sna->render.vertex_index - sna->render.vertex_start;
    sna->render.vertex_offset = 0;
}

int gen4_vertex_finish(struct sna *sna)
{
    struct kgem_bo *bo;
    unsigned int i;
    unsigned hint, size;

    DBG(("%s: used=%d / %d\n", __FUNCTION__,
         sna->render.vertex_used, sna->render.vertex_size));
    assert(sna->render.vertex_offset == 0);
    assert(sna->render.vertex_used);

	sna_vertex_wait__locked(&sna->render);

    /* Note: we only need dword alignment (currently) */

    bo = sna->render.vbo;
    if (bo) {
        for (i = 0; i < sna->render.nvertex_reloc; i++) {
            DBG(("%s: reloc[%d] = %d\n", __FUNCTION__,
                 i, sna->render.vertex_reloc[i]));

            sna->kgem.batch[sna->render.vertex_reloc[i]] =
                kgem_add_reloc(&sna->kgem,
                           sna->render.vertex_reloc[i], bo,
                           I915_GEM_DOMAIN_VERTEX << 16,
                           0);
        }

        assert(!sna->render.active);
        sna->render.nvertex_reloc = 0;
        sna->render.vertex_used = 0;
        sna->render.vertex_index = 0;
        sna->render.vbo = NULL;
        sna->render.vb_id = 0;

        kgem_bo_destroy(&sna->kgem, bo);
    }

    hint = CREATE_GTT_MAP;
    if (bo)
        hint |= CREATE_CACHED | CREATE_NO_THROTTLE;

    size = 256*1024;
    assert(!sna->render.active);
    sna->render.vertices = NULL;
    sna->render.vbo = kgem_create_linear(&sna->kgem, size, hint);
    while (sna->render.vbo == NULL && size > 16*1024) {
        size /= 2;
        sna->render.vbo = kgem_create_linear(&sna->kgem, size, hint);
    }
    if (sna->render.vbo == NULL)
        sna->render.vbo = kgem_create_linear(&sna->kgem,
                             256*1024, CREATE_GTT_MAP);
    if (sna->render.vbo)
        sna->render.vertices = kgem_bo_map(&sna->kgem, sna->render.vbo);
    if (sna->render.vertices == NULL) {
        if (sna->render.vbo) {
            kgem_bo_destroy(&sna->kgem, sna->render.vbo);
            sna->render.vbo = NULL;
        }
        sna->render.vertices = sna->render.vertex_data;
        sna->render.vertex_size = ARRAY_SIZE(sna->render.vertex_data);
        return 0;
    }

    if (sna->render.vertex_used) {
        DBG(("%s: copying initial buffer x %d to handle=%d\n",
             __FUNCTION__,
             sna->render.vertex_used,
             sna->render.vbo->handle));
        assert(sizeof(float)*sna->render.vertex_used <=
               __kgem_bo_size(sna->render.vbo));
        memcpy(sna->render.vertices,
               sna->render.vertex_data,
               sizeof(float)*sna->render.vertex_used);
    }

    size = __kgem_bo_size(sna->render.vbo)/4;
    if (size >= UINT16_MAX)
        size = UINT16_MAX - 1;

    DBG(("%s: create vbo handle=%d, size=%d\n",
         __FUNCTION__, sna->render.vbo->handle, size));

    sna->render.vertex_size = size;
    return sna->render.vertex_size - sna->render.vertex_used;
}

void gen4_vertex_close(struct sna *sna)
{
    struct kgem_bo *bo, *free_bo = NULL;
    unsigned int i, delta = 0;

    assert(sna->render.vertex_offset == 0);
    if (!sna->render.vb_id)
        return;

    DBG(("%s: used=%d, vbo active? %d, vb=%x, nreloc=%d\n",
         __FUNCTION__, sna->render.vertex_used, sna->render.vbo ? sna->render.vbo->handle : 0,
         sna->render.vb_id, sna->render.nvertex_reloc));

    assert(!sna->render.active);

    bo = sna->render.vbo;
    if (bo) {
        if (sna->render.vertex_size - sna->render.vertex_used < 64) {
            DBG(("%s: discarding vbo (full), handle=%d\n", __FUNCTION__, sna->render.vbo->handle));
            sna->render.vbo = NULL;
            sna->render.vertices = sna->render.vertex_data;
            sna->render.vertex_size = ARRAY_SIZE(sna->render.vertex_data);
            free_bo = bo;
        } else if (IS_CPU_MAP(bo->map) && !sna->kgem.has_llc) {
            DBG(("%s: converting CPU map to GTT\n", __FUNCTION__));
            sna->render.vertices =
                kgem_bo_map__gtt(&sna->kgem, sna->render.vbo);
            if (sna->render.vertices == NULL) {
                sna->render.vbo = NULL;
                sna->render.vertices = sna->render.vertex_data;
                sna->render.vertex_size = ARRAY_SIZE(sna->render.vertex_data);
                free_bo = bo;
            }

        }
    } else {
        if (sna->kgem.nbatch + sna->render.vertex_used <= sna->kgem.surface) {
            DBG(("%s: copy to batch: %d @ %d\n", __FUNCTION__,
                 sna->render.vertex_used, sna->kgem.nbatch));
            memcpy(sna->kgem.batch + sna->kgem.nbatch,
                   sna->render.vertex_data,
                   sna->render.vertex_used * 4);
            delta = sna->kgem.nbatch * 4;
            bo = NULL;
            sna->kgem.nbatch += sna->render.vertex_used;
        } else {
            bo = kgem_create_linear(&sna->kgem,
                        4*sna->render.vertex_used,
                        CREATE_NO_THROTTLE);
            if (bo && !kgem_bo_write(&sna->kgem, bo,
                         sna->render.vertex_data,
                         4*sna->render.vertex_used)) {
                kgem_bo_destroy(&sna->kgem, bo);
                bo = NULL;
            }
            DBG(("%s: new vbo: %d\n", __FUNCTION__,
                 sna->render.vertex_used));
            free_bo = bo;
        }
    }

    assert(sna->render.nvertex_reloc);
    for (i = 0; i < sna->render.nvertex_reloc; i++) {
        DBG(("%s: reloc[%d] = %d\n", __FUNCTION__,
             i, sna->render.vertex_reloc[i]));

        sna->kgem.batch[sna->render.vertex_reloc[i]] =
            kgem_add_reloc(&sna->kgem,
                       sna->render.vertex_reloc[i], bo,
                       I915_GEM_DOMAIN_VERTEX << 16,
                       delta);
    }
    sna->render.nvertex_reloc = 0;
    sna->render.vb_id = 0;

    if (sna->render.vbo == NULL) {
        assert(!sna->render.active);
        sna->render.vertex_used = 0;
        sna->render.vertex_index = 0;
        assert(sna->render.vertices == sna->render.vertex_data);
        assert(sna->render.vertex_size == ARRAY_SIZE(sna->render.vertex_data));
    }

    if (free_bo)
        kgem_bo_destroy(&sna->kgem, free_bo);
}

fastcall static void
emit_primitive_identity_source_mask(struct sna *sna,
				    const struct sna_composite_op *op,
				    const struct sna_composite_rectangles *r)
{
	union {
		struct sna_coordinate p;
		float f;
	} dst;
	float src_x, src_y;
	float msk_x, msk_y;
	float w, h;
	float *v;

	src_x = r->src.x + op->src.offset[0];
	src_y = r->src.y + op->src.offset[1];
	msk_x = r->mask.x + op->mask.offset[0];
	msk_y = r->mask.y + op->mask.offset[1];
	w = r->width;
	h = r->height;

	assert(op->floats_per_rect == 15);
	assert((sna->render.vertex_used % 5) == 0);
	v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 15;

	dst.p.x = r->dst.x + r->width;
	dst.p.y = r->dst.y + r->height;
	v[0] = dst.f;
	v[1] = (src_x + w) * op->src.scale[0];
	v[2] = (src_y + h) * op->src.scale[1];
	v[3] = (msk_x + w) * op->mask.scale[0];
	v[4] = (msk_y + h) * op->mask.scale[1];

	dst.p.x = r->dst.x;
	v[5] = dst.f;
	v[6] = src_x * op->src.scale[0];
	v[7] = v[2];
	v[8] = msk_x * op->mask.scale[0];
	v[9] = v[4];

	dst.p.y = r->dst.y;
	v[10] = dst.f;
	v[11] = v[6];
	v[12] = src_y * op->src.scale[1];
	v[13] = v[8];
	v[14] = msk_y * op->mask.scale[1];
}

unsigned gen4_choose_composite_emitter(struct sna_composite_op *tmp)
{
	unsigned vb;

	if (tmp->mask.bo) {
		if (tmp->mask.transform == NULL) {
			if (tmp->src.is_solid) {
				DBG(("%s: solid, identity mask\n", __FUNCTION__));
			} else if (tmp->src.is_linear) {
				DBG(("%s: linear, identity mask\n", __FUNCTION__));
			} else if (tmp->src.transform == NULL) {
				DBG(("%s: identity source, identity mask\n", __FUNCTION__));
				tmp->prim_emit = emit_primitive_identity_source_mask;
				tmp->floats_per_vertex = 5;
				vb = 2 << 2 | 2;
			} else if (tmp->src.is_affine) {
				DBG(("%s: simple src, identity mask\n", __FUNCTION__));
			} else {
				DBG(("%s: projective source, identity mask\n", __FUNCTION__));
			}
		} else {
			DBG(("%s: general mask: floats-per-vertex=%d, vb=%x\n",
			     __FUNCTION__,tmp->floats_per_vertex, vb));
		}
	} else {
	}
	tmp->floats_per_rect = 3 * tmp->floats_per_vertex;

	return vb;
}
