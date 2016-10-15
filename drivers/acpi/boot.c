/*
 *  boot.c - Architecture-Specific Low-Level ACPI Boot Support
 *
 *  Copyright (C) 2001, 2002 Paul Diefenbaugh <paul.s.diefenbaugh@intel.com>
 *  Copyright (C) 2001 Jun Nakajima <jun.nakajima@intel.com>
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#include <linux/init.h>
#include <linux/acpi.h>
#include <linux/module.h>
#include <linux/dmi.h>
#include <linux/pci.h>

static inline void outb(u8 v, u16 port)
{
        asm volatile("outb %0,%1" : : "a" (v), "dN" (port));
}
static inline u8 inb(u16 port)
{
        u8 v;
        asm volatile("inb %1,%0" : "=a" (v) : "dN" (port));
        return v;
}


static int __initdata acpi_force = 0;
int acpi_disabled;
EXPORT_SYMBOL(acpi_disabled);

#ifdef	CONFIG_X86_64
# include <asm/proto.h>
#endif				/* X86 */

#define PREFIX			"ACPI: "

int acpi_noirq;				/* skip ACPI IRQ initialization */
int acpi_pci_disabled;		/* skip ACPI PCI scan and IRQ initialization */
EXPORT_SYMBOL(acpi_pci_disabled);

int acpi_ioapic;
int acpi_strict;
u8 acpi_sci_flags __initdata;
/*
 * acpi_pic_sci_set_trigger()
 *
 * use ELCR to set PIC-mode trigger type for SCI
 *
 * If a PIC-mode SCI is not recognized or gives spurious IRQ7's
 * it may require Edge Trigger -- use "acpi_sci=edge"
 *
 * Port 0x4d0-4d1 are ECLR1 and ECLR2, the Edge/Level Control Registers
 * for the 8259 PIC.  bit[n] = 1 means irq[n] is Level, otherwise Edge.
 * ECLR1 is IRQs 0-7 (IRQ 0, 1, 2 must be 0)
 * ECLR2 is IRQs 8-15 (IRQ 8, 13 must be 0)
 */

void __init acpi_pic_sci_set_trigger(unsigned int irq, u16 trigger)
{
	unsigned int mask = 1 << irq;
	unsigned int old, new;

	/* Real old ELCR mask */
//	old = inb(0x4d0) | (inb(0x4d1) << 8);

	/*
	 * If we use ACPI to set PCI IRQs, then we should clear ELCR
	 * since we will set it correctly as we enable the PCI irq
	 * routing.
	 */
	new = acpi_noirq ? old : 0;

	/*
	 * Update SCI information in the ELCR, it isn't in the PCI
	 * routing tables..
	 */
	switch (trigger) {
	case 1:		/* Edge - clear */
		new &= ~mask;
		break;
	case 3:		/* Level - set */
		new |= mask;
		break;
	}

	if (old == new)
		return;

	printk(PREFIX "setting ELCR to %04x (from %04x)\n", new, old);
//	outb(new, 0x4d0);
//	outb(new >> 8, 0x4d1);
}

static int __init acpi_parse_sbf(struct acpi_table_header *table)
{
	struct acpi_table_boot *sb = (struct acpi_table_boot *)table;

	sbf_port = sb->cmos_index;	/* Save CMOS port */

	return 0;
}

#ifdef CONFIG_HPET_TIMER
#include <asm/hpet.h>

static struct resource *hpet_res __initdata;

static int __init acpi_parse_hpet(struct acpi_table_header *table)
{
	struct acpi_table_hpet *hpet_tbl = (struct acpi_table_hpet *)table;

	if (hpet_tbl->address.space_id != ACPI_SPACE_MEM) {
		printk(KERN_WARNING PREFIX "HPET timers must be located in "
		       "memory.\n");
		return -1;
	}

	hpet_address = hpet_tbl->address.address;
	hpet_blockid = hpet_tbl->sequence;

	/*
	 * Some broken BIOSes advertise HPET at 0x0. We really do not
	 * want to allocate a resource there.
	 */
	if (!hpet_address) {
		printk(KERN_WARNING PREFIX
		       "HPET id: %#x base: %#lx is invalid\n",
		       hpet_tbl->id, hpet_address);
		return 0;
	}
#ifdef CONFIG_X86_64
	/*
	 * Some even more broken BIOSes advertise HPET at
	 * 0xfed0000000000000 instead of 0xfed00000. Fix it up and add
	 * some noise:
	 */
	if (hpet_address == 0xfed0000000000000UL) {
		if (!hpet_force_user) {
			printk(KERN_WARNING PREFIX "HPET id: %#x "
			       "base: 0xfed0000000000000 is bogus\n "
			       "try hpet=force on the kernel command line to "
			       "fix it up to 0xfed00000.\n", hpet_tbl->id);
			hpet_address = 0;
			return 0;
		}
		printk(KERN_WARNING PREFIX
		       "HPET id: %#x base: 0xfed0000000000000 fixed up "
		       "to 0xfed00000.\n", hpet_tbl->id);
		hpet_address >>= 32;
	}
#endif
	printk(KERN_INFO PREFIX "HPET id: %#x base: %#lx\n",
	       hpet_tbl->id, hpet_address);

	/*
	 * Allocate and initialize the HPET firmware resource for adding into
	 * the resource tree during the lateinit timeframe.
	 */
#define HPET_RESOURCE_NAME_SIZE 9
	hpet_res = alloc_bootmem(sizeof(*hpet_res) + HPET_RESOURCE_NAME_SIZE);

	hpet_res->name = (void *)&hpet_res[1];
	hpet_res->flags = IORESOURCE_MEM;
	snprintf((char *)hpet_res->name, HPET_RESOURCE_NAME_SIZE, "HPET %u",
		 hpet_tbl->sequence);

	hpet_res->start = hpet_address;
	hpet_res->end = hpet_address + (1 * 1024) - 1;

	return 0;
}

