/*
 * Copyright © 2008 Keith Packard <keithp@keithp.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _LINUX_IO_MAPPING_H
#define _LINUX_IO_MAPPING_H

#include <linux/types.h>
#include <linux/slab.h>
#include <linux/bug.h>
#include <linux/io.h>
#include <asm/page.h>

/*
 * The io_mapping mechanism provides an abstraction for mapping
 * individual pages from an io device to the CPU in an efficient fashion.
 *
 * See Documentation/io-mapping.txt
 */

#ifdef CONFIG_HAVE_ATOMIC_IOMAP

#include <asm/iomap.h>

struct io_mapping {
    void            *vaddr;
	resource_size_t base;
    unsigned long   size;
};

/*
 * For small address space machines, mapping large objects
 * into the kernel virtual space isn't practical. Where
 * available, use fixmap support to dynamically map pages
 * of the object at run time.
 */

static inline struct io_mapping *
io_mapping_create_wc(resource_size_t base, unsigned long size)
{
	struct io_mapping *iomap;

	iomap = kmalloc(sizeof(*iomap), GFP_KERNEL);
	if (!iomap)
		goto out_err;

    iomap->vaddr = AllocKernelSpace(4096);
    if (iomap->vaddr == NULL)
		goto out_free;

	iomap->base = base;
	iomap->size = size;

	return iomap;

out_free:
	kfree(iomap);
out_err:
	return NULL;
}

static inline void
io_mapping_free(struct io_mapping *mapping)
{
    FreeKernelSpace(mapping->vaddr);
	kfree(mapping);
}

/* Atomic map/unmap */
static inline void __iomem *
io_mapping_map_atomic_wc(struct io_mapping *mapping,
			 unsigned long offset)
{
    addr_t phys_addr;

	BUG_ON(offset >= mapping->size);
    phys_addr = (mapping->base + offset) & PAGE_MASK;

    MapPage(mapping->vaddr, phys_addr, PG_WRITEC|PG_SW);
    return mapping->vaddr;
}

static inline void
io_mapping_unmap_atomic(void __iomem *vaddr)
{
    MapPage(vaddr, 0, 0);
}

static inline void __iomem *
io_mapping_map_wc(struct io_mapping *mapping, unsigned long offset)
{
    addr_t phys_addr;

	BUG_ON(offset >= mapping->size);
    phys_addr = (mapping->base + offset) & PAGE_MASK;

    MapPage(mapping->vaddr, phys_addr, PG_WRITEC|PG_SW);
    return mapping->vaddr;
}

static inline void
io_mapping_unmap(void __iomem *vaddr)
{
    MapPage(vaddr, 0, 0);
}

#else

#include <linux/uaccess.h>

/* this struct isn't actually defined anywhere */
struct io_mapping;

/* Create the io_mapping object*/
static inline struct io_mapping *
io_mapping_create_wc(resource_size_t base, unsigned long size)
{
	return (struct io_mapping __force *) ioremap_wc(base, size);
}

static inline void
io_mapping_free(struct io_mapping *mapping)
{
	iounmap((void __force __iomem *) mapping);
}

/* Atomic map/unmap */
static inline void __iomem *
io_mapping_map_atomic_wc(struct io_mapping *mapping,
			 unsigned long offset)
{
	preempt_disable();
	pagefault_disable();
	return ((char __force __iomem *) mapping) + offset;
}

static inline void
io_mapping_unmap_atomic(void __iomem *vaddr)
{
	pagefault_enable();
	preempt_enable();
}

/* Non-atomic map/unmap */
static inline void __iomem *
io_mapping_map_wc(struct io_mapping *mapping, unsigned long offset)
{
	return ((char __force __iomem *) mapping) + offset;
}

static inline void
io_mapping_unmap(void __iomem *vaddr)
{
}

#endif /* HAVE_ATOMIC_IOMAP */

#endif /* _LINUX_IO_MAPPING_H */
