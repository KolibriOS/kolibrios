#ifndef _LINUX_PLATFORM_DEVICE_H
#define _LINUX_PLATFORM_DEVICE_H
/* No platform bus; amdgpu_drv.h only needs the type to exist. */
#include <linux/device.h>
struct platform_device { struct device dev; const char *name; int id; };
struct platform_driver;
#endif