/*
 * hpet_insert_resource inserts the HPET resources used into the resource
 * tree.
 */
static __init int hpet_insert_resource(void)
{
	if (!hpet_res)
		return 1;

	return insert_resource(&iomem_resource, hpet_res);
}

late_initcall(hpet_insert_resource);

#else
#define	acpi_parse_hpet	NULL
#endif

static int __init acpi_parse_fadt(struct acpi_table_header *table)
{

#ifdef CONFIG_X86_PM_TIMER
	/* detect the location of the ACPI PM Timer */
	if (acpi_gbl_FADT.header.revision >= FADT2_REVISION_ID) {
		/* FADT rev. 2 */
		if (acpi_gbl_FADT.xpm_timer_block.space_id !=
		    ACPI_ADR_SPACE_SYSTEM_IO)
			return 0;

		pmtmr_ioport = acpi_gbl_FADT.xpm_timer_block.address;
		/*
		 * "X" fields are optional extensions to the original V1.0
		 * fields, so we must selectively expand V1.0 fields if the
		 * corresponding X field is zero.
	 	 */
		if (!pmtmr_ioport)
			pmtmr_ioport = acpi_gbl_FADT.pm_timer_block;
	} else {
		/* FADT rev. 1 */
		pmtmr_ioport = acpi_gbl_FADT.pm_timer_block;
	}
	if (pmtmr_ioport)
		printk(KERN_INFO PREFIX "PM-Timer IO Port: %#x\n",
		       pmtmr_ioport);
#endif
	return 0;
}


#ifdef	CONFIG_X86_IO_APIC
#else
static inline int acpi_parse_madt_ioapic_entries(void)
{
	return -1;
}
#endif	/* !CONFIG_X86_IO_APIC */

static void __init early_acpi_process_madt(void)
{
#ifdef CONFIG_X86_LOCAL_APIC
	int error;

	if (!acpi_table_parse(ACPI_SIG_MADT, acpi_parse_madt)) {

		/*
		 * Parse MADT LAPIC entries
		 */
		error = early_acpi_parse_madt_lapic_addr_ovr();
		if (!error) {
			acpi_lapic = 1;
			smp_found_config = 1;
		}
		if (error == -EINVAL) {
			/*
			 * Dell Precision Workstation 410, 610 come here.
			 */
			printk(KERN_ERR PREFIX
			       "Invalid BIOS MADT, disabling ACPI\n");
			disable_acpi();
		}
	}
#endif
}

static void __init acpi_process_madt(void)
{
#ifdef CONFIG_X86_LOCAL_APIC
	int error;

	if (!acpi_table_parse(ACPI_SIG_MADT, acpi_parse_madt)) {

		/*
		 * Parse MADT LAPIC entries
		 */
		error = acpi_parse_madt_lapic_entries();
		if (!error) {
			acpi_lapic = 1;

			/*
			 * Parse MADT IO-APIC entries
			 */
			mutex_lock(&acpi_ioapic_lock);
			error = acpi_parse_madt_ioapic_entries();
			mutex_unlock(&acpi_ioapic_lock);
			if (!error) {
				acpi_set_irq_model_ioapic();

				smp_found_config = 1;
			}
		}
		if (error == -EINVAL) {
			/*
			 * Dell Precision Workstation 410, 610 come here.
			 */
			printk(KERN_ERR PREFIX
			       "Invalid BIOS MADT, disabling ACPI\n");
			disable_acpi();
		}
	} else {
		/*
 		 * ACPI found no MADT, and so ACPI wants UP PIC mode.
 		 * In the event an MPS table was found, forget it.
 		 * Boot with "acpi=off" to use MPS on such a system.
 		 */
		if (smp_found_config) {
			printk(KERN_WARNING PREFIX
				"No APIC-table, disabling MPS\n");
			smp_found_config = 0;
		}
	}

	/*
	 * ACPI supports both logical (e.g. Hyper-Threading) and physical
	 * processors, where MPS only supports physical.
	 */
	if (acpi_lapic && acpi_ioapic)
		printk(KERN_INFO "Using ACPI (MADT) for SMP configuration "
		       "information\n");
	else if (acpi_lapic)
		printk(KERN_INFO "Using ACPI for processor (LAPIC) "
		       "configuration information\n");
#endif
	return;
}

