#ifndef _LINUX_KERNEL_H
#define _LINUX_KERNEL_H

/*
 * 'kernel.h' contains some often-used function prototypes etc
 */

#ifdef __KERNEL__

#include <stdarg.h>
#include <linux/stddef.h>
#include <linux/types.h>
#include <linux/compiler.h>
#include <linux/bitops.h>

#include <linux/typecheck.h>

#define __init

#define USHRT_MAX	((u16)(~0U))
#define SHRT_MAX	((s16)(USHRT_MAX>>1))
#define SHRT_MIN	((s16)(-SHRT_MAX - 1))
#define INT_MAX     ((int)(~0U>>1))
#define INT_MIN     (-INT_MAX - 1)
#define UINT_MAX    (~0U)
#define LONG_MAX    ((long)(~0UL>>1))
#define LONG_MIN    (-LONG_MAX - 1)
#define ULONG_MAX   (~0UL)
#define LLONG_MAX   ((long long)(~0ULL>>1))
#define LLONG_MIN   (-LLONG_MAX - 1)
#define ULLONG_MAX  (~0ULL)
#define SIZE_MAX	(~(size_t)0)

#define ALIGN(x,a)      __ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
#define PTR_ALIGN(p, a)     ((typeof(p))ALIGN((unsigned long)(p), (a)))
#define IS_ALIGNED(x, a)        (((x) & ((typeof(x))(a) - 1)) == 0)

#define roundup(x, y) ((((x) + ((y) - 1)) / (y)) * (y))

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define DIV_ROUND_CLOSEST(x, divisor)(                  \
{                                                       \
         typeof(divisor) __divisor = divisor;            \
         (((x) + ((__divisor) / 2)) / (__divisor));      \
}                                                       \
)

/**
 * upper_32_bits - return bits 32-63 of a number
 * @n: the number we're accessing
 *
 * A basic shift-right of a 64- or 32-bit quantity.  Use this to suppress
 * the "right shift count >= width of type" warning when that quantity is
 * 32-bits.
 */
#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))

/**
 * lower_32_bits - return bits 0-31 of a number
 * @n: the number we're accessing
 */
#define lower_32_bits(n) ((u32)(n))

#define KERN_EMERG      "<0>"   /* system is unusable                   */
#define KERN_ALERT      "<1>"   /* action must be taken immediately     */
#define KERN_CRIT       "<2>"   /* critical conditions                  */
#define KERN_ERR        "<3>"   /* error conditions                     */
#define KERN_WARNING    "<4>"   /* warning conditions                   */
#define KERN_NOTICE     "<5>"   /* normal but significant condition     */
#define KERN_INFO       "<6>"   /* informational                        */
#define KERN_DEBUG      "<7>"   /* debug-level messages                 */
extern const char hex_asc[];
#define hex_asc_lo(x)	hex_asc[((x) & 0x0f)]
#define hex_asc_hi(x)	hex_asc[((x) & 0xf0) >> 4]

static inline char *pack_hex_byte(char *buf, u8 byte)
{
	*buf++ = hex_asc_hi(byte);
	*buf++ = hex_asc_lo(byte);
	return buf;
}

extern int hex_to_bin(char ch);
extern void hex2bin(u8 *dst, const char *src, size_t count);


//int printk(const char *fmt, ...);

#define printk(fmt, arg...)    dbgprintf(fmt , ##arg)


