/**************************************************************************
 *
 * Copyright (c) 2006-2009 VMware, Inc., Palo Alto, CA., USA
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/
/*
 * Authors: Thomas Hellstrom <thellstrom-at-vmware-dot-com>
 */
/* Notes:
 *
 * We store bo pointer in drm_mm_node struct so we know which bo own a
 * specific node. There is no protection on the pointer, thus to make
 * sure things don't go berserk you have to access this pointer while
 * holding the global lru lock and make sure anytime you free a node you
 * reset the pointer to NULL.
 */

#include "ttm/ttm_module.h"
#include "ttm/ttm_bo_driver.h"
#include "ttm/ttm_placement.h"
#include <linux/module.h>



int ttm_bo_init_mm(struct ttm_bo_device *bdev, unsigned type,
            unsigned long p_size)
{
    int ret = -EINVAL;
    struct ttm_mem_type_manager *man;

    if (type >= TTM_NUM_MEM_TYPES) {
        printk(KERN_ERR TTM_PFX "Illegal memory type %d\n", type);
        return ret;
    }

    man = &bdev->man[type];
    if (man->has_type) {
        printk(KERN_ERR TTM_PFX
               "Memory manager already initialized for type %d\n",
               type);
        return ret;
    }

    ret = bdev->driver->init_mem_type(bdev, type, man);
    if (ret)
        return ret;

    ret = 0;
    if (type != TTM_PL_SYSTEM) {
        if (!p_size) {
            printk(KERN_ERR TTM_PFX
                   "Zero size memory manager type %d\n",
                   type);
            return ret;
        }
        ret = drm_mm_init(&man->manager, 0, p_size);
        if (ret)
            return ret;
    }
    man->has_type = true;
    man->use_type = true;
    man->size = p_size;

    INIT_LIST_HEAD(&man->lru);

    return 0;
}
EXPORT_SYMBOL(ttm_bo_init_mm);

int ttm_bo_global_init(struct ttm_global_reference *ref)
{
    struct ttm_bo_global_ref *bo_ref =
        container_of(ref, struct ttm_bo_global_ref, ref);
    struct ttm_bo_global *glob = ref->object;
    int ret;

//    mutex_init(&glob->device_list_mutex);
//    spin_lock_init(&glob->lru_lock);
    glob->mem_glob = bo_ref->mem_glob;
//    glob->dummy_read_page = alloc_page(__GFP_ZERO | GFP_DMA32);

    if (unlikely(glob->dummy_read_page == NULL)) {
        ret = -ENOMEM;
        goto out_no_drp;
    }

    INIT_LIST_HEAD(&glob->swap_lru);
    INIT_LIST_HEAD(&glob->device_list);

//    ttm_mem_init_shrink(&glob->shrink, ttm_bo_swapout);
    ret = ttm_mem_register_shrink(glob->mem_glob, &glob->shrink);
    if (unlikely(ret != 0)) {
        printk(KERN_ERR TTM_PFX
               "Could not register buffer object swapout.\n");
        goto out_no_shrink;
    }

    glob->ttm_bo_extra_size =
        ttm_round_pot(sizeof(struct ttm_tt)) +
        ttm_round_pot(sizeof(struct ttm_backend));

    glob->ttm_bo_size = glob->ttm_bo_extra_size +
        ttm_round_pot(sizeof(struct ttm_buffer_object));

    atomic_set(&glob->bo_count, 0);

//    kobject_init(&glob->kobj, &ttm_bo_glob_kobj_type);
//    ret = kobject_add(&glob->kobj, ttm_get_kobj(), "buffer_objects");
//    if (unlikely(ret != 0))
//        kobject_put(&glob->kobj);
    return ret;
out_no_shrink:
    __free_page(glob->dummy_read_page);
out_no_drp:
    kfree(glob);
    return ret;
}
EXPORT_SYMBOL(ttm_bo_global_init);