static int __init disable_acpi_irq(const struct dmi_system_id *d)
{
	if (!acpi_force) {
		printk(KERN_NOTICE "%s detected: force use of acpi=noirq\n",
		       d->ident);
		acpi_noirq_set();
	}
	return 0;
}

static int __init disable_acpi_pci(const struct dmi_system_id *d)
{
	if (!acpi_force) {
		printk(KERN_NOTICE "%s detected: force use of pci=noacpi\n",
		       d->ident);
		acpi_disable_pci();
	}
	return 0;
}

static int __init dmi_disable_acpi(const struct dmi_system_id *d)
{
	if (!acpi_force) {
		printk(KERN_NOTICE "%s detected: acpi off\n", d->ident);
		disable_acpi();
	} else {
		printk(KERN_NOTICE
		       "Warning: DMI blacklist says broken, but acpi forced\n");
	}
	return 0;
}

/*
 * Force ignoring BIOS IRQ0 override
 */
static int __init dmi_ignore_irq0_timer_override(const struct dmi_system_id *d)
{
	if (!acpi_skip_timer_override) {
		pr_notice("%s detected: Ignoring BIOS IRQ0 override\n",
			d->ident);
		acpi_skip_timer_override = 1;
	}
	return 0;
}

/*
 * If your system is blacklisted here, but you find that acpi=force
 * works for you, please contact linux-acpi@vger.kernel.org
 */
static struct dmi_system_id __initdata acpi_dmi_table[] = {
	/*
	 * Boxes that need ACPI disabled
	 */
	{
	 .callback = dmi_disable_acpi,
	 .ident = "IBM Thinkpad",
	 .matches = {
		     DMI_MATCH(DMI_BOARD_VENDOR, "IBM"),
		     DMI_MATCH(DMI_BOARD_NAME, "2629H1G"),
		     },
	 },

	/*
	 * Boxes that need ACPI PCI IRQ routing disabled
	 */
	{
	 .callback = disable_acpi_irq,
	 .ident = "ASUS A7V",
	 .matches = {
		     DMI_MATCH(DMI_BOARD_VENDOR, "ASUSTeK Computer INC"),
		     DMI_MATCH(DMI_BOARD_NAME, "<A7V>"),
		     /* newer BIOS, Revision 1011, does work */
		     DMI_MATCH(DMI_BIOS_VERSION,
			       "ASUS A7V ACPI BIOS Revision 1007"),
		     },
	 },
	{
		/*
		 * Latest BIOS for IBM 600E (1.16) has bad pcinum
		 * for LPC bridge, which is needed for the PCI
		 * interrupt links to work. DSDT fix is in bug 5966.
		 * 2645, 2646 model numbers are shared with 600/600E/600X
		 */
	 .callback = disable_acpi_irq,
	 .ident = "IBM Thinkpad 600 Series 2645",
	 .matches = {
		     DMI_MATCH(DMI_BOARD_VENDOR, "IBM"),
		     DMI_MATCH(DMI_BOARD_NAME, "2645"),
		     },
	 },
	{
	 .callback = disable_acpi_irq,
	 .ident = "IBM Thinkpad 600 Series 2646",
	 .matches = {
		     DMI_MATCH(DMI_BOARD_VENDOR, "IBM"),
		     DMI_MATCH(DMI_BOARD_NAME, "2646"),
		     },
	 },
	/*
	 * Boxes that need ACPI PCI IRQ routing and PCI scan disabled
	 */
	{			/* _BBN 0 bug */
	 .callback = disable_acpi_pci,
	 .ident = "ASUS PR-DLS",
	 .matches = {
		     DMI_MATCH(DMI_BOARD_VENDOR, "ASUSTeK Computer INC."),
		     DMI_MATCH(DMI_BOARD_NAME, "PR-DLS"),
		     DMI_MATCH(DMI_BIOS_VERSION,
			       "ASUS PR-DLS ACPI BIOS Revision 1010"),
		     DMI_MATCH(DMI_BIOS_DATE, "03/21/2003")
		     },
	 },
	{
	 .callback = disable_acpi_pci,
	 .ident = "Acer TravelMate 36x Laptop",
	 .matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "Acer"),
		     DMI_MATCH(DMI_PRODUCT_NAME, "TravelMate 360"),
		     },
	 },
	{}
};

