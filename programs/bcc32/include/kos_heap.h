#ifndef __KOLIBRI_HEAP_H_INCLUDED_
#define __KOLIBRI_HEAP_H_INCLUDED_

#include "kolibri.h"
#include "memheap.h"

// Kolibri memory heap interface.

namespace Kolibri   // All kolibri functions, types and data are nested in the (Kolibri) namespace.
{
	long _HeapInit()
	{
		return MemoryHeap::mem_Init();
	}

	void *Alloc(unsigned int size)
	{
		return MemoryHeap::mem_Alloc(size);
	}

	void *ReAlloc(void *mem, unsigned int size)
	{
		return MemoryHeap::mem_ReAlloc(size, mem);
	}

	void Free(void *mem)
	{
		MemoryHeap::mem_Free(mem);
	}
}

#endif  // ndef __KOLIBRI_HEAP_H_INCLUDED_
