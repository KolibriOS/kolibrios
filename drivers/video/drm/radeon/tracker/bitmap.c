
#include <drmP.h>
#include <drm.h>
#include "radeon_drm.h"
#include "../radeon.h"
#include "../display.h"

extern struct radeon_device *main_device;

typedef struct
{
    kobj_t   header;

    int       width;
    int       height;
    int       stride;
    uint64_t  gaddr;
    void     *uaddr;
    struct radeon_bo  *robj;
}bitmap_t;

int create_bitmap(bitmap_t **pbitmap, int width, int height)
{
    size_t size;
    size_t pitch;
    bitmap_t  *bitmap;
    uint64_t  gaddr;
    void     *uaddr;

    struct radeon_device *rdev = main_device;
    struct radeon_bo     *sobj = NULL;

    int   r;

    bitmap = CreateObject(GetPid(), sizeof(bitmap_t));
    if( bitmap == NULL)
    {
        *pbitmap = NULL;
        return -1;
    }

    pitch = radeon_align_pitch(rdev, width, 32, false) * 4;

    size = pitch * height;

    r = radeon_bo_create(rdev, size, PAGE_SIZE, true,
                         RADEON_GEM_DOMAIN_GTT, &sobj);
    if (r) {
        goto fail;
    }
    r = radeon_bo_reserve(sobj, false);
    if (unlikely(r != 0))
        goto fail;
    r = radeon_bo_pin(sobj, RADEON_GEM_DOMAIN_GTT, &gaddr);
    if (r) {
        goto fail;
    }

    r = radeon_bo_user_map(sobj, &uaddr);
    if (r) {
        goto fail;
    }

    bitmap->width  = width;
    bitmap->height = height;
    bitmap->stride = pitch;
    bitmap->gaddr  = gaddr;
    bitmap->uaddr  = uaddr;
    bitmap->robj   = sobj;

    *pbitmap       = bitmap;
    return 0;

fail:

    DestroyObject(bitmap);
    return -1;

};
