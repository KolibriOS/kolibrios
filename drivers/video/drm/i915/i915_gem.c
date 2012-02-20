/*
 * Copyright © 2008 Intel Corporation
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
 *
 */

#include "drmP.h"
#include "drm.h"
#include "i915_drm.h"
#include "i915_drv.h"
#include "i915_trace.h"
#include "intel_drv.h"
//#include <linux/shmem_fs.h>
#include <linux/slab.h>
//#include <linux/swap.h>
#include <linux/pci.h>

extern int x86_clflush_size;

#undef mb
#undef rmb
#undef wmb
#define mb() asm volatile("mfence")
#define rmb() asm volatile ("lfence")
#define wmb() asm volatile ("sfence")

static inline void clflush(volatile void *__p)
{
    asm volatile("clflush %0" : "+m" (*(volatile char*)__p));
}

#define MAX_ERRNO       4095

#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)

static inline long IS_ERR(const void *ptr)
{
    return IS_ERR_VALUE((unsigned long)ptr);
}

static inline void *ERR_PTR(long error)
{
    return (void *) error;
}

static inline long PTR_ERR(const void *ptr)
{
    return (long) ptr;
}

void
drm_gem_object_free(struct kref *kref)
{
    struct drm_gem_object *obj = (struct drm_gem_object *) kref;
    struct drm_device *dev = obj->dev;

    BUG_ON(!mutex_is_locked(&dev->struct_mutex));

    i915_gem_free_object(obj);
}

/**
 * Initialize an already allocated GEM object of the specified size with
 * shmfs backing store.
 */
int drm_gem_object_init(struct drm_device *dev,
            struct drm_gem_object *obj, size_t size)
{
    BUG_ON((size & (PAGE_SIZE - 1)) != 0);

    obj->dev = dev;
    kref_init(&obj->refcount);
    atomic_set(&obj->handle_count, 0);
    obj->size = size;

    return 0;
}

void
drm_gem_object_release(struct drm_gem_object *obj)
{ }


#define I915_EXEC_CONSTANTS_MASK        (3<<6)
#define I915_EXEC_CONSTANTS_REL_GENERAL (0<<6) /* default */
#define I915_EXEC_CONSTANTS_ABSOLUTE    (1<<6)
#define I915_EXEC_CONSTANTS_REL_SURFACE (2<<6) /* gen4/5 only */

static __must_check int i915_gem_object_flush_gpu_write_domain(struct drm_i915_gem_object *obj);
static void i915_gem_object_flush_gtt_write_domain(struct drm_i915_gem_object *obj);
static void i915_gem_object_flush_cpu_write_domain(struct drm_i915_gem_object *obj);
static __must_check int i915_gem_object_set_to_cpu_domain(struct drm_i915_gem_object *obj,
							  bool write);
static __must_check int i915_gem_object_set_cpu_read_domain_range(struct drm_i915_gem_object *obj,
								  uint64_t offset,
								  uint64_t size);
static void i915_gem_object_set_to_full_cpu_read_domain(struct drm_i915_gem_object *obj);
static __must_check int i915_gem_object_bind_to_gtt(struct drm_i915_gem_object *obj,
						    unsigned alignment,
						    bool map_and_fenceable);
static void i915_gem_clear_fence_reg(struct drm_device *dev,
				     struct drm_i915_fence_reg *reg);
static int i915_gem_phys_pwrite(struct drm_device *dev,
				struct drm_i915_gem_object *obj,
				struct drm_i915_gem_pwrite *args,
				struct drm_file *file);
static void i915_gem_free_object_tail(struct drm_i915_gem_object *obj);

//static int i915_gem_inactive_shrink(struct shrinker *shrinker,
//                   struct shrink_control *sc);

/* some bookkeeping */
static void i915_gem_info_add_obj(struct drm_i915_private *dev_priv,
				  size_t size)
{
	dev_priv->mm.object_count++;
	dev_priv->mm.object_memory += size;
}

static void i915_gem_info_remove_obj(struct drm_i915_private *dev_priv,
				     size_t size)
{
	dev_priv->mm.object_count--;
	dev_priv->mm.object_memory -= size;
}

#if 0

static int
i915_gem_wait_for_error(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct completion *x = &dev_priv->error_completion;
	unsigned long flags;
	int ret;

	if (!atomic_read(&dev_priv->mm.wedged))
		return 0;

	ret = wait_for_completion_interruptible(x);
	if (ret)
		return ret;

	if (atomic_read(&dev_priv->mm.wedged)) {
		/* GPU is hung, bump the completion count to account for
		 * the token we just consumed so that we never hit zero and
		 * end up waiting upon a subsequent completion event that
		 * will never happen.
		 */
		spin_lock_irqsave(&x->wait.lock, flags);
		x->done++;
		spin_unlock_irqrestore(&x->wait.lock, flags);
	}
	return 0;
}

int i915_mutex_lock_interruptible(struct drm_device *dev)
{
	int ret;

	ret = i915_gem_wait_for_error(dev);
	if (ret)
		return ret;

	ret = mutex_lock_interruptible(&dev->struct_mutex);
	if (ret)
		return ret;

	WARN_ON(i915_verify_lists(dev));
	return 0;
}
#endif

static inline bool
i915_gem_object_is_inactive(struct drm_i915_gem_object *obj)
{
	return obj->gtt_space && !obj->active && obj->pin_count == 0;
}

void i915_gem_do_init(struct drm_device *dev,
		      unsigned long start,
		      unsigned long mappable_end,
		      unsigned long end)
{
	drm_i915_private_t *dev_priv = dev->dev_private;

	drm_mm_init(&dev_priv->mm.gtt_space, start, end - start);

	dev_priv->mm.gtt_start = start;
	dev_priv->mm.gtt_mappable_end = mappable_end;
	dev_priv->mm.gtt_end = end;
	dev_priv->mm.gtt_total = end - start;
	dev_priv->mm.mappable_gtt_total = min(end, mappable_end) - start;

	/* Take over this portion of the GTT */
	intel_gtt_clear_range(start / PAGE_SIZE, (end-start) / PAGE_SIZE);
}

#if 0

int
i915_gem_init_ioctl(struct drm_device *dev, void *data,
		    struct drm_file *file)
{
	struct drm_i915_gem_init *args = data;

	if (args->gtt_start >= args->gtt_end ||
	    (args->gtt_end | args->gtt_start) & (PAGE_SIZE - 1))
		return -EINVAL;

	mutex_lock(&dev->struct_mutex);
	i915_gem_do_init(dev, args->gtt_start, args->gtt_end, args->gtt_end);
	mutex_unlock(&dev->struct_mutex);

	return 0;
}
#endif

int
i915_gem_get_aperture_ioctl(struct drm_device *dev, void *data,
			    struct drm_file *file)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct drm_i915_gem_get_aperture *args = data;
	struct drm_i915_gem_object *obj;
	size_t pinned;


	pinned = 0;
	mutex_lock(&dev->struct_mutex);
	list_for_each_entry(obj, &dev_priv->mm.pinned_list, mm_list)
		pinned += obj->gtt_space->size;
	mutex_unlock(&dev->struct_mutex);

	args->aper_size = dev_priv->mm.gtt_total;
	args->aper_available_size = args->aper_size - pinned;

	return 0;
}

#if 0

int i915_gem_create(struct drm_file *file,
		struct drm_device *dev,
		uint64_t size,
		uint32_t *handle_p)
{
	struct drm_i915_gem_object *obj;
	int ret;
	u32 handle;

	size = roundup(size, PAGE_SIZE);
	if (size == 0)
		return -EINVAL;

	/* Allocate the new object */
	obj = i915_gem_alloc_object(dev, size);
	if (obj == NULL)
		return -ENOMEM;

	ret = drm_gem_handle_create(file, &obj->base, &handle);
	if (ret) {
		drm_gem_object_release(&obj->base);
		i915_gem_info_remove_obj(dev->dev_private, obj->base.size);
		kfree(obj);
		return ret;
	}

	/* drop reference from allocate - handle holds it now */
	drm_gem_object_unreference(&obj->base);
	trace_i915_gem_object_create(obj);

	*handle_p = handle;
	return 0;
}

int
i915_gem_dumb_create(struct drm_file *file,
		     struct drm_device *dev,
		     struct drm_mode_create_dumb *args)
{
	/* have to work out size/pitch and return them */
	args->pitch = ALIGN(args->width * ((args->bpp + 7) / 8), 64);
	args->size = args->pitch * args->height;
	return i915_gem_create(file, dev,
			       args->size, &args->handle);
}

int i915_gem_dumb_destroy(struct drm_file *file,
			  struct drm_device *dev,
			  uint32_t handle)
{
	return drm_gem_handle_delete(file, handle);
}

