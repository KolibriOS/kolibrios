
#include <ddk.h>
#include <syscall.h>
#include "acpi.h"

acpi_rsdp_t* acpi_locate();

acpi_rsdp_t* acpi_rsdp;
acpi_rsdt_t* acpi_rsdt;
acpi_madt_t* acpi_madt;

addr_t acpi_rsdt_base;
addr_t acpi_madt_base;
addr_t acpi_lapic_base;
addr_t acpi_ioapic_base;

addr_t __fastcall rsdt_find(acpi_rsdt_t *rsdt, u32_t sig);

u32_t drvEntry(int action, char *cmdline)
{
    u32_t ret;

    if(action != 1)
        return 0;

    if( !dbg_open("/sys/drivers/acpi.log") )
    {
        printf("Can't open /sys/drivers/acpi.log\nExit\n");
        return 0;
    }

    acpi_rsdp = acpi_locate();

    if (unlikely(acpi_rsdp == NULL))
    {
        dbgprintf("No ACPI RSD table\n");
        return 0;
    };

    dbgprintf("rsd base address %x\n", acpi_rsdp);

    acpi_rsdt_base = acpi_rsdp->rsdt_ptr;
    acpi_rsdt = (acpi_rsdt_t*)(MapIoMem(acpi_rsdt_base,0x10000,5));

    dbgprintf("rsdt base 0x%x, kernel 0x%x\n", acpi_rsdt_base, acpi_rsdt);

    if (unlikely(acpi_rsdt == NULL))
    {
        dbgprintf("Invalid ACPI RSD table\n");
        return 0;
    };

//    print_rsdt(acpi_rsdt);

    acpi_madt_base = rsdt_find(acpi_rsdt, ACPI_MADT_SIGN);

    if( unlikely(acpi_madt_base == 0) )
    {
        dbgprintf("No ACPI MAD Table\n");
        return 0;
    };

    acpi_madt =(acpi_madt_t*)acpi_remap_table(acpi_madt_base);

    dbgprintf("madt base 0x%x, kernel 0x%x\n", acpi_madt_base, acpi_madt);

//    print_madt(acpi_madt);

    return 0;
};

