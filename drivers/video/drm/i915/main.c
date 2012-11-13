#include <drm/drmP.h>
#include <drm.h>
#include <drm/i915_drm.h>
#include "i915_drv.h"
//#include "intel_drv.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <errno-base.h>
#include <linux/pci.h>
#include <syscall.h>

#include "bitmap.h"

extern struct drm_device *main_device;

void cpu_detect();

void parse_cmdline(char *cmdline, char *log);
int _stdcall display_handler(ioctl_t *io);
int init_agp(void);

int blit_video(u32 hbitmap, int  dst_x, int dst_y,
               int src_x, int src_y, u32 w, u32 h);

int blit_textured(u32 hbitmap, int  dst_x, int dst_y,
               int src_x, int src_y, u32 w, u32 h);

int blit_tex(u32 hbitmap, int  dst_x, int dst_y,
             int src_x, int src_y, u32 w, u32 h);

static char  log[256];

int x86_clflush_size;

int i915_modeset = 1;

u32_t drvEntry(int action, char *cmdline)
{
    struct pci_device_id  *ent;

    int     err = 0;

    if(action != 1)
        return 0;

    if( GetService("DISPLAY") != 0 )
        return 0;

    if( cmdline && *cmdline )
        parse_cmdline(cmdline, log);

    if(!dbg_open(log))
    {
        strcpy(log, "/RD/1/DRIVERS/i915.log");
//        strcpy(log, "/BD1/2/i915.log");

        if(!dbg_open(log))
        {
            printf("Can't open %s\nExit\n", log);
            return 0;
        };
    }
    dbgprintf("i915 preview #08\n cmdline: %s\n", cmdline);

    cpu_detect();
    dbgprintf("\ncache line size %d\n", x86_clflush_size);

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

#define CURRENT_API     0x0200      /*      2.00     */
#define COMPATIBLE_API  0x0100      /*      1.00     */

#define API_VERSION     (COMPATIBLE_API << 16) | CURRENT_API
#define DISPLAY_VERSION  API_VERSION


#define SRV_GETVERSION          0
#define SRV_ENUM_MODES          1
#define SRV_SET_MODE            2
#define SRV_GET_CAPS            3

#define SRV_CREATE_SURFACE      10
#define SRV_DESTROY_SURFACE     11
#define SRV_LOCK_SURFACE        12
#define SRV_UNLOCK_SURFACE      13

#define SRV_BLIT_VIDEO          20

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
            *outp  = DISPLAY_VERSION;
            retval = 0;
            break;

        case SRV_ENUM_MODES:
//            dbgprintf("SRV_ENUM_MODES inp %x inp_size %x out_size %x\n",
//                       inp, io->inp_size, io->out_size );
            check_output(4);
//            check_input(*outp * sizeof(videomode_t));
            if( i915_modeset)
                retval = get_videomodes((videomode_t*)inp, outp);
            break;

        case SRV_SET_MODE:
//            dbgprintf("SRV_SET_MODE inp %x inp_size %x\n",
//                       inp, io->inp_size);
            check_input(sizeof(videomode_t));
            if( i915_modeset )
                retval = set_user_mode((videomode_t*)inp);
            break;

        case SRV_GET_CAPS:
            retval = get_driver_caps((hwcaps_t*)inp);
            break;

        case SRV_CREATE_SURFACE:
//            check_input(8);
            retval = create_surface(main_device, (struct io_call_10*)inp);
            break;

        case SRV_LOCK_SURFACE:
            retval = lock_surface((struct io_call_12*)inp);
            break;

        case SRV_BLIT_VIDEO:
            blit_video( inp[0], inp[1], inp[2],
                        inp[3], inp[4], inp[5], inp[6]);

//            blit_tex( inp[0], inp[1], inp[2],
//                    inp[3], inp[4], inp[5], inp[6]);


            retval = 0;
            break;

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


static char* parse_path(char *p, char *log)
{
    char  c;

    while( (c = *p++) == ' ');
        p--;
    while( (c = *log++ = *p++) && (c != ' '));
    *log = 0;

    return p;
};

void parse_cmdline(char *cmdline, char *log)
{
    char *p = cmdline;

    char c = *p++;

    while( c )
    {
        if( c == '-')
        {
            switch(*p++)
            {
                case 'l':
                    p = parse_path(p, log);
                    break;
            };
        };
        c = *p++;
    };
};


static inline void __cpuid(unsigned int *eax, unsigned int *ebx,
                unsigned int *ecx, unsigned int *edx)
{
    /* ecx is often an input as well as an output. */
    asm volatile("cpuid"
        : "=a" (*eax),
          "=b" (*ebx),
          "=c" (*ecx),
          "=d" (*edx)
        : "0" (*eax), "2" (*ecx)
        : "memory");
}



static inline void cpuid(unsigned int op,
                         unsigned int *eax, unsigned int *ebx,
                         unsigned int *ecx, unsigned int *edx)
{
        *eax = op;
        *ecx = 0;
        __cpuid(eax, ebx, ecx, edx);
}

void cpu_detect()
{
    u32 junk, tfms, cap0, misc;

    cpuid(0x00000001, &tfms, &misc, &junk, &cap0);

    if (cap0 & (1<<19))
    {
        x86_clflush_size = ((misc >> 8) & 0xff) * 8;
    }
}

