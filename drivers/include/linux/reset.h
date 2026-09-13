#ifndef _LINUX_RESET_H
#define _LINUX_RESET_H
/* Tegra-only; see linux/clk.h for why this exists at all. */
#include <linux/errno.h>
#include <linux/err.h>

struct reset_control;
struct device;

static inline struct reset_control *reset_control_get(struct device *dev, const char *id)
{ return ERR_PTR(-ENOENT); }
static inline void reset_control_put(struct reset_control *rstc) { }
static inline int reset_control_assert(struct reset_control *rstc) { return -ENOSYS; }
static inline int reset_control_deassert(struct reset_control *rstc) { return -ENOSYS; }
#endif
