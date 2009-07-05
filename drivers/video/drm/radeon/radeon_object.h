/*
 * Copyright 2008 Advanced Micro Devices, Inc.
 * Copyright 2008 Red Hat Inc.
 * Copyright 2009 Jerome Glisse.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Dave Airlie
 *          Alex Deucher
 *          Jerome Glisse
 */
#ifndef __RADEON_OBJECT_H__
#define __RADEON_OBJECT_H__

//#include <ttm/ttm_bo_api.h>
//#include <ttm/ttm_bo_driver.h>
//#include <ttm/ttm_placement.h>
//#include <ttm/ttm_module.h>

/*
 * TTM.
 */
//struct radeon_mman {
//   struct ttm_global_reference mem_global_ref;
//   bool                mem_global_referenced;
//   struct ttm_bo_device        bdev;
//};


#define TTM_PL_SYSTEM           0
#define TTM_PL_TT               1
#define TTM_PL_VRAM             2
#define TTM_PL_PRIV0            3
#define TTM_PL_PRIV1            4
#define TTM_PL_PRIV2            5
#define TTM_PL_PRIV3            6
#define TTM_PL_PRIV4            7
#define TTM_PL_PRIV5            8
#define TTM_PL_SWAPPED          15

#define TTM_PL_FLAG_SYSTEM      (1 << TTM_PL_SYSTEM)
#define TTM_PL_FLAG_TT          (1 << TTM_PL_TT)
#define TTM_PL_FLAG_VRAM        (1 << TTM_PL_VRAM)
#define TTM_PL_FLAG_PRIV0       (1 << TTM_PL_PRIV0)
#define TTM_PL_FLAG_PRIV1       (1 << TTM_PL_PRIV1)
#define TTM_PL_FLAG_PRIV2       (1 << TTM_PL_PRIV2)
#define TTM_PL_FLAG_PRIV3       (1 << TTM_PL_PRIV3)
#define TTM_PL_FLAG_PRIV4       (1 << TTM_PL_PRIV4)
#define TTM_PL_FLAG_PRIV5       (1 << TTM_PL_PRIV5)
#define TTM_PL_FLAG_SWAPPED     (1 << TTM_PL_SWAPPED)
#define TTM_PL_MASK_MEM         0x0000FFFF


struct ttm_mem_type_manager {

    /*
     * No protection. Constant from start.
     */

    bool            has_type;
    bool            use_type;
    uint32_t        flags;
    unsigned long   gpu_offset;
    unsigned long   io_offset;
    unsigned long   io_size;
    void            *io_addr;
    uint64_t        size;
    uint32_t        available_caching;
    uint32_t        default_caching;

    /*
     * Protected by the bdev->lru_lock.
     * TODO: Consider one lru_lock per ttm_mem_type_manager.
     * Plays ill with list removal, though.
     */

    struct drm_mm manager;
    struct list_head lru;
};

struct ttm_bo_driver {
    const uint32_t      *mem_type_prio;
    const uint32_t      *mem_busy_prio;
    uint32_t             num_mem_type_prio;
    uint32_t             num_mem_busy_prio;

    /**
     * struct ttm_bo_driver member create_ttm_backend_entry
     *
     * @bdev: The buffer object device.
     *
     * Create a driver specific struct ttm_backend.
     */

//    struct ttm_backend *(*create_ttm_backend_entry)(struct ttm_bo_device *bdev);

    /**
     * struct ttm_bo_driver member invalidate_caches
     *
     * @bdev: the buffer object device.
     * @flags: new placement of the rebound buffer object.
     *
     * A previosly evicted buffer has been rebound in a
     * potentially new location. Tell the driver that it might
     * consider invalidating read (texture) caches on the next command
     * submission as a consequence.
     */

//    int (*invalidate_caches) (struct ttm_bo_device *bdev, uint32_t flags);
//    int (*init_mem_type) (struct ttm_bo_device *bdev, uint32_t type,
//                  struct ttm_mem_type_manager *man);
    /**
     * struct ttm_bo_driver member evict_flags:
     *
     * @bo: the buffer object to be evicted
     *
     * Return the bo flags for a buffer which is not mapped to the hardware.
     * These will be placed in proposed_flags so that when the move is
     * finished, they'll end up in bo->mem.flags
     */

//     uint32_t(*evict_flags) (struct ttm_buffer_object *bo);
    /**
     * struct ttm_bo_driver member move:
     *
     * @bo: the buffer to move
     * @evict: whether this motion is evicting the buffer from
     * the graphics address space
     * @interruptible: Use interruptible sleeps if possible when sleeping.
     * @no_wait: whether this should give up and return -EBUSY
     * if this move would require sleeping
     * @new_mem: the new memory region receiving the buffer
     *
     * Move a buffer between two memory regions.
     */
//    int (*move) (struct ttm_buffer_object *bo,
//             bool evict, bool interruptible,
//             bool no_wait, struct ttm_mem_reg *new_mem);

