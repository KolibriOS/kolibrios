#include <syscall.h>

#include <drm/drmP.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/pci.h>

#include "vmwgfx_drv.h"

#include <display.h>

#define VMW_DEV_CLOSE 0
#define VMW_DEV_INIT  1
#define VMW_DEV_READY 2
void cpu_detect1();
int kmap_init();

unsigned long volatile jiffies;
int oops_in_progress;
int x86_clflush_size;
unsigned int tsc_khz;
struct workqueue_struct *system_wq;
int driver_wq_state;
struct drm_device *main_device;
struct drm_file   *drm_file_handlers[256];
int kms_modeset = 1;
static char  log[256];

int vmw_init(void);
int kms_init(struct drm_device *dev);
void vmw_driver_thread();

void parse_cmdline(char *cmdline, char *log);
int _stdcall display_handler(ioctl_t *io);
void kms_update();
void vmw_fb_update(struct vmw_private *vmw_priv);

int gem_getparam(struct drm_device *dev, void *data);

void vmw_driver_thread()
{
	struct vmw_private *dev_priv = NULL;
    struct workqueue_struct *cwq = NULL;
    unsigned long irqflags;

    printf("%s\n",__FUNCTION__);

    while(driver_wq_state == VMW_DEV_INIT)
    {
        jiffies = GetClockNs() / 10000000;
        delay(1);
    };

    if( driver_wq_state == VMW_DEV_CLOSE)
    {
        asm volatile ("int $0x40"::"a"(-1));
    };

    dev_priv = main_device->dev_private;
    cwq = system_wq;

    while(driver_wq_state != VMW_DEV_CLOSE )
    {
        jiffies = GetClockNs() / 10000000;

 //       kms_update();

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

        vmw_fb_update(dev_priv);
        delay(2);
    };

    asm volatile ("int $0x40"::"a"(-1));
}

