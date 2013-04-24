#include <ddk.h>
#include <linux/mm.h>
#include <drm/drmP.h>
#include <drm/i915_drm.h>
#include "i915_drv.h"
#include "intel_drv.h"
#include <linux/hdmi.h>


struct file *shmem_file_setup(const char *name, loff_t size, unsigned long flags)
{
    struct file *filep;
    int count;

    filep = malloc(sizeof(*filep));

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

unsigned long vm_mmap(struct file *file, unsigned long addr,
         unsigned long len, unsigned long prot,
         unsigned long flag, unsigned long offset)
{
    char *mem, *ptr;
    int i;

    if (unlikely(offset + PAGE_ALIGN(len) < offset))
        return -EINVAL;
    if (unlikely(offset & ~PAGE_MASK))
        return -EINVAL;

    mem = UserAlloc(len);
    if(unlikely(mem == NULL))
        return -ENOMEM;

    for(i = offset, ptr = mem; i < offset+len; i+= 4096, ptr+= 4096)
    {
        struct page *page;

        page = shmem_read_mapping_page_gfp(file, i/PAGE_SIZE,0);

        if (unlikely(IS_ERR(page)))
            goto err;

        MapPage(ptr, (addr_t)page, PG_SHARED|PG_UW);
    }

    return (unsigned long)mem;
err:
    UserFree(mem);
    return -ENOMEM;
};

void shmem_file_delete(struct file *filep)
{
//    printf("%s file %p pages %p count %d\n",
//            __FUNCTION__, filep, filep->pages, filep->count);

    if(filep->pages)
        kfree(filep->pages);
}

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

static inline unsigned char __tolower(unsigned char c)
{
    if (isupper(c))
        c -= 'A'-'a';
    return c;
}

static inline unsigned char __toupper(unsigned char c)
{
    if (islower(c))
        c -= 'a'-'A';
    return c;
}

#define tolower(c) __tolower(c)
#define toupper(c) __toupper(c)

/*
 * Fast implementation of tolower() for internal usage. Do not use in your
 * code.
 */
static inline char _tolower(const char c)
{
    return c | 0x20;
}



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