    /**
     * struct ttm_bo_driver_member verify_access
     *
     * @bo: Pointer to a buffer object.
     * @filp: Pointer to a struct file trying to access the object.
     *
     * Called from the map / write / read methods to verify that the
     * caller is permitted to access the buffer object.
     * This member may be set to NULL, which will refuse this kind of
     * access for all buffer objects.
     * This function should return 0 if access is granted, -EPERM otherwise.
     */
//    int (*verify_access) (struct ttm_buffer_object *bo,
//                  struct file *filp);

    /**
     * In case a driver writer dislikes the TTM fence objects,
     * the driver writer can replace those with sync objects of
     * his / her own. If it turns out that no driver writer is
     * using these. I suggest we remove these hooks and plug in
     * fences directly. The bo driver needs the following functionality:
     * See the corresponding functions in the fence object API
     * documentation.
     */

//    bool (*sync_obj_signaled) (void *sync_obj, void *sync_arg);
//    int (*sync_obj_wait) (void *sync_obj, void *sync_arg,
//                  bool lazy, bool interruptible);
//    int (*sync_obj_flush) (void *sync_obj, void *sync_arg);
//    void (*sync_obj_unref) (void **sync_obj);
//    void *(*sync_obj_ref) (void *sync_obj);
};

#define TTM_NUM_MEM_TYPES 8


struct ttm_bo_device {

    /*
     * Constant after bo device init / atomic.
     */

//    struct ttm_mem_global *mem_glob;
    struct ttm_bo_driver *driver;
//    struct page *dummy_read_page;
//    struct ttm_mem_shrink shrink;

    size_t      ttm_bo_extra_size;
    size_t      ttm_bo_size;

//   rwlock_t vm_lock;
    /*
     * Protected by the vm lock.
     */
    struct ttm_mem_type_manager man[TTM_NUM_MEM_TYPES];
//   struct rb_root addr_space_rb;
    struct drm_mm       addr_space_mm;

    /*
     * Might want to change this to one lock per manager.
     */
//   spinlock_t lru_lock;
    /*
     * Protected by the lru lock.
     */
    struct list_head ddestroy;
    struct list_head swap_lru;

    /*
     * Protected by load / firstopen / lastclose /unload sync.
     */

    bool nice_mode;
//   struct address_space *dev_mapping;

    /*
     * Internal protection.
     */

//   struct delayed_work wq;
};

struct ttm_mem_reg {
    struct drm_mm_node *mm_node;
    unsigned long       size;
    unsigned long       num_pages;
    uint32_t            page_alignment;
    uint32_t            mem_type;
    uint32_t            placement;
};

enum ttm_bo_type {
    ttm_bo_type_device,
    ttm_bo_type_user,
    ttm_bo_type_kernel
};

struct ttm_buffer_object {
    /**
     * Members constant at init.
     */

    struct ttm_bo_device   *bdev;
    unsigned long           buffer_start;
    enum ttm_bo_type        type;
    void (*destroy) (struct ttm_buffer_object *);
    unsigned long           num_pages;
    uint64_t                addr_space_offset;
    size_t                  acc_size;

    /**
    * Members not needing protection.
    */

//    struct kref kref;
//    struct kref list_kref;
//    wait_queue_head_t event_queue;
//    spinlock_t lock;

    /**
     * Members protected by the bo::reserved lock.
     */

    uint32_t                proposed_placement;
    struct ttm_mem_reg      mem;
//    struct file *persistant_swap_storage;
//    struct ttm_tt *ttm;
    bool evicted;

    /**
     * Members protected by the bo::reserved lock only when written to.
     */

//    atomic_t cpu_writers;

    /**
     * Members protected by the bdev::lru_lock.
     */

    struct list_head lru;
    struct list_head ddestroy;
    struct list_head swap;
    uint32_t val_seq;
    bool seq_valid;

    /**
     * Members protected by the bdev::lru_lock
     * only when written to.
     */

//    atomic_t reserved;


    /**
     * Members protected by the bo::lock
     */

    void *sync_obj_arg;
    void *sync_obj;
    unsigned long priv_flags;

    /**
     * Members protected by the bdev::vm_lock
     */

//    struct rb_node vm_rb;
    struct drm_mm_node *vm_node;


    /**
     * Special members that are protected by the reserve lock
     * and the bo::lock when written to. Can be read with
     * either of these locks held.
     */

    unsigned long offset;
    uint32_t cur_placement;
};

struct radeon_object
{
    struct ttm_buffer_object     tobj;
    struct list_head            list;
    struct radeon_device        *rdev;
    struct drm_gem_object       *gobj;
//   struct ttm_bo_kmap_obj      kmap;

    unsigned            pin_count;
    uint64_t            gpu_addr;
    void                *kptr;
    bool                is_iomem;

    struct drm_mm_node  *mm_node;
    u32_t                vm_addr;
    u32_t                cpu_addr;
    u32_t                flags;
};


#endif