u32  __attribute__((externally_visible)) drvEntry(int action, char *cmdline)
{
    static pci_dev_t device;
    const struct pci_device_id  *ent;
    char *safecmdline;
    int     err = 0;

    if(action != 1)
    {
        driver_wq_state = VMW_DEV_CLOSE;
        return 0;
    };

    if( GetService("DISPLAY") != 0 )
        return 0;

    if( cmdline && *cmdline )
        parse_cmdline(cmdline, log);

    if( *log && !dbg_open(log))
    {
            printf("Can't open %s\nExit\n", log);
            return 0;
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

    driver_wq_state = VMW_DEV_INIT;
    CreateKernelThread(vmw_driver_thread);
    err = vmw_init();
    if(unlikely(err!= 0))
    {
        driver_wq_state = VMW_DEV_CLOSE;
        dbgprintf("Epic Fail :(\n");
        delay(100);
        return 0;
    };
LINE();

    driver_wq_state = VMW_DEV_READY;

//    kms_init(main_device);

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
#define SRV_CMDLINE                 4

#define SRV_GET_PCI_INFO            20

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
 //           dbgprintf("SRV_ENUM_MODES inp %x inp_size %x out_size %x\n",
 //                      inp, io->inp_size, io->out_size );
 //           check_output(4);
//            check_input(*outp * sizeof(videomode_t));
            if( kms_modeset)
                retval = get_videomodes((videomode_t*)inp, outp);
            break;

        case SRV_SET_MODE:
//            dbgprintf("SRV_SET_MODE inp %x inp_size %x\n",
//                       inp, io->inp_size);
//            check_input(sizeof(videomode_t));
            if( kms_modeset )
                retval = set_user_mode((videomode_t*)inp);
            break;

#if 0
        case SRV_GET_CAPS:
            retval = get_driver_caps((hwcaps_t*)inp);
            break;


        case SRV_GET_PCI_INFO:
            get_pci_info((struct pci_device *)inp);
            retval = 0;
            break;

        case SRV_GET_PARAM:
            retval = gem_getparam(main_device, inp);
            break;

        case SRV_I915_GEM_CREATE:
            retval = i915_gem_create_ioctl(main_device, inp, file);
            break;

        case SRV_DRM_GEM_CLOSE:
            retval = drm_gem_close_ioctl(main_device, inp, file);
            break;

        case SRV_I915_GEM_PIN:
            retval = i915_gem_pin_ioctl(main_device, inp, file);
            break;

        case SRV_I915_GEM_SET_CACHEING:
            retval = i915_gem_set_caching_ioctl(main_device, inp, file);
            break;

        case SRV_I915_GEM_GET_APERTURE:
            retval = i915_gem_get_aperture_ioctl(main_device, inp, file);
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

        case SRV_I915_GEM_THROTTLE:
            retval = i915_gem_throttle_ioctl(main_device, inp, file);
            break;

        case SRV_I915_GEM_MMAP:
            retval = i915_gem_mmap_ioctl(main_device, inp, file);
            break;

        case SRV_I915_GEM_MMAP_GTT:
            retval = i915_gem_mmap_gtt_ioctl(main_device, inp, file);
            break;


        case SRV_FBINFO:
            retval = i915_fbinfo(inp);
            break;

        case SRV_I915_GEM_EXECBUFFER2:
            retval = i915_gem_execbuffer2(main_device, inp, file);
            break;

        case SRV_MASK_UPDATE:
            retval = i915_mask_update(main_device, inp, file);
            break;
#endif

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

    if(vendor == 0x15AD )
    {
        class = PciRead32(busnr, devfn, PCI_CLASS_REVISION);
        class >>= 16;

        if( class == PCI_CLASS_DISPLAY_VGA )
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

    tsc_khz = (unsigned int)(GetCpuFreq()/1000);
}

/*
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

*/

#include <ddk.h>
#include <linux/mm.h>
#include <drm/drmP.h>
#include <linux/ctype.h>




#include "vmwgfx_kms.h"

void kms_update();


extern struct drm_device *main_device;

#define CURSOR_WIDTH 64
#define CURSOR_HEIGHT 64


display_t *os_display;

static int count_connector_modes(struct drm_connector* connector)
{
    struct drm_display_mode  *mode;
    int count = 0;

    list_for_each_entry(mode, &connector->modes, head)
    {
        count++;
    };
    return count;
};

static void __stdcall restore_cursor(int x, int y){};
static void disable_mouse(void) {};

static void __stdcall move_cursor_kms(cursor_t *cursor, int x, int y)
{
    struct drm_crtc *crtc = os_display->crtc;
    struct vmw_private *dev_priv = vmw_priv(crtc->dev);
    struct vmw_display_unit *du = vmw_crtc_to_du(crtc);

    du->cursor_x = x;
    du->cursor_y = y;
    vmw_cursor_update_position(dev_priv, true, x,y);
};

static cursor_t* __stdcall select_cursor_kms(cursor_t *cursor)
{
    struct vmw_private *dev_priv = vmw_priv(os_display->ddev);
    struct vmw_display_unit *du = vmw_crtc_to_du(os_display->crtc);
    cursor_t *old;

    old = os_display->cursor;
    os_display->cursor = cursor;

    vmw_cursor_update_image(dev_priv, cursor->data,
                    64, 64, cursor->hot_x, cursor->hot_y);
    vmw_cursor_update_position(dev_priv, true,
                   du->cursor_x, du->cursor_y);
    return old;
};

int kms_init(struct drm_device *dev)
{
    struct drm_connector    *connector;
    struct drm_encoder      *encoder;
    struct drm_crtc         *crtc = NULL;
    struct vmw_display_unit *du;
    cursor_t  *cursor;
    int        mode_count;
    u32        ifl;
    int        err;

    crtc = list_entry(dev->mode_config.crtc_list.next, typeof(*crtc), head);
    encoder = list_entry(dev->mode_config.encoder_list.next, typeof(*encoder), head);
    connector = list_entry(dev->mode_config.connector_list.next, typeof(*connector), head);
    connector->encoder = encoder;

    mode_count = count_connector_modes(connector);
    if(mode_count == 0)
    {
        struct drm_display_mode *mode;

        connector->funcs->fill_modes(connector,
                                     dev->mode_config.max_width,
                                     dev->mode_config.max_height);

        list_for_each_entry(mode, &connector->modes, head)
        mode_count++;
    };

    DRM_DEBUG_KMS("CONNECTOR %x ID:%d status:%d ENCODER %x CRTC %x ID:%d\n",
               connector, connector->base.id,
               connector->status, connector->encoder,
               crtc, crtc->base.id );

    os_display = GetDisplay();

    os_display->ddev = dev;
    os_display->connector = connector;
    os_display->crtc = crtc;
    os_display->supported_modes = mode_count;

    ifl = safe_cli();
    {
        os_display->restore_cursor(0,0);
        os_display->select_cursor  = select_cursor_kms;
        os_display->show_cursor    = NULL;
        os_display->move_cursor    = move_cursor_kms;
        os_display->restore_cursor = restore_cursor;
        os_display->disable_mouse  = disable_mouse;
    };
    safe_sti(ifl);

    du = vmw_crtc_to_du(os_display->crtc);
    du->cursor_x = os_display->width/2;
    du->cursor_y = os_display->height/2;
    select_cursor_kms(os_display->cursor);

    return 0;
};


void kms_update()
{
    struct vmw_private *dev_priv = vmw_priv(main_device);
    size_t fifo_size;
    u32    ifl;
    int i;

    struct {
        uint32_t header;
        SVGAFifoCmdUpdate body;
    } *cmd;

    fifo_size = sizeof(*cmd);

    cmd = vmw_fifo_reserve(dev_priv, fifo_size);
    if (unlikely(cmd == NULL)) {
        DRM_ERROR("Fifo reserve failed.\n");
        return;
    }
    os_display = GetDisplay();
    cmd->header = cpu_to_le32(SVGA_CMD_UPDATE);
    cmd->body.x = 0;
    cmd->body.y = 0;
    cmd->body.width  = os_display->width;
    cmd->body.height = os_display->height;

    vmw_fifo_commit(dev_priv, fifo_size);
}

int get_videomodes(videomode_t *mode, int *count)
{
    struct drm_display_mode  *drmmode;
    int err = -1;

    if( *count == 0 )
    {
        *count = os_display->supported_modes;
        err = 0;
    }
    else if( mode != NULL )
    {
        int i = 0;

        if( *count > os_display->supported_modes)
            *count = os_display->supported_modes;

        list_for_each_entry(drmmode, &os_display->connector->modes, head)
        {
            if( i < *count)
            {
//                mode->width  = drm_mode_width(drmmode);
//                mode->height = drm_mode_height(drmmode);
                mode->bpp    = 32;
                mode->freq   = drmmode->vrefresh;
                i++;
                mode++;
            }
            else break;
        };

        *count = i;
        err = 0;
    };

    return err;
};


bool set_mode(struct drm_device *dev, struct drm_connector *connector,
              videomode_t *reqmode, bool strict);


int set_user_mode(videomode_t *mode)
{
    int err = -1;

    dbgprintf("width %d height %d vrefresh %d\n",
               mode->width, mode->height, mode->freq);

    if( (mode->width  != 0)  &&
        (mode->height != 0)  &&
        (mode->freq   != 0 ) &&
        ( (mode->width   != os_display->width)  ||
          (mode->height  != os_display->height) ||
          (mode->freq    != os_display->vrefresh) ) )
    {
//        if( set_mode(os_display->ddev, os_display->connector, mode, true) )
//            err = 0;
    };

    return err;
};

struct file *shmem_file_setup(const char *name, loff_t size, unsigned long flags)
{
    struct file *filep;
    int count;

    filep = __builtin_malloc(sizeof(*filep));

    if(unlikely(filep == NULL))
        return ERR_PTR(-ENOMEM);

    count = size / PAGE_SIZE;

    filep->pages = kzalloc(sizeof(struct page *) * count, 0);
    if(unlikely(filep->pages == NULL))
    {
        kfree(filep);
        return ERR_PTR(-ENOMEM);
    };

    filep->count     = count;
    filep->allocated = 0;
    filep->vma       = NULL;

//    printf("%s file %p pages %p count %d\n",
//              __FUNCTION__,filep, filep->pages, count);

    return filep;
}

struct page *shmem_read_mapping_page_gfp(struct file *filep,
                                         pgoff_t index, gfp_t gfp)
{
    struct page *page;

//    dbgprintf("%s, file %p index %d\n", __FUNCTION__, filep, index);

    if(unlikely(index >= filep->count))
        return ERR_PTR(-EINVAL);

    page = filep->pages[index];

    if(unlikely(page == NULL))
    {
        page = (struct page *)AllocPage();

        if(unlikely(page == NULL))
            return ERR_PTR(-ENOMEM);

        filep->pages[index] = page;
    };

    return page;
};

ktime_t ktime_get(void)
{
    ktime_t t;

    t.tv64 = GetClockNs();

    return t;
}

bool reservation_object_test_signaled_rcu(struct reservation_object *obj,
                                           bool test_all)
{
    return true;
}

int reservation_object_reserve_shared(struct reservation_object *obj)
{
    return 0;
}

void reservation_object_add_shared_fence(struct reservation_object *obj,
                                          struct fence *fence)
{};

void reservation_object_add_excl_fence(struct reservation_object *obj,
                                       struct fence *fence)
{};

#define KMAP_MAX    256

static struct mutex kmap_mutex;
static struct page* kmap_table[KMAP_MAX];
static int kmap_av;
static int kmap_first;
static void* kmap_base;


int kmap_init()
{
    kmap_base = AllocKernelSpace(KMAP_MAX*4096);
    if(kmap_base == NULL)
        return -1;

    kmap_av = KMAP_MAX;
    MutexInit(&kmap_mutex);
    return 0;
};

void *kmap(struct page *page)
{
    void *vaddr = NULL;
    int i;

    do
    {
        MutexLock(&kmap_mutex);
        if(kmap_av != 0)
        {
            for(i = kmap_first; i < KMAP_MAX; i++)
            {
                if(kmap_table[i] == NULL)
                {
                    kmap_av--;
                    kmap_first = i;
                    kmap_table[i] = page;
                    vaddr = kmap_base + (i<<12);
                    MapPage(vaddr,(addr_t)page,3);
                    break;
                };
            };
        };
        MutexUnlock(&kmap_mutex);
    }while(vaddr == NULL);

    return vaddr;
};

void *kmap_atomic(struct page *page) __attribute__ ((alias ("kmap")));

void kunmap(struct page *page)
{
    void *vaddr;
    int   i;

    MutexLock(&kmap_mutex);

    for(i = 0; i < KMAP_MAX; i++)
    {
        if(kmap_table[i] == page)
        {
            kmap_av++;
            if(i < kmap_first)
                kmap_first = i;
            kmap_table[i] = NULL;
            vaddr = kmap_base + (i<<12);
            MapPage(vaddr,0,0);
            break;
        };
    };

    MutexUnlock(&kmap_mutex);
};

void kunmap_atomic(void *vaddr)
{
    int i;

    MapPage(vaddr,0,0);

    i = (vaddr - kmap_base) >> 12;

    MutexLock(&kmap_mutex);

    kmap_av++;
    if(i < kmap_first)
        kmap_first = i;
    kmap_table[i] = NULL;

    MutexUnlock(&kmap_mutex);
}


#include <linux/rcupdate.h>

struct rcu_ctrlblk {
        struct rcu_head *rcucblist;     /* List of pending callbacks (CBs). */
        struct rcu_head **donetail;     /* ->next pointer of last "done" CB. */
        struct rcu_head **curtail;      /* ->next pointer of last CB. */
//        RCU_TRACE(long qlen);           /* Number of pending CBs. */
//        RCU_TRACE(unsigned long gp_start); /* Start time for stalls. */
//        RCU_TRACE(unsigned long ticks_this_gp); /* Statistic for stalls. */
//        RCU_TRACE(unsigned long jiffies_stall); /* Jiffies at next stall. */
//        RCU_TRACE(const char *name);    /* Name of RCU type. */
};

/* Definition for rcupdate control block. */
static struct rcu_ctrlblk rcu_sched_ctrlblk = {
        .donetail       = &rcu_sched_ctrlblk.rcucblist,
        .curtail        = &rcu_sched_ctrlblk.rcucblist,
//        RCU_TRACE(.name = "rcu_sched")
};

static void __call_rcu(struct rcu_head *head,
                       void (*func)(struct rcu_head *rcu),
                       struct rcu_ctrlblk *rcp)
{
        unsigned long flags;

//        debug_rcu_head_queue(head);
        head->func = func;
        head->next = NULL;

        local_irq_save(flags);
        *rcp->curtail = head;
        rcp->curtail = &head->next;
//        RCU_TRACE(rcp->qlen++);
        local_irq_restore(flags);
}

/*
 * Post an RCU callback to be invoked after the end of an RCU-sched grace
 * period.  But since we have but one CPU, that would be after any
 * quiescent state.
 */
void call_rcu_sched(struct rcu_head *head, void (*func)(struct rcu_head *rcu))
{
        __call_rcu(head, func, &rcu_sched_ctrlblk);
}


fb_get_options(const char *name, char **option)
{
    return 1;
}

static void *check_bytes8(const u8 *start, u8 value, unsigned int bytes)
{
        while (bytes) {
                if (*start != value)
                        return (void *)start;
                start++;
                bytes--;
        }
        return NULL;
}

/**
 * memchr_inv - Find an unmatching character in an area of memory.
 * @start: The memory area
 * @c: Find a character other than c
 * @bytes: The size of the area.
 *
 * returns the address of the first character other than @c, or %NULL
 * if the whole buffer contains just @c.
 */
void *memchr_inv(const void *start, int c, size_t bytes)
{
        u8 value = c;
        u64 value64;
        unsigned int words, prefix;

        if (bytes <= 16)
                return check_bytes8(start, value, bytes);

        value64 = value;
#if defined(ARCH_HAS_FAST_MULTIPLIER) && BITS_PER_LONG == 64
        value64 *= 0x0101010101010101;
#elif defined(ARCH_HAS_FAST_MULTIPLIER)
        value64 *= 0x01010101;
        value64 |= value64 << 32;
#else
        value64 |= value64 << 8;
        value64 |= value64 << 16;
        value64 |= value64 << 32;
#endif

        prefix = (unsigned long)start % 8;
        if (prefix) {
                u8 *r;

                prefix = 8 - prefix;
                r = check_bytes8(start, value, prefix);
                if (r)
                        return r;
                start += prefix;
                bytes -= prefix;
        }

        words = bytes / 8;

        while (words) {
                if (*(u64 *)start != value64)
                        return check_bytes8(start, value, 8);
                start += 8;
                words--;
        }

        return check_bytes8(start, value, bytes % 8);
}


void drm_master_put(struct drm_master **master)
{};


bool ttm_ref_object_exists(struct ttm_object_file *tfile,
                           struct ttm_base_object *base)
{
    return true;
};

int autoremove_wake_function(wait_queue_t *wait, unsigned mode, int sync, void *key)
{
    list_del_init(&wait->task_list);
    return 1;
}


struct file *fd_array[32];

struct file *fget(unsigned int fd)
{
    struct file *file;

    file = fd_array[fd];
    get_file_rcu(file);
    return file;
}

void fput(struct file *file)
{
    if (atomic_long_dec_and_test(&file->f_count))
    {

    }
}

struct dma_buf *dma_buf_get(int fd)
{
        struct file *file;

        file = fget(fd);

        if (!file)
                return ERR_PTR(-EBADF);

//        if (!is_dma_buf_file(file)) {
//                fput(file);
//                return ERR_PTR(-EINVAL);
//        }

        return file->private_data;
}

int get_unused_fd_flags(unsigned flags)
{
    return 1;
}

void fd_install(unsigned int fd, struct file *file)
{
    fd_array[fd] = file;
}

int dma_buf_fd(struct dma_buf *dmabuf, int flags)
{
        int fd;

        if (!dmabuf || !dmabuf->file)
                return -EINVAL;

        fd = get_unused_fd_flags(flags);
        if (fd < 0)
                return fd;

        fd_install(fd, dmabuf->file);

        return fd;
}

void dma_buf_put(struct dma_buf *dmabuf)
{
        if (WARN_ON(!dmabuf || !dmabuf->file))
                return;

        fput(dmabuf->file);
}


struct dma_buf *dma_buf_export(const struct dma_buf_export_info *exp_info)
{
        struct dma_buf *dmabuf;
        struct reservation_object *resv = exp_info->resv;
        struct file *file;
        size_t alloc_size = sizeof(struct dma_buf);

        if (!exp_info->resv)
                alloc_size += sizeof(struct reservation_object);
        else
                /* prevent &dma_buf[1] == dma_buf->resv */
                alloc_size += 1;

        if (WARN_ON(!exp_info->priv
                          || !exp_info->ops
                          || !exp_info->ops->map_dma_buf
                          || !exp_info->ops->unmap_dma_buf
                          || !exp_info->ops->release
                          || !exp_info->ops->kmap_atomic
                          || !exp_info->ops->kmap
                          || !exp_info->ops->mmap)) {
                return ERR_PTR(-EINVAL);
        }

        dmabuf = kzalloc(alloc_size, GFP_KERNEL);
        if (!dmabuf) {
                return ERR_PTR(-ENOMEM);
        }

        dmabuf->priv = exp_info->priv;
        dmabuf->ops = exp_info->ops;
        dmabuf->size = exp_info->size;
        dmabuf->exp_name = exp_info->exp_name;

        if (!resv) {
                resv = (struct reservation_object *)&dmabuf[1];
                reservation_object_init(resv);
        }
//        dmabuf->resv = resv;

//        file = anon_inode_getfile("dmabuf", &dma_buf_fops, dmabuf,
//                                        exp_info->flags);
//        if (IS_ERR(file)) {
//                kfree(dmabuf);
//                return ERR_CAST(file);
//        }

//        file->f_mode |= FMODE_LSEEK;
//        dmabuf->file = file;

        mutex_init(&dmabuf->lock);
        INIT_LIST_HEAD(&dmabuf->attachments);

//        mutex_lock(&db_list.lock);
//        list_add(&dmabuf->list_node, &db_list.head);
//        mutex_unlock(&db_list.lock);

        return dmabuf;
}

int dma_map_sg(struct device *dev, struct scatterlist *sglist,
                           int nelems, int dir)
{
    struct scatterlist *s;
    int i;

    for_each_sg(sglist, s, nelems, i) {
        s->dma_address = (dma_addr_t)sg_phys(s);
#ifdef CONFIG_NEED_SG_DMA_LENGTH
        s->dma_length  = s->length;
#endif
    }

    return nelems;
}

void *vmalloc(unsigned long size)
{
    return KernelAlloc(size);
}

void vfree(const void *addr)
{
    KernelFree(addr);
}
