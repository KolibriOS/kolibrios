
#ifndef __TYPES_H__
#define __TYPES_H__


typedef int                  bool;

#define false                0
#define true                 1

typedef unsigned int         size_t;
typedef unsigned int         count_t;
typedef unsigned int         addr_t;

typedef unsigned char        u8;
typedef unsigned short       u16;
typedef unsigned int         u32;
typedef unsigned long long   u64;

typedef unsigned char        __u8;
typedef unsigned short       __u16;
typedef unsigned int         __u32;
typedef unsigned long long   __u64;

typedef signed char         __s8;
typedef signed short        __s16;
typedef signed int          __s32;
typedef signed long long    __s64;


typedef unsigned char        uint8_t;
typedef unsigned short       uint16_t;
typedef unsigned int         uint32_t;
typedef unsigned long long   uint64_t;

typedef unsigned char        u8_t;
typedef unsigned short       u16_t;
typedef unsigned int         u32_t;
typedef unsigned long long   u64_t;

typedef signed char          int8_t;
typedef signed long long     int64_t;

#define  NULL     (void*)0

typedef uint32_t             dma_addr_t;
typedef uint32_t             resource_size_t;

#define __user

#define cpu_to_le16(v16) (v16)
#define cpu_to_le32(v32) (v32)
#define cpu_to_le64(v64) (v64)
#define le16_to_cpu(v16) (v16)
#define le32_to_cpu(v32) (v32)
#define le64_to_cpu(v64) (v64)

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#define BITS_PER_LONG 32

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#define BITS_TO_LONGS(nr)   DIV_ROUND_UP(nr, BITS_PER_LONG)

#define DECLARE_BITMAP(name,bits) \
        unsigned long name[BITS_TO_LONGS(bits)]


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


#define DRM_NAME    "drm"     /**< Name in kernel, /dev, and /proc */

#define DRM_INFO(fmt, arg...)  dbgprintf("DRM: "fmt , ##arg)

#define DRM_ERROR(fmt, arg...) \
    printk(KERN_ERR "[" DRM_NAME ":%s] *ERROR* " fmt , __func__ , ##arg)

#define BUILD_BUG_ON_ZERO(e) (sizeof(char[1 - 2 * !!(e)]) - 1)

#define __must_be_array(a) \
    BUILD_BUG_ON_ZERO(__builtin_types_compatible_p(typeof(a), typeof(&a[0])))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))




#ifndef HAVE_ARCH_BUG
#define BUG() do { \
         printk("BUG: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __FUNCTION__); \
       /*  panic("BUG!"); */ \
 } while (0)
#endif

#ifndef HAVE_ARCH_BUG_ON
#define BUG_ON(condition) do { if (unlikely(condition)) BUG(); } while(0)
#endif



#define MTRR_TYPE_UNCACHABLE 0
#define MTRR_TYPE_WRCOMB     1
#define MTRR_TYPE_WRTHROUGH  4
#define MTRR_TYPE_WRPROT     5
#define MTRR_TYPE_WRBACK     6
#define MTRR_NUM_TYPES       7

int dbgprintf(const char* format, ...);

#define GFP_KERNEL           0

//#include <stdio.h>

int snprintf(char *str, size_t size, const char *format, ...);


//#include <string.h>

void*   memcpy(void *s1, const void *s2, size_t n);
void*   memset(void *s, int c, size_t n);
size_t  strlen(const char *s);
char *strcpy(char *s1, const char *s2);
char *strncpy (char *dst, const char *src, size_t len);

void *malloc(size_t size);

#define kmalloc(s,f) malloc((s))
#define kfree free

static inline void *kzalloc(size_t size, u32_t flags)
{
    void *ret = malloc(size);
    memset(ret, 0, size);
    return ret;
}

struct drm_file;

#define offsetof(TYPE,MEMBER) __builtin_offsetof(TYPE,MEMBER)

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})



#define DRM_MEMORYBARRIER() __asm__ __volatile__("lock; addl $0,0(%esp)")
#define mb() __asm__ __volatile__("lock; addl $0,0(%esp)")

#define PAGE_SIZE 4096
#define PAGE_SHIFT      12

#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))

static inline void bitmap_zero(unsigned long *dst, int nbits)
{
        if (nbits <= BITS_PER_LONG)
                *dst = 0UL;
        else {
                int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
                memset(dst, 0, len);
        }
}

#define EXPORT_SYMBOL(x)

#define min(x,y) ({ \
        typeof(x) _x = (x);     \
        typeof(y) _y = (y);     \
        (void) (&_x == &_y);            \
        _x < _y ? _x : _y; })

#define max(x,y) ({ \
        typeof(x) _x = (x);     \
        typeof(y) _y = (y);     \
        (void) (&_x == &_y);            \
        _x > _y ? _x : _y; })


extern uint32_t __div64_32(uint64_t *dividend, uint32_t divisor);

# define do_div(n,base) ({                             \
       uint32_t __base = (base);                       \
       uint32_t __rem;                                 \
       (void)(((typeof((n)) *)0) == ((uint64_t *)0));  \
       if (likely(((n) >> 32) == 0)) {                 \
               __rem = (uint32_t)(n) % __base;         \
               (n) = (uint32_t)(n) / __base;           \
       } else                                          \
               __rem = __div64_32(&(n), __base);       \
       __rem;                                          \
})

#define lower_32_bits(n) ((u32)(n))

#define INT_MAX         ((int)(~0U>>1))
#define INT_MIN         (-INT_MAX - 1)
#define UINT_MAX        (~0U)
#define LONG_MAX        ((long)(~0UL>>1))
#define LONG_MIN        (-LONG_MAX - 1)
#define ULONG_MAX       (~0UL)
#define LLONG_MAX       ((long long)(~0ULL>>1))
#define LLONG_MIN       (-LLONG_MAX - 1)
#define ULLONG_MAX      (~0ULL)


static inline void *kcalloc(size_t n, size_t size, u32_t flags)
{
        if (n != 0 && size > ULONG_MAX / n)
                return NULL;
        return kzalloc(n * size, 0);
}

#define ENTRY()   dbgprintf("entry %s\n",__FUNCTION__)
#define LEAVE()   dbgprintf("leave %s\n",__FUNCTION__)

#define ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))


#endif  //__TYPES_H__
