#include <drm/drmP.h>
#include <drm/drm_crtc_helper.h>
#include <drm/radeon_drm.h>
#include "radeon_reg.h"
#include "radeon.h"
#include "bitmap.h"

#define DRV_NAME "atikms v4.6.7"

void __init dmi_scan_machine(void);
int printf ( const char * format, ... );
void parse_cmdline(char *cmdline, videomode_t *mode, char *log, int *kms);
int kmap_init();

#define KMS_DEV_CLOSE 0
#define KMS_DEV_INIT  1
#define KMS_DEV_READY 2

struct drm_device *main_device;
struct drm_file   *drm_file_handlers[256];
int oops_in_progress;

videomode_t usermode;

void cpu_detect1();

int _stdcall display_handler(ioctl_t *io);
static char  log[256];

unsigned long volatile jiffies;
u64 jiffies_64;

struct workqueue_struct *system_wq;
int driver_wq_state;

int x86_clflush_size;

void ati_driver_thread()
{
    struct radeon_device *rdev = NULL;
    struct workqueue_struct *cwq = NULL;
//    static int dpms = 1;
//   static int dpms_lock = 0;
//    oskey_t   key;
    unsigned long irqflags;
    int tmp;

    printf("%s\n",__FUNCTION__);

    while(driver_wq_state == KMS_DEV_INIT)
    {
        jiffies_64 = GetClockNs() / 10000000;
        jiffies = (unsigned long)jiffies_64;

        delay(1);
    };

    rdev = main_device->dev_private;
//    cwq = rdev->wq;

    asm volatile("int $0x40":"=a"(tmp):"a"(66),"b"(1),"c"(1));
    asm volatile("int $0x40":"=a"(tmp):"a"(66),"b"(4),"c"(0x46),"d"(0x330));
    asm volatile("int $0x40":"=a"(tmp):"a"(66),"b"(4),"c"(0xC6),"d"(0x330));

    while(driver_wq_state != KMS_DEV_CLOSE)
    {
        jiffies = GetTimerTicks();
#if 0
        key = get_key();

        if( (key.val != 1) && (key.state == 0x02))
        {
            if(key.code == 0x46 && dpms_lock == 0)
            {
                dpms_lock = 1;
                if(dpms == 1)
                {
                    i915_dpms(main_device, DRM_MODE_DPMS_OFF);
                    printf("dpms off\n");
                }
                else
                {
                    i915_dpms(main_device, DRM_MODE_DPMS_ON);
                    printf("dpms on\n");
                };
                dpms ^= 1;
                }
            else if(key.code == 0xC6)
                dpms_lock = 0;
        };
        spin_lock_irqsave(&cwq->lock, irqflags);

        while (!list_empty(&cwq->worklist))
        {
            struct work_struct *work = list_entry(cwq->worklist.next,
                                        struct work_struct, entry);
            work_func_t f = work->func;
            list_del_init(cwq->worklist.next);

            spin_unlock_irqrestore(&cwq->lock, irqflags);
            f(work);
            spin_lock_irqsave(&cwq->lock, irqflags);
        }

        spin_unlock_irqrestore(&cwq->lock, irqflags);
#endif

        delay(1);
    };

    asm volatile ("int $0x40"::"a"(-1));
}

u32  __attribute__((externally_visible)) drvEntry(int action, char *cmdline)
{
    struct radeon_device *rdev = NULL;

    const struct pci_device_id  *ent;

    int err = 0;

    if(action != 1)
    {
        driver_wq_state = KMS_DEV_CLOSE;
        return 0;
    };

    if( GetService("DISPLAY") != 0 )
        return 0;

    printf("%s cmdline %s\n",DRV_NAME, cmdline);

    if( cmdline && *cmdline )
        parse_cmdline(cmdline, &usermode, log, &radeon_modeset);

    if( *log && !dbg_open(log))
    {
        printf("Can't open %s\nExit\n", log);
        return 0;
    }
    else
    {
        dbgprintf("\nLOG: %s build %s %s\n",DRV_NAME,__DATE__, __TIME__);
    }

    cpu_detect1();

    err = enum_pci_devices();
    if( unlikely(err != 0) )
    {
        dbgprintf("Device enumeration failed\n");
        return 0;
    }

    err = kmap_init();
    if( unlikely(err != 0) )
    {
        dbgprintf("kmap initialization failed\n");
        return 0;
    }

    dmi_scan_machine();


    driver_wq_state = KMS_DEV_INIT;
    CreateKernelThread(ati_driver_thread);

    err = ati_init();
    if(unlikely(err!= 0))
    {
        driver_wq_state = KMS_DEV_CLOSE;
        dbgprintf("Epic Fail :(\n");
        return 0;
    };

    driver_wq_state = KMS_DEV_READY;

    rdev = main_device->dev_private;
    printf("current engine clock: %u0 kHz\n", radeon_get_engine_clock(rdev));
    printf("current memory clock: %u0 kHz\n", radeon_get_memory_clock(rdev));

    err = RegService("DISPLAY", display_handler);

    if( err != 0)
        dbgprintf("DISPLAY service installed\n");


    return err;
};