/*
 * min()/max()/clamp() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#define min(x, y) ({                \
    typeof(x) _min1 = (x);          \
    typeof(y) _min2 = (y);          \
    (void) (&_min1 == &_min2);      \
    _min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({                \
    typeof(x) _max1 = (x);          \
    typeof(y) _max2 = (y);          \
    (void) (&_max1 == &_max2);      \
    _max1 > _max2 ? _max1 : _max2; })

#define min3(x, y, z) ({			\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	typeof(z) _min3 = (z);			\
	(void) (&_min1 == &_min2);		\
	(void) (&_min1 == &_min3);		\
	_min1 < _min2 ? (_min1 < _min3 ? _min1 : _min3) : \
		(_min2 < _min3 ? _min2 : _min3); })

#define max3(x, y, z) ({			\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	typeof(z) _max3 = (z);			\
	(void) (&_max1 == &_max2);		\
	(void) (&_max1 == &_max3);		\
	_max1 > _max2 ? (_max1 > _max3 ? _max1 : _max3) : \
		(_max2 > _max3 ? _max2 : _max3); })

/**
 * min_not_zero - return the minimum that is _not_ zero, unless both are zero
 * @x: value1
 * @y: value2
 */
#define min_not_zero(x, y) ({			\
	typeof(x) __x = (x);			\
	typeof(y) __y = (y);			\
	__x == 0 ? __y : ((__y == 0) ? __x : min(__x, __y)); })

/**
 * clamp - return a value clamped to a given range with strict typechecking
 * @val: current value
 * @min: minimum allowable value
 * @max: maximum allowable value
 *
 * This macro does strict typechecking of min/max to make sure they are of the
 * same type as val.  See the unnecessary pointer comparisons.
 */
#define clamp(val, min, max) ({			\
	typeof(val) __val = (val);		\
	typeof(min) __min = (min);		\
	typeof(max) __max = (max);		\
	(void) (&__val == &__min);		\
	(void) (&__val == &__max);		\
	__val = __val < __min ? __min: __val;	\
	__val > __max ? __max: __val; })

/*
 * ..and if you can't take the strict
 * types, you can specify one yourself.
 *
 * Or not use min/max/clamp at all, of course.
 */
#define min_t(type, x, y) ({            \
    type __min1 = (x);          \
    type __min2 = (y);          \
    __min1 < __min2 ? __min1: __min2; })

#define max_t(type, x, y) ({            \
    type __max1 = (x);          \
    type __max2 = (y);          \
    __max1 > __max2 ? __max1: __max2; })

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:    the pointer to the member.
 * @type:   the type of the container struct this is embedded in.
 * @member: the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({          \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type,member) );})


static inline void *kcalloc(size_t n, size_t size, uint32_t flags)
{
        if (n != 0 && size > ULONG_MAX / n)
                return NULL;
        return kzalloc(n * size, 0);
}


void free (void *ptr);

#endif /* __KERNEL__ */

typedef unsigned long   pgprotval_t;

typedef struct pgprot { pgprotval_t pgprot; } pgprot_t;

struct file {};
struct vm_area_struct {};
struct address_space {};

struct device
{
    struct device   *parent;
    void            *driver_data;
};

static inline void dev_set_drvdata(struct device *dev, void *data)
{
    dev->driver_data = data;
}

static inline void *dev_get_drvdata(struct device *dev)
{
    return dev->driver_data;
}

#define preempt_disable()       do { } while (0)
#define preempt_enable_no_resched() do { } while (0)
#define preempt_enable()        do { } while (0)
#define preempt_check_resched()     do { } while (0)

#define preempt_disable_notrace()       do { } while (0)
#define preempt_enable_no_resched_notrace() do { } while (0)
#define preempt_enable_notrace()        do { } while (0)

#define in_dbg_master() (0)

#define HZ 100

#define time_after(a,b)         \
        (typecheck(unsigned long, a) && \
        typecheck(unsigned long, b) && \
        ((long)(b) - (long)(a) < 0))

struct tvec_base;

struct timer_list {
         struct list_head entry;
         unsigned long expires;

         void (*function)(unsigned long);
         unsigned long data;

//         struct tvec_base *base;
};

struct timespec {
    long tv_sec;                 /* seconds */
    long tv_nsec;                /* nanoseconds */
};


#define build_mmio_read(name, size, type, reg, barrier)     \
static inline type name(const volatile void __iomem *addr)  \
{ type ret; asm volatile("mov" size " %1,%0":reg (ret)      \
:"m" (*(volatile type __force *)addr) barrier); return ret; }

