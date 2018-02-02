#include <ddk.h>
#include <linux/mm.h>
#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>
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


int dma_map_sg(struct device *dev, struct scatterlist *sg, int nents,
           enum dma_data_direction direction)
{
    struct scatterlist *s;
    int i;

    for_each_sg(sg, s, nents, i) {
        s->dma_address = (dma_addr_t)sg_phys(s);
#ifdef CONFIG_NEED_SG_DMA_LENGTH
        s->dma_length  = s->length;
#endif
    }

    return nents;
}

void
dma_unmap_sg(struct device *dev, struct scatterlist *sg, int nhwentries,
             enum dma_data_direction direction)
{
};


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


void *kmemdup(const void *src, size_t len, gfp_t gfp)
{
    void *p;

    p = kmalloc(len, gfp);
    if (p)
        memcpy(p, src, len);
    return p;
}


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


unsigned long round_jiffies_up_relative(unsigned long j)
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


int fb_get_options(const char *name, char **option)
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

void *vmap(struct page **pages, unsigned int count,
           unsigned long flags, pgprot_t prot)
{
    void *vaddr;
    char *tmp;
    int i;

    vaddr = AllocKernelSpace(count << 12);
    if(vaddr == NULL)
        return NULL;

    for(i = 0, tmp = vaddr; i < count; i++)
    {
        MapPage(tmp, page_to_phys(pages[i]), PG_SW);
        tmp+= 4096;
    };

    return vaddr;
};

void vunmap(const void *addr)
{
    FreeKernelSpace((void*)addr);
}

void __iomem *ioremap_nocache(resource_size_t offset, unsigned long size)
{
    return (void __iomem*) MapIoMem(offset, size, PG_SW|PG_NOCACHE|0x100);
}

void __iomem *ioremap_wc(resource_size_t offset, unsigned long size)
{
//    return (void __iomem*) MapIoMem(offset, size, PG_SW|PG_WRITEC|0x100);
    return (void __iomem*) MapIoMem(offset, size, PG_SW|0x100);
}

void iounmap(volatile void __iomem *addr)
{
    FreeKernelSpace((void*)addr);
}

unsigned long _copy_from_user(void *to, const void __user *from, unsigned n)
{
//    if (access_ok(VERIFY_READ, from, n))
        n = __copy_from_user(to, from, n);
//    else
//        memset(to, 0, n);
    return n;
}
