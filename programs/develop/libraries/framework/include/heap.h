/*******************************************************************************
 *   Copyright (C) 2009 Vasiliy Kosenko                                        *
 *                                                                             *
 *   This program is free software; you can redistribute it and/or modify      *
 *   it under the terms of the GNU General Public License as published by      *
 *   the Free Software Foundation; either version 3 of the License, or         *
 *   (at your option) any later version.                                       *
 *                                                                             *
 *   This program is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 *   GNU General Public License for more details.                              *
 *                                                                             *
 *   You should have received a copy of the GNU General Public License         *
 *   along with this program; if not, write to the Free Software               *
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                 *
 *******************************************************************************/

#ifndef _HEAP_H_
#define _HEAP_H_

#include "defs.h"

#define PAGE_SIZE 0x1000

typedef struct MemBlock {
	struct MemBlock *next;
	struct MemBlock *previos;
	unsigned long nblocks;
	unsigned long align;
} MemBlock;

typedef struct Heap {
	struct MemBlock *free;
	struct MemBlock *allocated;
	void *(*alloc)(struct Heap *heap, int nbytes);
} Heap;

void *halMemAlloc(Heap *wheap, unsigned long n);
void halMemFree(Heap *wheap, void *addr);
MemBlock *halMemBlockAdd(MemBlock *heap, MemBlock *unit, bool optimize);
MemBlock *halMemBlockRemove(MemBlock *unit);

void halMemHeapInit(Heap *wheap, void *(*alloc)(Heap *heap, int nbytes), void *st_addr, int size);

#endif