#define build_mmio_write(name, size, type, reg, barrier) \
static inline void name(type val, volatile void __iomem *addr) \
{ asm volatile("mov" size " %0,%1": :reg (val), \
"m" (*(volatile type __force *)addr) barrier); }

build_mmio_read(readb, "b", unsigned char, "=q", :"memory")
build_mmio_read(readw, "w", unsigned short, "=r", :"memory")
build_mmio_read(readl, "l", unsigned int, "=r", :"memory")

build_mmio_read(__readb, "b", unsigned char, "=q", )
build_mmio_read(__readw, "w", unsigned short, "=r", )
build_mmio_read(__readl, "l", unsigned int, "=r", )

build_mmio_write(writeb, "b", unsigned char, "q", :"memory")
build_mmio_write(writew, "w", unsigned short, "r", :"memory")
build_mmio_write(writel, "l", unsigned int, "r", :"memory")

build_mmio_write(__writeb, "b", unsigned char, "q", )
build_mmio_write(__writew, "w", unsigned short, "r", )
build_mmio_write(__writel, "l", unsigned int, "r", )

#define readb_relaxed(a) __readb(a)
#define readw_relaxed(a) __readw(a)
#define readl_relaxed(a) __readl(a)
#define __raw_readb __readb
#define __raw_readw __readw
#define __raw_readl __readl

#define __raw_writeb __writeb
#define __raw_writew __writew
#define __raw_writel __writel

static inline __u64 readq(const volatile void __iomem *addr)
{
        const volatile u32 __iomem *p = addr;
        u32 low, high;

        low = readl(p);
        high = readl(p + 1);

        return low + ((u64)high << 32);
}

static inline void writeq(__u64 val, volatile void __iomem *addr)
{
        writel(val, addr);
        writel(val >> 32, addr+4);
}

#define swap(a, b) \
        do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)


#define mmiowb() barrier()

#define dev_err(dev, format, arg...)            \
        printk("Error %s " format, __func__ , ## arg)

#define dev_warn(dev, format, arg...)            \
        printk("Warning %s " format, __func__ , ## arg)

#define dev_info(dev, format, arg...)       \
        printk("Info %s " format , __func__, ## arg)

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))


struct scatterlist {
    unsigned long   page_link;
    unsigned int    offset;
    unsigned int    length;
    dma_addr_t      dma_address;
    unsigned int    dma_length;
};

struct sg_table {
    struct scatterlist *sgl;        /* the list */
    unsigned int nents;             /* number of mapped entries */
    unsigned int orig_nents;        /* original size of list */
};

#define SG_MAX_SINGLE_ALLOC             (4096 / sizeof(struct scatterlist))

struct scatterlist *sg_next(struct scatterlist *sg);

#define sg_dma_address(sg)      ((sg)->dma_address)
#define sg_dma_len(sg)         ((sg)->length)

#define sg_is_chain(sg)         ((sg)->page_link & 0x01)
#define sg_is_last(sg)          ((sg)->page_link & 0x02)
#define sg_chain_ptr(sg)        \
        ((struct scatterlist *) ((sg)->page_link & ~0x03))

static inline addr_t sg_page(struct scatterlist *sg)
{
    return (addr_t)((sg)->page_link & ~0x3);
}

#define for_each_sg(sglist, sg, nr, __i)        \
        for (__i = 0, sg = (sglist); __i < (nr); __i++, sg = sg_next(sg))



struct page
{
    unsigned int addr;
};


struct vm_fault {
    unsigned int flags;             /* FAULT_FLAG_xxx flags */
    pgoff_t pgoff;                  /* Logical page offset based on vma */
    void __user *virtual_address;   /* Faulting virtual address */

    struct page *page;              /* ->fault handlers should return a
                                     * page here, unless VM_FAULT_NOPAGE
                                     * is set (which is also implied by
                                     * VM_FAULT_ERROR).
                                     */
};

struct pagelist {
    dma_addr_t    *page;
    unsigned int   nents;
};

#endif

