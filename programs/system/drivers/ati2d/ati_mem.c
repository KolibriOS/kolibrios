/* radeon_mem.c -- Simple GART/fb memory manager for radeon -*- linux-c -*- */
/*
 * Copyright (C) The Weather Channel, Inc.  2002.  All Rights Reserved.
 *
 * The Weather Channel (TM) funded Tungsten Graphics to develop the
 * initial release of the Radeon 8500 driver under the XFree86 license.
 * This notice must be preserved.
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
 * PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Keith Whitwell <keith@tungstengraphics.com>
 */

#define USED_BLOCK  1

#define list_for_each(entry, head)        \
   for (entry = (head)->next; entry != head; entry = (entry)->next)


/* Very simple allocator for GART memory, working on a static range
 * already mapped into each client's address space.
 */

struct mem_block {
	struct mem_block *next;
	struct mem_block *prev;
	int start;
	int size;
};

/* Initialize.  How to check for an uninitialized heap?
 */
static int init_heap(struct mem_block **heap, int start, int size)
{
  struct mem_block *blocks = kmalloc(sizeof(*blocks));

	if (!blocks)
    return -1; //-ENOMEM;

  *heap = kmalloc(sizeof(**heap));
  if (!*heap)
  {
    kfree(blocks);
    return -1; //-ENOMEM;
	}

  blocks->start = start;
  blocks->size  = size;
  blocks->next  = blocks->prev = *heap;

  __clear(*heap,sizeof(**heap));
	(*heap)->next = (*heap)->prev = blocks;
	return 0;
}

static struct mem_block **get_heap(RHDPtr rhdPtr, int region)
{
  switch (region)
  {
    case RHD_MEM_GART:
      return &rhdPtr->gart_heap;
    case RHD_MEM_FB:
      return &rhdPtr->fb_heap;
    default:
      return NULL;
	}
}

static struct mem_block *split_block(struct mem_block *p, int size)
{

	/* Maybe cut off the end of an existing block */
    if (size < p->size)
    {
      struct mem_block *newblock = kmalloc(sizeof(*newblock));
      if (!newblock)
        goto out;
      newblock->start = p->start + size;
      newblock->size = p->size - size;
      newblock->next = p->next;
      newblock->prev = p;
      p->next->prev = newblock;
      p->next = newblock;
      p->size = size;
      p->start|=1;
    }

out:
    return p;
}

static struct mem_block *alloc_block(struct mem_block *heap, int size)
{
	struct mem_block *p;

  list_for_each(p, heap)
  {
    if ( !(p->start & USED_BLOCK) && size <= p->size)
      return split_block(p, size);
	}

	return NULL;
}


static struct mem_block *find_block(struct mem_block *heap, int start)
{
	struct mem_block *p;

	list_for_each(p, heap)
    if ((p->start & ~USED_BLOCK) == start)
			return p;

	return NULL;
}

static void free_block(struct mem_block *p)
{

	/* Assumes a single contiguous range.  Needs a special file_priv in
	 * 'heap' to stop it being subsumed.
	 */

  p->start &= ~USED_BLOCK;

  if ( !(p->next->start & USED_BLOCK))
  {
		struct mem_block *q = p->next;
		p->size += q->size;
		p->next = q->next;
		p->next->prev = p;
    kfree(q);
	}

  if ( !(p->prev->start & USED_BLOCK))
  {
		struct mem_block *q = p->prev;
		q->size += p->size;
		q->next = p->next;
		q->next->prev = q;
    kfree(p);
	}
}

int rhdInitHeap(RHDPtr rhdPtr)
{
  int base = rhdPtr->FbBase + rhdPtr->FbFreeStart;

  return init_heap(&rhdPtr->fb_heap, base, rhdPtr->FbFreeSize);
};

void *rhd_mem_alloc(RHDPtr rhdPtr,int region, int size)
{
	struct mem_block *block, **heap;

  heap = get_heap(rhdPtr, region);
	if (!heap || !*heap)
    return NULL;

	/* Make things easier on ourselves: all allocations at least
	 * 4k aligned.
	 */

  size = (size+4095) & ~4095;

  block = alloc_block(*heap, size);

	if (!block)
    return NULL;

  return (void*)(block->start & ~USED_BLOCK);
}

int rhd_mem_free(RHDPtr rhdPtr, int region, void *offset)
{
	struct mem_block *block, **heap;

  heap = get_heap(rhdPtr, region);
	if (!heap || !*heap)
    return -1;

  block = find_block(*heap, (int)offset);
	if (!block)
    return -1;

  if ( !(block->start & 1))
    return -1;

	free_block(block);
	return 0;
}


