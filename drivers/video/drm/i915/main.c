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

struct pci_device {
    uint16_t    domain;
    uint8_t     bus;
    uint8_t     dev;
    uint8_t     func;
    uint16_t    vendor_id;
    uint16_t    device_id;
    uint16_t    subvendor_id;
    uint16_t    subdevice_id;
    uint32_t    device_class;
    uint8_t     revision;
};

struct drm_device *main_device;
struct drm_file   *drm_file_handlers[256];
videomode_t usermode;

void cpu_detect();

int _stdcall display_handler(ioctl_t *io);
int init_agp(void);

int srv_blit_bitmap(u32 hbitmap, int  dst_x, int dst_y,
               int src_x, int src_y, u32 w, u32 h);

int blit_textured(u32 hbitmap, int  dst_x, int dst_y,
               int src_x, int src_y, u32 w, u32 h);

int blit_tex(u32 hbitmap, int  dst_x, int dst_y,
             int src_x, int src_y, u32 w, u32 h);

void get_pci_info(struct pci_device *dev);
int i915_getparam(struct drm_device *dev, void *data,
             struct drm_file *file_priv);

int i915_mask_update(struct drm_device *dev, void *data,
            struct drm_file *file);

struct cmdtable cmdtable[]= {
    CMDENTRY("-pm=", i915_powersave),
    CMDENTRY("-rc6=", i915_enable_rc6),
    CMDENTRY("-fbc=", i915_enable_fbc),
    CMDENTRY("-ppgt=", i915_enable_ppgtt),
    CMDENTRY("-pc8=", i915_enable_pc8),
    {NULL, 0}
};


static char  log[256];

struct workqueue_struct *system_wq;
int driver_wq_state;

int x86_clflush_size;
unsigned int tsc_khz;

int i915_modeset = 1;

typedef union __attribute__((packed))
{
    uint32_t val;
    struct
    {
        uint8_t   state;
        uint8_t   code;
        uint16_t  ctrl_key;
    };
}oskey_t;

static inline oskey_t get_key(void)
{
    oskey_t val;
    asm volatile("int $0x40":"=a"(val):"a"(2));
    return val;
};

void i915_dpms(struct drm_device *dev, int mode);

void i915_driver_thread()
{
    struct drm_i915_private *dev_priv = main_device->dev_private;
    struct workqueue_struct *cwq = dev_priv->wq;
    static int dpms = 1;
    static int dpms_lock = 0;
    oskey_t   key;
    unsigned long irqflags;
    int tmp;

    printf("%s\n",__FUNCTION__);

    asm volatile("int $0x40":"=a"(tmp):"a"(66),"b"(1),"c"(1));
    asm volatile("int $0x40":"=a"(tmp):"a"(66),"b"(4),"c"(0x46),"d"(0x330));
    asm volatile("int $0x40":"=a"(tmp):"a"(66),"b"(4),"c"(0xC6),"d"(0x330));

    while(driver_wq_state != 0)
    {
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

        delay(1);
    };

    asm volatile ("int $0x40"::"a"(-1));
}

