
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

#define DRM_DEBUG(fmt, arg...)     \
    printk(KERN_ERR "[" DRM_NAME ":%s] *ERROR* " fmt , __func__ , ##arg)

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

//#include <string.h>

void*   memcpy(void *s1, const void *s2, size_t n);
void*   memset(void *s, int c, size_t n);
size_t  strlen(const char *s);

void *malloc(size_t size);

#define kfree free

static inline void *kzalloc(size_t size, u32_t flags)
{
    void *ret = malloc(size);
    memset(ret, 0, size);
    return ret;
}

struct drm_gem_object {

    /** Reference count of this object */
//    struct kref refcount;

    /** Handle count of this object. Each handle also holds a reference */
//    struct kref handlecount;

    /** Related drm device */
//    struct drm_device *dev;

    /** File representing the shmem storage */
//    struct file *filp;

    /* Mapping info for this object */
//    struct drm_map_list map_list;

    /**
     * Size of the object, in bytes.  Immutable over the object's
     * lifetime.
     */
    size_t size;

    /**
     * Global name for this object, starts at 1. 0 means unnamed.
     * Access is covered by the object_name_lock in the related drm_device
     */
    int name;

    /**
     * Memory domains. These monitor which caches contain read/write data
     * related to the object. When transitioning from one set of domains
     * to another, the driver is called to ensure that caches are suitably
     * flushed and invalidated
     */
    uint32_t read_domains;
    uint32_t write_domain;

    /**
     * While validating an exec operation, the
     * new read/write domain values are computed here.
     * They will be transferred to the above values
     * at the point that any cache flushing occurs
     */
    uint32_t pending_read_domains;
    uint32_t pending_write_domain;

    void *driver_private;
};

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


#endif  //__TYPES_H__