/**
 * Creates a new mm object and returns a handle to it.
 */
int
i915_gem_create_ioctl(struct drm_device *dev, void *data,
		      struct drm_file *file)
{
	struct drm_i915_gem_create *args = data;
	return i915_gem_create(file, dev,
			       args->size, &args->handle);
}

static int i915_gem_object_needs_bit17_swizzle(struct drm_i915_gem_object *obj)
{
	drm_i915_private_t *dev_priv = obj->base.dev->dev_private;

	return dev_priv->mm.bit_6_swizzle_x == I915_BIT_6_SWIZZLE_9_10_17 &&
		obj->tiling_mode != I915_TILING_NONE;
}

static inline void
slow_shmem_copy(struct page *dst_page,
		int dst_offset,
		struct page *src_page,
		int src_offset,
		int length)
{
	char *dst_vaddr, *src_vaddr;

	dst_vaddr = kmap(dst_page);
	src_vaddr = kmap(src_page);

	memcpy(dst_vaddr + dst_offset, src_vaddr + src_offset, length);

	kunmap(src_page);
	kunmap(dst_page);
}

static inline void
slow_shmem_bit17_copy(struct page *gpu_page,
		      int gpu_offset,
		      struct page *cpu_page,
		      int cpu_offset,
		      int length,
		      int is_read)
{
	char *gpu_vaddr, *cpu_vaddr;

	/* Use the unswizzled path if this page isn't affected. */
	if ((page_to_phys(gpu_page) & (1 << 17)) == 0) {
		if (is_read)
			return slow_shmem_copy(cpu_page, cpu_offset,
					       gpu_page, gpu_offset, length);
		else
			return slow_shmem_copy(gpu_page, gpu_offset,
					       cpu_page, cpu_offset, length);
	}

	gpu_vaddr = kmap(gpu_page);
	cpu_vaddr = kmap(cpu_page);

	/* Copy the data, XORing A6 with A17 (1). The user already knows he's
	 * XORing with the other bits (A9 for Y, A9 and A10 for X)
	 */
	while (length > 0) {
		int cacheline_end = ALIGN(gpu_offset + 1, 64);
		int this_length = min(cacheline_end - gpu_offset, length);
		int swizzled_gpu_offset = gpu_offset ^ 64;

		if (is_read) {
			memcpy(cpu_vaddr + cpu_offset,
			       gpu_vaddr + swizzled_gpu_offset,
			       this_length);
		} else {
			memcpy(gpu_vaddr + swizzled_gpu_offset,
			       cpu_vaddr + cpu_offset,
			       this_length);
		}
		cpu_offset += this_length;
		gpu_offset += this_length;
		length -= this_length;
	}

	kunmap(cpu_page);
	kunmap(gpu_page);
}

/**
 * This is the fast shmem pread path, which attempts to copy_from_user directly
 * from the backing pages of the object to the user's address space.  On a
 * fault, it fails so we can fall back to i915_gem_shmem_pwrite_slow().
 */
static int
i915_gem_shmem_pread_fast(struct drm_device *dev,
			  struct drm_i915_gem_object *obj,
			  struct drm_i915_gem_pread *args,
			  struct drm_file *file)
{
	struct address_space *mapping = obj->base.filp->f_path.dentry->d_inode->i_mapping;
	ssize_t remain;
	loff_t offset;
	char __user *user_data;
	int page_offset, page_length;

	user_data = (char __user *) (uintptr_t) args->data_ptr;
	remain = args->size;

	offset = args->offset;

	while (remain > 0) {
		struct page *page;
		char *vaddr;
		int ret;

		/* Operation in this page
		 *
		 * page_offset = offset within page
		 * page_length = bytes to copy for this page
		 */
		page_offset = offset_in_page(offset);
		page_length = remain;
		if ((page_offset + remain) > PAGE_SIZE)
			page_length = PAGE_SIZE - page_offset;

		page = shmem_read_mapping_page(mapping, offset >> PAGE_SHIFT);
		if (IS_ERR(page))
			return PTR_ERR(page);

		vaddr = kmap_atomic(page);
		ret = __copy_to_user_inatomic(user_data,
					      vaddr + page_offset,
					      page_length);
		kunmap_atomic(vaddr);

		mark_page_accessed(page);
		page_cache_release(page);
		if (ret)
			return -EFAULT;

		remain -= page_length;
		user_data += page_length;
		offset += page_length;
	}

	return 0;
}

/**
 * This is the fallback shmem pread path, which allocates temporary storage
 * in kernel space to copy_to_user into outside of the struct_mutex, so we
 * can copy out of the object's backing pages while holding the struct mutex
 * and not take page faults.
 */
static int
i915_gem_shmem_pread_slow(struct drm_device *dev,
			  struct drm_i915_gem_object *obj,
			  struct drm_i915_gem_pread *args,
			  struct drm_file *file)
{
	struct address_space *mapping = obj->base.filp->f_path.dentry->d_inode->i_mapping;
	struct mm_struct *mm = current->mm;
	struct page **user_pages;
	ssize_t remain;
	loff_t offset, pinned_pages, i;
	loff_t first_data_page, last_data_page, num_pages;
	int shmem_page_offset;
	int data_page_index, data_page_offset;
	int page_length;
	int ret;
	uint64_t data_ptr = args->data_ptr;
	int do_bit17_swizzling;

	remain = args->size;

	/* Pin the user pages containing the data.  We can't fault while
	 * holding the struct mutex, yet we want to hold it while
	 * dereferencing the user data.
	 */
	first_data_page = data_ptr / PAGE_SIZE;
	last_data_page = (data_ptr + args->size - 1) / PAGE_SIZE;
	num_pages = last_data_page - first_data_page + 1;

	user_pages = drm_malloc_ab(num_pages, sizeof(struct page *));
	if (user_pages == NULL)
		return -ENOMEM;

	mutex_unlock(&dev->struct_mutex);
	down_read(&mm->mmap_sem);
	pinned_pages = get_user_pages(current, mm, (uintptr_t)args->data_ptr,
				      num_pages, 1, 0, user_pages, NULL);
	up_read(&mm->mmap_sem);
	mutex_lock(&dev->struct_mutex);
	if (pinned_pages < num_pages) {
		ret = -EFAULT;
		goto out;
	}

	ret = i915_gem_object_set_cpu_read_domain_range(obj,
							args->offset,
							args->size);
	if (ret)
		goto out;

	do_bit17_swizzling = i915_gem_object_needs_bit17_swizzle(obj);

	offset = args->offset;

	while (remain > 0) {
		struct page *page;

		/* Operation in this page
		 *
		 * shmem_page_offset = offset within page in shmem file
		 * data_page_index = page number in get_user_pages return
		 * data_page_offset = offset with data_page_index page.
		 * page_length = bytes to copy for this page
		 */
		shmem_page_offset = offset_in_page(offset);
		data_page_index = data_ptr / PAGE_SIZE - first_data_page;
		data_page_offset = offset_in_page(data_ptr);

		page_length = remain;
		if ((shmem_page_offset + page_length) > PAGE_SIZE)
			page_length = PAGE_SIZE - shmem_page_offset;
		if ((data_page_offset + page_length) > PAGE_SIZE)
			page_length = PAGE_SIZE - data_page_offset;

		page = shmem_read_mapping_page(mapping, offset >> PAGE_SHIFT);
		if (IS_ERR(page)) {
			ret = PTR_ERR(page);
			goto out;
		}

		if (do_bit17_swizzling) {
			slow_shmem_bit17_copy(page,
					      shmem_page_offset,
					      user_pages[data_page_index],
					      data_page_offset,
					      page_length,
					      1);
		} else {
			slow_shmem_copy(user_pages[data_page_index],
					data_page_offset,
					page,
					shmem_page_offset,
					page_length);
		}

		mark_page_accessed(page);
		page_cache_release(page);

		remain -= page_length;
		data_ptr += page_length;
		offset += page_length;
	}

out:
	for (i = 0; i < pinned_pages; i++) {
		SetPageDirty(user_pages[i]);
		mark_page_accessed(user_pages[i]);
		page_cache_release(user_pages[i]);
	}
	drm_free_large(user_pages);

	return ret;
}
#endif









































































static uint32_t
i915_gem_get_gtt_size(struct drm_device *dev, uint32_t size, int tiling_mode)
{
	uint32_t gtt_size;

	if (INTEL_INFO(dev)->gen >= 4 ||
	    tiling_mode == I915_TILING_NONE)
		return size;

	/* Previous chips need a power-of-two fence region when tiling */
	if (INTEL_INFO(dev)->gen == 3)
		gtt_size = 1024*1024;
	else
		gtt_size = 512*1024;

	while (gtt_size < size)
		gtt_size <<= 1;

	return gtt_size;
}

