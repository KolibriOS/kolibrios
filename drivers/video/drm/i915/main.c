#include <syscall.h>

#include <drm/drmP.h>
#include <drm/i915_drm.h>
#include "i915_drv.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/pci.h>

#include "bitmap.h"

#define DRV_NAME "i915 v4.4"

#define I915_DEV_CLOSE 0
#define I915_DEV_INIT  1
#define I915_DEV_READY 2


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

struct cmdtable
{
    char *key;
    int   size;
    int  *val;
};

#define CMDENTRY(key, val) {(key), (sizeof(key)-1), &val}
void parse_cmdline(char *cmdline, struct cmdtable *table, char *log, videomode_t *mode);


int oops_in_progress;
int i915_fbsize = 16;
struct drm_device *main_device;
struct drm_file   *drm_file_handlers[256];
videomode_t usermode;

void cpu_detect1();
int kmap_init();

int _stdcall display_handler(ioctl_t *io);
int init_agp(void);

void get_pci_info(struct pci_device *dev);
int i915_getparam(struct drm_device *dev, void *data,
             struct drm_file *file_priv);

int i915_mask_update(struct drm_device *dev, void *data,
            struct drm_file *file);

struct cmdtable cmdtable[]= {
    CMDENTRY("-FB=", i915_fbsize),
/*    CMDENTRY("-pm=", i915.powersave),     */
    CMDENTRY("-rc6=", i915.enable_rc6),
    CMDENTRY("-fbc=", i915.enable_fbc),
    CMDENTRY("-ppgt=", i915.enable_ppgtt),
    {NULL, 0}
};


static char  log[256];

