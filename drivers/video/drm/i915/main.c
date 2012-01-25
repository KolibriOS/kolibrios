#include "drmP.h"
#include "drm.h"
#include "i915_drm.h"
#include "i915_drv.h"
#include "intel_drv.h"


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <errno-base.h>
#include <linux/pci.h>
#include <syscall.h>

int _stdcall display_handler(ioctl_t *io);
int init_agp(void);

static char  log[256];

int i915_modeset = 1;

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

    if(err)
    {
        dbgprintf("Epic Fail :(/n");

    };

    err = RegService("DISPLAY", display_handler);

    if( err != 0)
        dbgprintf("Set DISPLAY handler\n");

    return err;
};

#define API_VERSION     0x01000100

#define SRV_GETVERSION      0
#define SRV_ENUM_MODES      1
#define SRV_SET_MODE        2

#define SRV_CREATE_VIDEO    9
#define SRV_BLIT_VIDEO     10
#define SRV_CREATE_BITMAP  11

#define check_input(size) \
    if( unlikely((inp==NULL)||(io->inp_size != (size))) )   \
        break;

#define check_output(size) \
    if( unlikely((outp==NULL)||(io->out_size != (size))) )   \
        break;

int _stdcall display_handler(ioctl_t *io)
{
    int    retval = -1;
    u32_t *inp;
    u32_t *outp;

    inp = io->input;
    outp = io->output;

    switch(io->io_code)
    {
        case SRV_GETVERSION:
            check_output(4);
            *outp  = API_VERSION;
            retval = 0;
            break;

        case SRV_ENUM_MODES:
            dbgprintf("SRV_ENUM_MODES inp %x inp_size %x out_size %x\n",
                       inp, io->inp_size, io->out_size );
//            check_output(4);
//            check_input(*outp * sizeof(videomode_t));
            if( i915_modeset)
                retval = get_videomodes((videomode_t*)inp, outp);
            break;

        case SRV_SET_MODE:
            dbgprintf("SRV_SET_MODE inp %x inp_size %x\n",
                       inp, io->inp_size);
            check_input(sizeof(videomode_t));
            if( i915_modeset )
                retval = set_user_mode((videomode_t*)inp);
            break;

/*
        case SRV_CREATE_VIDEO:
            retval = r600_create_video(inp[0], inp[1], outp);
            break;

        case SRV_BLIT_VIDEO:
            r600_video_blit( ((uint64_t*)inp)[0], inp[2], inp[3],
                    inp[4], inp[5], inp[6]);

            retval = 0;
            break;

        case SRV_CREATE_BITMAP:
            check_input(8);
            check_output(4);
            retval = create_bitmap(outp, inp[0], inp[1]);
            break;
*/
    };

    return retval;
}


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
