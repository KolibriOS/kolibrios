/***************************************************************************************************
 *  Copyright (C) Vasiliy Kosenko (vkos), 2009                                                     *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the *
 *  GNU General Public License as published by the Free Software Foundation, either version 3      *
 *  of the License, or (at your option) any later version.                                         *
 *                                                                                                 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;      *
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See  *
 *  the GNU General Public License for more details.                                               *
 *                                                                                                 *
 *  You should have received a copy of the GNU General Public License along with this program.     *
 *  If not, see <http://www.gnu.org/licenses/>.                                                    *
 ***************************************************************************************************/

/***************************************************************************************************
 *  malloc.c - realisation of malloc & free functions based on heap.c                              *
 ***************************************************************************************************/

#include "malloc.h"
#include "kolibri.h"
#include "heap.h"

Heap malloc_heap;

void malloc_init(){
	// Init system heap
	kolibri_heap_init();
	
	// Init main heap for malloc
	halMemHeapInit(&malloc_heap, &heap_alloc, NULL, 0);
}

void *heap_alloc(Heap *wheap, int nbytes){
	return kolibri_malloc(nbytes);
}

void *malloc(int nbytes){
	return halMemAlloc(&malloc_heap, nbytes);
}

void free(void *addr){
	halMemFree(&malloc_heap, addr);
}