/**
 * i915_gem_get_gtt_alignment - return required GTT alignment for an object
 * @obj: object to check
 *
 * Return the required GTT alignment for an object, taking into account
 * potential fence register mapping.
 */
static uint32_t
i915_gem_get_gtt_alignment(struct drm_device *dev,
			   uint32_t size,
			   int tiling_mode)
{
	/*
	 * Minimum alignment is 4k (GTT page size), but might be greater
	 * if a fence register is needed for the object.
	 */
	if (INTEL_INFO(dev)->gen >= 4 ||
	    tiling_mode == I915_TILING_NONE)
		return 4096;

	/*
	 * Previous chips need to be aligned to the size of the smallest
	 * fence register that can contain the object.
	 */
	return i915_gem_get_gtt_size(dev, size, tiling_mode);
}

/**
 * i915_gem_get_unfenced_gtt_alignment - return required GTT alignment for an
 *					 unfenced object
 * @dev: the device
 * @size: size of the object
 * @tiling_mode: tiling mode of the object
 *
 * Return the required GTT alignment for an object, only taking into account
 * unfenced tiled surface requirements.
 */
uint32_t
i915_gem_get_unfenced_gtt_alignment(struct drm_device *dev,
				    uint32_t size,
				    int tiling_mode)
{
	/*
	 * Minimum alignment is 4k (GTT page size) for sane hw.
	 */
	if (INTEL_INFO(dev)->gen >= 4 || IS_G33(dev) ||
	    tiling_mode == I915_TILING_NONE)
		return 4096;

	/* Previous hardware however needs to be aligned to a power-of-two
	 * tile height. The simplest method for determining this is to reuse
	 * the power-of-tile object size.
	 */
	return i915_gem_get_gtt_size(dev, size, tiling_mode);
}















static int
i915_gem_object_get_pages_gtt(struct drm_i915_gem_object *obj,
			      gfp_t gfpmask)
{
	int page_count, i;
	struct page *page;

	/* Get the list of pages out of our struct file.  They'll be pinned
	 * at this point until we release them.
	 */
	page_count = obj->base.size / PAGE_SIZE;
	BUG_ON(obj->pages != NULL);
    obj->pages = malloc(page_count * sizeof(struct page *));
	if (obj->pages == NULL)
		return -ENOMEM;


	for (i = 0; i < page_count; i++) {
        page = (struct page*)AllocPage(); // oh-oh
		if (IS_ERR(page))
			goto err_pages;

		obj->pages[i] = page;
	}

//   if (obj->tiling_mode != I915_TILING_NONE)
//       i915_gem_object_do_bit_17_swizzle(obj);



	return 0;

err_pages:
    while (i--)
        FreePage((addr_t)obj->pages[i]);

    free(obj->pages);
	obj->pages = NULL;
	return PTR_ERR(page);
}

static void
i915_gem_object_put_pages_gtt(struct drm_i915_gem_object *obj)
{
	int page_count = obj->base.size / PAGE_SIZE;
	int i;

	BUG_ON(obj->madv == __I915_MADV_PURGED);

//   if (obj->tiling_mode != I915_TILING_NONE)
//       i915_gem_object_save_bit_17_swizzle(obj);

	if (obj->madv == I915_MADV_DONTNEED)
		obj->dirty = 0;

	for (i = 0; i < page_count; i++) {
        FreePage((addr_t)obj->pages[i]);
	}
	obj->dirty = 0;

    free(obj->pages);
	obj->pages = NULL;
}

void
i915_gem_object_move_to_active(struct drm_i915_gem_object *obj,
			       struct intel_ring_buffer *ring,
			       u32 seqno)
{
	struct drm_device *dev = obj->base.dev;
	struct drm_i915_private *dev_priv = dev->dev_private;

	BUG_ON(ring == NULL);
	obj->ring = ring;

	/* Add a reference if we're newly entering the active list. */
	if (!obj->active) {
		drm_gem_object_reference(&obj->base);
		obj->active = 1;
	}

	/* Move from whatever list we were on to the tail of execution. */
	list_move_tail(&obj->mm_list, &dev_priv->mm.active_list);
	list_move_tail(&obj->ring_list, &ring->active_list);

	obj->last_rendering_seqno = seqno;
	if (obj->fenced_gpu_access) {
		struct drm_i915_fence_reg *reg;

		BUG_ON(obj->fence_reg == I915_FENCE_REG_NONE);

		obj->last_fenced_seqno = seqno;
		obj->last_fenced_ring = ring;

		reg = &dev_priv->fence_regs[obj->fence_reg];
		list_move_tail(&reg->lru_list, &dev_priv->mm.fence_list);
	}
}

static void
i915_gem_object_move_off_active(struct drm_i915_gem_object *obj)
{
	list_del_init(&obj->ring_list);
	obj->last_rendering_seqno = 0;
}

static void
i915_gem_object_move_to_flushing(struct drm_i915_gem_object *obj)
{
	struct drm_device *dev = obj->base.dev;
	drm_i915_private_t *dev_priv = dev->dev_private;

	BUG_ON(!obj->active);
	list_move_tail(&obj->mm_list, &dev_priv->mm.flushing_list);

	i915_gem_object_move_off_active(obj);
}

static void
i915_gem_object_move_to_inactive(struct drm_i915_gem_object *obj)
{
	struct drm_device *dev = obj->base.dev;
	struct drm_i915_private *dev_priv = dev->dev_private;

	if (obj->pin_count != 0)
		list_move_tail(&obj->mm_list, &dev_priv->mm.pinned_list);
	else
		list_move_tail(&obj->mm_list, &dev_priv->mm.inactive_list);

	BUG_ON(!list_empty(&obj->gpu_write_list));
	BUG_ON(!obj->active);
	obj->ring = NULL;

	i915_gem_object_move_off_active(obj);
	obj->fenced_gpu_access = false;

	obj->active = 0;
	obj->pending_gpu_write = false;
	drm_gem_object_unreference(&obj->base);

	WARN_ON(i915_verify_lists(dev));
}

/* Immediately discard the backing storage */
static void
i915_gem_object_truncate(struct drm_i915_gem_object *obj)
{
	struct inode *inode;

	/* Our goal here is to return as much of the memory as
	 * is possible back to the system as we are called from OOM.
	 * To do this we must instruct the shmfs to drop all of its
	 * backing pages, *now*.
	 */

	obj->madv = __I915_MADV_PURGED;
}

static inline int
i915_gem_object_is_purgeable(struct drm_i915_gem_object *obj)
{
	return obj->madv == I915_MADV_DONTNEED;
}

static void
i915_gem_process_flushing_list(struct intel_ring_buffer *ring,
			       uint32_t flush_domains)
{
	struct drm_i915_gem_object *obj, *next;

	list_for_each_entry_safe(obj, next,
				 &ring->gpu_write_list,
				 gpu_write_list) {
		if (obj->base.write_domain & flush_domains) {
			uint32_t old_write_domain = obj->base.write_domain;

			obj->base.write_domain = 0;
			list_del_init(&obj->gpu_write_list);
			i915_gem_object_move_to_active(obj, ring,
						       i915_gem_next_request_seqno(ring));

			trace_i915_gem_object_change_domain(obj,
							    obj->base.read_domains,
							    old_write_domain);
		}
	}
}

int
i915_add_request(struct intel_ring_buffer *ring,
		 struct drm_file *file,
		 struct drm_i915_gem_request *request)
{
	drm_i915_private_t *dev_priv = ring->dev->dev_private;
	uint32_t seqno;
	int was_empty;
	int ret;

	BUG_ON(request == NULL);

	ret = ring->add_request(ring, &seqno);
	if (ret)
	    return ret;

	trace_i915_gem_request_add(ring, seqno);

	request->seqno = seqno;
	request->ring = ring;
	request->emitted_jiffies = jiffies;
	was_empty = list_empty(&ring->request_list);
	list_add_tail(&request->list, &ring->request_list);


	ring->outstanding_lazy_request = false;

	if (!dev_priv->mm.suspended) {
		if (i915_enable_hangcheck) {
//			mod_timer(&dev_priv->hangcheck_timer,
//				  jiffies +
//				  msecs_to_jiffies(DRM_I915_HANGCHECK_PERIOD));
		}
        if (was_empty)
           queue_delayed_work(dev_priv->wq,
                      &dev_priv->mm.retire_work, HZ);
	}
	return 0;
}






















/**
 * This function clears the request list as sequence numbers are passed.
 */
