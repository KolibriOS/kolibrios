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

/*******************************************************************************
 *   This is my heap realization adapted for KolibriOS                         *
 *******************************************************************************/

#include "kolibri.h"
#include "heap.h"
#include "defs.h"

void *halMemAlloc(Heap *wheap, unsigned long nbytes){
	MemBlock *heap = wheap->free, *best = 0;
	int good_best = 0;						// Such memory block, that no memory will left
	unsigned long nblocks = (nbytes-1)/sizeof(MemBlock)+1;
	
	if (heap) {
		while (heap) {
			if (heap->nblocks > nblocks && (!best || heap->nblocks < best->nblocks) && (!good_best || heap->nblocks > nblocks+1)) {
				best = heap;
				if (heap->nblocks > nblocks+1) {
					good_best = 1;
				}
			} else if (heap->nblocks == nblocks) {
				best = heap;
				good_best = 0;
				break;
			}
			heap = heap->next;
		}
	}
	
	if (best && good_best) {
		heap = best+best->nblocks-nblocks;
		heap->nblocks = nblocks;
		wheap->allocated = halMemBlockAdd(wheap->allocated, heap, false);
		best->nblocks -= nblocks+1;
		best = heap;
	} else if (best) {
		wheap->free = halMemBlockRemove(best);
		wheap->allocated = halMemBlockAdd(wheap->allocated, best, false);
	} else {
		// We need some memory for heap
		int npages = ((nblocks+1)-1)/(PAGE_SIZE/sizeof(MemBlock))+1;
		void *pages;
		
// 		debug(DEBUG_MESSAGE, 4, "no enough memory in heap, needed %d page%s", npages, npages == 1 ? "" : "s");
		
		// Try to get npages at once
		pages = wheap->alloc(wheap, npages*PAGE_SIZE);
		if (pages) {
// 			debug(DEBUG_MESSAGE, 5, "physic pages allocated");
			best = (MemBlock *) pages;
			if (npages*PAGE_SIZE <= (nblocks+2)*sizeof(MemBlock)) {
				best->nblocks = npages*PAGE_SIZE/sizeof(MemBlock)-1;
			} else {
				best->nblocks = nblocks;
				heap = best+best->nblocks+1;
				heap->nblocks = npages*PAGE_SIZE/sizeof(MemBlock)-(nblocks+1)-1;
				wheap->free = halMemBlockAdd(wheap->free, heap, true);
			}
			wheap->allocated = halMemBlockAdd(wheap->allocated, best, false);
		} else {
// 			debug(DEBUG_WARNING, 3, "no available physic pages");
			return NULL;
		}
	}
	
// 	debug(DEBUG_MESSAGE, 5, "memory allocated");
	
	return (void *) (best + 1);
}

void halMemFree(Heap *wheap, void *addr){
	MemBlock *heap = wheap->allocated;
	MemBlock *block = (MemBlock *)addr - 1;
	
	while (heap->next && block < heap) {
		heap = heap->next;
	}
	
	if (block != heap) {
		return;
	}
	
	wheap->allocated = halMemBlockRemove(block);
	wheap->free = halMemBlockAdd(wheap->free, block, true);
}

void halMemHeapInit(Heap *wheap, void *(*alloc)(Heap *heap, int nbytes), void *st_addr, int size){
	MemBlock *st_unit = (MemBlock *)st_addr;
	
	if (st_unit) {
		st_unit->nblocks = size/sizeof(MemBlock)-1;
		wheap->free = halMemBlockAdd(NULL, st_unit, true);
	} else {
		wheap->free = NULL;
	}
	wheap->allocated = NULL;
	wheap->alloc = alloc;
}

/////////////////////////////////////////////////////////////////////////////////
// inside functions                                                            //
/////////////////////////////////////////////////////////////////////////////////
MemBlock *halMemBlockAdd(MemBlock *heap, MemBlock *unit, bool optimize){
	MemBlock *heap_st = heap;
// 	int optimize = (heap_st != wheap->allocated);
	
	if (!heap || (heap > unit)) {
		heap_st = unit;
		if (heap) {
			heap->previos = unit;
		}
		unit->next = heap;
		unit->previos = NULL;
	} else {
		if (heap->next) {
			while (heap->next && (heap < unit)) {
				heap = heap->next;
			}
			heap = heap->previos;
		}
		unit->next = heap->next;
		unit->previos = heap;
		if (heap->next) {
			heap->next->previos = unit;
		}
		heap->next = unit;
	}
	
	if (optimize) {
		if (unit->previos && unit->previos+unit->previos->nblocks == unit) {
			halMemBlockRemove(unit);
			unit->previos->nblocks += unit->nblocks+1;
		}
		
		if (unit->next && unit+unit->nblocks == unit->next) {
			unit->nblocks += unit->next->nblocks+1;
			halMemBlockRemove(unit->next);
		}
	}
	
	return heap_st;
}

MemBlock *halMemBlockRemove(MemBlock *unit) {
	if (unit->next) {
		unit->next->previos = unit->previos;
	}
	
	if (unit->previos) {
		unit->previos->next = unit->next;
		while (unit->previos) {
			unit = unit->previos;
		}
		return unit;
	} else {
		return unit->next;
	}
}
