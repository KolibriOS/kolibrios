
#define PCI_CLASS_BRIDGE_HOST    0x06

#define INTEL_82443LX_0         (0x7180<<16)|0x8086
#define INTEL_82443BX_0         (0x7190<<16)|0x8086
#define INTEL_82443GX_0         (0x71a0<<16)|0x8086
#define INTEL_82810_MC1         (0x7120<<16)|0x8086
#define INTEL_82810_MC3         (0x7122<<16)|0x8086
#define INTEL_82810E_MC         (0x7124<<16)|0x8086
#define INTEL_82815_MC          (0x1130<<16)|0x8086
#define INTEL_82820_HB          (0x2500<<16)|0x8086
#define INTEL_82820_UP_HB       (0x2501<<16)|0x8086
#define INTEL_82830_HB          (0x3575<<16)|0x8086
#define INTEL_82840_HB          (0x1a21<<16)|0x8086
#define INTEL_82845_HB          (0x1a30<<16)|0x8086
#define INTEL_82845G_HB         (0x2560<<16)|0x8086
#define INTEL_82850_HB          (0x2530<<16)|0x8086
#define INTEL_82855PM_HB        (0x3340<<16)|0x8086
#define INTEL_82855GM_HB        (0x3580<<16)|0x8086
#define INTEL_82860_HB          (0x2531<<16)|0x8086
#define INTEL_82865_HB          (0x2570<<16)|0x8086
#define INTEL_82875_HB          (0x2578<<16)|0x8086
#define INTEL_7505_0            (0x2550<<16)|0x8086
#define INTEL_7205_0            (0x255d<<16)|0x8086
#define INTEL_82915G_HB         (0x2580<<16)|0x8086
#define INTEL_82915GM_HB        (0x2590<<16)|0x8086
#define INTEL_82945G_HB         (0x2770<<16)|0x8086
#define INTEL_82945GM_HB        (0x27A0<<16)|0x8086


typedef struct
{
    int    id;
    int    driver;
}pci_device_t;


static pci_device_t agp_dev_table[] = {

//    { INTEL_82443LX_0,  0 },
//    { INTEL_82443BX_0,  0 },
//    { INTEL_82443GX_0,  0 },
//    { INTEL_82810_MC1,  0 },
//    { INTEL_82810_MC3,  0 },
//    { INTEL_82810E_MC,  0 },
//    { INTEL_82815_MC,   0 },
//    { INTEL_82820_HB,   0 },
//    { INTEL_82820_UP_HB,0 },
//    { INTEL_82830_HB,   0 },
//    { INTEL_82840_HB,   0 },
//    { INTEL_82845_HB,   0 },
//    { INTEL_82845G_HB,  0 },
//    { INTEL_82850_HB,   0 },
//    { INTEL_82855PM_HB, 0 },
//    { INTEL_82855GM_HB, 0 },
//    { INTEL_82860_HB,   0 },
    { INTEL_82865_HB,   0 },
//    { INTEL_82875_HB,   0 },
//    { INTEL_7505_0,     0 },
//    { INTEL_7205_0,     0 },
//    { INTEL_82915G_HB,  0 },
//    { INTEL_82915GM_HB, 0 },
//    { INTEL_82945G_HB,  0 },
//    { INTEL_82945GM_HB, 0 },
    { 0, 0 }
};

pci_device_t* agp_dev_match(u32_t dev, pci_device_t *list)
{
  while(list->id)
  {
    if(dev == list->id)
      return list;
    list++;
  }
  return NULL;
}

int FindPciDevice()
{
    u32_t bus, last_bus;
    PCITAG tag;

    if( (last_bus = PciApi(1))==-1)
        return 0;

    for(bus=0;bus<=last_bus;bus++)
    {
        u32_t devfn;

        for(devfn=0;devfn<256;devfn++)
        {
            u32_t pciId;
            u8_t  devclass;
            pci_device_t *dev;

            pciId  = PciRead32(bus,devfn, 0);
            devclass = PciRead8(bus,devfn, 0x0B);

            if( devclass != PCI_CLASS_BRIDGE_HOST)
                continue;

            if( (dev = agp_dev_match(pciId, agp_dev_table))!=NULL)
            {
                dbgprintf("detect agp host %x\n",dev->id);

                PCITAG PciTag = pciTag(bus,(devfn>>3)&0x1F,devfn&0x7);

                return init_bridge(PciTag);
            };
        };
    };
    return 0;
};


