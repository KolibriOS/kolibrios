
#define PCI_FIND_CAP_TTL    48

static int __pci_find_next_cap_ttl(PCITAG pciTag, u8_t pos,
                                   int cap, int *ttl)
{
    u8_t id;

    while ((*ttl)--)
    {
        pos = pciReadByte(pciTag, pos);
		if (pos < 0x40)
			break;
		pos &= ~3;
        id = pciReadByte(pciTag, pos + PCI_CAP_LIST_ID);
		if (id == 0xff)
			break;
		if (id == cap)
			return pos;
		pos += PCI_CAP_LIST_NEXT;
	}
	return 0;
}

static int __pci_find_next_cap(PCITAG pciTag, u8_t pos, int cap)
{
	int ttl = PCI_FIND_CAP_TTL;

    return __pci_find_next_cap_ttl(pciTag, pos, cap, &ttl);
}

static int __pci_bus_find_cap_start(PCITAG pciTag)
{
    u16_t status;
    u8_t  hdr_type;

    status = pciReadWord(pciTag, PCI_STATUS);
	if (!(status & PCI_STATUS_CAP_LIST))
		return 0;

    hdr_type = pciReadByte(pciTag, 0x0E);
    switch (hdr_type)
    {
        case PCI_HEADER_TYPE_NORMAL:
        case PCI_HEADER_TYPE_BRIDGE:
            return PCI_CAPABILITY_LIST;
        case PCI_HEADER_TYPE_CARDBUS:
            return PCI_CB_CAPABILITY_LIST;
        default:
            return 0;
	}
	return 0;
}


int pci_find_capability(PCITAG pciTag, int cap)
{
	int pos;

    pos = __pci_bus_find_cap_start(pciTag);
	if (pos)
        pos = __pci_find_next_cap(pciTag, pos, cap);

	return pos;
}


PCITAG pci_find_class(u16_t class)
{
    u32_t bus, last_bus;
    PCITAG tag;

    if( (last_bus = PciApi(1))==-1)
        return -1;

    for(bus=0;bus<=last_bus;bus++)
    {
        u32_t devfn;

        for(devfn=0;devfn<256;devfn++)
        {
            u16_t devclass;

            devclass = PciRead16(bus,devfn, 0x0A);

            if( devclass != class)
                continue;

            return pciTag(bus,(devfn>>3)&0x1F,devfn&0x7);
        };
    };
    return -1;
};


PCITAG pci_get_device(u32_t vendor, u32_t device, PCITAG from)
{
    u32_t bus, last_bus;
    u32_t devfn;

    if( (last_bus = PciApi(1))==-1)
        return -1;

    bus   = PCI_BUS_FROM_TAG(from);
    devfn = PCI_DFN_FROM_TAG(from);

    devfn++;

    for(;bus<=last_bus; bus++)
    {
        for(;devfn < 256;devfn++)
        {
            u32_t tmp;
            u32_t dev_vendor;
            u32_t dev_id;

            tmp = PciRead32(bus,devfn, 0);

            dev_vendor = (u16_t)tmp;
            dev_id = tmp >> 16;

            if ((vendor == PCI_ANY_ID || dev_vendor == vendor))
                return pciTag(bus,(devfn>>3)&0x1F,devfn&0x7);
        };
    };
    return -1;
}

