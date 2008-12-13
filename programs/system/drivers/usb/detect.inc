

Bool FindPciDevice()
{
    Bool retval = FALSE;
    u32_t bus, last_bus;
    PCITAG tag;

    if( (last_bus = PciApi(1))==-1)
        return retval;

    for(bus=0;bus<=last_bus;bus++)
    {
        u32_t devfn;

        for(devfn=0;devfn<256;devfn++)
        {
            hc_t *hc;

            u32_t id;
            u16_t pcicmd;
            u16_t devclass;
            int i;

            devclass = PciRead16(bus,devfn, 0x0A);

            if( devclass != 0x0C03)
                continue;

            pcicmd = PciRead16(bus,devfn, PCI_COMMAND);
            if (! pcicmd & PCI_COMMAND_IO)
                continue;

            hc = (hc_t*)malloc(sizeof(hc_t));
            memset(hc, 0, sizeof(hc_t));
            link_initialize(&hc->link);

            hc->pciId  = PciRead32(bus,devfn, 0);
            hc->PciTag = pciTag(bus,(devfn>>3)&0x1F,devfn&0x7);

            for (i = 0; i < 6; i++)
            {
                u32_t base;
                Bool validSize;

                base = PciRead32(bus,devfn, PCI_MAP_REG_START + (i << 2));
                if(base)
                {
                    if (base & PCI_MAP_IO) {
                        hc->ioBase[i] = (addr_t)PCIGETIO(base);
                        hc->memType[i]   = base & PCI_MAP_IO_ATTR_MASK;
                    } else {
                        hc->memBase[i] = (u32_t)PCIGETMEMORY(base);
                        hc->memType[i] = base & PCI_MAP_MEMORY_ATTR_MASK;
                    }
                }
            };
            list_prepend(&hc->link, &hc_list);
            retval = TRUE;
        };
    };
    return retval;
};
