
#include <drmP.h>
#include <drm.h>
#include "i915_drm.h"
#include "i915_drv.h"
#include "intel_drv.h"
#include "bitmap.h"

void __attribute__((regparm(1))) destroy_bitmap(bitmap_t *bitmap)
{
/*
 *
 *
 *
*/
    __DestroyObject(bitmap);
};

extern struct drm_device *main_device;

struct hman bm_man;

int init_bitmaps()
{
    int ret;

    ret = init_hman(&bm_man, 1024);

    return ret;
};


int create_bitmap(struct ubitmap *pbitmap)
{
    struct drm_i915_gem_object *obj;

    bitmap_t   *bitmap;
    u32         handle;
    u32         width;
    u32         height;
    u32         size;
    u32         pitch;
    void       *uaddr;

    int   ret;

    pbitmap->handle = 0;
    pbitmap->data   = NULL;

    width = pbitmap->width;
    height = pbitmap->height;

    if((width==0)||(height==0)||(width>4096)||(height>4096))
        goto err1;

    handle = alloc_handle(&bm_man);
//    printf("%s %d\n",__FUNCTION__, handle);

    if(handle == 0)
        goto err1;

    bitmap = CreateObject(GetPid(), sizeof(*bitmap));
    bitmap->handle = handle;
    bitmap->header.destroy = destroy_bitmap;
    bitmap->obj    = NULL;

//    printf("bitmap %x\n", bitmap);
    if( bitmap == NULL)
        goto err1;

    hman_set_data(&bm_man, handle, bitmap);

    pitch = ALIGN(width*4,64);

    size =  roundup(pitch*height, PAGE_SIZE);
//    printf("pitch %d size %d\n", pitch, size);

    obj = i915_gem_alloc_object(main_device, size);
    if (obj == NULL)
        goto err2;

    ret = i915_gem_object_pin(obj, 4096, true);
    if (ret)
        goto err3;

    uaddr = UserAlloc(size);
    if( uaddr == NULL)
        goto err4;
    else
    {
        u32_t *src, *dst;
        int count;

#define page_tabs  0xFDC00000      /* really dirty hack */

        src =  (u32_t*)obj->pages;
        dst =  &((u32_t*)page_tabs)[(u32_t)uaddr >> 12];
        count = size/4096;

        while(count--)
        {
            *dst++ = (0xFFFFF000 & *src++) | 0x207 ; // map as shared page
        };
    }

    bitmap->handle = handle;
    bitmap->width  = width;
    bitmap->height = height;
    bitmap->pitch  = pitch;
    bitmap->gaddr  = obj->gtt_offset;
    bitmap->uaddr  = uaddr;
    bitmap->obj    = obj;
    bitmap->header.destroy = destroy_bitmap;

    pbitmap->pitch  = pitch;
    pbitmap->handle = handle;
    pbitmap->data   = uaddr;

//    printf("%s handle %d pitch %d gpu %x user %x\n",
//            __FUNCTION__, handle, pitch, obj->gtt_offset, uaddr);

    return 0;

err4:
//    drm_gem_object_unpin;
err3:
//    drm_gem_object_unreference(&obj->base);
err2:
    free_handle(&bm_man, handle);
    __DestroyObject(bitmap);
err1:
    return -1;

};


int init_hman(struct hman *man, u32 count)
{
    u32* data;

    data = malloc(count*sizeof(u32*));
    if(data)
    {
        int i;

        for(i=0;i < count-1;)
            data[i] = ++i;
        data[i] = 0;

        man->table = data;
        man->next  = 0;
        man->avail = count;
        man->count = count;

        return 0;
    };
    return -ENOMEM;
};

u32  alloc_handle(struct hman *man)
{
    u32 handle = 0;

    if(man->avail)
    {
        handle = man->next;
        man->next = man->table[handle];
        man->avail--;
        handle++;
    }
    return handle;
};

int free_handle(struct hman *man, u32 handle)
{
    int ret = -1;

    handle--;

    if(handle < man->count)
    {
        man->table[handle] = man->next;
        man->next = handle;
        man->avail++;
        ret = 0;
    };

    return ret;
};


