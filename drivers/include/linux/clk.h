#ifndef _LINUX_CLK_H
#define _LINUX_CLK_H
/*
 * Tegra-only.  nvif/os.h pulls this in unconditionally, but the only nouveau
 * code that calls a clk_* helper is nvkm/engine/device/tegra.c, which this
 * port does not build.
 */
#include <linux/errno.h>
#include <linux/err.h>

struct clk;
struct device;

static inline struct clk *clk_get(struct device *dev, const char *id)
{ return ERR_PTR(-ENOENT); }
static inline void clk_put(struct clk *clk) { }
static inline int clk_prepare_enable(struct clk *clk) { return -ENOSYS; }
static inline void clk_disable_unprepare(struct clk *clk) { }
static inline unsigned long clk_get_rate(struct clk *clk) { return 0; }
static inline int clk_set_rate(struct clk *clk, unsigned long rate) { return -ENOSYS; }
#endif