static void
i915_gem_retire_requests_ring(struct intel_ring_buffer *ring)
{
	uint32_t seqno;
	int i;

	if (list_empty(&ring->request_list))
		return;

	WARN_ON(i915_verify_lists(ring->dev));

	seqno = ring->get_seqno(ring);

	for (i = 0; i < ARRAY_SIZE(ring->sync_seqno); i++)
		if (seqno >= ring->sync_seqno[i])
			ring->sync_seqno[i] = 0;

	while (!list_empty(&ring->request_list)) {
		struct drm_i915_gem_request *request;

		request = list_first_entry(&ring->request_list,
					   struct drm_i915_gem_request,
					   list);

		if (!i915_seqno_passed(seqno, request->seqno))
			break;

		trace_i915_gem_request_retire(ring, request->seqno);

		list_del(&request->list);
		kfree(request);
	}

	/* Move any buffers on the active list that are no longer referenced
	 * by the ringbuffer to the flushing/inactive lists as appropriate.
	 */
	while (!list_empty(&ring->active_list)) {
		struct drm_i915_gem_object *obj;

		obj = list_first_entry(&ring->active_list,
				      struct drm_i915_gem_object,
				      ring_list);

		if (!i915_seqno_passed(seqno, obj->last_rendering_seqno))
			break;

		if (obj->base.write_domain != 0)
			i915_gem_object_move_to_flushing(obj);
		else
			i915_gem_object_move_to_inactive(obj);
	}

	if (unlikely(ring->trace_irq_seqno &&
		     i915_seqno_passed(seqno, ring->trace_irq_seqno))) {
		ring->irq_put(ring);
		ring->trace_irq_seqno = 0;
	}

	WARN_ON(i915_verify_lists(ring->dev));
}

void
i915_gem_retire_requests(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	int i;

	if (!list_empty(&dev_priv->mm.deferred_free_list)) {
	    struct drm_i915_gem_object *obj, *next;

	    /* We must be careful that during unbind() we do not
	     * accidentally infinitely recurse into retire requests.
	     * Currently:
	     *   retire -> free -> unbind -> wait -> retire_ring
	     */
	    list_for_each_entry_safe(obj, next,
				     &dev_priv->mm.deferred_free_list,
				     mm_list)
		    i915_gem_free_object_tail(obj);
	}

	for (i = 0; i < I915_NUM_RINGS; i++)
		i915_gem_retire_requests_ring(&dev_priv->ring[i]);
}

static void
i915_gem_retire_work_handler(struct work_struct *work)
{
	drm_i915_private_t *dev_priv;
	struct drm_device *dev;
	bool idle;
	int i;

//    ENTER();

	dev_priv = container_of(work, drm_i915_private_t,
				mm.retire_work.work);
	dev = dev_priv->dev;

	/* Come back later if the device is busy... */
	if (!mutex_trylock(&dev->struct_mutex)) {
        queue_delayed_work(dev_priv->wq, &dev_priv->mm.retire_work, HZ);
//        LEAVE();
		return;
	}

	i915_gem_retire_requests(dev);

	/* Send a periodic flush down the ring so we don't hold onto GEM
	 * objects indefinitely.
	 */
	idle = true;
	for (i = 0; i < I915_NUM_RINGS; i++) {
		struct intel_ring_buffer *ring = &dev_priv->ring[i];

		if (!list_empty(&ring->gpu_write_list)) {
			struct drm_i915_gem_request *request;
			int ret;

			ret = i915_gem_flush_ring(ring,
						  0, I915_GEM_GPU_DOMAINS);
			request = kzalloc(sizeof(*request), GFP_KERNEL);
			if (ret || request == NULL ||
			    i915_add_request(ring, NULL, request))
			    kfree(request);
		}

		idle &= list_empty(&ring->request_list);
	}

   if (!dev_priv->mm.suspended && !idle)
       queue_delayed_work(dev_priv->wq, &dev_priv->mm.retire_work, HZ);

	mutex_unlock(&dev->struct_mutex);
//    LEAVE();
}

/**
 * Waits for a sequence number to be signaled, and cleans up the
 * request and object lists appropriately for that event.
 */
int
i915_wait_request(struct intel_ring_buffer *ring,
		  uint32_t seqno)
{
	drm_i915_private_t *dev_priv = ring->dev->dev_private;
	u32 ier;
	int ret = 0;

	BUG_ON(seqno == 0);

//   if (atomic_read(&dev_priv->mm.wedged)) {
//       struct completion *x = &dev_priv->error_completion;
//       bool recovery_complete;
//       unsigned long flags;

		/* Give the error handler a chance to run. */
//       spin_lock_irqsave(&x->wait.lock, flags);
//       recovery_complete = x->done > 0;
//       spin_unlock_irqrestore(&x->wait.lock, flags);
//
//       return recovery_complete ? -EIO : -EAGAIN;
//   }

	if (seqno == ring->outstanding_lazy_request) {
		struct drm_i915_gem_request *request;

		request = kzalloc(sizeof(*request), GFP_KERNEL);
		if (request == NULL)
			return -ENOMEM;

		ret = i915_add_request(ring, NULL, request);
		if (ret) {
			kfree(request);
			return ret;
		}

		seqno = request->seqno;
	}

	if (!i915_seqno_passed(ring->get_seqno(ring), seqno)) {
		if (HAS_PCH_SPLIT(ring->dev))
			ier = I915_READ(DEIER) | I915_READ(GTIER);
		else
			ier = I915_READ(IER);
		if (!ier) {
			DRM_ERROR("something (likely vbetool) disabled "
				  "interrupts, re-enabling\n");
//           ring->dev->driver->irq_preinstall(ring->dev);
//           ring->dev->driver->irq_postinstall(ring->dev);
		}

		trace_i915_gem_request_wait_begin(ring, seqno);

		ring->waiting_seqno = seqno;
        if (ring->irq_get(ring)) {
//            printf("enter wait\n");
            wait_event(ring->irq_queue,
                      i915_seqno_passed(ring->get_seqno(ring), seqno)
                      || atomic_read(&dev_priv->mm.wedged));

           ring->irq_put(ring);
        } else if (wait_for_atomic(i915_seqno_passed(ring->get_seqno(ring),
							     seqno) ||
					   atomic_read(&dev_priv->mm.wedged), 3000))
			ret = -EBUSY;
		ring->waiting_seqno = 0;

		trace_i915_gem_request_wait_end(ring, seqno);
	}
	if (atomic_read(&dev_priv->mm.wedged))
		ret = -EAGAIN;

	if (ret && ret != -ERESTARTSYS)
		DRM_ERROR("%s returns %d (awaiting %d at %d, next %d)\n",
			  __func__, ret, seqno, ring->get_seqno(ring),
			  dev_priv->next_seqno);

	/* Directly dispatch request retiring.  While we have the work queue
	 * to handle this, the waiter on a request often wants an associated
	 * buffer to have made it to the inactive list, and we would need
	 * a separate wait queue to handle that.
	 */
	if (ret == 0)
		i915_gem_retire_requests_ring(ring);

	return ret;
}

/**
 * Ensures that all rendering to the object has completed and the object is
 * safe to unbind from the GTT or access from the CPU.
 */
int
i915_gem_object_wait_rendering(struct drm_i915_gem_object *obj)
{
	int ret;

	/* This function only exists to support waiting for existing rendering,
	 * not for emitting required flushes.
	 */
	BUG_ON((obj->base.write_domain & I915_GEM_GPU_DOMAINS) != 0);

	/* If there is rendering queued on the buffer being evicted, wait for
	 * it.
	 */
	if (obj->active) {
       ret = i915_wait_request(obj->ring, obj->last_rendering_seqno);
       if (ret)
           return ret;
	}

	return 0;
}

static void i915_gem_object_finish_gtt(struct drm_i915_gem_object *obj)
{
	u32 old_write_domain, old_read_domains;

	/* Act a barrier for all accesses through the GTT */
	mb();

	/* Force a pagefault for domain tracking on next user access */
//	i915_gem_release_mmap(obj);

	if ((obj->base.read_domains & I915_GEM_DOMAIN_GTT) == 0)
		return;

	old_read_domains = obj->base.read_domains;
	old_write_domain = obj->base.write_domain;

	obj->base.read_domains &= ~I915_GEM_DOMAIN_GTT;
	obj->base.write_domain &= ~I915_GEM_DOMAIN_GTT;

	trace_i915_gem_object_change_domain(obj,
					    old_read_domains,
					    old_write_domain);
}

/**
 * Unbinds an object from the GTT aperture.
 */
