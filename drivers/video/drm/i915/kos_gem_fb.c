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


struct drm_i915_gem_object *
kos_gem_fb_object_create(struct drm_device *dev,
                           u32 gtt_offset,
                           u32 size)
{
    struct drm_i915_private *dev_priv = dev->dev_private;
    struct i915_address_space *ggtt = &dev_priv->gtt.base;
    struct drm_i915_gem_object *obj;
    struct drm_mm_node *fb_node;
    struct i915_vma *vma;
    int ret;

    DRM_DEBUG_KMS("creating preallocated framebuffer object: gtt_offset=%x, size=%x\n",
                  gtt_offset, size);

    /* KISS and expect everything to be page-aligned */
    BUG_ON(size & 4095);

    if (WARN_ON(size == 0))
        return NULL;

    fb_node = kzalloc(sizeof(*fb_node), GFP_KERNEL);
    if (!fb_node)
        return NULL;

    fb_node->start = gtt_offset;
    fb_node->size = size;

    obj = _kos_fb_object_create(dev, fb_node);
    if (obj == NULL) {
        DRM_DEBUG_KMS("failed to preallocate framebuffer object\n");
        kfree(fb_node);
        return NULL;
    }

    vma = i915_gem_obj_lookup_or_create_vma(obj, ggtt);
    if (IS_ERR(vma)) {
        ret = PTR_ERR(vma);
        goto err_out;
    }

    /* To simplify the initialisation sequence between KMS and GTT,
     * we allow construction of the stolen object prior to
     * setting up the GTT space. The actual reservation will occur
     * later.
     */
    vma->node.start = gtt_offset;
    vma->node.size = size;
    if (drm_mm_initialized(&ggtt->mm)) {
        ret = drm_mm_reserve_node(&ggtt->mm, &vma->node);
        if (ret) {
            DRM_DEBUG_KMS("failed to allocate framebuffer GTT space\n");
            goto err_vma;
        }
    }

//    obj->has_global_gtt_mapping = 1;

    list_add_tail(&obj->global_list, &dev_priv->mm.bound_list);
    list_add_tail(&vma->mm_list, &ggtt->inactive_list);

    mutex_lock(&dev->object_name_lock);
    idr_preload(GFP_KERNEL);

    if (!obj->base.name) {
        ret = idr_alloc(&dev->object_name_idr, &obj->base, 1, 0, GFP_NOWAIT);
        if (ret < 0)
            goto err_gem;

        obj->base.name = ret;

        /* Allocate a reference for the name table.  */
        drm_gem_object_reference(&obj->base);

        DRM_DEBUG_KMS("%s allocate fb name %d\n", __FUNCTION__, obj->base.name );
    }

    idr_preload_end();
    mutex_unlock(&dev->object_name_lock);
    drm_gem_object_unreference(&obj->base);
    return obj;

err_gem:
    idr_preload_end();
    mutex_unlock(&dev->object_name_lock);
err_vma:
    i915_gem_vma_destroy(vma);
err_out:
    kfree(fb_node);
    drm_gem_object_unreference(&obj->base);
    return NULL;
}

