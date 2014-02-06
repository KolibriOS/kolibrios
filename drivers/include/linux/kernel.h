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
#include <linux/errno.h>
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

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))

/*
 * This looks more complex than it should be. But we need to
 * get the type for the ~ right in round_down (it needs to be
 * as wide as the result!), and we want to evaluate the macro
 * arguments just once each.
 */
#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))

#define FIELD_SIZEOF(t, f) (sizeof(((t*)0)->f))
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define DIV_ROUND_UP_ULL(ll,d) \
	({ unsigned long long _tmp = (ll)+(d)-1; do_div(_tmp, d); _tmp; })

#if BITS_PER_LONG == 32
# define DIV_ROUND_UP_SECTOR_T(ll,d) DIV_ROUND_UP_ULL(ll, d)
#else
# define DIV_ROUND_UP_SECTOR_T(ll,d) DIV_ROUND_UP(ll,d)
#endif

/* The `const' in roundup() prevents gcc-3.3 from calling __divdi3 */
#define roundup(x, y) (                                 \
{                                                       \
        const typeof(y) __y = y;                        \
        (((x) + (__y - 1)) / __y) * __y;                \
}                                                       \
)
#define rounddown(x, y) (				\
{							\
	typeof(x) __x = (x);				\
	__x - (__x % (y));				\
}							\
)

/*
 * Divide positive or negative dividend by positive divisor and round
 * to closest integer. Result is undefined for negative divisors and
 * for negative dividends if the divisor variable type is unsigned.
 */
#define DIV_ROUND_CLOSEST(x, divisor)(                  \
{                                                       \
	typeof(x) __x = x;				\
	typeof(divisor) __d = divisor;			\
	(((typeof(x))-1) > 0 ||				\
	 ((typeof(divisor))-1) > 0 || (__x) > 0) ?	\
		(((__x) + ((__d) / 2)) / (__d)) :	\
		(((__x) - ((__d) / 2)) / (__d));	\
}                                                       \
)

/*
 * Multiplies an integer by a fraction, while avoiding unnecessary
 * overflow or loss of precision.
 */
#define mult_frac(x, numer, denom)(			\
{							\
	typeof(x) quot = (x) / (denom);			\
	typeof(x) rem  = (x) % (denom);			\
	(quot * (numer)) + ((rem * (numer)) / (denom));	\
}							\
)

#define clamp_t(type, val, min, max) ({         \
        type __val = (val);                     \
        type __min = (min);                     \
        type __max = (max);                     \
        __val = __val < __min ? __min: __val;   \
        __val > __max ? __max: __val; })



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

enum {
    DUMP_PREFIX_NONE,
    DUMP_PREFIX_ADDRESS,
    DUMP_PREFIX_OFFSET
};

int hex_to_bin(char ch);
int hex2bin(u8 *dst, const char *src, size_t count);


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

struct file
{
    struct page  **pages;         /* physical memory backend */
    unsigned int   count;
    unsigned int   allocated;
    void           *vma;
};

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

struct tvec_base;

struct timer_list {
         struct list_head entry;
         unsigned long expires;

         void (*function)(unsigned long);
         unsigned long data;
         u32  handle;
};

#define setup_timer(_timer, _fn, _data)                                 \
        do {                                                            \
                (_timer)->function = (_fn);                             \
                (_timer)->data = (_data);                               \
                (_timer)->handle = 0;                                   \
        } while (0)

int del_timer(struct timer_list *timer);

# define del_timer_sync(t)              del_timer(t)

struct timespec {
    long tv_sec;                 /* seconds */
    long tv_nsec;                /* nanoseconds */
};


#define mb()    asm volatile("mfence" : : : "memory")
#define rmb()   asm volatile("lfence" : : : "memory")
#define wmb()   asm volatile("sfence" : : : "memory")


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

//#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
#define BUILD_BUG_ON(condition)

struct page
{
    unsigned int addr;
};

#define page_to_phys(page)    ((dma_addr_t)(page))

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

#define page_cache_release(page)        FreePage(page_to_phys(page))

#define alloc_page(gfp_mask) (struct page*)AllocPage()

#define __free_page(page) FreePage(page_to_phys(page))

#define get_page(a)
#define put_page(a)
#define set_pages_uc(a,b)
#define set_pages_wb(a,b)

#define pci_map_page(dev, page, offset, size, direction) \
        (dma_addr_t)( (offset)+page_to_phys(page))

#define pci_unmap_page(dev, dma_address, size, direction)

#define GFP_TEMPORARY  0
#define __GFP_NOWARN   0
#define __GFP_NORETRY  0
#define GFP_NOWAIT     0

#define IS_ENABLED(a)  0


#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))

#define RCU_INIT_POINTER(p, v) \
        do { \
                p = (typeof(*v) __force __rcu *)(v); \
        } while (0)


#define rcu_dereference_raw(p)  ({ \
                                typeof(p) _________p1 = ACCESS_ONCE(p); \
                                (_________p1); \
                                })
#define rcu_assign_pointer(p, v) \
        ({ \
                if (!__builtin_constant_p(v) || \
                    ((v) != NULL)) \
                (p) = (v); \
        })


unsigned int hweight16(unsigned int w);

#define cpufreq_quick_get_max(x) GetCpuFreq()

extern unsigned int tsc_khz;

#define on_each_cpu(func,info,wait)             \
        ({                                      \
                func(info);                     \
                0;                              \
        })


#endif

