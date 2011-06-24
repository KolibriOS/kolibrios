#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

#include <asm/types.h>

#ifndef __ASSEMBLY__
#ifdef	__KERNEL__

#define DECLARE_BITMAP(name,bits) \
        unsigned long name[BITS_TO_LONGS(bits)]
#else
#ifndef __EXPORTED_HEADERS__
#warning "Attempt to use kernel headers from user space, see http://kernelnewbies.org/KernelHeaders"
#endif /* __EXPORTED_HEADERS__ */
#endif

#include <linux/posix_types.h>

#ifdef __KERNEL__

typedef __u32 __kernel_dev_t;

typedef __kernel_fd_set		fd_set;
typedef __kernel_dev_t		dev_t;
typedef __kernel_ino_t		ino_t;
typedef __kernel_mode_t		mode_t;
typedef __kernel_nlink_t	nlink_t;
typedef __kernel_off_t		off_t;
typedef __kernel_pid_t		pid_t;
typedef __kernel_daddr_t	daddr_t;
typedef __kernel_key_t		key_t;
typedef __kernel_suseconds_t	suseconds_t;
typedef __kernel_timer_t	timer_t;
typedef __kernel_clockid_t	clockid_t;
typedef __kernel_mqd_t		mqd_t;

typedef _Bool			bool;

typedef __kernel_uid32_t	uid_t;
typedef __kernel_gid32_t	gid_t;
typedef __kernel_uid16_t        uid16_t;
typedef __kernel_gid16_t        gid16_t;

typedef unsigned long		uintptr_t;

#ifdef CONFIG_UID16
/* This is defined by include/asm-{arch}/posix_types.h */
typedef __kernel_old_uid_t	old_uid_t;
typedef __kernel_old_gid_t	old_gid_t;
#endif /* CONFIG_UID16 */

#if defined(__GNUC__)
typedef __kernel_loff_t		loff_t;
#endif

/*
 * The following typedefs are also protected by individual ifdefs for
 * historical reasons:
 */
#ifndef _SIZE_T
#define _SIZE_T
typedef __kernel_size_t		size_t;
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
typedef __kernel_ssize_t	ssize_t;
#endif

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef __kernel_ptrdiff_t	ptrdiff_t;
#endif

#ifndef _TIME_T
#define _TIME_T
typedef __kernel_time_t		time_t;
#endif

#ifndef _CLOCK_T
#define _CLOCK_T
typedef __kernel_clock_t	clock_t;
#endif

#ifndef _CADDR_T
#define _CADDR_T
typedef __kernel_caddr_t	caddr_t;
#endif

/* bsd */
typedef unsigned char		u_char;
typedef unsigned short		u_short;
typedef unsigned int		u_int;
typedef unsigned long		u_long;

/* sysv */
typedef unsigned char		unchar;
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long		ulong;

#ifndef __BIT_TYPES_DEFINED__
#define __BIT_TYPES_DEFINED__

typedef		__u8		u_int8_t;
typedef		__s8		int8_t;
typedef		__u16		u_int16_t;
typedef		__s16		int16_t;
typedef		__u32		u_int32_t;
typedef		__s32		int32_t;

#endif /* !(__BIT_TYPES_DEFINED__) */

typedef		__u8		uint8_t;
typedef		__u16		uint16_t;
typedef		__u32		uint32_t;

#if defined(__GNUC__)
typedef		__u64		uint64_t;
typedef		__u64		u_int64_t;
typedef		__s64		int64_t;
#endif

/* this is a special 64bit data type that is 8-byte aligned */
#define aligned_u64 __u64 __attribute__((aligned(8)))
#define aligned_be64 __be64 __attribute__((aligned(8)))
#define aligned_le64 __le64 __attribute__((aligned(8)))

/**
 * The type used for indexing onto a disc or disc partition.
 *
 * Linux always considers sectors to be 512 bytes long independently
 * of the devices real block size.
 *
 * blkcnt_t is the type of the inode's block count.
 */
#ifdef CONFIG_LBDAF
typedef u64 sector_t;
typedef u64 blkcnt_t;
#else
typedef unsigned long sector_t;
typedef unsigned long blkcnt_t;
#endif

/*
 * The type of an index into the pagecache.  Use a #define so asm/types.h
 * can override it.
 */
#ifndef pgoff_t
#define pgoff_t unsigned long
#endif

#endif /* __KERNEL__ */

/*
 * Below are truly Linux-specific types that should never collide with
 * any application/library that wants linux/types.h.
 */

#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif
#ifdef __CHECK_ENDIAN__
#define __bitwise __bitwise__
#else
#define __bitwise
#endif

