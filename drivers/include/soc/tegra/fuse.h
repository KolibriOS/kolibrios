#ifndef __SOC_TEGRA_FUSE_H
#define __SOC_TEGRA_FUSE_H
/* Tegra-only; nvif/os.h includes it unconditionally. */
static inline int tegra_sku_info_dummy(void) { return 0; }
#endif
