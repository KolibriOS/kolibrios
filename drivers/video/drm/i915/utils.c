#include <ddk.h>
#include <linux/mm.h>
#include <drm/drmP.h>
#include <drm/i915_drm.h>
#include "i915_drv.h"
#include "intel_drv.h"
#include <linux/hdmi.h>
#include <linux/seq_file.h>
#include <linux/fence.h>
#include "i915_kos32.h"

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

    if(unlikely(index >= filep->count))
        return ERR_PTR(-EINVAL);

    page = filep->pages[index];

    if(unlikely(page == NULL))
    {
        page = (struct page *)AllocPage();

        if(unlikely(page == NULL))
            return ERR_PTR(-ENOMEM);

        filep->pages[index] = page;
//        printf("file %p index %d page %x\n", filep, index, page);
//        delay(1);

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
int hex_dump_to_buffer(const void *buf, size_t len, int rowsize, int groupsize,
               char *linebuf, size_t linebuflen, bool ascii)
{
    const u8 *ptr = buf;
    int ngroups;
    u8 ch;
    int j, lx = 0;
    int ascii_column;
    int ret;

    if (rowsize != 16 && rowsize != 32)
        rowsize = 16;

    if (len > rowsize)      /* limit to one line at a time */
        len = rowsize;
    if (!is_power_of_2(groupsize) || groupsize > 8)
        groupsize = 1;
    if ((len % groupsize) != 0) /* no mixed size output */
        groupsize = 1;

    ngroups = len / groupsize;
    ascii_column = rowsize * 2 + rowsize / groupsize + 1;

    if (!linebuflen)
        goto overflow1;

    if (!len)
        goto nil;

    if (groupsize == 8) {
        const u64 *ptr8 = buf;

        for (j = 0; j < ngroups; j++) {
            ret = snprintf(linebuf + lx, linebuflen - lx,
                       "%s%16.16llx", j ? " " : "",
                       (unsigned long long)*(ptr8 + j));
            if (ret >= linebuflen - lx)
                goto overflow1;
            lx += ret;
        }
    } else if (groupsize == 4) {
        const u32 *ptr4 = buf;

        for (j = 0; j < ngroups; j++) {
            ret = snprintf(linebuf + lx, linebuflen - lx,
                       "%s%8.8x", j ? " " : "",
                       *(ptr4 + j));
            if (ret >= linebuflen - lx)
                goto overflow1;
            lx += ret;
        }
    } else if (groupsize == 2) {
        const u16 *ptr2 = buf;

        for (j = 0; j < ngroups; j++) {
            ret = snprintf(linebuf + lx, linebuflen - lx,
                       "%s%4.4x", j ? " " : "",
                       *(ptr2 + j));
            if (ret >= linebuflen - lx)
                goto overflow1;
            lx += ret;
        }
    } else {
        for (j = 0; j < len; j++) {
            if (linebuflen < lx + 3)
                goto overflow2;
            ch = ptr[j];
            linebuf[lx++] = hex_asc_hi(ch);
            linebuf[lx++] = hex_asc_lo(ch);
            linebuf[lx++] = ' ';
        }
        if (j)
            lx--;
    }
    if (!ascii)
        goto nil;

    while (lx < ascii_column) {
        if (linebuflen < lx + 2)
            goto overflow2;
        linebuf[lx++] = ' ';
    }
    for (j = 0; j < len; j++) {
        if (linebuflen < lx + 2)
            goto overflow2;
        ch = ptr[j];
        linebuf[lx++] = (isascii(ch) && isprint(ch)) ? ch : '.';
    }
nil:
    linebuf[lx] = '\0';
    return lx;
overflow2:
    linebuf[lx++] = '\0';
overflow1:
    return ascii ? ascii_column + len : (groupsize * 2 + 1) * ngroups - 1;
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

void *kmemdup(const void *src, size_t len, gfp_t gfp)
{
    void *p;

    p = kmalloc(len, gfp);
    if (p)
        memcpy(p, src, len);
    return p;
}


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

size_t strlcat(char *dest, const char *src, size_t count)
{
        size_t dsize = strlen(dest);
        size_t len = strlen(src);
        size_t res = dsize + len;

        /* This would be a bug */
        BUG_ON(dsize >= count);

        dest += dsize;
        count -= dsize;
        if (len >= count)
                len = count-1;
        memcpy(dest, src, len);
        dest[len] = 0;
        return res;
}
EXPORT_SYMBOL(strlcat);

void msleep(unsigned int msecs)
{
    msecs /= 10;
    if(!msecs) msecs = 1;

     __asm__ __volatile__ (
     "call *__imp__Delay"
     ::"b" (msecs));
     __asm__ __volatile__ (
     "":::"ebx");

};


/* simple loop based delay: */
static void delay_loop(unsigned long loops)
{
        asm volatile(
                "       test %0,%0      \n"
                "       jz 3f           \n"
                "       jmp 1f          \n"

                ".align 16              \n"
                "1:     jmp 2f          \n"

                ".align 16              \n"
                "2:     dec %0          \n"
                "       jnz 2b          \n"
                "3:     dec %0          \n"

                : /* we don't need output */
                :"a" (loops)
        );
}


static void (*delay_fn)(unsigned long) = delay_loop;

void __delay(unsigned long loops)
{
        delay_fn(loops);
}


inline void __const_udelay(unsigned long xloops)
{
        int d0;

        xloops *= 4;
        asm("mull %%edx"
                : "=d" (xloops), "=&a" (d0)
                : "1" (xloops), ""
                (loops_per_jiffy * (HZ/4)));

        __delay(++xloops);
}

void __udelay(unsigned long usecs)
{
        __const_udelay(usecs * 0x000010c7); /* 2**32 / 1000000 (rounded up) */
}

unsigned int _sw_hweight32(unsigned int w)
{
#ifdef CONFIG_ARCH_HAS_FAST_MULTIPLIER
        w -= (w >> 1) & 0x55555555;
        w =  (w & 0x33333333) + ((w >> 2) & 0x33333333);
        w =  (w + (w >> 4)) & 0x0f0f0f0f;
        return (w * 0x01010101) >> 24;
#else
        unsigned int res = w - ((w >> 1) & 0x55555555);
        res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
        res = (res + (res >> 4)) & 0x0F0F0F0F;
        res = res + (res >> 8);
        return (res + (res >> 16)) & 0x000000FF;
#endif
}
EXPORT_SYMBOL(_sw_hweight32);


void usleep_range(unsigned long min, unsigned long max)
{
    udelay(max);
}
EXPORT_SYMBOL(usleep_range);


static unsigned long round_jiffies_common(unsigned long j, int cpu,
                bool force_up)
{
        int rem;
        unsigned long original = j;

        /*
         * We don't want all cpus firing their timers at once hitting the
         * same lock or cachelines, so we skew each extra cpu with an extra
         * 3 jiffies. This 3 jiffies came originally from the mm/ code which
         * already did this.
         * The skew is done by adding 3*cpunr, then round, then subtract this
         * extra offset again.
         */
        j += cpu * 3;

        rem = j % HZ;

        /*
         * If the target jiffie is just after a whole second (which can happen
         * due to delays of the timer irq, long irq off times etc etc) then
         * we should round down to the whole second, not up. Use 1/4th second
         * as cutoff for this rounding as an extreme upper bound for this.
         * But never round down if @force_up is set.
         */
        if (rem < HZ/4 && !force_up) /* round down */
                j = j - rem;
        else /* round up */
                j = j - rem + HZ;

        /* now that we have rounded, subtract the extra skew again */
        j -= cpu * 3;

        /*
         * Make sure j is still in the future. Otherwise return the
         * unmodified value.
         */
        return time_is_after_jiffies(j) ? j : original;
}


unsigned long round_jiffies_up_relative(unsigned long j, int cpu)
{
        unsigned long j0 = jiffies;

        /* Use j0 because jiffies might change while we run */
        return round_jiffies_common(j + j0, 0, true) - j0;
}
EXPORT_SYMBOL_GPL(__round_jiffies_up_relative);


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

int seq_puts(struct seq_file *m, const char *s)
{
    return 0;
};

__printf(2, 3) int seq_printf(struct seq_file *m, const char *f, ...)
{
    return 0;
}


signed long
fence_wait_timeout(struct fence *fence, bool intr, signed long timeout)
{
        signed long ret;

        if (WARN_ON(timeout < 0))
                return -EINVAL;

//        trace_fence_wait_start(fence);
        ret = fence->ops->wait(fence, intr, timeout);
//        trace_fence_wait_end(fence);
        return ret;
}

void fence_release(struct kref *kref)
{
        struct fence *fence =
                        container_of(kref, struct fence, refcount);

//        trace_fence_destroy(fence);

        BUG_ON(!list_empty(&fence->cb_list));

        if (fence->ops->release)
                fence->ops->release(fence);
        else
                fence_free(fence);
}

void fence_free(struct fence *fence)
{
        kfree_rcu(fence, rcu);
}
EXPORT_SYMBOL(fence_free);


ktime_t ktime_get(void)
{
    ktime_t t;

    t.tv64 = GetClockNs();

    return t;
}

char *strdup(const char *str)
{
    size_t len = strlen(str) + 1;
    char *copy = __builtin_malloc(len);
    if (copy)
    {
        memcpy (copy, str, len);
    }
    return copy;
}

int split_cmdline(char *cmdline, char **argv)
{
    enum quote_state
    {
        QUOTE_NONE,         /* no " active in current parm       */
        QUOTE_DELIMITER,    /* " was first char and must be last */
        QUOTE_STARTED       /* " was seen, look for a match      */
    };

    enum quote_state state;
    unsigned int argc;
    char *p = cmdline;
    char *new_arg, *start;

    argc = 0;

    for(;;)
    {
        /* skip over spaces and tabs */
        if ( *p )
        {
            while (*p == ' ' || *p == '\t')
                ++p;
        }

        if (*p == '\0')
            break;

        state = QUOTE_NONE;
        if( *p == '\"' )
        {
            p++;
            state = QUOTE_DELIMITER;
        }
        new_arg = start = p;
        for (;;)
        {
            if( *p == '\"' )
            {
                p++;
                if( state == QUOTE_NONE )
                {
                    state = QUOTE_STARTED;
                }
                else
                {
                    state = QUOTE_NONE;
                }
                continue;
            }

            if( *p == ' ' || *p == '\t' )
            {
                if( state == QUOTE_NONE )
                {
                    break;
                }
            }

            if( *p == '\0' )
                break;

            if( *p == '\\' )
            {
                if( p[1] == '\"' )
                {
                    ++p;
                    if( p[-2] == '\\' )
                    {
                        continue;
                    }
                }
            }
            if( argv )
            {
                *(new_arg++) = *p;
            }
            ++p;
        };

        if( argv )
        {
            argv[ argc ] = start;
            ++argc;

            /*
              The *new = '\0' is req'd in case there was a \" to "
              translation. It must be after the *p check against
              '\0' because new and p could point to the same char
              in which case the scan would be terminated too soon.
            */

            if( *p == '\0' )
            {
                *new_arg = '\0';
                break;
            }
            *new_arg = '\0';
            ++p;
        }
        else
        {
            ++argc;
            if( *p == '\0' )
            {
                break;
            }
            ++p;
        }
    }

    return argc;
};


fb_get_options(const char *name, char **option)
{
    char *opt, *options = NULL;
    int retval = 1;
    int name_len;

    if(i915.cmdline_mode == NULL)
        return 1;

    name_len = __builtin_strlen(name);

    if (name_len )
    {
        opt = i915.cmdline_mode;
        if (!__builtin_strncmp(name, opt, name_len) &&
             opt[name_len] == ':')
        {
             options = opt + name_len + 1;
             retval = 0;
        }
    }

    if (option)
        *option = options;

    return retval;
}
