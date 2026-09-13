#ifndef ACPI_BUTTON_H
#define ACPI_BUTTON_H
#include <linux/errno.h>
#include <linux/notifier.h>
/*
 * nouveau_connector.c asks the ACPI lid device whether the panel is open
 * before trusting an LVDS hotplug.  No ACPI here, so report "open" - the same
 * answer the upstream !CONFIG_ACPI_BUTTON stub gives.
 */
static inline int acpi_lid_open(void) { return 1; }
static inline int acpi_lid_notifier_register(struct notifier_block *nb) { return -ENODEV; }
static inline int acpi_lid_notifier_unregister(struct notifier_block *nb) { return -ENODEV; }
#endif
