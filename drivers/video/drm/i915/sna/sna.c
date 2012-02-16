#include <drmP.h>
#include <drm.h>
#include "i915_drm.h"
#include "i915_drv.h"
#include "intel_drv.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <errno-base.h>
#include <memory.h>

#include <syscall.h>

#include "../bitmap.h"

#include "sna.h"

struct kgem_bo *create_bo(bitmap_t *bitmap);

static Bool sna_solid_cache_init(struct sna *sna);

struct sna *sna_device;

void no_render_init(struct sna *sna)
{
    struct sna_render *render = &sna->render;

    memset (render,0, sizeof (*render));

    render->vertices = render->vertex_data;
    render->vertex_size = ARRAY_SIZE(render->vertex_data);

//    render->composite = no_render_composite;

//    render->copy_boxes = no_render_copy_boxes;
//    render->copy = no_render_copy;

//    render->fill_boxes = no_render_fill_boxes;
//    render->fill = no_render_fill;
//    render->fill_one = no_render_fill_one;
//    render->clear = no_render_clear;

//    render->reset = no_render_reset;
//    render->flush = no_render_flush;
//    render->fini = no_render_fini;

//    sna->kgem.context_switch = no_render_context_switch;
//    sna->kgem.retire = no_render_retire;

//    if (sna->kgem.gen >= 60)
        sna->kgem.ring = KGEM_RENDER;
}


Bool sna_accel_init(struct sna *sna)
{
    const char *backend;

//    list_init(&sna->deferred_free);
//    list_init(&sna->dirty_pixmaps);
//    list_init(&sna->active_pixmaps);
//    list_init(&sna->inactive_clock[0]);
//    list_init(&sna->inactive_clock[1]);

//    sna_accel_install_timers(sna);


    backend = "no";
    sna->have_render = false;
    sna->default_tiling = 0; //I915_TILING_X;
    no_render_init(sna);

    if ((sna->have_render = gen6_render_init(sna)))
        backend = "SandyBridge";

/*
    if (sna->chipset.info->gen >= 80) {
    } else if (sna->chipset.info->gen >= 70) {
        if ((sna->have_render = gen7_render_init(sna)))
            backend = "IvyBridge";
    } else if (sna->chipset.info->gen >= 60) {
        if ((sna->have_render = gen6_render_init(sna)))
            backend = "SandyBridge";
    } else if (sna->chipset.info->gen >= 50) {
        if ((sna->have_render = gen5_render_init(sna)))
            backend = "Ironlake";
    } else if (sna->chipset.info->gen >= 40) {
        if ((sna->have_render = gen4_render_init(sna)))
            backend = "Broadwater";
    } else if (sna->chipset.info->gen >= 30) {
        if ((sna->have_render = gen3_render_init(sna)))
            backend = "gen3";
    } else if (sna->chipset.info->gen >= 20) {
        if ((sna->have_render = gen2_render_init(sna)))
            backend = "gen2";
    }
*/
    DBG(("%s(backend=%s, have_render=%d)\n",
         __FUNCTION__, backend, sna->have_render));

    kgem_reset(&sna->kgem);

    if (!sna_solid_cache_init(sna))
        return FALSE;

    sna_device = sna;
#if 0
    {
        struct kgem_bo *screen_bo;
        bitmap_t        screen;

        screen.pitch  = 1024*4;
        screen.gaddr  = 0;
        screen.width  = 1024;
        screen.height = 768;
        screen.obj    = (void*)-1;

        screen_bo = create_bo(&screen);

        sna->render.clear(sna, &screen, screen_bo);
    }
#endif

    return TRUE;
}

int sna_init()
{
    struct sna *sna;

    DBG(("%s\n", __FUNCTION__));

    sna = kzalloc(sizeof(struct sna), 0);
    if (sna == NULL)
        return FALSE;

//    sna->mode.cpp = 4;

    kgem_init(&sna->kgem, 60);
/*
    if (!xf86ReturnOptValBool(sna->Options,
                  OPTION_RELAXED_FENCING,
                  sna->kgem.has_relaxed_fencing)) {
        xf86DrvMsg(scrn->scrnIndex,
               sna->kgem.has_relaxed_fencing ? X_CONFIG : X_PROBED,
               "Disabling use of relaxed fencing\n");
        sna->kgem.has_relaxed_fencing = 0;
    }
    if (!xf86ReturnOptValBool(sna->Options,
                  OPTION_VMAP,
                  sna->kgem.has_vmap)) {
        xf86DrvMsg(scrn->scrnIndex,
               sna->kgem.has_vmap ? X_CONFIG : X_PROBED,
               "Disabling use of vmap\n");
        sna->kgem.has_vmap = 0;
    }
*/

    /* Disable tiling by default */
    sna->tiling = SNA_TILING_DISABLE;

    /* Default fail-safe value of 75 Hz */
//    sna->vblank_interval = 1000 * 1000 * 1000 / 75;

    sna->flags = 0;
    sna->flags |= SNA_NO_THROTTLE;
    sna->flags |= SNA_NO_DELAYED_FLUSH;

    return sna_accel_init(sna);
}


