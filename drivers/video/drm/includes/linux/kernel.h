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

#define USHORT_MAX  ((u16)(~0U))
#define SHORT_MAX   ((s16)(USHORT_MAX>>1))
#define SHORT_MIN   (-SHORT_MAX - 1)
#define INT_MAX     ((int)(~0U>>1))
#define INT_MIN     (-INT_MAX - 1)
#define UINT_MAX    (~0U)
#define LONG_MAX    ((long)(~0UL>>1))
#define LONG_MIN    (-LONG_MAX - 1)
#define ULONG_MAX   (~0UL)
#define LLONG_MAX   ((long long)(~0ULL>>1))
#define LLONG_MIN   (-LLONG_MAX - 1)
#define ULLONG_MAX  (~0ULL)

#define ALIGN(x,a)      __ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
#define PTR_ALIGN(p, a)     ((typeof(p))ALIGN((unsigned long)(p), (a)))
#define IS_ALIGNED(x, a)        (((x) & ((typeof(x))(a) - 1)) == 0)

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

#endif /* __KERNEL__ */

typedef unsigned long   pgprotval_t;

typedef struct pgprot { pgprotval_t pgprot; } pgprot_t;

struct file {};
struct vm_area_struct {};
struct address_space {};

#endif

