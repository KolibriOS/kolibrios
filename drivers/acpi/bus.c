/*
 *  acpi_bus.c - ACPI Bus Driver ($Revision: 80 $)
 *
 *  Copyright (C) 2001, 2002 Paul Diefenbaugh <paul.s.diefenbaugh@intel.com>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or (at
 *  your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/acpi.h>
#include <linux/pci.h>
#include <linux/dmi.h>

#include "internal.h"
struct acpi_device *acpi_root;
#ifdef CONFIG_X86
#ifdef CONFIG_ACPI_CUSTOM_DSDT
static inline int set_copy_dsdt(const struct dmi_system_id *id)
{
	return 0;
}
#else
static int set_copy_dsdt(const struct dmi_system_id *id)
{
	printk(KERN_NOTICE "%s detected - "
		"force copy of DSDT to local memory\n", id->ident);
	acpi_gbl_copy_dsdt_locally = 1;
	return 0;
}
#endif

static struct dmi_system_id dsdt_dmi_table[] __initdata = {
	/*
	 * Invoke DSDT corruption work-around on all Toshiba Satellite.
	 * https://bugzilla.kernel.org/show_bug.cgi?id=14679
	 */
	{
	 .callback = set_copy_dsdt,
	 .ident = "TOSHIBA Satellite",
	 .matches = {
		DMI_MATCH(DMI_SYS_VENDOR, "TOSHIBA"),
		DMI_MATCH(DMI_PRODUCT_NAME, "Satellite"),
		},
	},
	{}
};
#else
static struct dmi_system_id dsdt_dmi_table[] __initdata = {
	{}
};
#endif
/**
 * acpi_early_init - Initialize ACPICA and populate the ACPI namespace.
 *
 * The ACPI tables are accessible after this, but the handling of events has not
 * been initialized and the global lock is not available yet, so AML should not
 * be executed at this point.
 *
 * Doing this before switching the EFI runtime services to virtual mode allows
 * the EfiBootServices memory to be freed slightly earlier on boot.
 */
void __init acpi_early_init(void)
{
	acpi_status status;

	if (acpi_disabled)
		return;

	printk(KERN_INFO PREFIX "Core revision %08x\n", ACPI_CA_VERSION);

	/* It's safe to verify table checksums during late stage */
	acpi_gbl_verify_table_checksum = TRUE;

	/* enable workarounds, unless strict ACPI spec. compliance */
	if (!acpi_strict)
		acpi_gbl_enable_interpreter_slack = TRUE;

	acpi_gbl_permanent_mmap = 1;

	/*
	 * If the machine falls into the DMI check table,
	 * DSDT will be copied to memory
	 */
	dmi_check_system(dsdt_dmi_table);

	status = acpi_reallocate_root_table();
	if (ACPI_FAILURE(status)) {
		printk(KERN_ERR PREFIX
		       "Unable to reallocate ACPI tables\n");
		goto error0;
	}

	status = acpi_initialize_subsystem();
	if (ACPI_FAILURE(status)) {
		printk(KERN_ERR PREFIX
		       "Unable to initialize the ACPI Interpreter\n");
		goto error0;
	}

	status = acpi_load_tables();
	if (ACPI_FAILURE(status)) {
		printk(KERN_ERR PREFIX
		       "Unable to load the System Description Tables\n");
		goto error0;
	}

#ifdef CONFIG_X86
	if (!acpi_ioapic) {
		/* compatible (0) means level (3) */
		if (!(acpi_sci_flags & ACPI_MADT_TRIGGER_MASK)) {
			acpi_sci_flags &= ~ACPI_MADT_TRIGGER_MASK;
			acpi_sci_flags |= ACPI_MADT_TRIGGER_LEVEL;
		}
		/* Set PIC-mode SCI trigger type */
		acpi_pic_sci_set_trigger(acpi_gbl_FADT.sci_interrupt,
					 (acpi_sci_flags & ACPI_MADT_TRIGGER_MASK) >> 2);
	} else {
		/*
		 * now that acpi_gbl_FADT is initialized,
		 * update it with result from INT_SRC_OVR parsing
		 */
		acpi_gbl_FADT.sci_interrupt = acpi_sci_override_gsi;
	}
#endif
	return;

 error0:
	disable_acpi();
}
