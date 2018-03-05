#include <drm/drmP.h>
#include "i915_drv.h"
#include "intel_drv.h"
#include <syscall.h>
#include <display.h>

static struct mutex cursor_lock;

static void __stdcall move_cursor_kms(cursor_t *cursor, int x, int y)
{
    struct drm_crtc *crtc = os_display->crtc;
    struct drm_plane_state *cursor_state = crtc->cursor->state;

    x-= cursor->hot_x;
    y-= cursor->hot_y;

    crtc->cursor_x = x;
    crtc->cursor_y = y;

    cursor_state->crtc_x = x;
    cursor_state->crtc_y = y;

    intel_crtc_update_cursor(crtc, cursor_state);
};

static cursor_t* __stdcall select_cursor_kms(cursor_t *cursor)
{
    struct drm_i915_private *dev_priv = os_display->ddev->dev_private;
    struct drm_crtc   *crtc = os_display->crtc;
    struct intel_crtc *intel_crtc = to_intel_crtc(crtc);
    struct drm_plane  *cursor_plane = crtc->cursor;
    struct intel_plane_state *cursor_state = to_intel_plane_state(cursor_plane->state);

    cursor_t *old;

    old = os_display->cursor;

    mutex_lock(&cursor_lock);

    os_display->cursor = cursor;

    if (!dev_priv->info.cursor_needs_physical)
       intel_crtc->cursor_addr = i915_gem_obj_ggtt_offset(cursor->cobj);
    else
        intel_crtc->cursor_addr = (addr_t)cursor->cobj;

    cursor_state->visible = 1;

    cursor_plane->state->crtc_w   = 64;
    cursor_plane->state->crtc_h   = 64;
    cursor_plane->state->rotation = 0;

    mutex_unlock(&cursor_lock);

    move_cursor_kms(cursor, crtc->cursor_x, crtc->cursor_y);
    return old;
};

static void __stdcall restore_cursor(int x, int y){};
static void disable_mouse(void){};

static void __attribute__((regparm(1))) destroy_cursor(cursor_t *cursor)
{
    struct drm_i915_private *dev_priv = main_device->dev_private;
    struct drm_i915_gem_object *obj = cursor->cobj;
    u32      ifl;

    ifl = safe_cli();
    list_del(&cursor->list);
    safe_sti(ifl);

    if (!dev_priv->info.cursor_needs_physical)
    {
    i915_gem_object_ggtt_unpin(cursor->cobj);

    mutex_lock(&main_device->struct_mutex);
    drm_gem_object_unreference(&obj->base);
    mutex_unlock(&main_device->struct_mutex);
    }
    else
    {
        addr_t page = (addr_t)obj;
        for(addr_t i = 0; i < (KMS_CURSOR_WIDTH*KMS_CURSOR_HEIGHT*8)/4096; i+=4096)
            FreePage(page+i);
    }

    __DestroyObject(cursor);
};

static int init_cursor(cursor_t *cursor)
{
    display_t *display = GetDisplay();
    struct drm_device *dev = display->ddev;
    struct drm_i915_private *dev_priv = dev->dev_private;
    struct drm_i915_gem_object *obj;
    uint32_t *bits;
    uint32_t *src;
    void     *mapped;

    int       i,j;
    int       ret;

    mutex_lock(&dev->struct_mutex);

    if (dev_priv->info.cursor_needs_physical)
    {
        mapped = bits = (uint32_t*)KernelAlloc(KMS_CURSOR_WIDTH*KMS_CURSOR_HEIGHT*8);
        if (unlikely(bits == NULL))
        {
            ret = -ENOMEM;
            goto unlock;
        };
        cursor->cobj = (struct drm_i915_gem_object *)GetPgAddr(bits);
    }
    else
    {
        obj = i915_gem_alloc_object(display->ddev, KMS_CURSOR_WIDTH*KMS_CURSOR_HEIGHT*4);
        if (unlikely(obj == NULL))
        {
            ret = -ENOMEM;
            goto unlock;
        };

        ret = i915_gem_object_ggtt_pin(obj, &i915_ggtt_view_normal, 128*1024, PIN_GLOBAL);
        if (ret)
            goto unref;

        ret = i915_gem_object_set_to_gtt_domain(obj, true);
        if (ret)
            goto unpin;

/* You don't need to worry about fragmentation issues.
 * GTT space is continuous. I guarantee it.                           */

        mapped = bits = (u32*)MapIoMem(dev_priv->gtt.mappable_base + i915_gem_obj_ggtt_offset(obj),
                    KMS_CURSOR_WIDTH*KMS_CURSOR_HEIGHT*4, PG_SW);

        if (unlikely(bits == NULL))
        {
            ret = -ENOMEM;
            goto unpin;
        };
        cursor->cobj = obj;
    };

    mutex_unlock(&dev->struct_mutex);

    src = cursor->data;

    for(i = 0; i < 32; i++)
    {
        for(j = 0; j < 32; j++)
            *bits++ = *src++;
        for(j = 32; j < KMS_CURSOR_WIDTH; j++)
            *bits++ = 0;
    }
    for(i = 0; i < KMS_CURSOR_WIDTH*(KMS_CURSOR_HEIGHT-32); i++)
        *bits++ = 0;

    FreeKernelSpace(mapped);
    KernelFree(cursor->data);
    cursor->data = bits;
    cursor->header.destroy = destroy_cursor;

    return 0;

unpin:
    i915_gem_object_ggtt_unpin(obj);
unref:
    drm_gem_object_unreference(&obj->base);
unlock:
    mutex_unlock(&dev->struct_mutex);
    return ret;
}

void init_system_cursors(struct drm_device *dev)
{
    cursor_t  *cursor;
    display_t *display;
    u32      ifl;

    display = GetDisplay();

    mutex_init(&cursor_lock);

    ifl = safe_cli();
    {
        list_for_each_entry(cursor, &display->cursors, list)
        {
            init_cursor(cursor);
        };

        display->restore_cursor(0,0);
        display->init_cursor    = init_cursor;
        display->select_cursor  = select_cursor_kms;
        display->show_cursor    = NULL;
        display->move_cursor    = move_cursor_kms;
        display->restore_cursor = restore_cursor;
        display->disable_mouse  = disable_mouse;
        display->crtc->cursor_x = display->width/2;
        display->crtc->cursor_y = display->height/2;

        select_cursor_kms(display->cursor);
    };
    safe_sti(ifl);
}
