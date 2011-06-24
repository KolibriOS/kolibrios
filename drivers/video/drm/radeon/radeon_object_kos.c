
#include <linux/list.h>
#include <drm/drmP.h>
#include "radeon_drm.h"
#include "radeon.h"


static struct drm_mm   mm_gtt;
static struct drm_mm   mm_vram;

int drm_mm_alloc(struct drm_mm *mm, size_t num_pages,
                 struct drm_mm_node **node)
{
    struct drm_mm_node *vm_node;
    int    r;

retry_pre_get:

    r = drm_mm_pre_get(mm);

    if (unlikely(r != 0))
       return r;

    vm_node = drm_mm_search_free(mm, num_pages, 0, 0);

    if (unlikely(vm_node == NULL)) {
        r = -ENOMEM;
        return r;
    }

    *node =  drm_mm_get_block_atomic(vm_node, num_pages, 0);

    if (unlikely(*node == NULL)) {
            goto retry_pre_get;
    }

    return 0;
};


void radeon_ttm_placement_from_domain(struct radeon_bo *rbo, u32 domain)
{
    u32 c = 0;

    rbo->placement.fpfn = 0;
    rbo->placement.lpfn = 0;
    rbo->placement.placement = rbo->placements;
    rbo->placement.busy_placement = rbo->placements;
    if (domain & RADEON_GEM_DOMAIN_VRAM)
        rbo->placements[c++] = TTM_PL_FLAG_WC | TTM_PL_FLAG_UNCACHED |
                    TTM_PL_FLAG_VRAM;
    if (domain & RADEON_GEM_DOMAIN_GTT)
        rbo->placements[c++] = TTM_PL_MASK_CACHING | TTM_PL_FLAG_TT;
    if (domain & RADEON_GEM_DOMAIN_CPU)
        rbo->placements[c++] = TTM_PL_MASK_CACHING | TTM_PL_FLAG_SYSTEM;
    if (!c)
        rbo->placements[c++] = TTM_PL_MASK_CACHING | TTM_PL_FLAG_SYSTEM;
    rbo->placement.num_placement = c;
    rbo->placement.num_busy_placement = c;
}


int radeon_bo_init(struct radeon_device *rdev)
{
    int r;

    DRM_INFO("Detected VRAM RAM=%lluM, BAR=%lluM\n",
        rdev->mc.mc_vram_size >> 20,
        (unsigned long long)rdev->mc.aper_size >> 20);
    DRM_INFO("RAM width %dbits %cDR\n",
            rdev->mc.vram_width, rdev->mc.vram_is_ddr ? 'D' : 'S');

    r = drm_mm_init(&mm_vram, 0xC00000 >> PAGE_SHIFT,
               ((rdev->mc.real_vram_size - 0xC00000) >> PAGE_SHIFT));
    if (r) {
        DRM_ERROR("Failed initializing VRAM heap.\n");
        return r;
    };

    r = drm_mm_init(&mm_gtt, 0, rdev->mc.gtt_size >> PAGE_SHIFT);
    if (r) {
        DRM_ERROR("Failed initializing GTT heap.\n");
        return r;
    }

    return 0;
}


int radeon_bo_reserve(struct radeon_bo *bo, bool no_wait)
{
    int r;

    bo->tbo.reserved.counter = 1;

    return 0;
}

void ttm_bo_unreserve(struct ttm_buffer_object *bo)
{
    bo->reserved.counter = 1;
}

int radeon_bo_create(struct radeon_device *rdev,
                unsigned long size, int byte_align,
                bool kernel, u32 domain,
                struct radeon_bo **bo_ptr)
{
    enum ttm_bo_type type;

    struct radeon_bo   *bo;
    size_t num_pages;
    struct drm_mm      *mman;
    u32                 bo_domain;
    int r;

    num_pages = (size + PAGE_SIZE - 1) >> PAGE_SHIFT;

    if (num_pages == 0) {
        dbgprintf("Illegal buffer object size.\n");
        return -EINVAL;
    }

    if(domain & RADEON_GEM_DOMAIN_VRAM)
    {
        mman = &mm_vram;
        bo_domain = RADEON_GEM_DOMAIN_VRAM;
    }
    else if(domain & RADEON_GEM_DOMAIN_GTT)
    {
        mman = &mm_gtt;
        bo_domain = RADEON_GEM_DOMAIN_GTT;
    }
    else return -EINVAL;

    if (kernel) {
        type = ttm_bo_type_kernel;
    } else {
        type = ttm_bo_type_device;
    }
    *bo_ptr = NULL;
    bo = kzalloc(sizeof(struct radeon_bo), GFP_KERNEL);
    if (bo == NULL)
        return -ENOMEM;

    bo->rdev = rdev;
    bo->surface_reg = -1;
    bo->tbo.num_pages = num_pages;
    bo->domain = domain;

    INIT_LIST_HEAD(&bo->list);

//    radeon_ttm_placement_from_domain(bo, domain);
    /* Kernel allocation are uninterruptible */

    r = drm_mm_alloc(mman, num_pages, &bo->tbo.vm_node);
    if (unlikely(r != 0))
        return r;

    *bo_ptr = bo;

    return 0;
}

#define page_tabs  0xFDC00000      /* just another hack */