int
i915_gem_object_unbind(struct drm_i915_gem_object *obj)
{
	int ret = 0;

	if (obj->gtt_space == NULL)
		return 0;

	if (obj->pin_count != 0) {
		DRM_ERROR("Attempting to unbind pinned buffer\n");
		return -EINVAL;
	}

	ret = i915_gem_object_finish_gpu(obj);
	if (ret == -ERESTARTSYS)
		return ret;
	/* Continue on if we fail due to EIO, the GPU is hung so we
	 * should be safe and we need to cleanup or else we might
	 * cause memory corruption through use-after-free.
	 */

	i915_gem_object_finish_gtt(obj);

	/* Move the object to the CPU domain to ensure that
	 * any possible CPU writes while it's not in the GTT
	 * are flushed when we go to remap it.
	 */
	if (ret == 0)
		ret = i915_gem_object_set_to_cpu_domain(obj, 1);
	if (ret == -ERESTARTSYS)
		return ret;
	if (ret) {
		/* In the event of a disaster, abandon all caches and
		 * hope for the best.
		 */
		i915_gem_clflush_object(obj);
		obj->base.read_domains = obj->base.write_domain = I915_GEM_DOMAIN_CPU;
	}

	/* release the fence reg _after_ flushing */
	ret = i915_gem_object_put_fence(obj);
	if (ret == -ERESTARTSYS)
		return ret;

	trace_i915_gem_object_unbind(obj);

	i915_gem_gtt_unbind_object(obj);
	i915_gem_object_put_pages_gtt(obj);

	list_del_init(&obj->gtt_list);
	list_del_init(&obj->mm_list);
	/* Avoid an unnecessary call to unbind on rebind. */
	obj->map_and_fenceable = true;

	drm_mm_put_block(obj->gtt_space);
	obj->gtt_space = NULL;
	obj->gtt_offset = 0;

	if (i915_gem_object_is_purgeable(obj))
		i915_gem_object_truncate(obj);

	return ret;
}

int
i915_gem_flush_ring(struct intel_ring_buffer *ring,
		    uint32_t invalidate_domains,
		    uint32_t flush_domains)
{
	int ret;

	if (((invalidate_domains | flush_domains) & I915_GEM_GPU_DOMAINS) == 0)
		return 0;

	trace_i915_gem_ring_flush(ring, invalidate_domains, flush_domains);

	ret = ring->flush(ring, invalidate_domains, flush_domains);
	if (ret)
		return ret;

	if (flush_domains & I915_GEM_GPU_DOMAINS)
		i915_gem_process_flushing_list(ring, flush_domains);

	return 0;
}

static int i915_ring_idle(struct intel_ring_buffer *ring)
{
	int ret;

	if (list_empty(&ring->gpu_write_list) && list_empty(&ring->active_list))
		return 0;

	if (!list_empty(&ring->gpu_write_list)) {
		ret = i915_gem_flush_ring(ring,
				    I915_GEM_GPU_DOMAINS, I915_GEM_GPU_DOMAINS);
		if (ret)
			return ret;
	}

	return i915_wait_request(ring, i915_gem_next_request_seqno(ring));
}

int
i915_gpu_idle(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	int ret, i;

	/* Flush everything onto the inactive list. */
	for (i = 0; i < I915_NUM_RINGS; i++) {
		ret = i915_ring_idle(&dev_priv->ring[i]);
		if (ret)
			return ret;
	}

	return 0;
}






















static bool ring_passed_seqno(struct intel_ring_buffer *ring, u32 seqno)
{
	return i915_seqno_passed(ring->get_seqno(ring), seqno);
}

static int
i915_gem_object_flush_fence(struct drm_i915_gem_object *obj,
			    struct intel_ring_buffer *pipelined)
{
	int ret;

	if (obj->fenced_gpu_access) {
		if (obj->base.write_domain & I915_GEM_GPU_DOMAINS) {
			ret = i915_gem_flush_ring(obj->last_fenced_ring,
						  0, obj->base.write_domain);
			if (ret)
				return ret;
		}

		obj->fenced_gpu_access = false;
	}

	if (obj->last_fenced_seqno && pipelined != obj->last_fenced_ring) {
		if (!ring_passed_seqno(obj->last_fenced_ring,
				       obj->last_fenced_seqno)) {
			ret = i915_wait_request(obj->last_fenced_ring,
						obj->last_fenced_seqno);
			if (ret)
				return ret;
		}

		obj->last_fenced_seqno = 0;
		obj->last_fenced_ring = NULL;
	}

	/* Ensure that all CPU reads are completed before installing a fence
	 * and all writes before removing the fence.
	 */
	if (obj->base.read_domains & I915_GEM_DOMAIN_GTT)
		mb();

	return 0;
}

int
i915_gem_object_put_fence(struct drm_i915_gem_object *obj)
{
	int ret;

//   if (obj->tiling_mode)
//       i915_gem_release_mmap(obj);

	ret = i915_gem_object_flush_fence(obj, NULL);
	if (ret)
		return ret;

	if (obj->fence_reg != I915_FENCE_REG_NONE) {
		struct drm_i915_private *dev_priv = obj->base.dev->dev_private;
		i915_gem_clear_fence_reg(obj->base.dev,
					 &dev_priv->fence_regs[obj->fence_reg]);

		obj->fence_reg = I915_FENCE_REG_NONE;
	}

	return 0;
}































/**
 * i915_gem_clear_fence_reg - clear out fence register info
 * @obj: object to clear
 *
 * Zeroes out the fence register itself and clears out the associated
 * data structures in dev_priv and obj.
 */
static void
i915_gem_clear_fence_reg(struct drm_device *dev,
             struct drm_i915_fence_reg *reg)
{
    drm_i915_private_t *dev_priv = dev->dev_private;
    uint32_t fence_reg = reg - dev_priv->fence_regs;

    switch (INTEL_INFO(dev)->gen) {
    case 7:
    case 6:
        I915_WRITE64(FENCE_REG_SANDYBRIDGE_0 + fence_reg*8, 0);
        break;
    case 5:
    case 4:
        I915_WRITE64(FENCE_REG_965_0 + fence_reg*8, 0);
        break;
    case 3:
        if (fence_reg >= 8)
            fence_reg = FENCE_REG_945_8 + (fence_reg - 8) * 4;
        else
    case 2:
            fence_reg = FENCE_REG_830_0 + fence_reg * 4;

        I915_WRITE(fence_reg, 0);
        break;
    }

    list_del_init(&reg->lru_list);
    reg->obj = NULL;
    reg->setup_seqno = 0;
}

/**
 * Finds free space in the GTT aperture and binds the object there.
 */
static int
i915_gem_object_bind_to_gtt(struct drm_i915_gem_object *obj,
			    unsigned alignment,
			    bool map_and_fenceable)
{
	struct drm_device *dev = obj->base.dev;
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_mm_node *free_space;
    gfp_t gfpmask = 0; //__GFP_NORETRY | __GFP_NOWARN;
	u32 size, fence_size, fence_alignment, unfenced_alignment;
	bool mappable, fenceable;
	int ret;

	if (obj->madv != I915_MADV_WILLNEED) {
		DRM_ERROR("Attempting to bind a purgeable object\n");
		return -EINVAL;
	}

	fence_size = i915_gem_get_gtt_size(dev,
					   obj->base.size,
					   obj->tiling_mode);
	fence_alignment = i915_gem_get_gtt_alignment(dev,
						     obj->base.size,
						     obj->tiling_mode);
	unfenced_alignment =
		i915_gem_get_unfenced_gtt_alignment(dev,
						    obj->base.size,
						    obj->tiling_mode);

	if (alignment == 0)
		alignment = map_and_fenceable ? fence_alignment :
						unfenced_alignment;
	if (map_and_fenceable && alignment & (fence_alignment - 1)) {
		DRM_ERROR("Invalid object alignment requested %u\n", alignment);
		return -EINVAL;
	}

	size = map_and_fenceable ? fence_size : obj->base.size;

	/* If the object is bigger than the entire aperture, reject it early
	 * before evicting everything in a vain attempt to find space.
	 */
	if (obj->base.size >
	    (map_and_fenceable ? dev_priv->mm.gtt_mappable_end : dev_priv->mm.gtt_total)) {
		DRM_ERROR("Attempting to bind an object larger than the aperture\n");
		return -E2BIG;
	}

 search_free:
	if (map_and_fenceable)
		free_space =
			drm_mm_search_free_in_range(&dev_priv->mm.gtt_space,
						    size, alignment, 0,
						    dev_priv->mm.gtt_mappable_end,
						    0);
	else
		free_space = drm_mm_search_free(&dev_priv->mm.gtt_space,
						size, alignment, 0);

	if (free_space != NULL) {
		if (map_and_fenceable)
			obj->gtt_space =
				drm_mm_get_block_range_generic(free_space,
							       size, alignment, 0,
							       dev_priv->mm.gtt_mappable_end,
							       0);
		else
			obj->gtt_space =
				drm_mm_get_block(free_space, size, alignment);
	}
	if (obj->gtt_space == NULL) {
		/* If the gtt is empty and we're still having trouble
		 * fitting our object in, we're out of memory.
		 */
        ret = 1; //i915_gem_evict_something(dev, size, alignment,
                 //         map_and_fenceable);
		if (ret)
			return ret;

		goto search_free;
	}

	ret = i915_gem_object_get_pages_gtt(obj, gfpmask);
	if (ret) {
		drm_mm_put_block(obj->gtt_space);
		obj->gtt_space = NULL;
#if 0
		if (ret == -ENOMEM) {
			/* first try to reclaim some memory by clearing the GTT */
			ret = i915_gem_evict_everything(dev, false);
			if (ret) {
				/* now try to shrink everyone else */
				if (gfpmask) {
					gfpmask = 0;
					goto search_free;
				}

				return -ENOMEM;
			}

			goto search_free;
		}
#endif
		return ret;
	}

	ret = i915_gem_gtt_bind_object(obj);
	if (ret) {
        i915_gem_object_put_pages_gtt(obj);
		drm_mm_put_block(obj->gtt_space);
		obj->gtt_space = NULL;

//       if (i915_gem_evict_everything(dev, false))
			return ret;

//       goto search_free;
	}

	list_add_tail(&obj->gtt_list, &dev_priv->mm.gtt_list);
	list_add_tail(&obj->mm_list, &dev_priv->mm.inactive_list);

	/* Assert that the object is not currently in any GPU domain. As it
	 * wasn't in the GTT, there shouldn't be any way it could have been in
	 * a GPU cache
	 */
	BUG_ON(obj->base.read_domains & I915_GEM_GPU_DOMAINS);
	BUG_ON(obj->base.write_domain & I915_GEM_GPU_DOMAINS);

	obj->gtt_offset = obj->gtt_space->start;

	fenceable =
		obj->gtt_space->size == fence_size &&
		(obj->gtt_space->start & (fence_alignment - 1)) == 0;

	mappable =
		obj->gtt_offset + obj->base.size <= dev_priv->mm.gtt_mappable_end;

	obj->map_and_fenceable = mappable && fenceable;

	trace_i915_gem_object_bind(obj, map_and_fenceable);
	return 0;
}

