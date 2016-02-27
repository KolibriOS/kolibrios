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

#define pr_fmt(fmt) "[TTM] " fmt

#include <drm/ttm/ttm_memory.h>
#include <drm/ttm/ttm_module.h>
#include <drm/ttm/ttm_page_alloc.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/slab.h>

#define TTM_MEMORY_ALLOC_RETRIES 4


int ttm_mem_global_init(struct ttm_mem_global *glob)
{
	int ret;
	int i;

	spin_lock_init(&glob->lock);



	ttm_page_alloc_init(glob, 4*1024);

	return 0;
out_no_zone:
	ttm_mem_global_release(glob);
	return ret;
}
EXPORT_SYMBOL(ttm_mem_global_init);

void ttm_mem_global_release(struct ttm_mem_global *glob)
{
	unsigned int i;

	/* let the page allocator first stop the shrink work. */
//	ttm_page_alloc_fini();
//	ttm_dma_page_alloc_fini();


}

void ttm_mem_global_free(struct ttm_mem_global *glob,
			 uint64_t amount)
{

}
EXPORT_SYMBOL(ttm_mem_global_free);

int ttm_mem_global_alloc(struct ttm_mem_global *glob, uint64_t memory,
			 bool no_wait, bool interruptible)
{
	/**
	 * Normal allocations of kernel memory are registered in
	 * all zones.
	 */

	return 0;
}
EXPORT_SYMBOL(ttm_mem_global_alloc);

size_t ttm_round_pot(size_t size)
{
	if ((size & (size - 1)) == 0)
		return size;
	else if (size > PAGE_SIZE)
		return PAGE_ALIGN(size);
	else {
		size_t tmp_size = 4;

		while (tmp_size < size)
			tmp_size <<= 1;

		return tmp_size;
	}
	return 0;
}
EXPORT_SYMBOL(ttm_round_pot);