int radeon_bo_pin(struct radeon_bo *bo, u32 domain, u64 *gpu_addr)
{
    int r=0, i;

    if (bo->pin_count) {
        bo->pin_count++;
        if (gpu_addr)
            *gpu_addr = radeon_bo_gpu_offset(bo);
        return 0;
    }

    bo->tbo.offset = bo->tbo.vm_node->start << PAGE_SHIFT;

    if(bo->domain & RADEON_GEM_DOMAIN_VRAM)
    {
        bo->tbo.offset += (u64)bo->rdev->mc.vram_start;
    }
    else if (bo->domain & RADEON_GEM_DOMAIN_GTT)
    {
        u32_t *pagelist;
        bo->kptr  = KernelAlloc( bo->tbo.num_pages << PAGE_SHIFT );
        dbgprintf("kernel alloc %x\n", bo->kptr );

        pagelist =  &((u32_t*)page_tabs)[(u32_t)bo->kptr >> 12];
        dbgprintf("pagelist %x\n", pagelist);
        radeon_gart_bind(bo->rdev, bo->tbo.offset,
                         bo->tbo.vm_node->size,  pagelist);
        bo->tbo.offset += (u64)bo->rdev->mc.gtt_start;
    }
    else
    {
        DRM_ERROR("Unknown placement %x\n", bo->domain);
        bo->tbo.offset = -1;
        r = -1;
    };

    if (unlikely(r != 0)) {
        DRM_ERROR("radeon: failed to pin object.\n");
    }

    if (likely(r == 0)) {
        bo->pin_count = 1;
        if (gpu_addr != NULL)
            *gpu_addr = radeon_bo_gpu_offset(bo);
    }

    if (unlikely(r != 0))
        dev_err(bo->rdev->dev, "%p pin failed\n", bo);
    return r;
};

int radeon_bo_unpin(struct radeon_bo *bo)
{
    int r = 0;

    if (!bo->pin_count) {
        dev_warn(bo->rdev->dev, "%p unpin not necessary\n", bo);
        return 0;
    }
    bo->pin_count--;
    if (bo->pin_count)
        return 0;

    if( bo->tbo.vm_node )
    {
        drm_mm_put_block(bo->tbo.vm_node);
        bo->tbo.vm_node = NULL;
    };

    return r;
}

int radeon_bo_kmap(struct radeon_bo *bo, void **ptr)
{
    bool is_iomem;

    if (bo->kptr) {
        if (ptr) {
            *ptr = bo->kptr;
        }
        return 0;
    }

    if(bo->domain & RADEON_GEM_DOMAIN_VRAM)
    {
        bo->cpu_addr = bo->rdev->mc.aper_base +
                       (bo->tbo.vm_node->start << PAGE_SHIFT);
        bo->kptr = (void*)MapIoMem(bo->cpu_addr,
                        bo->tbo.vm_node->size << 12, PG_SW);
    }
    else
    {
        return -1;
    }

    if (ptr) {
        *ptr = bo->kptr;
    }

    return 0;
}

void radeon_bo_kunmap(struct radeon_bo *bo)
{
    if (bo->kptr == NULL)
        return;

    if (bo->domain & RADEON_GEM_DOMAIN_VRAM)
    {
        FreeKernelSpace(bo->kptr);
    }

    bo->kptr = NULL;

}

void radeon_bo_unref(struct radeon_bo **bo)
{
    struct ttm_buffer_object *tbo;

    if ((*bo) == NULL)
        return;

    *bo = NULL;
}


void radeon_bo_get_tiling_flags(struct radeon_bo *bo,
                uint32_t *tiling_flags,
                uint32_t *pitch)
{
//    BUG_ON(!atomic_read(&bo->tbo.reserved));
    if (tiling_flags)
        *tiling_flags = bo->tiling_flags;
    if (pitch)
        *pitch = bo->pitch;
}


/**
 * Allocate a GEM object of the specified size with shmfs backing store
 */
struct drm_gem_object *
drm_gem_object_alloc(struct drm_device *dev, size_t size)
{
    struct drm_gem_object *obj;

    BUG_ON((size & (PAGE_SIZE - 1)) != 0);

    obj = kzalloc(sizeof(*obj), GFP_KERNEL);

    obj->dev = dev;
    obj->size = size;
    return obj;
}


int radeon_fb_bo_create(struct radeon_device *rdev, struct drm_gem_object *gobj,
            unsigned long size, bool kernel, u32 domain,
            struct radeon_bo **bo_ptr)
{
    enum ttm_bo_type    type;

    struct radeon_bo    *bo;
    struct drm_mm       *mman;
    struct drm_mm_node  *vm_node;

    size_t  num_pages;
    u32     bo_domain;
    int     r;

    num_pages = (size + PAGE_SIZE - 1) >> PAGE_SHIFT;

    if (num_pages == 0) {
        dbgprintf("Illegal buffer object size.\n");
        return -EINVAL;
    }

    if( (domain & RADEON_GEM_DOMAIN_VRAM) !=
        RADEON_GEM_DOMAIN_VRAM )
    {
        return -EINVAL;
    };

    if (kernel) {
        type = ttm_bo_type_kernel;
    } else {
        type = ttm_bo_type_device;
    }
    *bo_ptr = NULL;
    bo = kzalloc(sizeof(struct radeon_bo), GFP_KERNEL);
    if (bo == NULL)
        return -ENOMEM;

    bo->rdev = rdev;
//    bo->gobj = gobj;
    bo->surface_reg = -1;
    bo->tbo.num_pages = num_pages;
    bo->domain = domain;

    INIT_LIST_HEAD(&bo->list);

//    radeon_ttm_placement_from_domain(bo, domain);
    /* Kernel allocation are uninterruptible */

    vm_node = kzalloc(sizeof(*vm_node),0);

    vm_node->size = 0xC00000 >> 12;
    vm_node->start = 0;
    vm_node->mm = NULL;

    bo->tbo.vm_node = vm_node;
    bo->tbo.offset  = bo->tbo.vm_node->start << PAGE_SHIFT;
    bo->tbo.offset += (u64)bo->rdev->mc.vram_start;
    bo->kptr        = (void*)0xFE000000;
    bo->pin_count   = 1;

    *bo_ptr = bo;

    return 0;
}
