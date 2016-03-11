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
#include <display.h>
#include "intel_drv.h"
#include "i915_drv.h"

static addr_t dummy_fb_page;
static struct kos_framebuffer *fake_fb;

int fake_framebuffer_create()
{
    struct kos_framebuffer *kfb;
    addr_t dummy_table;
    addr_t *pt_addr;
    int pde, pte;

    kfb = kzalloc(sizeof(struct kos_framebuffer),0);
    if(kfb == NULL)
        goto err_0;

    dummy_fb_page = AllocPage();
    if(dummy_fb_page == 0)
        goto err_1;

    for(pde = 0; pde < 8; pde++)
    {
        dummy_table = AllocPage();
        if(dummy_table == 0)
            goto err_2;

        kfb->pde[pde] = dummy_table|PG_UW;

        pt_addr = kmap_atomic((struct page*)dummy_table);
        for(pte = 0; pte < 1024; pte++)
            pt_addr[pte] = dummy_fb_page|PG_UW;
        kunmap_atomic(pt_addr);
    };

    fake_fb = kfb;

    return 0;

err_2:
    for(pte = 0; pte < pde; pte++)
        FreePage(kfb->pde[pte]);
    FreePage(dummy_fb_page);
err_1:
    kfree(kfb);
err_0:
    return -ENOMEM;
};

int kolibri_framebuffer_init(void *param)
{
    struct intel_framebuffer *intel_fb = param;
    struct kos_framebuffer *kfb;
    addr_t dummy_table;
    addr_t *pt_addr = NULL;
    int pde, pte;

    kfb = kzalloc(sizeof(struct kos_framebuffer),0);
    if(kfb == NULL)
        goto err_0;

    kfb->private = intel_fb;

    for(pde = 0; pde < 8; pde++)
    {
        dummy_table = AllocPage();
        if(dummy_table == 0)
            goto err_1;

        kfb->pde[pde] = dummy_table|PG_UW;

        pt_addr = kmap_atomic((struct page*)dummy_table);
        for(pte = 0; pte < 1024; pte++)
            pt_addr[pte] = dummy_fb_page|PG_UW;
        kunmap_atomic(pt_addr);
    };

    intel_fb->private = kfb;

    return 0;
err_1:
    for(pte = 0; pte < pde; pte++)
        FreePage(kfb->pde[pte]);
    kfree(kfb);
err_0:
    return -ENOMEM;
};

void kolibri_framebuffer_update(struct drm_device *dev, struct kos_framebuffer *kfb)
{
    struct drm_i915_private *dev_priv = dev->dev_private;
    struct intel_framebuffer *intel_fb = kfb->private;
    addr_t *pt_addr = NULL;
    int pte = 0;
    int pde = 0;
    int num_pages;
    addr_t pfn;

    num_pages = intel_fb->obj->base.size/4096;
    pfn = dev_priv->gtt.mappable_base + i915_gem_obj_ggtt_offset(intel_fb->obj);

    while(num_pages)
    {
        if (pt_addr == NULL)
        {
            addr_t pt = kfb->pde[pde] & 0xFFFFF000;
            pde++;
            pt_addr = kmap_atomic((struct page*)pt);
        }
        pt_addr[pte] = pfn|PG_UW|PG_WRITEC;
        pfn+= 4096;
        num_pages--;
        if (++pte == 1024)
        {
            kunmap_atomic(pt_addr);
            pt_addr = NULL;
            if (pde == 8)
                break;
            pte = 0;
        }
    }

    if(pt_addr)
    {
        for(; pte < 1024; pte++)
            pt_addr[pte] = dummy_fb_page|PG_UW;
        kunmap_atomic(pt_addr);
    };

    for(; pde < 8; pde++)
    {
        addr_t pt = kfb->pde[pde] & 0xFFFFF000;
        pt_addr = kmap_atomic((struct page*)pt);
        for(pte = 0; pte < 1024; pte++)
            pt_addr[pte] = dummy_fb_page|PG_UW;
        kunmap_atomic(pt_addr);
    }
};

void set_fake_framebuffer()
{
    sysSetFramebuffer(fake_fb);
}