void
i915_gem_clflush_object(struct drm_i915_gem_object *obj)
{
	/* If we don't have a page list set up, then we're not pinned
	 * to GPU, and we can ignore the cache flush because it'll happen
	 * again at bind time.
	 */
	if (obj->pages == NULL)
		return;

	/* If the GPU is snooping the contents of the CPU cache,
	 * we do not need to manually clear the CPU cache lines.  However,
	 * the caches are only snooped when the render cache is
	 * flushed/invalidated.  As we always have to emit invalidations
	 * and flushes when moving into and out of the RENDER domain, correct
	 * snooping behaviour occurs naturally as the result of our domain
	 * tracking.
	 */
	if (obj->cache_level != I915_CACHE_NONE)
		return;

     if(obj->mapped != NULL)
     {
        uint8_t *page_virtual;
        unsigned int i;

        page_virtual = obj->mapped;
        asm volatile("mfence");
        for (i = 0; i < obj->base.size; i += x86_clflush_size)
            clflush(page_virtual + i);
        asm volatile("mfence");
     }
     else
     {
        uint8_t *page_virtual;
        unsigned int i;
        page_virtual = AllocKernelSpace(obj->base.size);
        if(page_virtual != NULL)
        {
            u32_t *src, *dst;
            u32 count;

#define page_tabs  0xFDC00000      /* really dirty hack */

            src =  (u32_t*)obj->pages;
            dst =  &((u32_t*)page_tabs)[(u32_t)page_virtual >> 12];
            count = obj->base.size/4096;

            while(count--)
            {
                *dst++ = (0xFFFFF000 & *src++) | 0x001 ;
            };

            asm volatile("mfence");
            for (i = 0; i < obj->base.size; i += x86_clflush_size)
                clflush(page_virtual + i);
            asm volatile("mfence");
            FreeKernelSpace(page_virtual);
        }
        else
        {
            asm volatile (
            "mfence         \n"
            "wbinvd         \n"                 /* this is really ugly  */
            "mfence");
        }
     }
}

/** Flushes any GPU write domain for the object if it's dirty. */
static int
i915_gem_object_flush_gpu_write_domain(struct drm_i915_gem_object *obj)
{
	if ((obj->base.write_domain & I915_GEM_GPU_DOMAINS) == 0)
		return 0;

	/* Queue the GPU write cache flushing we need. */
	return i915_gem_flush_ring(obj->ring, 0, obj->base.write_domain);
}

/** Flushes the GTT write domain for the object if it's dirty. */
static void
i915_gem_object_flush_gtt_write_domain(struct drm_i915_gem_object *obj)
{
	uint32_t old_write_domain;

	if (obj->base.write_domain != I915_GEM_DOMAIN_GTT)
		return;

	/* No actual flushing is required for the GTT write domain.  Writes
	 * to it immediately go to main memory as far as we know, so there's
	 * no chipset flush.  It also doesn't land in render cache.
	 *
	 * However, we do have to enforce the order so that all writes through
	 * the GTT land before any writes to the device, such as updates to
	 * the GATT itself.
	 */
	wmb();

	old_write_domain = obj->base.write_domain;
	obj->base.write_domain = 0;

	trace_i915_gem_object_change_domain(obj,
					    obj->base.read_domains,
					    old_write_domain);
}

/** Flushes the CPU write domain for the object if it's dirty. */
static void
i915_gem_object_flush_cpu_write_domain(struct drm_i915_gem_object *obj)
{
	uint32_t old_write_domain;

	if (obj->base.write_domain != I915_GEM_DOMAIN_CPU)
		return;

	i915_gem_clflush_object(obj);
	intel_gtt_chipset_flush();
	old_write_domain = obj->base.write_domain;
	obj->base.write_domain = 0;

	trace_i915_gem_object_change_domain(obj,
					    obj->base.read_domains,
					    old_write_domain);
}

/**
 * Moves a single object to the GTT read, and possibly write domain.
 *
 * This function returns when the move is complete, including waiting on
 * flushes to occur.
 */
int
i915_gem_object_set_to_gtt_domain(struct drm_i915_gem_object *obj, bool write)
{
	uint32_t old_write_domain, old_read_domains;
	int ret;

	/* Not valid to be called on unbound objects. */
	if (obj->gtt_space == NULL)
		return -EINVAL;

	if (obj->base.write_domain == I915_GEM_DOMAIN_GTT)
		return 0;

	ret = i915_gem_object_flush_gpu_write_domain(obj);
	if (ret)
		return ret;

	if (obj->pending_gpu_write || write) {
		ret = i915_gem_object_wait_rendering(obj);
		if (ret)
			return ret;
	}

	i915_gem_object_flush_cpu_write_domain(obj);

	old_write_domain = obj->base.write_domain;
	old_read_domains = obj->base.read_domains;

	/* It should now be out of any other write domains, and we can update
	 * the domain values for our changes.
	 */
	BUG_ON((obj->base.write_domain & ~I915_GEM_DOMAIN_GTT) != 0);
	obj->base.read_domains |= I915_GEM_DOMAIN_GTT;
	if (write) {
		obj->base.read_domains = I915_GEM_DOMAIN_GTT;
		obj->base.write_domain = I915_GEM_DOMAIN_GTT;
		obj->dirty = 1;
	}

	trace_i915_gem_object_change_domain(obj,
					    old_read_domains,
					    old_write_domain);

	return 0;
}

int i915_gem_object_set_cache_level(struct drm_i915_gem_object *obj,
				    enum i915_cache_level cache_level)
{
	int ret;

	if (obj->cache_level == cache_level)
		return 0;

	if (obj->pin_count) {
		DRM_DEBUG("can not change the cache level of pinned objects\n");
		return -EBUSY;
	}

	if (obj->gtt_space) {
		ret = i915_gem_object_finish_gpu(obj);
		if (ret)
			return ret;

		i915_gem_object_finish_gtt(obj);

		/* Before SandyBridge, you could not use tiling or fence
		 * registers with snooped memory, so relinquish any fences
		 * currently pointing to our region in the aperture.
		 */
		if (INTEL_INFO(obj->base.dev)->gen < 6) {
			ret = i915_gem_object_put_fence(obj);
			if (ret)
				return ret;
		}

		i915_gem_gtt_rebind_object(obj, cache_level);
	}

