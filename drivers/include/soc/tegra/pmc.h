#ifndef __SOC_TEGRA_PMC_H
#define __SOC_TEGRA_PMC_H
/* Tegra-only; nvif/os.h includes it unconditionally. */
static inline int tegra_pmc_dummy(void) { return 0; }
#endif
