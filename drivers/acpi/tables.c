/*
 *  acpi_tables.c - ACPI Boot-Time Table Parsing
 *
 *  Copyright (C) 2001 Paul Diefenbaugh <paul.s.diefenbaugh@intel.com>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 */

/* Uncomment next line to get verbose printout */
/* #define DEBUG */
#define pr_fmt(fmt) "ACPI: " fmt

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/acpi.h>

#define ACPI_MAX_TABLES		128
static struct acpi_table_desc initial_tables[ACPI_MAX_TABLES] __initdata;

static int acpi_apic_instance __initdata;

/*
 * Disable table checksum verification for the early stage due to the size
 * limitation of the current x86 early mapping implementation.
 */
static bool acpi_verify_table_checksum __initdata = false;
/**
 * acpi_table_parse - find table with @id, run @handler on it
 * @id: table id to find
 * @handler: handler to run
 *
 * Scan the ACPI System Descriptor Table (STD) for a table matching @id,
 * run @handler on it.
 *
 * Return 0 if table found, -errno if not.
 */
int __init acpi_table_parse(char *id, acpi_tbl_table_handler handler)
{
	struct acpi_table_header *table = NULL;
	acpi_size tbl_size;

	if (acpi_disabled)
		return -ENODEV;

	if (!id || !handler)
		return -EINVAL;

	if (strncmp(id, ACPI_SIG_MADT, 4) == 0)
		acpi_get_table_with_size(id, acpi_apic_instance, &table, &tbl_size);
	else
		acpi_get_table_with_size(id, 0, &table, &tbl_size);

	if (table) {
		handler(table);
		early_acpi_os_unmap_memory(table, tbl_size);
		return 0;
	} else
		return -ENODEV;
}
 
/*
 * acpi_table_init()
 *
 * find RSDP, find and checksum SDT/XSDT.
 * checksum all tables, print SDT/XSDT
 *
 * result: sdt_entry[] is initialized
 */

int __init acpi_table_init(void)
{
	acpi_status status;

	if (acpi_verify_table_checksum) {
		pr_info("Early table checksum verification enabled\n");
		acpi_gbl_verify_table_checksum = TRUE;
	} else {
		pr_info("Early table checksum verification disabled\n");
		acpi_gbl_verify_table_checksum = FALSE;
	}

	status = acpi_initialize_tables(initial_tables, ACPI_MAX_TABLES, 0);
	if (ACPI_FAILURE(status))
		return -EINVAL;

	return 0;
}