	if (cache_level == I915_CACHE_NONE) {
		u32 old_read_domains, old_write_domain;

		/* If we're coming from LLC cached, then we haven't
		 * actually been tracking whether the data is in the
		 * CPU cache or not, since we only allow one bit set
		 * in obj->write_domain and have been skipping the clflushes.
		 * Just set it to the CPU cache for now.
		 */
		WARN_ON(obj->base.write_domain & ~I915_GEM_DOMAIN_CPU);
		WARN_ON(obj->base.read_domains & ~I915_GEM_DOMAIN_CPU);

		old_read_domains = obj->base.read_domains;
		old_write_domain = obj->base.write_domain;

		obj->base.read_domains = I915_GEM_DOMAIN_CPU;
		obj->base.write_domain = I915_GEM_DOMAIN_CPU;

		trace_i915_gem_object_change_domain(obj,
						    old_read_domains,
						    old_write_domain);
    }

	obj->cache_level = cache_level;
	return 0;
}

/*
 * Prepare buffer for display plane (scanout, cursors, etc).
 * Can be called from an uninterruptible phase (modesetting) and allows
 * any flushes to be pipelined (for pageflips).
 *
 * For the display plane, we want to be in the GTT but out of any write
 * domains. So in many ways this looks like set_to_gtt_domain() apart from the
 * ability to pipeline the waits, pinning and any additional subtleties
 * that may differentiate the display plane from ordinary buffers.
 */
int
i915_gem_object_pin_to_display_plane(struct drm_i915_gem_object *obj,
				     u32 alignment,
				     struct intel_ring_buffer *pipelined)
{
	u32 old_read_domains, old_write_domain;
	int ret;

	ret = i915_gem_object_flush_gpu_write_domain(obj);
	if (ret)
		return ret;

	if (pipelined != obj->ring) {
		ret = i915_gem_object_wait_rendering(obj);
		if (ret == -ERESTARTSYS)
			return ret;
	}

	/* The display engine is not coherent with the LLC cache on gen6.  As
	 * a result, we make sure that the pinning that is about to occur is
	 * done with uncached PTEs. This is lowest common denominator for all
	 * chipsets.
	 *
	 * However for gen6+, we could do better by using the GFDT bit instead
	 * of uncaching, which would allow us to flush all the LLC-cached data
	 * with that bit in the PTE to main memory with just one PIPE_CONTROL.
	 */
	ret = i915_gem_object_set_cache_level(obj, I915_CACHE_NONE);
	if (ret)
		return ret;

	/* As the user may map the buffer once pinned in the display plane
	 * (e.g. libkms for the bootup splash), we have to ensure that we
	 * always use map_and_fenceable for all scanout buffers.
	 */
	ret = i915_gem_object_pin(obj, alignment, true);
	if (ret)
		return ret;

	i915_gem_object_flush_cpu_write_domain(obj);

	old_write_domain = obj->base.write_domain;
	old_read_domains = obj->base.read_domains;

	/* It should now be out of any other write domains, and we can update
	 * the domain values for our changes.
	 */
	BUG_ON((obj->base.write_domain & ~I915_GEM_DOMAIN_GTT) != 0);
	obj->base.read_domains |= I915_GEM_DOMAIN_GTT;

	trace_i915_gem_object_change_domain(obj,
					    old_read_domains,
					    old_write_domain);

	return 0;
}

int
i915_gem_object_finish_gpu(struct drm_i915_gem_object *obj)
{
	int ret;

	if ((obj->base.read_domains & I915_GEM_GPU_DOMAINS) == 0)
		return 0;

	if (obj->base.write_domain & I915_GEM_GPU_DOMAINS) {
		ret = i915_gem_flush_ring(obj->ring, 0, obj->base.write_domain);
		if (ret)
			return ret;
	}

	/* Ensure that we invalidate the GPU's caches and TLBs. */
	obj->base.read_domains &= ~I915_GEM_GPU_DOMAINS;

	return i915_gem_object_wait_rendering(obj);
}

/**
 * Moves a single object to the CPU read, and possibly write domain.
 *
 * This function returns when the move is complete, including waiting on
 * flushes to occur.
 */
static int
i915_gem_object_set_to_cpu_domain(struct drm_i915_gem_object *obj, bool write)
{
	uint32_t old_write_domain, old_read_domains;
	int ret;

	if (obj->base.write_domain == I915_GEM_DOMAIN_CPU)
		return 0;

	ret = i915_gem_object_flush_gpu_write_domain(obj);
	if (ret)
		return ret;

	ret = i915_gem_object_wait_rendering(obj);
	if (ret)
		return ret;

	i915_gem_object_flush_gtt_write_domain(obj);


	old_write_domain = obj->base.write_domain;
	old_read_domains = obj->base.read_domains;

	/* Flush the CPU cache if it's still invalid. */
	if ((obj->base.read_domains & I915_GEM_DOMAIN_CPU) == 0) {
		i915_gem_clflush_object(obj);

		obj->base.read_domains |= I915_GEM_DOMAIN_CPU;
	}

	/* It should now be out of any other write domains, and we can update
	 * the domain values for our changes.
	 */
	BUG_ON((obj->base.write_domain & ~I915_GEM_DOMAIN_CPU) != 0);

	/* If we're writing through the CPU, then the GPU read domains will
	 * need to be invalidated at next use.
	 */
	if (write) {
		obj->base.read_domains = I915_GEM_DOMAIN_CPU;
		obj->base.write_domain = I915_GEM_DOMAIN_CPU;
	}

	trace_i915_gem_object_change_domain(obj,
					    old_read_domains,
					    old_write_domain);

	return 0;
}

/**
 * Moves the object from a partially CPU read to a full one.
 *
 * Note that this only resolves i915_gem_object_set_cpu_read_domain_range(),
 * and doesn't handle transitioning from !(read_domains & I915_GEM_DOMAIN_CPU).
 */
static void
i915_gem_object_set_to_full_cpu_read_domain(struct drm_i915_gem_object *obj)
{
	if (!obj->page_cpu_valid)
		return;

	/* If we're partially in the CPU read domain, finish moving it in.
	 */
	if (obj->base.read_domains & I915_GEM_DOMAIN_CPU) {
	}

	/* Free the page_cpu_valid mappings which are now stale, whether
	 * or not we've got I915_GEM_DOMAIN_CPU.
	 */
	kfree(obj->page_cpu_valid);
	obj->page_cpu_valid = NULL;
}




int gem_object_lock(struct drm_i915_gem_object *obj)
{
    return i915_gem_object_set_to_cpu_domain(obj, true);
}



















int
i915_gem_object_pin(struct drm_i915_gem_object *obj,
		    uint32_t alignment,
		    bool map_and_fenceable)
{
	struct drm_device *dev = obj->base.dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	int ret;

	BUG_ON(obj->pin_count == DRM_I915_GEM_OBJECT_MAX_PIN_COUNT);
	WARN_ON(i915_verify_lists(dev));

#if 0
	if (obj->gtt_space != NULL) {
		if ((alignment && obj->gtt_offset & (alignment - 1)) ||
		    (map_and_fenceable && !obj->map_and_fenceable)) {
			WARN(obj->pin_count,
			     "bo is already pinned with incorrect alignment:"
			     " offset=%x, req.alignment=%x, req.map_and_fenceable=%d,"
			     " obj->map_and_fenceable=%d\n",
			     obj->gtt_offset, alignment,
			     map_and_fenceable,
			     obj->map_and_fenceable);
			ret = i915_gem_object_unbind(obj);
			if (ret)
				return ret;
		}
	}
#endif

	if (obj->gtt_space == NULL) {
		ret = i915_gem_object_bind_to_gtt(obj, alignment,
						  map_and_fenceable);
		if (ret)
			return ret;
	}

	if (obj->pin_count++ == 0) {
		if (!obj->active)
			list_move_tail(&obj->mm_list,
				       &dev_priv->mm.pinned_list);
	}
	obj->pin_mappable |= map_and_fenceable;

	WARN_ON(i915_verify_lists(dev));
	return 0;
}

void
i915_gem_object_unpin(struct drm_i915_gem_object *obj)
{
	struct drm_device *dev = obj->base.dev;
	drm_i915_private_t *dev_priv = dev->dev_private;

	WARN_ON(i915_verify_lists(dev));
	BUG_ON(obj->pin_count == 0);
	BUG_ON(obj->gtt_space == NULL);

	if (--obj->pin_count == 0) {
		if (!obj->active)
			list_move_tail(&obj->mm_list,
				       &dev_priv->mm.inactive_list);
		obj->pin_mappable = false;
	}
	WARN_ON(i915_verify_lists(dev));
}






























