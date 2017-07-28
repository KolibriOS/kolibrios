/*
 * Copyright 2006 PathScale, Inc.  All Rights Reserved.
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

#ifndef _LINUX_IO_H
#define _LINUX_IO_H

#include <linux/types.h>
#include <linux/init.h>
#include <linux/bug.h>
#include <linux/err.h>
#include <asm/io.h>
struct device;
struct resource;

__visible void __iowrite32_copy(void __iomem *to, const void *from, size_t count);
void __ioread32_copy(void *to, const void __iomem *from, size_t count);
void __iowrite64_copy(void __iomem *to, const void *from, size_t count);
void *memremap(resource_size_t offset, size_t size, unsigned long flags);
void memunmap(void *addr);

#endif /* _LINUX_IO_H */
