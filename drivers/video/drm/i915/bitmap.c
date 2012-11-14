
#include <drm/drmP.h>
#include <drm/i915_drm.h>
#include "i915_drv.h"
#include "intel_drv.h"
#include "hmm.h"
#include "bitmap.h"

#define DRIVER_CAPS_0   HW_BIT_BLIT;
#define DRIVER_CAPS_1   0

struct context *context_map[256];

struct hmm bm_mm;

extern struct drm_device *main_device;


void __attribute__((regparm(1))) destroy_bitmap(bitmap_t *bitmap)
{
    dma_addr_t *pages = bitmap->obj->allocated_pages;
    int i;

    free_handle(&bm_mm, bitmap->handle);
    bitmap->handle = 0;
    bitmap->obj->base.read_domains = I915_GEM_DOMAIN_GTT;
    bitmap->obj->base.write_domain = I915_GEM_DOMAIN_CPU;

    mutex_lock(&main_device->struct_mutex);

    drm_gem_object_unreference(&bitmap->obj->base);
    mutex_unlock(&main_device->struct_mutex);

    if(pages != NULL)
    {
        for (i = 0; i < bitmap->page_count; i++)
            FreePage(pages[i]);

        DRM_DEBUG("%s freec %d pages\n", __FUNCTION__, bitmap->page_count);

        free(pages);
    };
    UserFree(bitmap->uaddr);
    __DestroyObject(bitmap);
};


static int bitmap_get_pages_gtt(struct drm_i915_gem_object *obj)
{
    int page_count;

    /* Get the list of pages out of our struct file.  They'll be pinned
     * at this point until we release them.
     */
    page_count = obj->base.size / PAGE_SIZE;
    BUG_ON(obj->allocated_pages == NULL);
    BUG_ON(obj->pages.page != NULL);

    obj->pages.page = obj->allocated_pages;
    obj->pages.nents = page_count;


//   if (obj->tiling_mode != I915_TILING_NONE)
//       i915_gem_object_do_bit_17_swizzle(obj);

    return 0;
}

static void bitmap_put_pages_gtt(struct drm_i915_gem_object *obj)
{
    int ret, i;

    BUG_ON(obj->madv == __I915_MADV_PURGED);

    ret = i915_gem_object_set_to_cpu_domain(obj, true);
    if (ret) {
        /* In the event of a disaster, abandon all caches and
         * hope for the best.
         */
        WARN_ON(ret != -EIO);
        i915_gem_clflush_object(obj);
        obj->base.read_domains = obj->base.write_domain = I915_GEM_DOMAIN_CPU;
    }

    if (obj->madv == I915_MADV_DONTNEED)
        obj->dirty = 0;

    obj->dirty = 0;
}

static const struct drm_i915_gem_object_ops bitmap_object_ops = {
    .get_pages = bitmap_get_pages_gtt,
    .put_pages = bitmap_put_pages_gtt,
};



#if 0
struct  io_call_10         /*     SRV_CREATE_SURFACE    */
{
    u32     handle;       // ignored
    void   *data;         // ignored

    u32     width;
    u32     height;
    u32     pitch;        // ignored

    u32     max_width;
    u32     max_height;
    u32     format;       // reserved mbz
};

#endif

