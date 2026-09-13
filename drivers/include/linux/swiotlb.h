#ifndef _LINUX_SWIOTLB_H
#define _LINUX_SWIOTLB_H
/* No bounce buffers on this port; amdgpu_ttm.c only queries swiotlb_nr_tbl(). */
static inline unsigned long swiotlb_nr_tbl(void) { return 0; }
#endif