typedef __u16 __bitwise __le16;
typedef __u16 __bitwise __be16;
typedef __u32 __bitwise __le32;
typedef __u32 __bitwise __be32;
typedef __u64 __bitwise __le64;
typedef __u64 __bitwise __be64;

typedef __u16 __bitwise __sum16;
typedef __u32 __bitwise __wsum;

/*
 * aligned_u64 should be used in defining kernel<->userspace ABIs to avoid
 * common 32/64-bit compat problems.
 * 64-bit values align to 4-byte boundaries on x86_32 (and possibly other
 * architectures) and to 8-byte boundaries on 64-bit architetures.  The new
 * aligned_64 type enforces 8-byte alignment so that structs containing
 * aligned_64 values have the same alignment on 32-bit and 64-bit architectures.
 * No conversions are necessary between 32-bit user-space and a 64-bit kernel.
 */
#define __aligned_u64 __u64 __attribute__((aligned(8)))
#define __aligned_be64 __be64 __attribute__((aligned(8)))
#define __aligned_le64 __le64 __attribute__((aligned(8)))

#ifdef __KERNEL__
typedef unsigned __bitwise__ gfp_t;
typedef unsigned __bitwise__ fmode_t;

#ifdef CONFIG_PHYS_ADDR_T_64BIT
typedef u64 phys_addr_t;
#else
typedef u32 phys_addr_t;
#endif

typedef phys_addr_t resource_size_t;

typedef struct {
	int counter;
} atomic_t;

#ifdef CONFIG_64BIT
typedef struct {
	long counter;
} atomic64_t;
#endif

struct list_head {
	struct list_head *next, *prev;
};

struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

struct ustat {
	__kernel_daddr_t	f_tfree;
	__kernel_ino_t		f_tinode;
	char			f_fname[6];
	char			f_fpack[6];
};

#endif	/* __KERNEL__ */
#endif /*  __ASSEMBLY__ */




typedef unsigned char        u8_t;
typedef unsigned short       u16_t;
typedef unsigned int         u32_t;
typedef unsigned long long   u64_t;

typedef unsigned int         addr_t;
typedef unsigned int         count_t;

# define WARN(condition, format...)


#define false                0
#define true                 1


#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#define BITS_PER_LONG 32

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))



#define DRM_NAME    "drm"     /**< Name in kernel, /dev, and /proc */

#define DRM_INFO(fmt, arg...)  dbgprintf("DRM: "fmt , ##arg)

#define DRM_ERROR(fmt, arg...) \
    printk(KERN_ERR "[" DRM_NAME ":%s] *ERROR* " fmt , __func__ , ##arg)

#define BUILD_BUG_ON_ZERO(e) (sizeof(char[1 - 2 * !!(e)]) - 1)


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
#define GFP_ATOMIC           0

//#include <stdio.h>

int snprintf(char *str, size_t size, const char *format, ...);


//#include <string.h>

void*   memcpy(void *s1, const void *s2, size_t n);
void*   memset(void *s, int c, size_t n);
size_t  strlen(const char *s);
char *strcpy(char *s1, const char *s2);
char *strncpy (char *dst, const char *src, size_t len);

void *malloc(size_t size);
#define kfree free

static inline void *kzalloc(size_t size, uint32_t flags)
{
    void *ret = malloc(size);
    memset(ret, 0, size);
    return ret;
}

#define kmalloc(s,f) kzalloc((s), (f))

struct drm_file;


#define DRM_MEMORYBARRIER() __asm__ __volatile__("lock; addl $0,0(%esp)")
#define mb() __asm__ __volatile__("lock; addl $0,0(%esp)")


#define PAGE_SHIFT      12
#define PAGE_SIZE       (1UL << PAGE_SHIFT)
#define PAGE_MASK       (~(PAGE_SIZE-1))


#define do_div(n, base)                     \
({                              \
    unsigned long __upper, __low, __high, __mod, __base;    \
    __base = (base);                    \
    asm("":"=a" (__low), "=d" (__high) : "A" (n));      \
    __upper = __high;                   \
    if (__high) {                       \
        __upper = __high % (__base);            \
        __high = __high / (__base);         \
    }                           \
    asm("divl %2":"=a" (__low), "=d" (__mod)        \
        : "rm" (__base), "0" (__low), "1" (__upper));   \
    asm("":"=A" (n) : "a" (__low), "d" (__high));       \
    __mod;                          \
})



#define ENTER()   dbgprintf("enter %s\n",__FUNCTION__)
#define LEAVE()   dbgprintf("leave %s\n",__FUNCTION__)

struct timeval
{
  __kernel_time_t         tv_sec;         /* seconds */
  __kernel_suseconds_t    tv_usec;        /* microseconds */
};


#define PCI_DEVICE_ID_ATI_RADEON_QY 0x5159

#endif /* _LINUX_TYPES_H */





