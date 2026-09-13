#ifndef _LINUX_PAGEMAP_H
#define _LINUX_PAGEMAP_H
/* Only the shift/size helpers are used by the TTM/GEM paths on this port. */
#include <linux/mm.h>
#define PAGE_CACHE_SHIFT	PAGE_SHIFT
#define PAGE_CACHE_SIZE		PAGE_SIZE
#define PAGE_CACHE_MASK		PAGE_MASK
#endif