u32_t  __attribute__((externally_visible)) drvEntry(int action, char *cmdline)
{
    int err = 0;

    if(action != 1)
    {
        driver_wq_state = 0;
        return 0;
    };

    if( GetService("DISPLAY") != 0 )
        return 0;

    printf("i915 v3.12\n\nusage: i915 [options]\n"
           "-pm=<0,1>     Enable powersavings, fbc, downclocking, etc. (default: 0 - false)\n");
    printf("-rc6=<-1,0-7> Enable power-saving render C-state 6.\n"
           "              Different stages can be selected via bitmask values\n"
           "              (0 = disable; 1 = enable rc6; 2 = enable deep rc6; 4 = enable deepest rc6).\n"
           "              For example, 3 would enable rc6 and deep rc6, and 7 would enable everything.\n"
           "              default: -1 (use per-chip default)\n");
    printf("-fbc=<-1,0,1> Enable frame buffer compression for power savings\n"
           "              (default: 0 - false)\n");
    printf("-ppgt=<0,1>   Enable PPGTT (default: 0 - false)\n");
    printf("-pc8=<0,1>    Enable support for low power package C states (PC8+) (default: 0 - false)\n");
    printf("-l<path>      path to log file\n");
    printf("-m<WxHxHz>    set videomode\n");

    if( cmdline && *cmdline )
        parse_cmdline(cmdline, cmdtable, log, &usermode);

    if(!dbg_open(log))
    {
        strcpy(log, "/tmp1/1/i915.log");

        if(!dbg_open(log))
        {
            printf("Can't open %s\nExit\n", log);
            return 0;
        };
    }

    cpu_detect();
//    dbgprintf("\ncache line size %d\n", x86_clflush_size);

    enum_pci_devices();

    err = i915_init();
    if(err)
    {
        dbgprintf("Epic Fail :(\n");
        return 0;
    };

    init_display_kms(main_device, &usermode);

    err = RegService("DISPLAY", display_handler);

    if( err != 0)
        dbgprintf("Set DISPLAY handler\n");

    driver_wq_state = 1;

    CreateKernelThread(i915_driver_thread);

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
#define SRV_RESIZE_SURFACE      14
#define SRV_BLIT_BITMAP         15
#define SRV_BLIT_TEXTURE        16
#define SRV_BLIT_VIDEO          17


#define SRV_GET_PCI_INFO            20
#define SRV_I915_GET_PARAM              21
#define SRV_I915_GEM_CREATE         22
#define SRV_DRM_GEM_CLOSE           23
#define SRV_DRM_GEM_FLINK               24
#define SRV_DRM_GEM_OPEN                25
#define SRV_I915_GEM_PIN                26
#define SRV_I915_GEM_UNPIN              27
#define SRV_I915_GEM_SET_CACHING        28
#define SRV_I915_GEM_PWRITE             29
#define SRV_I915_GEM_BUSY               30
#define SRV_I915_GEM_SET_DOMAIN         31
#define SRV_I915_GEM_MMAP               32
#define SRV_I915_GEM_SET_TILING         33
#define SRV_I915_GEM_GET_TILING         34
#define SRV_I915_GEM_GET_APERTURE       35
#define SRV_I915_GEM_MMAP_GTT           36
#define SRV_I915_GEM_THROTTLE           37
#define SRV_I915_GEM_EXECBUFFER2        38
#define SRV_I915_GEM_WAIT               39
#define SRV_I915_GEM_CONTEXT_CREATE     40
#define SRV_I915_GEM_CONTEXT_DESTROY    41
#define SRV_I915_REG_READ               42

#define SRV_FBINFO                      43
#define SRV_MASK_UPDATE                 44


#define check_input(size) \
    if( unlikely((inp==NULL)||(io->inp_size != (size))) )   \
        break;

#define check_output(size) \
    if( unlikely((outp==NULL)||(io->out_size != (size))) )   \
        break;

int _stdcall display_handler(ioctl_t *io)
{
    struct drm_file *file;

    int    retval = -1;
    u32_t *inp;
    u32_t *outp;

    inp = io->input;
    outp = io->output;

    file = drm_file_handlers[0];

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
//            retval = create_surface(main_device, (struct io_call_10*)inp);
            break;

        case SRV_LOCK_SURFACE:
//            retval = lock_surface((struct io_call_12*)inp);
            break;

        case SRV_RESIZE_SURFACE:
//            retval = resize_surface((struct io_call_14*)inp);
            break;

        case SRV_BLIT_BITMAP:
//            srv_blit_bitmap( inp[0], inp[1], inp[2],
//                        inp[3], inp[4], inp[5], inp[6]);

//            blit_tex( inp[0], inp[1], inp[2],
//                    inp[3], inp[4], inp[5], inp[6]);

            break;

        case SRV_GET_PCI_INFO:
            get_pci_info((struct pci_device *)inp);
            retval = 0;
            break;

        case SRV_I915_GET_PARAM:
            retval = i915_getparam(main_device, inp, file);
            break;

        case SRV_I915_GEM_CREATE:
            retval = i915_gem_create_ioctl(main_device, inp, file);
            break;

        case SRV_DRM_GEM_CLOSE:
            retval = drm_gem_close_ioctl(main_device, inp, file);
            break;

        case SRV_DRM_GEM_FLINK:
            retval = drm_gem_flink_ioctl(main_device, inp, file);
            break;

        case SRV_DRM_GEM_OPEN:
            retval = drm_gem_open_ioctl(main_device, inp, file);
            break;

        case SRV_I915_GEM_PIN:
            retval = i915_gem_pin_ioctl(main_device, inp, file);
            break;

        case SRV_I915_GEM_UNPIN:
            retval = i915_gem_unpin_ioctl(main_device, inp, file);
            break;

        case SRV_I915_GEM_SET_CACHING:
            retval = i915_gem_set_caching_ioctl(main_device, inp, file);
            break;

        case SRV_I915_GEM_PWRITE:
            retval = i915_gem_pwrite_ioctl(main_device, inp, file);
            break;

        case SRV_I915_GEM_BUSY:
            retval = i915_gem_busy_ioctl(main_device, inp, file);
            break;

        case SRV_I915_GEM_SET_DOMAIN:
            retval = i915_gem_set_domain_ioctl(main_device, inp, file);
            break;

        case SRV_I915_GEM_MMAP:
            retval = i915_gem_mmap_ioctl(main_device, inp, file);
            break;

        case SRV_I915_GEM_SET_TILING:
            retval = i915_gem_set_tiling(main_device, inp, file);
            break;

        case SRV_I915_GEM_GET_TILING:
            retval = i915_gem_get_tiling(main_device, inp, file);
            break;

        case SRV_I915_GEM_GET_APERTURE:
//            printf("SRV_I915_GEM_GET_APERTURE ");
            retval = i915_gem_get_aperture_ioctl(main_device, inp, file);
//            printf(" retval=%d\n", retval);
            break;

        case SRV_I915_GEM_MMAP_GTT:
            retval = i915_gem_mmap_gtt_ioctl(main_device, inp, file);
            break;

        case SRV_I915_GEM_THROTTLE:
            retval = i915_gem_throttle_ioctl(main_device, inp, file);
            break;

        case SRV_I915_GEM_EXECBUFFER2:
//            printf("SRV_I915_GEM_EXECBUFFER2\n");
            retval = i915_gem_execbuffer2(main_device, inp, file);
            break;

        case SRV_I915_GEM_WAIT:
            retval = i915_gem_wait_ioctl(main_device, inp, file);
            break;

        case SRV_I915_GEM_CONTEXT_CREATE:
            retval = i915_gem_context_create_ioctl(main_device, inp, file);
            break;

        case SRV_I915_GEM_CONTEXT_DESTROY:
            retval = i915_gem_context_destroy_ioctl(main_device, inp, file);
            break;

        case SRV_I915_REG_READ:
            retval = i915_reg_read_ioctl(main_device, inp, file);
            break;

        case SRV_FBINFO:
            retval = i915_fbinfo(inp);
            break;

        case SRV_MASK_UPDATE:
            retval = i915_mask_update(main_device, inp, file);
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

    tsc_khz = (unsigned int)(GetCpuFreq()/1000);
}


int get_driver_caps(hwcaps_t *caps)
{
    int ret = 0;

    switch(caps->idx)
    {
        case 0:
            caps->opt[0] = 0;
            caps->opt[1] = 0;
            break;

        case 1:
            caps->cap1.max_tex_width  = 4096;
            caps->cap1.max_tex_height = 4096;
            break;
        default:
            ret = 1;
    };
    caps->idx = 1;
    return ret;
}


void get_pci_info(struct pci_device *dev)
{
    struct pci_dev *pdev = main_device->pdev;

    memset(dev, sizeof(*dev), 0);

    dev->domain     = 0;
    dev->bus        = pdev->busnr;
    dev->dev        = pdev->devfn >> 3;
    dev->func       = pdev->devfn & 7;
    dev->vendor_id  = pdev->vendor;
    dev->device_id  = pdev->device;
    dev->revision   = pdev->revision;
};



char *strstr(const char *cs, const char *ct);

static int my_atoi(char **cmd)
{
    char* p = *cmd;
    int val = 0;
    int sign = 1;

    if(*p == '-')
    {
        sign = -1;
        p++;
    };

    for (;; *p++) {
        switch (*p) {
        case '0' ... '9':
            val = 10*val+(*p-'0');
            break;
        default:
            *cmd = p;
            return val*sign;
        }
    }
}

char* parse_mode(char *p, videomode_t *mode)
{
    char c;

    while( (c = *p++) == ' ');

    if( c )
    {
        p--;

        mode->width = my_atoi(&p);
        if(*p == 'x') p++;

        mode->height = my_atoi(&p);
        if(*p == 'x') p++;

        mode->bpp = 32;

        mode->freq = my_atoi(&p);

        if( mode->freq == 0 )
            mode->freq = 60;
    }

    return p;
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

void parse_cmdline(char *cmdline, struct cmdtable *table, char *log, videomode_t *mode)
{
    char *p = cmdline;
    char *p1;
    int val;
    char c = *p++;

    if( table )
    {
        while(table->key)
        {
            if(p1 = strstr(cmdline, table->key))
            {
                p1+= table->size;
                *table->val = my_atoi(&p1);
//                printf("%s %d\n", table->key, *table->val);
            }
            table++;
        }
    }

    while( c )
    {
        if( c == '-')
        {
            switch(*p++)
            {
                case 'l':
                    p = parse_path(p, log);
                    break;

                case 'm':
                    p = parse_mode(p, mode);
                    break;
            };
        };
        c = *p++;
    };
};

char *strstr(const char *cs, const char *ct)
{
int d0, d1;
register char *__res;
__asm__ __volatile__(
    "movl %6,%%edi\n\t"
    "repne\n\t"
    "scasb\n\t"
    "notl %%ecx\n\t"
    "decl %%ecx\n\t"    /* NOTE! This also sets Z if searchstring='' */
    "movl %%ecx,%%edx\n"
    "1:\tmovl %6,%%edi\n\t"
    "movl %%esi,%%eax\n\t"
    "movl %%edx,%%ecx\n\t"
    "repe\n\t"
    "cmpsb\n\t"
    "je 2f\n\t"     /* also works for empty string, see above */
    "xchgl %%eax,%%esi\n\t"
    "incl %%esi\n\t"
    "cmpb $0,-1(%%eax)\n\t"
    "jne 1b\n\t"
    "xorl %%eax,%%eax\n\t"
    "2:"
    : "=a" (__res), "=&c" (d0), "=&S" (d1)
    : "0" (0), "1" (0xffffffff), "2" (cs), "g" (ct)
    : "dx", "di");
return __res;
}
