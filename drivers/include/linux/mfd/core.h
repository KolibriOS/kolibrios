#ifndef _LINUX_MFD_CORE_H
#define _LINUX_MFD_CORE_H
/*
 * No multi-function-device bus on KolibriOS.  amdgpu_acp.h embeds an
 * mfd_cell pointer in struct amdgpu_acp; the ACP block itself is not built.
 */
#include <linux/types.h>
struct device;
struct resource;

struct mfd_cell {
	const char	*name;
	int		 id;
	void		*platform_data;
	size_t		 pdata_size;
	int		 num_resources;
	const struct resource *resources;
};

static inline int mfd_add_hotplug_devices(struct device *parent,
					  const struct mfd_cell *cells, int n)
	{ return -ENODEV; }
static inline void mfd_remove_devices(struct device *parent) { }
#endif