unsigned long volatile jiffies;

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
    struct drm_i915_private *dev_priv = NULL;
    struct workqueue_struct *cwq = NULL;
    static int dpms = 1;
    static int dpms_lock = 0;
    oskey_t   key;
    unsigned long irqflags;
    int tmp;

    printf("%s\n",__FUNCTION__);

    while(driver_wq_state == I915_DEV_INIT)
    {
        jiffies = GetClockNs() / 10000000;
        delay(1);
    };

    if( driver_wq_state == I915_DEV_CLOSE)
    {
        asm volatile ("int $0x40"::"a"(-1));
    };

    dev_priv = main_device->dev_private;
    cwq = dev_priv->wq;

    asm volatile("int $0x40":"=a"(tmp):"a"(66),"b"(1),"c"(1));
    asm volatile("int $0x40":"=a"(tmp):"a"(66),"b"(4),"c"(0x46),"d"(0x330));
    asm volatile("int $0x40":"=a"(tmp):"a"(66),"b"(4),"c"(0xC6),"d"(0x330));

    while(driver_wq_state != I915_DEV_CLOSE)
    {
        jiffies = GetClockNs() / 10000000;

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

u32  __attribute__((externally_visible)) drvEntry(int action, char *cmdline)
{
    static pci_dev_t device;
    const struct pci_device_id  *ent;

    int err = 0;

    if(action != 1)
    {
        driver_wq_state = I915_DEV_CLOSE;
        return 0;
    };

    if( GetService("DISPLAY") != 0 )
        return 0;

    printf("\n%s build %s %s\nusage: i915 [options]\n"
           "-FB=<0-9>     Set framebuffer size in megabytes (default: 16)\n",
           "-pm=<0,1>     Enable powersavings, fbc, downclocking, etc. (default: 1 - true)\n",
           DRV_NAME, __DATE__, __TIME__);

    printf("-rc6=<-1,0-7> Enable power-saving render C-state 6.\n"
           "              Different stages can be selected via bitmask values\n"
           "              (0 = disable; 1 = enable rc6; 2 = enable deep rc6; 4 = enable deepest rc6).\n"
           "              For example, 3 would enable rc6 and deep rc6, and 7 would enable everything.\n"
           "              default: -1 (use per-chip default)\n");
    printf("-fbc=<-1,0,1> Enable frame buffer compression for power savings\n"
           "              (default: -1 (use per-chip default))\n");
    printf("-ppgt=<0,1>   Enable PPGTT (default: true)\n");

    printf("-l<path>      path to log file\n");
    printf("-m<WxHxHz>    set videomode\n");

    printf("cmdline %s\n", cmdline);
    if( cmdline && *cmdline )
        parse_cmdline(cmdline, cmdtable, log, &usermode);

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
//    dbgprintf("\ncache line size %d\n", x86_clflush_size);

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

    driver_wq_state = I915_DEV_INIT;
    CreateKernelThread(i915_driver_thread);

    err = i915_init();
    if(unlikely(err!= 0))
    {
        driver_wq_state = I915_DEV_CLOSE;
        dbgprintf("Epic Fail :(\n");
        delay(100);
        return 0;
    };

    driver_wq_state = I915_DEV_READY;

    init_display_kms(main_device, &usermode);

    err = RegService("DISPLAY", display_handler);

    if( err != 0)
        dbgprintf("Set DISPLAY handler\n");

    return err;
};


#define CURRENT_API     0x0200      /*      2.00     */
#define COMPATIBLE_API  0x0100      /*      1.00     */

#define API_VERSION     (COMPATIBLE_API << 16) | CURRENT_API
#define DISPLAY_VERSION  API_VERSION


#define SRV_GETVERSION              0
#define SRV_ENUM_MODES              1
#define SRV_SET_MODE                2
#define SRV_GET_CAPS                3


#define SRV_GET_PCI_INFO                20
#define SRV_I915_GET_PARAM              21
#define SRV_I915_GEM_CREATE             22
#define SRV_DRM_GEM_CLOSE               23
#define SRV_DRM_GEM_FLINK               24
#define SRV_DRM_GEM_OPEN                25
#define SRV_I915_GEM_PIN                26
#define SRV_I915_GEM_UNPIN              27
#define SRV_I915_GEM_GET_CACHING        28
#define SRV_I915_GEM_SET_CACHING        29
#define SRV_I915_GEM_PWRITE             30
#define SRV_I915_GEM_BUSY               31
#define SRV_I915_GEM_SET_DOMAIN         32
#define SRV_I915_GEM_MMAP               33
#define SRV_I915_GEM_SET_TILING         34
#define SRV_I915_GEM_GET_TILING         35
#define SRV_I915_GEM_GET_APERTURE       36
#define SRV_I915_GEM_MMAP_GTT           37
#define SRV_I915_GEM_THROTTLE           38
#define SRV_I915_GEM_EXECBUFFER2        39
#define SRV_I915_GEM_WAIT               40
#define SRV_I915_GEM_CONTEXT_CREATE     41
#define SRV_I915_GEM_CONTEXT_DESTROY    42
#define SRV_I915_REG_READ               43

#define SRV_FBINFO                      44
#define SRV_MASK_UPDATE                 45
#define SRV_MASK_UPDATE_EX              46

#define check_input(size) \
    if( unlikely((inp==NULL)||(io->inp_size != (size))) )   \
        break;

#define check_output(size) \
    if( unlikely((outp==NULL)||(io->out_size != (size))) )   \
        break;

int _stdcall display_handler(ioctl_t *io)
{
    struct drm_file *file;

    int  retval = -1;
    u32 *inp;
    u32 *outp;

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

        case SRV_I915_GEM_GET_CACHING:
            retval = i915_gem_get_caching_ioctl(main_device, inp, file);
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

        case SRV_MASK_UPDATE_EX:
            retval = i915_mask_update_ex(main_device, inp, file);
            break;
    };

    return retval;
}


#define PCI_CLASS_REVISION      0x08
#define PCI_CLASS_DISPLAY_VGA   0x0300
#define PCI_CLASS_BRIDGE_HOST   0x0600
#define PCI_CLASS_BRIDGE_ISA    0x0601