#define CURRENT_API     0x0200      /*      2.00     */
#define COMPATIBLE_API  0x0100      /*      1.00     */

#define API_VERSION     (COMPATIBLE_API << 16) | CURRENT_API

#define SRV_GETVERSION          0
#define SRV_ENUM_MODES          1
#define SRV_SET_MODE            2
#define SRV_GET_CAPS            3

#define SRV_CREATE_SURFACE      10
#define SRV_DESTROY_SURFACE     11
#define SRV_LOCK_SURFACE        12
#define SRV_UNLOCK_SURFACE      13
#define SRV_RESIZE_SURFACE      14
#define SRV_BLIT_BITMAP         15
#define SRV_BLIT_TEXTURE        16
#define SRV_BLIT_VIDEO          17



int r600_video_blit(uint64_t src_offset, int  x, int y,
                    int w, int h, int pitch);

#define check_input(size) \
    if( unlikely((inp==NULL)||(io->inp_size != (size))) )   \
        break;

#define check_output(size) \
    if( unlikely((outp==NULL)||(io->out_size != (size))) )   \
        break;

int _stdcall display_handler(ioctl_t *io)
{
    int    retval = -1;
    u32 *inp;
    u32 *outp;

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
//            dbgprintf("SRV_ENUM_MODES inp %x inp_size %x out_size %x\n",
//                       inp, io->inp_size, io->out_size );
            check_output(4);
            if( radeon_modeset)
                retval = get_videomodes((videomode_t*)inp, outp);
            break;

        case SRV_SET_MODE:
//            dbgprintf("SRV_SET_MODE inp %x inp_size %x\n",
//                       inp, io->inp_size);
            check_input(sizeof(videomode_t));
            if( radeon_modeset )
                retval = set_user_mode((videomode_t*)inp);
            break;
/*
        case SRV_GET_CAPS:
            retval = get_driver_caps((hwcaps_t*)inp);
            break;

        case SRV_CREATE_SURFACE:
//            check_input(8);
            retval = create_surface(main_drm_device, (struct io_call_10*)inp);
            break;

        case SRV_LOCK_SURFACE:
            retval = lock_surface((struct io_call_12*)inp);
            break;

        case SRV_BLIT_BITMAP:
            srv_blit_bitmap( inp[0], inp[1], inp[2],
                        inp[3], inp[4], inp[5], inp[6]);
*/
    };

    return retval;
}


#define PCI_CLASS_REVISION      0x08
#define PCI_CLASS_DISPLAY_VGA   0x0300

int pci_scan_filter(u32 id, u32 busnr, u32 devfn)
{
    u16 vendor, device;
    u32 class;
    int ret = 0;

    vendor   = id & 0xffff;
    device   = (id >> 16) & 0xffff;

    if(vendor == 0x1002)
    {
        class = PciRead32(busnr, devfn, PCI_CLASS_REVISION);
        class >>= 24;

        if( class ==PCI_BASE_CLASS_DISPLAY)
            ret = 1;
    }
    return ret;
}


int seq_printf(struct seq_file *m, const char *f, ...)
{
//        int ret;
//        va_list args;

//        va_start(args, f);
//        ret = seq_vprintf(m, f, args);
//        va_end(args);

//        return ret;
    return 0;
}
