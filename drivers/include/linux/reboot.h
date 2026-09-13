#ifndef _LINUX_REBOOT_H
#define _LINUX_REBOOT_H
/*
 * No reboot notifier chain on this port.  nvif/os.h includes this header for
 * every nvkm translation unit; nothing in the built set registers a notifier.
 */
#include <linux/notifier.h>

#define SYS_RESTART	0x0001
#define SYS_HALT	0x0002
#define SYS_POWER_OFF	0x0003

static inline int register_reboot_notifier(struct notifier_block *nb) { return 0; }
static inline int unregister_reboot_notifier(struct notifier_block *nb) { return 0; }

/*
 * nvkm/subdev/therm/temp.c asks for this when the GPU passes its shutdown
 * threshold.  There is no orderly shutdown to request on KolibriOS; the
 * driver's implementation logs and returns.
 */
extern int orderly_poweroff(bool force);
#endif
