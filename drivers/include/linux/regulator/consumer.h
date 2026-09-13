#ifndef _LINUX_REGULATOR_CONSUMER_H
#define _LINUX_REGULATOR_CONSUMER_H
/* Tegra-only; see linux/clk.h for why this exists at all. */
#include <linux/errno.h>
#include <linux/err.h>

struct regulator;
struct device;

static inline struct regulator *regulator_get(struct device *dev, const char *id)
{ return ERR_PTR(-ENOENT); }
static inline void regulator_put(struct regulator *r) { }
static inline int regulator_enable(struct regulator *r) { return -ENOSYS; }
static inline int regulator_disable(struct regulator *r) { return -ENOSYS; }
static inline int regulator_get_voltage(struct regulator *r) { return -ENODEV; }
static inline int regulator_set_voltage(struct regulator *r, int min_uV, int max_uV)
{ return -ENODEV; }
#endif