/* second table for DMI checks that should run after early-quirks */
static struct dmi_system_id __initdata acpi_dmi_table_late[] = {
	/*
	 * HP laptops which use a DSDT reporting as HP/SB400/10000,
	 * which includes some code which overrides all temperature
	 * trip points to 16C if the INTIN2 input of the I/O APIC
	 * is enabled.  This input is incorrectly designated the
	 * ISA IRQ 0 via an interrupt source override even though
	 * it is wired to the output of the master 8259A and INTIN0
	 * is not connected at all.  Force ignoring BIOS IRQ0
	 * override in that cases.
	 */
	{
	 .callback = dmi_ignore_irq0_timer_override,
	 .ident = "HP nx6115 laptop",
	 .matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "Hewlett-Packard"),
		     DMI_MATCH(DMI_PRODUCT_NAME, "HP Compaq nx6115"),
		     },
	 },
	{
	 .callback = dmi_ignore_irq0_timer_override,
	 .ident = "HP NX6125 laptop",
	 .matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "Hewlett-Packard"),
		     DMI_MATCH(DMI_PRODUCT_NAME, "HP Compaq nx6125"),
		     },
	 },
	{
	 .callback = dmi_ignore_irq0_timer_override,
	 .ident = "HP NX6325 laptop",
	 .matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "Hewlett-Packard"),
		     DMI_MATCH(DMI_PRODUCT_NAME, "HP Compaq nx6325"),
		     },
	 },
	{
	 .callback = dmi_ignore_irq0_timer_override,
	 .ident = "HP 6715b laptop",
	 .matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "Hewlett-Packard"),
		     DMI_MATCH(DMI_PRODUCT_NAME, "HP Compaq 6715b"),
		     },
	 },
	{
	 .callback = dmi_ignore_irq0_timer_override,
	 .ident = "FUJITSU SIEMENS",
	 .matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "FUJITSU SIEMENS"),
		     DMI_MATCH(DMI_PRODUCT_NAME, "AMILO PRO V2030"),
		     },
	 },
	{}
};

/*
 * acpi_boot_table_init() and acpi_boot_init()
 *  called from setup_arch(), always.
 *	1. checksums all tables
 *	2. enumerates lapics
 *	3. enumerates io-apics
 *
 * acpi_table_init() is separate to allow reading SRAT without
 * other side effects.
 *
 * side effects of acpi_boot_init:
 *	acpi_lapic = 1 if LAPIC found
 *	acpi_ioapic = 1 if IOAPIC found
 *	if (acpi_lapic && acpi_ioapic) smp_found_config = 1;
 *	if acpi_blacklisted() acpi_disabled = 1;
 *	acpi_irq_model=...
 *	...
 */

void __init acpi_boot_table_init(void)
{
	dmi_check_system(acpi_dmi_table);

	/*
	 * If acpi_disabled, bail out
	 */
	if (acpi_disabled)
		return;

	/*
	 * Initialize the ACPI boot-time table parser.
	 */
	if (acpi_table_init()) {
		disable_acpi();
		return;
	}

	acpi_table_parse(ACPI_SIG_BOOT, acpi_parse_sbf);

	/*
	 * blacklist may disable ACPI entirely
	 */
	if (acpi_blacklisted()) {
		if (acpi_force) {
			printk(KERN_WARNING PREFIX "acpi=force override\n");
		} else {
			printk(KERN_WARNING PREFIX "Disabling ACPI support\n");
			disable_acpi();
			return;
		}
	}
}

int __init early_acpi_boot_init(void)
{
	/*
	 * If acpi_disabled, bail out
	 */
	if (acpi_disabled)
		return 1;

	/*
	 * Process the Multiple APIC Description Table (MADT), if present
	 */
	early_acpi_process_madt();

	/*
	 * Hardware-reduced ACPI mode initialization:
	 */
//	acpi_reduced_hw_init();

	return 0;
}

int __init acpi_boot_init(void)
{
	/* those are executed after early-quirks are executed */
	dmi_check_system(acpi_dmi_table_late);

	/*
	 * If acpi_disabled, bail out
	 */
	if (acpi_disabled)
		return 1;

//	acpi_table_parse(ACPI_SIG_BOOT, acpi_parse_sbf);

	/*
	 * set sci_int and PM timer address
	 */
	acpi_table_parse(ACPI_SIG_FADT, acpi_parse_fadt);

	/*
	 * Process the Multiple APIC Description Table (MADT), if present
	 */
	acpi_process_madt();

	acpi_table_parse(ACPI_SIG_HPET, acpi_parse_hpet);

//	if (!acpi_noirq)
//		x86_init.pci.init = pci_acpi_init;

	return 0;
}