struct drm_i915_gem_object *i915_gem_alloc_object(struct drm_device *dev,
						  size_t size)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct drm_i915_gem_object *obj;

	obj = kzalloc(sizeof(*obj), GFP_KERNEL);
	if (obj == NULL)
		return NULL;

	if (drm_gem_object_init(dev, &obj->base, size) != 0) {
		kfree(obj);
		return NULL;
	}


	i915_gem_info_add_obj(dev_priv, size);

	obj->base.write_domain = I915_GEM_DOMAIN_CPU;
	obj->base.read_domains = I915_GEM_DOMAIN_CPU;

	if (IS_GEN6(dev) || IS_GEN7(dev)) {
		/* On Gen6, we can have the GPU use the LLC (the CPU
		 * cache) for about a 10% performance improvement
		 * compared to uncached.  Graphics requests other than
		 * display scanout are coherent with the CPU in
		 * accessing this cache.  This means in this mode we
		 * don't need to clflush on the CPU side, and on the
		 * GPU side we only need to flush internal caches to
		 * get data visible to the CPU.
		 *
		 * However, we maintain the display planes as UC, and so
		 * need to rebind when first used as such.
		 */
		obj->cache_level = I915_CACHE_LLC;
	} else
		obj->cache_level = I915_CACHE_NONE;

	obj->base.driver_private = NULL;
	obj->fence_reg = I915_FENCE_REG_NONE;
	INIT_LIST_HEAD(&obj->mm_list);
	INIT_LIST_HEAD(&obj->gtt_list);
	INIT_LIST_HEAD(&obj->ring_list);
	INIT_LIST_HEAD(&obj->exec_list);
	INIT_LIST_HEAD(&obj->gpu_write_list);
	obj->madv = I915_MADV_WILLNEED;
	/* Avoid an unnecessary call to unbind on the first bind. */
	obj->map_and_fenceable = true;

	return obj;
}

int i915_gem_init_object(struct drm_gem_object *obj)
{
	BUG();

	return 0;
}

static void i915_gem_free_object_tail(struct drm_i915_gem_object *obj)
{
	struct drm_device *dev = obj->base.dev;
	drm_i915_private_t *dev_priv = dev->dev_private;
	int ret;

	ret = i915_gem_object_unbind(obj);
	if (ret == -ERESTARTSYS) {
		list_move(&obj->mm_list,
			  &dev_priv->mm.deferred_free_list);
		return;
	}

	trace_i915_gem_object_destroy(obj);

//	if (obj->base.map_list.map)
//		drm_gem_free_mmap_offset(&obj->base);

	drm_gem_object_release(&obj->base);
	i915_gem_info_remove_obj(dev_priv, obj->base.size);

	kfree(obj->page_cpu_valid);
	kfree(obj->bit_17);
	kfree(obj);
}

void i915_gem_free_object(struct drm_gem_object *gem_obj)
{
	struct drm_i915_gem_object *obj = to_intel_bo(gem_obj);
	struct drm_device *dev = obj->base.dev;

    while (obj->pin_count > 0)
		i915_gem_object_unpin(obj);

//	if (obj->phys_obj)
//		i915_gem_detach_phys_object(dev, obj);

	i915_gem_free_object_tail(obj);
}












int
i915_gem_init_ringbuffer(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	int ret;

	ret = intel_init_render_ring_buffer(dev);
	if (ret)
		return ret;

    if (HAS_BSD(dev)) {
		ret = intel_init_bsd_ring_buffer(dev);
		if (ret)
			goto cleanup_render_ring;
	}

	if (HAS_BLT(dev)) {
		ret = intel_init_blt_ring_buffer(dev);
		if (ret)
			goto cleanup_bsd_ring;
	}

	dev_priv->next_seqno = 1;

	return 0;

cleanup_bsd_ring:
	intel_cleanup_ring_buffer(&dev_priv->ring[VCS]);
cleanup_render_ring:
	intel_cleanup_ring_buffer(&dev_priv->ring[RCS]);
	return ret;
}

#if 0
void
i915_gem_cleanup_ringbuffer(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	int i;

	for (i = 0; i < I915_NUM_RINGS; i++)
		intel_cleanup_ring_buffer(&dev_priv->ring[i]);
}

int
i915_gem_entervt_ioctl(struct drm_device *dev, void *data,
		       struct drm_file *file_priv)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	int ret, i;

	if (drm_core_check_feature(dev, DRIVER_MODESET))
		return 0;

	if (atomic_read(&dev_priv->mm.wedged)) {
		DRM_ERROR("Reenabling wedged hardware, good luck\n");
		atomic_set(&dev_priv->mm.wedged, 0);
	}

	mutex_lock(&dev->struct_mutex);
	dev_priv->mm.suspended = 0;

	ret = i915_gem_init_ringbuffer(dev);
	if (ret != 0) {
		mutex_unlock(&dev->struct_mutex);
		return ret;
	}

	BUG_ON(!list_empty(&dev_priv->mm.active_list));
	BUG_ON(!list_empty(&dev_priv->mm.flushing_list));
	BUG_ON(!list_empty(&dev_priv->mm.inactive_list));
	for (i = 0; i < I915_NUM_RINGS; i++) {
		BUG_ON(!list_empty(&dev_priv->ring[i].active_list));
		BUG_ON(!list_empty(&dev_priv->ring[i].request_list));
	}
	mutex_unlock(&dev->struct_mutex);

	ret = drm_irq_install(dev);
	if (ret)
		goto cleanup_ringbuffer;

	return 0;

cleanup_ringbuffer:
	mutex_lock(&dev->struct_mutex);
	i915_gem_cleanup_ringbuffer(dev);
	dev_priv->mm.suspended = 1;
	mutex_unlock(&dev->struct_mutex);

	return ret;
}

int
i915_gem_leavevt_ioctl(struct drm_device *dev, void *data,
		       struct drm_file *file_priv)
{
	if (drm_core_check_feature(dev, DRIVER_MODESET))
		return 0;

	drm_irq_uninstall(dev);
	return i915_gem_idle(dev);
}

void
i915_gem_lastclose(struct drm_device *dev)
{
	int ret;

	if (drm_core_check_feature(dev, DRIVER_MODESET))
		return;

	ret = i915_gem_idle(dev);
	if (ret)
		DRM_ERROR("failed to idle hardware: %d\n", ret);
}
#endif

static void
init_ring_lists(struct intel_ring_buffer *ring)
{
    INIT_LIST_HEAD(&ring->active_list);
    INIT_LIST_HEAD(&ring->request_list);
    INIT_LIST_HEAD(&ring->gpu_write_list);
}

void
i915_gem_load(struct drm_device *dev)
{
    int i;
    drm_i915_private_t *dev_priv = dev->dev_private;

    INIT_LIST_HEAD(&dev_priv->mm.active_list);
    INIT_LIST_HEAD(&dev_priv->mm.flushing_list);
    INIT_LIST_HEAD(&dev_priv->mm.inactive_list);
    INIT_LIST_HEAD(&dev_priv->mm.pinned_list);
    INIT_LIST_HEAD(&dev_priv->mm.fence_list);
    INIT_LIST_HEAD(&dev_priv->mm.deferred_free_list);
    INIT_LIST_HEAD(&dev_priv->mm.gtt_list);
    for (i = 0; i < I915_NUM_RINGS; i++)
        init_ring_lists(&dev_priv->ring[i]);
	for (i = 0; i < I915_MAX_NUM_FENCES; i++)
        INIT_LIST_HEAD(&dev_priv->fence_regs[i].lru_list);
	INIT_DELAYED_WORK(&dev_priv->mm.retire_work,
			  i915_gem_retire_work_handler);

    /* On GEN3 we really need to make sure the ARB C3 LP bit is set */
    if (IS_GEN3(dev)) {
        u32 tmp = I915_READ(MI_ARB_STATE);
        if (!(tmp & MI_ARB_C3_LP_WRITE_ENABLE)) {
            /* arb state is a masked write, so set bit + bit in mask */
            tmp = MI_ARB_C3_LP_WRITE_ENABLE | (MI_ARB_C3_LP_WRITE_ENABLE << MI_ARB_MASK_SHIFT);
            I915_WRITE(MI_ARB_STATE, tmp);
        }
    }

    dev_priv->relative_constants_mode = I915_EXEC_CONSTANTS_REL_GENERAL;

    if (INTEL_INFO(dev)->gen >= 4 || IS_I945G(dev) || IS_I945GM(dev) || IS_G33(dev))
        dev_priv->num_fence_regs = 16;
    else
        dev_priv->num_fence_regs = 8;

    /* Initialize fence registers to zero */
    for (i = 0; i < dev_priv->num_fence_regs; i++) {
        i915_gem_clear_fence_reg(dev, &dev_priv->fence_regs[i]);
    }

    i915_gem_detect_bit_6_swizzle(dev);

    dev_priv->mm.interruptible = true;

//    dev_priv->mm.inactive_shrinker.shrink = i915_gem_inactive_shrink;
//    dev_priv->mm.inactive_shrinker.seeks = DEFAULT_SEEKS;
//    register_shrinker(&dev_priv->mm.inactive_shrinker);
}


