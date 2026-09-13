#ifndef _LINUX_OF_DEVICE_H
#define _LINUX_OF_DEVICE_H
/* No device tree on x86; pulled in unconditionally by nvif/os.h. */
struct device;
struct of_device_id;

static inline const struct of_device_id *
of_match_device(const struct of_device_id *m, const struct device *dev)
{ return NULL; }
#endif
