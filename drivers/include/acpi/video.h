#ifndef __ACPI_VIDEO_H
#define __ACPI_VIDEO_H
/* No ACPI video extensions; nouveau_display.c only calls the register hook. */
#include <linux/errno.h>

enum acpi_backlight_type {
	acpi_backlight_undef = -1,
	acpi_backlight_none = 0,
	acpi_backlight_video,
	acpi_backlight_vendor,
	acpi_backlight_native,
};

static inline int acpi_video_register(void) { return -ENODEV; }
static inline void acpi_video_unregister(void) { }
static inline enum acpi_backlight_type acpi_video_get_backlight_type(void)
{ return acpi_backlight_vendor; }
#endif
