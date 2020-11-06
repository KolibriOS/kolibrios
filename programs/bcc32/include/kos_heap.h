#ifndef __KOLIBRI_HEAP_H_INCLUDED_
#define __KOLIBRI_HEAP_H_INCLUDED_

#include "kolibri.h"
#include "memheap.h"

// Kolibri memory heap interface.

namespace Kolibri   // All kolibri functions, types and data are nested in the (Kolibri) namespace.
{
	void *Alloc(unsigned int size);
	void *ReAlloc(void *mem, unsigned int size);
	void Free(void *mem);
}

#ifdef __KOLIBRI__

namespace Kolibri
{

// Global variables

	MemoryHeap::TFreeSpace _KolibriFreeSpace;
	MemoryHeap::TMemBlock  _KolibriMemBlock;
	TMutex _MemHeapMutex = KOLIBRI_MUTEX_INIT;

// Functions

	void *_HeapInit(void *begin, void *use_end, void *end)
	{
		MemoryHeap::InitFreeSpace(_KolibriFreeSpace);
		_KolibriMemBlock = MemoryHeap::CreateBlock(begin, end, _KolibriFreeSpace);
		unsigned int use_beg = (unsigned int)MemoryHeap::BlockBegin(_KolibriMemBlock) +
					MemoryHeap::BlockAddSize - MemoryHeap::BlockEndSize;
		unsigned int use_size = (unsigned int)use_end;
		if (use_size <= use_beg) return 0;
		else use_size -= use_beg;
		return MemoryHeap::Alloc(_KolibriFreeSpace, use_size);
	}

	bool _SetUseMemory(unsigned int use_mem);

	int _RecalculateUseMemory(unsigned int use_mem);

	void *Alloc(unsigned int size)
	{
		if (!size) return 0;
		Lock(&_MemHeapMutex);
		void *res = MemoryHeap::Alloc(_KolibriFreeSpace, size);
		if (!res)
		{
			unsigned use_mem = (unsigned int)MemoryHeap::BlockEndFor(_KolibriMemBlock, size);
			if (_SetUseMemory(_RecalculateUseMemory(use_mem)))
			{
				res = MemoryHeap::Alloc(_KolibriFreeSpace, size);
			}
		}
		UnLock(&_MemHeapMutex);
		return res;
	}

	void *ReAlloc(void *mem, unsigned int size)
	{
		Lock(&_MemHeapMutex);
		void *res = MemoryHeap::ReAlloc(_KolibriFreeSpace, mem, size);
		if (!res && size)
		{
			unsigned use_mem = (unsigned int)MemoryHeap::BlockEndFor(_KolibriMemBlock, size);
			if (_SetUseMemory(_RecalculateUseMemory(use_mem)))
			{
				res = MemoryHeap::ReAlloc(_KolibriFreeSpace, mem, size);
			}
		}
		UnLock(&_MemHeapMutex);
		return res;
	}

	void Free(void *mem)
	{
		Lock(&_MemHeapMutex);
		MemoryHeap::Free(_KolibriFreeSpace, mem);
		UnLock(&_MemHeapMutex);
	}

	void _FreeAndThreadFinish(void *mem, int *exit_proc_now);
}

#endif  // def  __KOLIBRI__

#endif  // ndef __KOLIBRI_HEAP_H_INCLUDED_
