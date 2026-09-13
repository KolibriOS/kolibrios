#ifndef _LINUX_IOMMU_H
#define _LINUX_IOMMU_H
/*
 * No IOMMU on this port.  nvif/os.h includes it unconditionally; the only user
 * is the Tegra SMMU path in nvkm/engine/device/tegra.c, which is not built.
 */
#include <linux/errno.h>
#include <linux/types.h>

#define IOMMU_READ	(1 << 0)
#define IOMMU_WRITE	(1 << 1)
#define IOMMU_CACHE	(1 << 2)

struct device;
struct iommu_domain;
struct iommu_group;

struct iommu_domain_geometry {
	dma_addr_t aperture_start;
	dma_addr_t aperture_end;
	bool force_aperture;
};

static inline struct iommu_domain *iommu_domain_alloc(void *bus) { return NULL; }
static inline void iommu_domain_free(struct iommu_domain *d) { }
static inline int iommu_attach_device(struct iommu_domain *d, struct device *dev)
{ return -ENODEV; }
static inline void iommu_detach_device(struct iommu_domain *d, struct device *dev) { }
static inline struct iommu_group *iommu_group_get(struct device *dev) { return NULL; }
static inline int iommu_map(struct iommu_domain *domain, unsigned long iova,
			    phys_addr_t paddr, size_t size, int prot)
{ return -ENODEV; }
static inline size_t iommu_unmap(struct iommu_domain *domain, unsigned long iova,
				 size_t size)
{ return 0; }
static inline void iommu_group_put(struct iommu_group *g) { }
#endif
