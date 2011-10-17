#include <ddk.h>
#include <syscall.h>
#include "acpi.h"

extern acpi_rsdp_t* acpi_rsdp;
extern acpi_rsdt_t* acpi_rsdt;

addr_t acpi_ioapic;
addr_t acpi_local_apic;

u8_t __fastcall acpi_table_checksum(u32_t length, u8_t *buffer);

acpi_rsdp_t* acpi_locate()
{
    /** @todo checksum, check version */
    addr_t p;

    for (p = ACPI_HI_RSDP_WINDOW_START; p < ACPI_HI_RSDP_WINDOW_END; p+=16)
    {
        acpi_rsdp_t* r = (acpi_rsdp_t*) p;
        if ((r->sig[0] == 0x20445352) &&
            (r->sig[1] == 0x20525450) &&
             acpi_table_checksum(ACPI_RSDP_CHECKSUM_LENGTH, (u8_t*)r)==0 )
        {
            return r;
        };
    };
    /* not found */
    return NULL;
};

addr_t __fastcall rsdt_find(acpi_rsdt_t *rsdt, u32_t sig)
{
    addr_t head = 0;
    u32_t i;

    for (i = 0; i < ((rsdt->header.len-sizeof(acpi_thead_t))/
                sizeof(rsdt->ptrs[0])); i++)
    {
        acpi_thead_t* t= (acpi_thead_t*)acpi_remap_table(rsdt->ptrs[i]);

        if (t->sig == sig)
        {
           head = rsdt->ptrs[i];
           break;
        };
    }
    return head;
};

