#ifndef __MEMORY_HEAP_H_INCLUDED_
#define __MEMORY_HEAP_H_INCLUDED_

namespace MemoryHeap
{
	long mem_Init();
	void *mem_Alloc(unsigned long size);
	void *mem_ReAlloc(unsigned long size, void *mem);
	bool mem_Free(void *mem);
}

#endif  // ndef __MEMORY_HEAP_H_INCLUDED_

