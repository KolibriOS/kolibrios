#ifndef _LINUX_MM_H
#define _LINUX_MM_H

#include <kernel.h>

#define VM_NORESERVE    0x00200000

#define nth_page(page,n) ((void*)(((page_to_phys(page)>>12)+(n))<<12))

#define page_to_pfn(page) (page_to_phys(page)>>12)

/* to align the pointer to the (next) page boundary */
#define PAGE_ALIGN(addr) ALIGN(addr, PAGE_SIZE)

#define offset_in_page(p)       ((unsigned long)(p) & ~PAGE_MASK)

#endif
