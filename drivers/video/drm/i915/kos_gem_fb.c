/*
 * Copyright © 2008-2012 Intel Corporation
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
 *    Chris Wilson <chris@chris-wilson.co.uk>
 *
 */

#include <drm/drmP.h>
#include <drm/i915_drm.h>
#include "i915_drv.h"

static struct sg_table *
i915_pages_create_for_fb(struct drm_device *dev,
                 u32 offset, u32 size)
{
    struct drm_i915_private *dev_priv = dev->dev_private;
    struct sg_table *st;
    struct scatterlist *sg;
    addr_t fb_pages;

    DRM_DEBUG_DRIVER("offset=0x%x, size=%d\n", offset, size);
//    BUG_ON(offset > dev_priv->gtt.stolen_size - size);

    /* We hide that we have no struct page backing our stolen object
     * by wrapping the contiguous physical allocation with a fake
     * dma mapping in a single scatterlist.
     */

    st = kmalloc(sizeof(*st), GFP_KERNEL);
    if (st == NULL)
        return NULL;

    if (sg_alloc_table(st, 1, GFP_KERNEL)) {
        kfree(st);
        return NULL;
    }

    fb_pages = AllocPages(size/PAGE_SIZE);
    if(fb_pages == 0)
    {
        kfree(st);
        return NULL;
    };

    sg = st->sgl;
    sg->offset = offset;
    sg->length = size;

    sg_dma_address(sg) = (dma_addr_t)fb_pages;
    sg_dma_len(sg) = size;

    return st;
}


static int kos_fb_object_get_pages(struct drm_i915_gem_object *obj)
{
    BUG();
    return -EINVAL;
}

static void kos_fb_object_put_pages(struct drm_i915_gem_object *obj)
{
    /* Should only be called during free */
    sg_free_table(obj->pages);
    kfree(obj->pages);
}

static const struct drm_i915_gem_object_ops kos_fb_object_ops = {
    .get_pages = kos_fb_object_get_pages,
    .put_pages = kos_fb_object_put_pages,
};

static struct drm_i915_gem_object *
_kos_fb_object_create(struct drm_device *dev,
                   struct drm_mm_node *fb_node)
{
    struct drm_i915_gem_object *obj;

    obj = i915_gem_object_alloc(dev);
    if (obj == NULL)
        return NULL;

    drm_gem_private_object_init(dev, &obj->base, fb_node->size);
    i915_gem_object_init(obj, &kos_fb_object_ops);

    obj->pages = i915_pages_create_for_fb(dev,
                          fb_node->start, fb_node->size);
    if (obj->pages == NULL)
        goto cleanup;

    obj->has_dma_mapping = true;
    i915_gem_object_pin_pages(obj);
    obj->stolen = fb_node;

    obj->base.read_domains = I915_GEM_DOMAIN_GTT;
    obj->cache_level = I915_CACHE_NONE;
    obj->tiling_mode = I915_TILING_X;

    return obj;

cleanup:
    i915_gem_object_free(obj);
    return NULL;
}