int create_surface(struct drm_device *dev, struct io_call_10 *pbitmap)
{
    struct drm_i915_gem_object *obj;

    bitmap_t   *bitmap;
    u32         handle;
    u32         width, max_width;
    u32         height, max_height;
    u32         size,  max_size;
    u32         pitch, max_pitch;
    void       *uaddr;
    dma_addr_t *pages;
    u32         page_count;

    int         i;

    int   ret;

    pbitmap->handle = 0;
    pbitmap->data   = (void*)-1;

    width  = pbitmap->width;
    height = pbitmap->height;

    if((width == 0)||(height == 0)||(width > 4096)||(height > 4096))
        goto err1;

/*
    if( ((pbitmap->max_width !=0 ) &&
         (pbitmap->max_width < width)) ||
         (pbitmap->max_width > 4096) )
        goto err1;

    if( ((pbitmap->max_height !=0 ) &&
         (pbitmap->max_height < width)) ||
         (pbitmap->max_height > 4096) )
        goto err1;

    if( pbitmap->format != 0)
        goto err1;
*/

    max_width  = (pbitmap->max_width ==0) ? width  : pbitmap->max_width;
    max_height = (pbitmap->max_height==0) ? height : pbitmap->max_height;

    handle = alloc_handle(&bm_mm);
//    printf("%s %d\n",__FUNCTION__, handle);

    if(handle == 0)
        goto err1;

    bitmap = CreateObject(GetPid(), sizeof(*bitmap));
//    printf("bitmap %x\n", bitmap);
    if( bitmap == NULL)
        goto err2;

    bitmap->handle = handle;
    bitmap->header.destroy = destroy_bitmap;
    bitmap->obj    = NULL;

    hmm_set_data(&bm_mm, handle, bitmap);

    pitch = ALIGN(width*4,64);
    size =  roundup(pitch*height, PAGE_SIZE);

//    printf("pitch %d size %d\n", pitch, size);

    max_pitch = ALIGN(max_width*4,64);
    max_size =  roundup(max_pitch*max_height, PAGE_SIZE);

//    printf("max_pitch %d max_size %d\n", max_pitch, max_size);

    uaddr = UserAlloc(max_size);
    if( uaddr == NULL)
        goto err3;
    else
    {
        u32           max_count;
        dma_addr_t    page;
        char *vaddr = uaddr;

        page_count = size/PAGE_SIZE;
        max_count = max_size/PAGE_SIZE;

        pages = kzalloc(max_count*sizeof(dma_addr_t), 0);
        if( pages == NULL)
            goto err4;

        for(i = 0; i < page_count; i++, vaddr+= PAGE_SIZE)
        {
            page = AllocPage();
            if ( page == 0 )
                goto err4;
            pages[i] = page;

            MapPage(vaddr, page, 0x207);        //map as shared page
        };
        bitmap->page_count = page_count;
        bitmap->max_count  = max_count;
    };

    DRM_DEBUG("%s alloc %d pages\n", __FUNCTION__, page_count);

    obj = i915_gem_alloc_object(dev, size);
    if (obj == NULL)
        goto err4;

    obj->ops = &bitmap_object_ops;
    obj->allocated_pages = pages;

    ret = i915_gem_object_pin(obj, PAGE_SIZE, true,true);
    if (ret)
        goto err5;

    obj->mapped = uaddr ;

    bitmap->handle = handle;
    bitmap->uaddr  = uaddr;
    bitmap->pitch  = pitch;
    bitmap->gaddr  = obj->gtt_offset;

    bitmap->width  = width;
    bitmap->height = height;
    bitmap->max_width  = max_width;
    bitmap->max_height = max_height;

    bitmap->obj    = obj;
    bitmap->header.destroy = destroy_bitmap;

    pbitmap->handle = handle;
    pbitmap->data   = uaddr;
    pbitmap->pitch  = pitch;


    DRM_DEBUG("%s handle: %d pitch: %d gpu_addr: %x user_addr: %x\n",
            __FUNCTION__, handle, pitch, obj->gtt_offset, uaddr);

    return 0;

err5:
    mutex_lock(&dev->struct_mutex);
    drm_gem_object_unreference(&obj->base);
    mutex_unlock(&dev->struct_mutex);

err4:
    while (i--)
        FreePage(pages[i]);
    free(pages);
    UserFree(uaddr);

err3:
    __DestroyObject(bitmap);
err2:
    free_handle(&bm_mm, handle);
err1:
    return -1;
};


int lock_surface(struct io_call_12 *pbitmap)
{
    int ret;

    bitmap_t  *bitmap;

    if(unlikely(pbitmap->handle == 0))
        return -1;

    bitmap = (bitmap_t*)hmm_get_data(&bm_mm, pbitmap->handle);

    if(unlikely(bitmap==NULL))
        return -1;

    mutex_lock(&main_device->struct_mutex);
    ret = i915_gem_object_set_to_cpu_domain(bitmap->obj, true);
    mutex_unlock(&main_device->struct_mutex);

    if(ret != 0 )
    {
        pbitmap->data  = NULL;
        pbitmap->pitch = 0;

        dbgprintf("%s fail\n", __FUNCTION__);
        return ret;
    };

    pbitmap->data  = bitmap->uaddr;
    pbitmap->pitch = bitmap->pitch;

    return 0;
};




int init_bitmaps()
{
    int ret;

    ret = init_hmm(&bm_mm, 1024);

    return ret;
};



int get_driver_caps(hwcaps_t *caps)
{
    int ret = 0;

    switch(caps->idx)
    {
        case 0:
            caps->opt[0] = DRIVER_CAPS_0;
            caps->opt[1] = DRIVER_CAPS_1;
            break;

        case 1:
            caps->cap1.max_tex_width  = 4096;
            caps->cap1.max_tex_height = 4096;
            break;
        default:
            ret = 1;
    };
    caps->idx = 1;
    return ret;
}


void __attribute__((regparm(1))) destroy_context(struct context *context)
{
    DRM_DEBUG("destroy context %x\n", context);

    context_map[context->slot] = NULL;

    mutex_lock(&main_device->struct_mutex);
    drm_gem_object_unreference(&context->obj->base);
    mutex_unlock(&main_device->struct_mutex);

    __DestroyObject(context);
};


#define CURRENT_TASK             (0x80003000)

struct context *get_context(struct drm_device *dev)
{
    struct context *context;
    struct io_call_10 io_10;
    int    slot = *((u8*)CURRENT_TASK);
    int    ret;

    context = context_map[slot];

    if( context != NULL)
        return context;

    context = CreateObject(GetPid(), sizeof(*context));

    if( context != NULL)
    {
        drm_i915_private_t *dev_priv = dev->dev_private;
        struct drm_i915_gem_object *obj;

        obj = i915_gem_alloc_object(dev, 4096);
        i915_gem_object_pin(obj, 4096, true, true);

        context->obj = obj;
        context->cmd_buffer = MapIoMem((addr_t)obj->pages.page[0], 4096, PG_SW|PG_NOCACHE);
        context->cmd_offset = obj->gtt_offset;

        context->header.destroy = destroy_context;
        context->mask  = NULL;
        context->seqno = 0;
        context->slot  = slot;

        context_map[slot] = context;
    };
    return context;
};

