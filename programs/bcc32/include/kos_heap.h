#ifndef __KOLIBRI_HEAP_H_INCLUDED_
#define __KOLIBRI_HEAP_H_INCLUDED_

#include "kolibri.h"

// Kolibri memory heap interface.

namespace Kolibri   // All kolibri functions, types and data are nested in the (Kolibri) namespace.
{
	long HeapInit();
	void *Alloc(unsigned long int size);
	void *ReAlloc(void *mem, unsigned long int size);
	void Free(void *mem);
}

#endif  // ndef __KOLIBRI_HEAP_H_INCLUDED_
