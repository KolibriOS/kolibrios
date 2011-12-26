#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <errno-base.h>
#include <linux/pci.h>
#include <syscall.h>

int init_agp(void);

static char  log[256];

u32_t drvEntry(int action, char *cmdline)
{
    struct pci_device_id  *ent;

    int     err = 0;

    if(action != 1)
        return 0;

    if( GetService("DISPLAY") != 0 )
        return 0;

//    if( cmdline && *cmdline )
//        parse_cmdline(cmdline, &usermode, log, &radeon_modeset);

    if(!dbg_open(log))
    {
        strcpy(log, "/HD1/2/i915.log");

        if(!dbg_open(log))
        {
            printf("Can't open %s\nExit\n", log);
            return 0;
        };
    }
    dbgprintf("i915 RC01 cmdline %s\n", cmdline);

    enum_pci_devices();

    err = i915_init();

//    rdev = rdisplay->ddev->dev_private;

//    err = RegService("DISPLAY", display_handler);

//    if( err != 0)
//        dbgprintf("Set DISPLAY handler\n");

    return err;
};

#define PCI_CLASS_REVISION      0x08
#define PCI_CLASS_DISPLAY_VGA   0x0300
#define PCI_CLASS_BRIDGE_HOST   0x0600
#define PCI_CLASS_BRIDGE_ISA    0x0601

int pci_scan_filter(u32_t id, u32_t busnr, u32_t devfn)
{
    u16_t vendor, device;
    u32_t class;
    int   ret = 0;

    vendor   = id & 0xffff;
    device   = (id >> 16) & 0xffff;

    if(vendor == 0x8086)
    {
        class = PciRead32(busnr, devfn, PCI_CLASS_REVISION);
        class >>= 16;

        if( (class == PCI_CLASS_DISPLAY_VGA) ||
            (class == PCI_CLASS_BRIDGE_HOST) ||
            (class == PCI_CLASS_BRIDGE_ISA))
            ret = 1;
    }
    return ret;
};