static Bool sna_solid_cache_init(struct sna *sna)
{
    struct sna_solid_cache *cache = &sna->render.solid_cache;

    DBG(("%s\n", __FUNCTION__));

    cache->cache_bo =
        kgem_create_linear(&sna->kgem, sizeof(cache->color));
    if (!cache->cache_bo)
        return FALSE;

    /*
     * Initialise [0] with white since it is very common and filling the
     * zeroth slot simplifies some of the checks.
     */
    cache->color[0] = 0xffffffff;
    cache->bo[0] = kgem_create_proxy(cache->cache_bo, 0, sizeof(uint32_t));
    cache->bo[0]->pitch = 4;
    cache->dirty = 1;
    cache->size = 1;
    cache->last = 0;

    return TRUE;
}

void
sna_render_flush_solid(struct sna *sna)
{
    struct sna_solid_cache *cache = &sna->render.solid_cache;

    DBG(("sna_render_flush_solid(size=%d)\n", cache->size));
    assert(cache->dirty);
    assert(cache->size);

    kgem_bo_write(&sna->kgem, cache->cache_bo,
              cache->color, cache->size*sizeof(uint32_t));
    cache->dirty = 0;
    cache->last = 0;
}

static void
sna_render_finish_solid(struct sna *sna, bool force)
{
    struct sna_solid_cache *cache = &sna->render.solid_cache;
    int i;

    DBG(("sna_render_finish_solid(force=%d, domain=%d, busy=%d, dirty=%d)\n",
         force, cache->cache_bo->domain, cache->cache_bo->rq != NULL, cache->dirty));

    if (!force && cache->cache_bo->domain != DOMAIN_GPU)
        return;

    if (cache->dirty)
        sna_render_flush_solid(sna);

    for (i = 0; i < cache->size; i++) {
        if (cache->bo[i] == NULL)
            continue;

        kgem_bo_destroy(&sna->kgem, cache->bo[i]);
        cache->bo[i] = NULL;
    }
    kgem_bo_destroy(&sna->kgem, cache->cache_bo);

    DBG(("sna_render_finish_solid reset\n"));

    cache->cache_bo = kgem_create_linear(&sna->kgem, sizeof(cache->color));
    cache->bo[0] = kgem_create_proxy(cache->cache_bo, 0, sizeof(uint32_t));
    cache->bo[0]->pitch = 4;
    if (force)
        cache->size = 1;
}


struct kgem_bo *
sna_render_get_solid(struct sna *sna, uint32_t color)
{
    struct sna_solid_cache *cache = &sna->render.solid_cache;
    int i;

    DBG(("%s: %08x\n", __FUNCTION__, color));

//    if ((color & 0xffffff) == 0) /* alpha only */
//        return kgem_bo_reference(sna->render.alpha_cache.bo[color>>24]);

    if (color == 0xffffffff) {
        DBG(("%s(white)\n", __FUNCTION__));
        return kgem_bo_reference(cache->bo[0]);
    }

    if (cache->color[cache->last] == color) {
        DBG(("sna_render_get_solid(%d) = %x (last)\n",
             cache->last, color));
        return kgem_bo_reference(cache->bo[cache->last]);
    }

    for (i = 1; i < cache->size; i++) {
        if (cache->color[i] == color) {
            if (cache->bo[i] == NULL) {
                DBG(("sna_render_get_solid(%d) = %x (recreate)\n",
                     i, color));
                goto create;
            } else {
                DBG(("sna_render_get_solid(%d) = %x (old)\n",
                     i, color));
                goto done;
            }
        }
    }

    sna_render_finish_solid(sna, i == ARRAY_SIZE(cache->color));

    i = cache->size++;
    cache->color[i] = color;
    cache->dirty = 1;
    DBG(("sna_render_get_solid(%d) = %x (new)\n", i, color));

create:
    cache->bo[i] = kgem_create_proxy(cache->cache_bo,
                     i*sizeof(uint32_t), sizeof(uint32_t));
    cache->bo[i]->pitch = 4;

done:
    cache->last = i;
    return kgem_bo_reference(cache->bo[i]);
}


int sna_blit_copy(bitmap_t *dst_bitmap, int dst_x, int dst_y,
                  int w, int h, bitmap_t *src_bitmap, int src_x, int src_y)

{
    struct kgem_bo src_bo, dst_bo;

    memset(&src_bo, 0, sizeof(src_bo));
    memset(&dst_bo, 0, sizeof(dst_bo));

    src_bo.gaddr  = src_bitmap->gaddr;
    src_bo.pitch  = src_bitmap->pitch;
    src_bo.tiling = 0;

    dst_bo.gaddr  = dst_bitmap->gaddr;
    dst_bo.pitch  = dst_bitmap->pitch;
    dst_bo.tiling = 0;

    sna_device->render.copy(sna_device, 0, src_bitmap, &src_bo,
                            dst_bitmap, &dst_bo, dst_x, dst_y,
                            src_x, src_y, w, h);
};


