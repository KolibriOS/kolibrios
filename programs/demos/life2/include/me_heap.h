#ifndef __MENUET_HEAP_H_INCLUDED_
#define __MENUET_HEAP_H_INCLUDED_

#include <menuet.h>
#include <memheap.h>

// Menuet memory heap interface.

namespace Menuet   // All menuet functions, types and data are nested in the (Menuet) namespace.
{
	void *Alloc(unsigned int size);
	void *ReAlloc(void *mem, unsigned int size);
	void Free(void *mem);
}

#ifdef __MENUET__

namespace Menuet
{

// Global variables

	MemoryHeap::TFreeSpace _MenuetFreeSpace;
	MemoryHeap::TMemBlock  _MenuetMemBlock;
	TMutex _MemHeapMutex = MENUET_MUTEX_INIT;

// Functions

	void *_HeapInit(void *begin, void *use_end, void *end)
	{
		MemoryHeap::InitFreeSpace(_MenuetFreeSpace);
		_MenuetMemBlock = MemoryHeap::CreateBlock(begin, end, _MenuetFreeSpace);
		unsigned int use_beg = (unsigned int)MemoryHeap::BlockBegin(_MenuetMemBlock) +
					MemoryHeap::BlockAddSize - MemoryHeap::BlockEndSize;
		unsigned int use_size = (unsigned int)use_end;
		if (use_size <= use_beg) return 0;
		else use_size -= use_beg;
		return MemoryHeap::Alloc(_MenuetFreeSpace, use_size);
	}

	bool _SetUseMemory(unsigned int use_mem);

	int _RecalculateUseMemory(unsigned int use_mem);

	void *Alloc(unsigned int size)
	{
		if (!size) return 0;
		Lock(&_MemHeapMutex);
		void *res = MemoryHeap::Alloc(_MenuetFreeSpace, size);
		if (!res)
		{
			unsigned use_mem = (unsigned int)MemoryHeap::BlockEndFor(_MenuetMemBlock, size);
			if (_SetUseMemory(_RecalculateUseMemory(use_mem)))
			{
				res = MemoryHeap::Alloc(_MenuetFreeSpace, size);
			}
		}
		UnLock(&_MemHeapMutex);
		return res;
	}

	void *ReAlloc(void *mem, unsigned int size)
	{
		Lock(&_MemHeapMutex);
		void *res = MemoryHeap::ReAlloc(_MenuetFreeSpace, mem, size);
		if (!res && size)
		{
			unsigned use_mem = (unsigned int)MemoryHeap::BlockEndFor(_MenuetMemBlock, size);
			if (_SetUseMemory(_RecalculateUseMemory(use_mem)))
			{
				res = MemoryHeap::ReAlloc(_MenuetFreeSpace, mem, size);
			}
		}
		UnLock(&_MemHeapMutex);
		return res;
	}

	void Free(void *mem)
	{
		Lock(&_MemHeapMutex);
		MemoryHeap::Free(_MenuetFreeSpace, mem);
		UnLock(&_MemHeapMutex);
	}

	void _FreeAndThreadFinish(void *mem, int *exit_proc_now);
}

#endif  // def  __MENUET__

#endif  // ndef __MENUET_HEAP_H_INCLUDED_
