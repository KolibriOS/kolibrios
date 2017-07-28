#ifndef _LINUX_DMA_MAPPING_H
#define _LINUX_DMA_MAPPING_H

#include <linux/sizes.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/dma-attrs.h>
#include <linux/dma-direction.h>
#include <linux/scatterlist.h>
#include <linux/bug.h>

extern void *
dma_alloc_coherent(struct device *dev, size_t size, dma_addr_t *dma_handle,
		   gfp_t flag); 
#define DMA_BIT_MASK(n)	(((n) == 64) ? ~0ULL : ((1ULL<<(n))-1))

#define DMA_MASK_NONE	0x0ULL

static inline int valid_dma_direction(int dma_direction)
{
	return ((dma_direction == DMA_BIDIRECTIONAL) ||
		(dma_direction == DMA_TO_DEVICE) ||
		(dma_direction == DMA_FROM_DEVICE));
}

static inline int is_device_dma_capable(struct device *dev)
{
	return dev->dma_mask != NULL && *dev->dma_mask != DMA_MASK_NONE;
}

#ifdef CONFIG_HAS_DMA
#include <asm/dma-mapping.h>
#else
#include <asm-generic/dma-mapping-broken.h>
#endif
#ifndef dma_max_pfn
static inline unsigned long dma_max_pfn(struct device *dev)
{
	return *dev->dma_mask >> PAGE_SHIFT;
}
#endif

static inline void *dma_zalloc_coherent(struct device *dev, size_t size,
					dma_addr_t *dma_handle, gfp_t flag)
{
	void *ret = dma_alloc_coherent(dev, size, dma_handle,
				       flag | __GFP_ZERO);
	return ret;
}

#endif
