

bool FindUSBControllers()
{
    bool retval = false;
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
            u8_t  interface;
            int i;

            interface = PciRead8(bus,devfn, 0x09);
            devclass = PciRead16(bus,devfn, 0x0A);
            if( devclass != 0x0C03)
                continue;

            if( interface != 0)
                continue;

            pcicmd = PciRead16(bus,devfn, PCI_COMMAND);
            if (! pcicmd & PCI_COMMAND_IO)
                continue;

            hc = (hc_t*)kmalloc(sizeof(hc_t), 0);
            INIT_LIST_HEAD(&hc->list);

            hc->pciId  = PciRead32(bus,devfn, 0);
            hc->PciTag = pciTag(bus,(devfn>>3)&0x1F,devfn&0x7);

            for (i = 0; i < 6; i++)
            {
                u32_t base;
                bool validSize;

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
            list_add_tail(&hc->list, &hc_list);
            retval = true;
        };
    };
    return retval;
};


#if 0

/* these helpers provide future and backwards compatibility
 * for accessing popular PCI BAR info */
#define pci_resource_start(dev, bar)    ((dev)->resource[(bar)].start)
#define pci_resource_end(dev, bar)  ((dev)->resource[(bar)].end)
#define pci_resource_flags(dev, bar)    ((dev)->resource[(bar)].flags)
#define pci_resource_len(dev,bar) \
    ((pci_resource_start((dev), (bar)) == 0 &&  \
      pci_resource_end((dev), (bar)) ==     \
      pci_resource_start((dev), (bar))) ? 0 :   \
                            \
     (pci_resource_end((dev), (bar)) -      \
      pci_resource_start((dev), (bar)) + 1))

static int __devinit mmio_resource_enabled(struct pci_dev *pdev, int idx)
{
    return pci_resource_start(pdev, idx) && mmio_enabled(pdev);
}

static void __devinit quirk_usb_disable_ehci(struct pci_dev *pdev)
{
    int wait_time, delta;
    void __iomem *base, *op_reg_base;
    u32 hcc_params, val;
    u8  offset, cap_length;
    int count = 256/4;
    int tried_handoff = 0;

    if (!mmio_resource_enabled(pdev, 0))
        return;

    base = pci_ioremap_bar(pdev, 0);
    if (base == NULL)
        return;

    cap_length = readb(base);
    op_reg_base = base + cap_length;

    /* EHCI 0.96 and later may have "extended capabilities"
     * spec section 5.1 explains the bios handoff, e.g. for
     * booting from USB disk or using a usb keyboard
     */
    hcc_params = readl(base + EHCI_HCC_PARAMS);
    offset = (hcc_params >> 8) & 0xff;
    while (offset && --count) {
        u32     cap;
        int     msec;

        pci_read_config_dword(pdev, offset, &cap);
        switch (cap & 0xff) {
        case 1:         /* BIOS/SMM/... handoff support */
            if ((cap & EHCI_USBLEGSUP_BIOS)) {
                dev_dbg(&pdev->dev, "EHCI: BIOS handoff\n");

#if 0
/* aleksey_gorelov@phoenix.com reports that some systems need SMI forced on,
 * but that seems dubious in general (the BIOS left it off intentionally)
 * and is known to prevent some systems from booting.  so we won't do this
 * unless maybe we can determine when we're on a system that needs SMI forced.
 */
                /* BIOS workaround (?): be sure the
                 * pre-Linux code receives the SMI
                 */
                pci_read_config_dword(pdev,
                        offset + EHCI_USBLEGCTLSTS,
                        &val);
                pci_write_config_dword(pdev,
                        offset + EHCI_USBLEGCTLSTS,
                        val | EHCI_USBLEGCTLSTS_SOOE);
#endif

                /* some systems get upset if this semaphore is
                 * set for any other reason than forcing a BIOS
                 * handoff..
                 */
                pci_write_config_byte(pdev, offset + 3, 1);
            }

            /* if boot firmware now owns EHCI, spin till
             * it hands it over.
             */
            msec = 1000;
            while ((cap & EHCI_USBLEGSUP_BIOS) && (msec > 0)) {
                tried_handoff = 1;
                msleep(10);
                msec -= 10;
                pci_read_config_dword(pdev, offset, &cap);
            }

            if (cap & EHCI_USBLEGSUP_BIOS) {
                /* well, possibly buggy BIOS... try to shut
                 * it down, and hope nothing goes too wrong
                 */
                dev_warn(&pdev->dev, "EHCI: BIOS handoff failed"
                        " (BIOS bug?) %08x\n", cap);
                pci_write_config_byte(pdev, offset + 2, 0);
            }

            /* just in case, always disable EHCI SMIs */
            pci_write_config_dword(pdev,
                    offset + EHCI_USBLEGCTLSTS,
                    0);

            /* If the BIOS ever owned the controller then we
             * can't expect any power sessions to remain intact.
             */
            if (tried_handoff)
                writel(0, op_reg_base + EHCI_CONFIGFLAG);
            break;
        case 0:         /* illegal reserved capability */
            cap = 0;
            /* FALLTHROUGH */
        default:
            dev_warn(&pdev->dev, "EHCI: unrecognized capability "
                    "%02x\n", cap & 0xff);
            break;
        }
        offset = (cap >> 8) & 0xff;
    }
    if (!count)
        dev_printk(KERN_DEBUG, &pdev->dev, "EHCI: capability loop?\n");

    /*
     * halt EHCI & disable its interrupts in any case
     */
    val = readl(op_reg_base + EHCI_USBSTS);
    if ((val & EHCI_USBSTS_HALTED) == 0) {
        val = readl(op_reg_base + EHCI_USBCMD);
        val &= ~EHCI_USBCMD_RUN;
        writel(val, op_reg_base + EHCI_USBCMD);

        wait_time = 2000;
        delta = 100;
        do {
            writel(0x3f, op_reg_base + EHCI_USBSTS);
            udelay(delta);
            wait_time -= delta;
            val = readl(op_reg_base + EHCI_USBSTS);
            if ((val == ~(u32)0) || (val & EHCI_USBSTS_HALTED)) {
                break;
            }
        } while (wait_time > 0);
    }
    writel(0, op_reg_base + EHCI_USBINTR);
    writel(0x3f, op_reg_base + EHCI_USBSTS);

    iounmap(base);

    return;
}

#endif
