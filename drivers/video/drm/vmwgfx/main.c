#include <drm/drmP.h>
#include <drm.h>

#include <linux/kernel.h>
#include <linux/module.h>

#include "vmwgfx_drv.h"

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

extern struct drm_device *main_device;
extern struct drm_file   *drm_file_handlers[256];

int vmw_init(void);
void cpu_detect();

void parse_cmdline(char *cmdline, char *log);
int _stdcall display_handler(ioctl_t *io);

int srv_blit_bitmap(u32 hbitmap, int  dst_x, int dst_y,
               int src_x, int src_y, u32 w, u32 h);

int blit_textured(u32 hbitmap, int  dst_x, int dst_y,
               int src_x, int src_y, u32 w, u32 h);

int blit_tex(u32 hbitmap, int  dst_x, int dst_y,
             int src_x, int src_y, u32 w, u32 h);

void get_pci_info(struct pci_device *dev);
int gem_getparam(struct drm_device *dev, void *data);

int i915_mask_update(struct drm_device *dev, void *data,
            struct drm_file *file);


static char  log[256];

struct workqueue_struct *system_wq;
int driver_wq_state;

int x86_clflush_size;
unsigned int tsc_khz;

int kms_modeset = 1;

u32_t  __attribute__((externally_visible)) drvEntry(int action, char *cmdline)
{

    int     err = 0;

    if(action != 1)
    {
        driver_wq_state = 0;
        return 0;
    };

    if( GetService("DISPLAY") != 0 )
        return 0;

    if( cmdline && *cmdline )
        parse_cmdline(cmdline, log);

    if(!dbg_open(log))
    {
//        strcpy(log, "/tmp1/1/vmw.log");
//        strcpy(log, "/RD/1/DRIVERS/VMW.log");
        strcpy(log, "/HD0/1/vmw.log");

        if(!dbg_open(log))
        {
            printf("Can't open %s\nExit\n", log);
            return 0;
        };
    }
    dbgprintf(" vmw v3.10\n cmdline: %s\n", cmdline);

    cpu_detect();
    dbgprintf("\ncache line size %d\n", x86_clflush_size);

    enum_pci_devices();

    err = vmw_init();
    if(err)
    {
        dbgprintf("Epic Fail :(\n");
        return 0;
    };

    err = RegService("DISPLAY", display_handler);

    if( err != 0)
        dbgprintf("Set DISPLAY handler\n");

//    struct drm_i915_private *dev_priv = main_device->dev_private;
//    driver_wq_state = 1;
//    run_workqueue(dev_priv->wq);

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
#define SRV_GET_PARAM               21
#define SRV_I915_GEM_CREATE         22
#define SRV_DRM_GEM_CLOSE           23
#define SRV_I915_GEM_PIN            24
#define SRV_I915_GEM_SET_CACHEING   25
#define SRV_I915_GEM_GET_APERTURE   26
#define SRV_I915_GEM_PWRITE         27
#define SRV_I915_GEM_BUSY           28
#define SRV_I915_GEM_SET_DOMAIN     29
#define SRV_I915_GEM_MMAP           30
#define SRV_I915_GEM_MMAP_GTT       31
#define SRV_I915_GEM_THROTTLE       32
#define SRV_FBINFO                  33
#define SRV_I915_GEM_EXECBUFFER2    34
#define SRV_MASK_UPDATE             35



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
            dbgprintf("SRV_ENUM_MODES inp %x inp_size %x out_size %x\n",
                       inp, io->inp_size, io->out_size );
            check_output(4);
//            check_input(*outp * sizeof(videomode_t));
            if( kms_modeset)
                retval = get_videomodes((videomode_t*)inp, outp);
            break;

        case SRV_SET_MODE:
            dbgprintf("SRV_SET_MODE inp %x inp_size %x\n",
                       inp, io->inp_size);
            check_input(sizeof(videomode_t));
            if( kms_modeset )
                retval = set_user_mode((videomode_t*)inp);
            break;

#if 0
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

int pci_scan_filter(u32_t id, u32_t busnr, u32_t devfn)
{
    u16_t vendor, device;
    u32_t class;
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
#include <linux/hdmi.h>
#include <linux/ctype.h>

/**
 * hdmi_avi_infoframe_init() - initialize an HDMI AVI infoframe
 * @frame: HDMI AVI infoframe
 *
 * Returns 0 on success or a negative error code on failure.
 */
int hdmi_avi_infoframe_init(struct hdmi_avi_infoframe *frame)
{
    memset(frame, 0, sizeof(*frame));

    frame->type = HDMI_INFOFRAME_TYPE_AVI;
    frame->version = 2;
    frame->length = 13;

    return 0;
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

int vscnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
    int i;

    i = vsnprintf(buf, size, fmt, args);

    if (likely(i < size))
            return i;
    if (size != 0)
            return size - 1;
    return 0;
}


int scnprintf(char *buf, size_t size, const char *fmt, ...)
{
        va_list args;
        int i;

        va_start(args, fmt);
        i = vscnprintf(buf, size, fmt, args);
        va_end(args);

        return i;
}



#define _U  0x01    /* upper */
#define _L  0x02    /* lower */
#define _D  0x04    /* digit */
#define _C  0x08    /* cntrl */
#define _P  0x10    /* punct */
#define _S  0x20    /* white space (space/lf/tab) */
#define _X  0x40    /* hex digit */
#define _SP 0x80    /* hard space (0x20) */

extern const unsigned char _ctype[];

#define __ismask(x) (_ctype[(int)(unsigned char)(x)])

#define isalnum(c)  ((__ismask(c)&(_U|_L|_D)) != 0)
#define isalpha(c)  ((__ismask(c)&(_U|_L)) != 0)
#define iscntrl(c)  ((__ismask(c)&(_C)) != 0)
#define isdigit(c)  ((__ismask(c)&(_D)) != 0)
#define isgraph(c)  ((__ismask(c)&(_P|_U|_L|_D)) != 0)
#define islower(c)  ((__ismask(c)&(_L)) != 0)
#define isprint(c)  ((__ismask(c)&(_P|_U|_L|_D|_SP)) != 0)
#define ispunct(c)  ((__ismask(c)&(_P)) != 0)
/* Note: isspace() must return false for %NUL-terminator */
#define isspace(c)  ((__ismask(c)&(_S)) != 0)
#define isupper(c)  ((__ismask(c)&(_U)) != 0)
#define isxdigit(c) ((__ismask(c)&(_D|_X)) != 0)

#define isascii(c) (((unsigned char)(c))<=0x7f)
#define toascii(c) (((unsigned char)(c))&0x7f)



//const char hex_asc[] = "0123456789abcdef";

/**
 * hex_to_bin - convert a hex digit to its real value
 * @ch: ascii character represents hex digit
 *
 * hex_to_bin() converts one hex digit to its actual value or -1 in case of bad
 * input.
 */
int hex_to_bin(char ch)
{
    if ((ch >= '0') && (ch <= '9'))
        return ch - '0';
    ch = tolower(ch);
    if ((ch >= 'a') && (ch <= 'f'))
        return ch - 'a' + 10;
    return -1;
}
EXPORT_SYMBOL(hex_to_bin);

/**
 * hex2bin - convert an ascii hexadecimal string to its binary representation
 * @dst: binary result
 * @src: ascii hexadecimal string
 * @count: result length
 *
 * Return 0 on success, -1 in case of bad input.
 */
int hex2bin(u8 *dst, const char *src, size_t count)
{
    while (count--) {
        int hi = hex_to_bin(*src++);
        int lo = hex_to_bin(*src++);

        if ((hi < 0) || (lo < 0))
            return -1;

        *dst++ = (hi << 4) | lo;
    }
    return 0;
}
EXPORT_SYMBOL(hex2bin);

/**
 * hex_dump_to_buffer - convert a blob of data to "hex ASCII" in memory
 * @buf: data blob to dump
 * @len: number of bytes in the @buf
 * @rowsize: number of bytes to print per line; must be 16 or 32
 * @groupsize: number of bytes to print at a time (1, 2, 4, 8; default = 1)
 * @linebuf: where to put the converted data
 * @linebuflen: total size of @linebuf, including space for terminating NUL
 * @ascii: include ASCII after the hex output
 *
 * hex_dump_to_buffer() works on one "line" of output at a time, i.e.,
 * 16 or 32 bytes of input data converted to hex + ASCII output.
 *
 * Given a buffer of u8 data, hex_dump_to_buffer() converts the input data
 * to a hex + ASCII dump at the supplied memory location.
 * The converted output is always NUL-terminated.
 *
 * E.g.:
 *   hex_dump_to_buffer(frame->data, frame->len, 16, 1,
 *          linebuf, sizeof(linebuf), true);
 *
 * example output buffer:
 * 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f  @ABCDEFGHIJKLMNO
 */
void hex_dump_to_buffer(const void *buf, size_t len, int rowsize,
            int groupsize, char *linebuf, size_t linebuflen,
            bool ascii)
{
    const u8 *ptr = buf;
    u8 ch;
    int j, lx = 0;
    int ascii_column;

    if (rowsize != 16 && rowsize != 32)
        rowsize = 16;

    if (!len)
        goto nil;
    if (len > rowsize)      /* limit to one line at a time */
        len = rowsize;
    if ((len % groupsize) != 0) /* no mixed size output */
        groupsize = 1;

    switch (groupsize) {
    case 8: {
        const u64 *ptr8 = buf;
        int ngroups = len / groupsize;

        for (j = 0; j < ngroups; j++)
            lx += scnprintf(linebuf + lx, linebuflen - lx,
                    "%s%16.16llx", j ? " " : "",
                    (unsigned long long)*(ptr8 + j));
        ascii_column = 17 * ngroups + 2;
        break;
    }

    case 4: {
        const u32 *ptr4 = buf;
        int ngroups = len / groupsize;

        for (j = 0; j < ngroups; j++)
            lx += scnprintf(linebuf + lx, linebuflen - lx,
                    "%s%8.8x", j ? " " : "", *(ptr4 + j));
        ascii_column = 9 * ngroups + 2;
        break;
    }

    case 2: {
        const u16 *ptr2 = buf;
        int ngroups = len / groupsize;

        for (j = 0; j < ngroups; j++)
            lx += scnprintf(linebuf + lx, linebuflen - lx,
                    "%s%4.4x", j ? " " : "", *(ptr2 + j));
        ascii_column = 5 * ngroups + 2;
        break;
    }

    default:
        for (j = 0; (j < len) && (lx + 3) <= linebuflen; j++) {
            ch = ptr[j];
            linebuf[lx++] = hex_asc_hi(ch);
            linebuf[lx++] = hex_asc_lo(ch);
            linebuf[lx++] = ' ';
        }
        if (j)
            lx--;

        ascii_column = 3 * rowsize + 2;
        break;
    }
    if (!ascii)
        goto nil;

    while (lx < (linebuflen - 1) && lx < (ascii_column - 1))
        linebuf[lx++] = ' ';
    for (j = 0; (j < len) && (lx + 2) < linebuflen; j++) {
        ch = ptr[j];
        linebuf[lx++] = (isascii(ch) && isprint(ch)) ? ch : '.';
    }
nil:
    linebuf[lx++] = '\0';
}

/**
 * print_hex_dump - print a text hex dump to syslog for a binary blob of data
 * @level: kernel log level (e.g. KERN_DEBUG)
 * @prefix_str: string to prefix each line with;
 *  caller supplies trailing spaces for alignment if desired
 * @prefix_type: controls whether prefix of an offset, address, or none
 *  is printed (%DUMP_PREFIX_OFFSET, %DUMP_PREFIX_ADDRESS, %DUMP_PREFIX_NONE)
 * @rowsize: number of bytes to print per line; must be 16 or 32
 * @groupsize: number of bytes to print at a time (1, 2, 4, 8; default = 1)
 * @buf: data blob to dump
 * @len: number of bytes in the @buf
 * @ascii: include ASCII after the hex output
 *
 * Given a buffer of u8 data, print_hex_dump() prints a hex + ASCII dump
 * to the kernel log at the specified kernel log level, with an optional
 * leading prefix.
 *
 * print_hex_dump() works on one "line" of output at a time, i.e.,
 * 16 or 32 bytes of input data converted to hex + ASCII output.
 * print_hex_dump() iterates over the entire input @buf, breaking it into
 * "line size" chunks to format and print.
 *
 * E.g.:
 *   print_hex_dump(KERN_DEBUG, "raw data: ", DUMP_PREFIX_ADDRESS,
 *          16, 1, frame->data, frame->len, true);
 *
 * Example output using %DUMP_PREFIX_OFFSET and 1-byte mode:
 * 0009ab42: 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f  @ABCDEFGHIJKLMNO
 * Example output using %DUMP_PREFIX_ADDRESS and 4-byte mode:
 * ffffffff88089af0: 73727170 77767574 7b7a7978 7f7e7d7c  pqrstuvwxyz{|}~.
 */
void print_hex_dump(const char *level, const char *prefix_str, int prefix_type,
            int rowsize, int groupsize,
            const void *buf, size_t len, bool ascii)
{
    const u8 *ptr = buf;
    int i, linelen, remaining = len;
    unsigned char linebuf[32 * 3 + 2 + 32 + 1];

    if (rowsize != 16 && rowsize != 32)
        rowsize = 16;

    for (i = 0; i < len; i += rowsize) {
        linelen = min(remaining, rowsize);
        remaining -= rowsize;

        hex_dump_to_buffer(ptr + i, linelen, rowsize, groupsize,
                   linebuf, sizeof(linebuf), ascii);

        switch (prefix_type) {
        case DUMP_PREFIX_ADDRESS:
            printk("%s%s%p: %s\n",
                   level, prefix_str, ptr + i, linebuf);
            break;
        case DUMP_PREFIX_OFFSET:
            printk("%s%s%.8x: %s\n", level, prefix_str, i, linebuf);
            break;
        default:
            printk("%s%s%s\n", level, prefix_str, linebuf);
            break;
        }
    }
}

void print_hex_dump_bytes(const char *prefix_str, int prefix_type,
                          const void *buf, size_t len)
{
    print_hex_dump(KERN_DEBUG, prefix_str, prefix_type, 16, 1,
                       buf, len, true);
}












#include "vmwgfx_kms.h"

void kms_update();

//#define iowrite32(v, addr)      writel((v), (addr))

//#include "bitmap.h"

extern struct drm_device *main_device;

typedef struct
{
    kobj_t     header;

    uint32_t  *data;
    uint32_t   hot_x;
    uint32_t   hot_y;

    struct list_head   list;
//    struct drm_i915_gem_object  *cobj;
}cursor_t;

#define CURSOR_WIDTH 64
#define CURSOR_HEIGHT 64

struct tag_display
{
    int  x;
    int  y;
    int  width;
    int  height;
    int  bpp;
    int  vrefresh;
    int  pitch;
    int  lfb;

    int  supported_modes;
    struct drm_device    *ddev;
    struct drm_connector *connector;
    struct drm_crtc      *crtc;

    struct list_head   cursors;

    cursor_t   *cursor;
    int       (*init_cursor)(cursor_t*);
    cursor_t* (__stdcall *select_cursor)(cursor_t*);
    void      (*show_cursor)(int show);
    void      (__stdcall *move_cursor)(cursor_t *cursor, int x, int y);
    void      (__stdcall *restore_cursor)(int x, int y);
    void      (*disable_mouse)(void);
    u32  mask_seqno;
    u32  check_mouse;
    u32  check_m_pixel;
    u32  dirty;
    void (*update)(void);
};

static display_t *os_display;

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

int kms_init(struct drm_device *dev)
{
    struct drm_connector    *connector;
    struct drm_connector_helper_funcs *connector_funcs;
    struct drm_encoder      *encoder;
    struct drm_crtc         *crtc = NULL;
    struct drm_framebuffer  *fb;

    cursor_t  *cursor;
    int        mode_count;
    u32_t      ifl;
    int        err;

    ENTER();

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

    printf("%s %d\n",__FUNCTION__, mode_count);

    DRM_DEBUG_KMS("CONNECTOR %x ID:%d status:%d ENCODER %x CRTC %x ID:%d\n",
               connector, connector->base.id,
               connector->status, connector->encoder,
               crtc, crtc->base.id );

    DRM_DEBUG_KMS("[Select CRTC:%d]\n", crtc->base.id);

    os_display = GetDisplay();

    ifl = safe_cli();
    {
        os_display->ddev = dev;
        os_display->connector = connector;
        os_display->crtc = crtc;
        os_display->supported_modes = mode_count;
        os_display->update = kms_update;

//        struct intel_crtc *intel_crtc = to_intel_crtc(os_display->crtc);

//        list_for_each_entry(cursor, &os_display->cursors, list)
//        {
//            init_cursor(cursor);
//        };

//        os_display->restore_cursor(0,0);
//        os_display->init_cursor    = init_cursor;
//        os_display->select_cursor  = select_cursor_kms;
//        os_display->show_cursor    = NULL;
//        os_display->move_cursor    = move_cursor_kms;
//        os_display->restore_cursor = restore_cursor;
//        os_display->disable_mouse  = disable_mouse;

//        intel_crtc->cursor_x = os_display->width/2;
//        intel_crtc->cursor_y = os_display->height/2;

//        select_cursor_kms(os_display->cursor);
    };
    safe_sti(ifl);

    main_device = dev;

#ifdef __HWA__
    err = init_bitmaps();
#endif

    LEAVE();

    return 0;
};


void kms_update()
{
    struct vmw_private *dev_priv = vmw_priv(main_device);
    size_t fifo_size;
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

    cmd->header = cpu_to_le32(SVGA_CMD_UPDATE);
    cmd->body.x = 0;
    cmd->body.y = 0;
    cmd->body.width  = os_display->width; //cpu_to_le32(clips->x2 - clips->x1);
    cmd->body.height = os_display->height; //cpu_to_le32(clips->y2 - clips->y1);

    vmw_fifo_commit(dev_priv, fifo_size);
}

int get_videomodes(videomode_t *mode, int *count)
{
    int err = -1;

    dbgprintf("mode %x count %d\n", mode, *count);

    if( *count == 0 )
    {
        *count = os_display->supported_modes;
        err = 0;
    }
    else if( mode != NULL )
    {
        struct drm_display_mode  *drmmode;
        int i = 0;

        if( *count > os_display->supported_modes)
            *count = os_display->supported_modes;

        list_for_each_entry(drmmode, &os_display->connector->modes, head)
        {
            if( i < *count)
            {
                mode->width  = drm_mode_width(drmmode);
                mode->height = drm_mode_height(drmmode);
                mode->bpp    = 32;
                mode->freq   = drm_mode_vrefresh(drmmode);
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
        if( set_mode(os_display->ddev, os_display->connector, mode, true) )
            err = 0;
    };

    return err;
};