int pci_scan_filter(u32 id, u32 busnr, u32 devfn)
{
    u16 vendor, device;
    u32 class;
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


struct mtrr
{
    u64  base;
    u64  mask;
};

struct cpuinfo
{
    u64  caps;
    u64  def_mtrr;
    u64  mtrr_cap;
    int    var_mtrr_count;
    int    fix_mtrr_count;
    struct mtrr var_mtrr[9];
    char   model_name[64];
};

#define MTRRphysBase_MSR(reg) (0x200 + 2 * (reg))
#define MTRRphysMask_MSR(reg) (0x200 + 2 * (reg) + 1)

#define MSR_MTRRdefType                 0x000002ff

#define IA32_MTRRCAP            0xFE
#define IA32_CR_PAT_MSR         0x277

#define PAT_TYPE_UC             0
#define PAT_TYPE_WC             1
#define PAT_TYPE_WB             6
#define PAT_TYPE_UCM            7


#define MTRR_UC                 0
#define MTRR_WC                 1
#define MTRR_WB                 6

static inline u64 read_msr(u32 msr)
{
    union {
        u64  val;
        struct {
            u32 low;
            u32 high;
        };
    }tmp;

    asm volatile (
    "rdmsr"
    : "=a" (tmp.low), "=d" (tmp.high)
    : "c" (msr));
    return tmp.val;
}

static inline void write_msr(u32 msr, u64 val)
{
    union {
        u64  val;
        struct {
            u32 low;
            u32 high;
        };
    }tmp;

    tmp.val = val;

    asm volatile (
    "wrmsr"
    :: "a" (tmp.low), "d" (tmp.high), "c" (msr));
}

#define SIZE_OR_MASK_BITS(n)  (~((1ULL << ((n) - PAGE_SHIFT)) - 1))

static void set_mtrr(unsigned int reg, unsigned long base,
                 unsigned long size, int type)
{
    unsigned int base_lo, base_hi, mask_lo, mask_hi;
    u64 size_or_mask, size_and_mask;

    size_or_mask = SIZE_OR_MASK_BITS(36);
    size_and_mask = 0x00f00000;

    if (size == 0) {
        /*
         * The invalid bit is kept in the mask, so we simply
         * clear the relevant mask register to disable a range.
         */
        native_write_msr(MTRRphysMask_MSR(reg), 0, 0);
    }
    else {
        base_lo = base << PAGE_SHIFT | type;
        base_hi = (base & size_and_mask) >> (32 - PAGE_SHIFT);
        mask_lo = -size << PAGE_SHIFT | 0x800;
        mask_hi = (-size & size_and_mask) >> (32 - PAGE_SHIFT);

        native_write_msr(MTRRphysBase_MSR(reg), base_lo, base_hi);
        native_write_msr(MTRRphysMask_MSR(reg), mask_lo, mask_hi);
    };
}


static u32 deftype_lo, deftype_hi;

void cpu_detect1()
{
    struct cpuinfo cpuinfo;

    u32 junk, tfms, cap0, misc;
    int i;

    cpuid(0x00000001, &tfms, &misc, &junk, &cap0);

    if (cap0 & (1<<19))
    {
        x86_clflush_size = ((misc >> 8) & 0xff) * 8;
    }

#if 0
    cpuid(0x80000002, (unsigned int*)&cpuinfo.model_name[0], (unsigned int*)&cpuinfo.model_name[4],
          (unsigned int*)&cpuinfo.model_name[8], (unsigned int*)&cpuinfo.model_name[12]);
    cpuid(0x80000003, (unsigned int*)&cpuinfo.model_name[16], (unsigned int*)&cpuinfo.model_name[20],
          (unsigned int*)&cpuinfo.model_name[24], (unsigned int*)&cpuinfo.model_name[28]);
    cpuid(0x80000004, (unsigned int*)&cpuinfo.model_name[32], (unsigned int*)&cpuinfo.model_name[36],
          (unsigned int*)&cpuinfo.model_name[40], (unsigned int*)&cpuinfo.model_name[44]);

    printf("\n%s\n\n",cpuinfo.model_name);

    cpuinfo.def_mtrr = read_msr(MSR_MTRRdefType);
    cpuinfo.mtrr_cap = read_msr(IA32_MTRRCAP);

    printf("MSR_MTRRdefType %016llx\n\n", cpuinfo.def_mtrr);

    cpuinfo.var_mtrr_count = (u8_t)cpuinfo.mtrr_cap;

    for(i = 0; i < cpuinfo.var_mtrr_count; i++)
    {
        u64_t mtrr_base;
        u64_t mtrr_mask;

        cpuinfo.var_mtrr[i].base = read_msr(MTRRphysBase_MSR(i));
        cpuinfo.var_mtrr[i].mask = read_msr(MTRRphysMask_MSR(i));

        printf("MTRR_%d base: %016llx mask: %016llx\n", i,
               cpuinfo.var_mtrr[i].base,
               cpuinfo.var_mtrr[i].mask);
    };

    unsigned int cr0, cr3, cr4, eflags;

    eflags = safe_cli();

    /* Enter the no-fill (CD=1, NW=0) cache mode and flush caches. */
    cr0 = read_cr0() | (1<<30);
    write_cr0(cr0);
    wbinvd();

    cr4 = read_cr4();
    write_cr4(cr4 & ~(1<<7));

    cr3 = read_cr3();
    write_cr3(cr3);

    /* Save MTRR state */
    rdmsr(MSR_MTRRdefType, deftype_lo, deftype_hi);

    /* Disable MTRRs, and set the default type to uncached */
    native_write_msr(MSR_MTRRdefType, deftype_lo & ~0xcff, deftype_hi);
    wbinvd();

    i = 0;
    set_mtrr(i++,0,0x80000000>>12,MTRR_WB);
    set_mtrr(i++,0x80000000>>12,0x40000000>>12,MTRR_WB);
    set_mtrr(i++,0xC0000000>>12,0x20000000>>12,MTRR_WB);
    set_mtrr(i++,0xdb800000>>12,0x00800000>>12,MTRR_UC);
    set_mtrr(i++,0xdc000000>>12,0x04000000>>12,MTRR_UC);
    set_mtrr(i++,0xE0000000>>12,0x10000000>>12,MTRR_WC);

    for(; i < cpuinfo.var_mtrr_count; i++)
        set_mtrr(i,0,0,0);

    write_cr3(cr3);

    /* Intel (P6) standard MTRRs */
    native_write_msr(MSR_MTRRdefType, deftype_lo, deftype_hi);

    /* Enable caches */
    write_cr0(read_cr0() & ~(1<<30));

    /* Restore value of CR4 */
    write_cr4(cr4);

    safe_sti(eflags);

    printf("\nnew MTRR map\n\n");

    for(i = 0; i < cpuinfo.var_mtrr_count; i++)
    {
        u64_t mtrr_base;
        u64_t mtrr_mask;

        cpuinfo.var_mtrr[i].base = read_msr(MTRRphysBase_MSR(i));
        cpuinfo.var_mtrr[i].mask = read_msr(MTRRphysMask_MSR(i));

        printf("MTRR_%d base: %016llx mask: %016llx\n", i,
               cpuinfo.var_mtrr[i].base,
               cpuinfo.var_mtrr[i].mask);
    };
#endif

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
    while((c = *p++) && (c != ' '))
        *log++ = c;

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

#include <linux/math64.h>

u64 long_div(u64 dividend, u64 divisor)
{
#if 1
    u32 high = divisor >> 32;
    u64 quot;

    if (high == 0) {
            quot = div_u64(dividend, divisor);
    } else {
            int n = 1 + fls(high);
            quot = div_u64(dividend >> n, divisor >> n);

            if (quot != 0)
                    quot--;
            if ((dividend - quot * divisor) >= divisor)
                    quot++;
    }

    return quot;
#endif
//    return dividend / divisor;
};




