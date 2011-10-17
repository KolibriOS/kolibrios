#include <ddk.h>
#include <syscall.h>
#include "acpi.h"

extern acpi_rsdp_t* acpi_rsdp;
extern acpi_rsdt_t* acpi_rsdt;

addr_t acpi_ioapic;
addr_t acpi_local_apic;

u8_t __fastcall acpi_table_checksum(u32_t length, u8_t *buffer);

u8_t __fastcall acpi_table_checksum(u32_t length, u8_t *buffer)
{
    u8_t  sum = 0;
    u8_t *end = buffer + length;

    while (buffer < end)
    {
        sum = (u8_t)(sum + *(buffer++));
    }

    return sum;
};

void* acpi_remap_table(addr_t table)
{
    addr_t ptr;

    if( (table >= acpi_rsdp->rsdt_ptr)&&
        (table < acpi_rsdp->rsdt_ptr+0x10000))
        ptr = (table - acpi_rsdp->rsdt_ptr+(addr_t)acpi_rsdt);
    return (void*)ptr;
};


void print_rsdt(acpi_rsdt_t *rsdt)
{
    u32_t i;
    dbgprintf("ACPI RSD Table\n");
    for (i = 0; i < ((rsdt->header.len-sizeof(acpi_thead_t))/
                sizeof(rsdt->ptrs[0])); i++)
    {
        acpi_thead_t* t = (acpi_thead_t*)acpi_remap_table(rsdt->ptrs[i]);
        char *p = (char*)&t->sig;
        dbgprintf("sig %d: %c%c%c%c  base %x\n", i,
        p[0],p[1],p[2],p[3], t);
    };
};


void print_madt (acpi_madt_t * madt)
{
    u32_t i;

    acpi_local_apic = madt->local_apic_addr;

    dbgprintf ("Local APIC at 0x%x\n\n", acpi_local_apic);

    for (i = 0; i < (madt->header.len - sizeof (acpi_madt_t));)
    {
        acpi_madt_hdr_t * h = (acpi_madt_hdr_t*) &madt->data[i];
        switch (h->type)
        {
            case 0:
            {
                // Local APIC
                acpi_madt_lapic_t * lapic = (acpi_madt_lapic_t *) h;
                dbgprintf ("Local APIC  ");
                dbgprintf ("[Id: 0x%x, CPU Id: 0x%x, %s]\n",
                lapic->id, lapic->apic_processor_id,
                lapic->flags.enabled ? "enabled" : "disabled");
                break;
            }
            case 1:
            {
                // I/O Apic
                acpi_madt_ioapic_t * ioapic = (acpi_madt_ioapic_t *) h;

                acpi_ioapic = ioapic->address;

                dbgprintf ("I/O APIC    ");
                dbgprintf ("[Id: 0x%x, IRQ base: %d, Addr: 0x%x]\n",
                ioapic->id, ioapic->irq_base, acpi_ioapic);
                break;
            }
            case 2:
            {
                // Interrupt Source Override
                acpi_madt_irq_t * irq = (acpi_madt_irq_t *) h;
                polarity_t p = irq_get_polarity(irq);
                trigger_mode_t t = irq_get_trigger_mode(irq);

                dbgprintf ("Interrupt Override  ");
                dbgprintf ("[%s, Bus IRQ: %d, Glob IRQ: %d, Pol: %s, Trigger: %s]\n",
                irq->src_bus == 0 ? "ISA" : "unknown bus",
                irq->src_irq, irq->dest,
                p == conform_polarity ? "conform" :
                p == active_high ? "active high" :
                p == active_low ? "active low" : "?",
                t == conform_trigger ? "conform" :
                t == edge ? "edge" :
                t == level ? "level" : "?");
                break;
            }
            case 3:
            {
                // NMI Source

                acpi_madt_nmi_t * nmi = (acpi_madt_nmi_t *) h;
                polarity_t p = nmi_get_polarity(nmi);
                trigger_mode_t t = nmi_get_trigger_mode(nmi);
                dbgprintf ("NMI Source  ");
                dbgprintf ("[Glob IRQ: %d, Pol: %s, Trigger: %s]\n",
                    nmi->irq,
                    p == conform_polarity ? "conform" :
                    p == active_high ? "active high" :
                    p == active_low ? "active low" : "?",
                    t == conform_trigger ? "conform" :
                    t == edge ? "edge" :
                    t == level ? "level" : "?");

                break;
            }
            case 4:
            {
                // Local APIC NMI
                acpi_lapic_nmi_t *nmi = (acpi_lapic_nmi_t *) h;

                polarity_t p = lapic_nmi_get_polarity(nmi);
                trigger_mode_t t = lapic_nmi_get_trigger_mode(nmi);

                dbgprintf ("Local APIC NMI\n");

                dbgprintf ("[CPU id: %d, LINT#: %d Pol: %s, Trigger: %s]\n",
                    nmi->apic_processor_id,
                    nmi->lint,
                    p == conform_polarity ? "conform" :
                    p == active_high ? "active high" :
                    p == active_low ? "active low" : "?",
                    t == conform_trigger ? "conform" :
                    t == edge ? "edge" :
                    t == level ? "level" : "?");
                break;
            }
            case 5:
            {
                // Local APIC Address Override
                dbgprintf ("Local APIC Address Override\n");
                break;
            }
            case 8:
            {
                // Platform Interrupt Source
                dbgprintf ("Platform Interrupt Source\n");
                break;
            }
        }

        i += h->len;
    }
};


