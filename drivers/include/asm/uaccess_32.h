#ifndef _ASM_X86_UACCESS_32_H
#define _ASM_X86_UACCESS_32_H

/*
 * User space memory access functions
 */
#include <linux/errno.h>
#include <linux/thread_info.h>
#include <linux/string.h>
#include <asm/asm.h>
#include <asm/page.h>

unsigned long __must_check __copy_to_user_ll
		(void __user *to, const void *from, unsigned long n);
unsigned long __must_check __copy_from_user_ll
		(void *to, const void __user *from, unsigned long n);
unsigned long __must_check __copy_from_user_ll_nozero
		(void *to, const void __user *from, unsigned long n);
unsigned long __must_check __copy_from_user_ll_nocache
		(void *to, const void __user *from, unsigned long n);
unsigned long __must_check __copy_from_user_ll_nocache_nozero
		(void *to, const void __user *from, unsigned long n);

/**
 * __copy_to_user_inatomic: - Copy a block of data into user space, with less checking.
 * @to:   Destination address, in user space.
 * @from: Source address, in kernel space.
 * @n:    Number of bytes to copy.
 *
 * Context: User context only.
 *
 * Copy data from kernel space to user space.  Caller must check
 * the specified block with access_ok() before calling this function.
 * The caller should also make sure he pins the user space address
 * so that we don't result in page fault and sleep.
 *
 * Here we special-case 1, 2 and 4-byte copy_*_user invocations.  On a fault
 * we return the initial request size (1, 2 or 4), as copy_*_user should do.
 * If a store crosses a page boundary and gets a fault, the x86 will not write
 * anything, so this is accurate.
 */

static __always_inline unsigned long __must_check
__copy_to_user_inatomic(void __user *to, const void *from, unsigned long n)
{
    if (__builtin_constant_p(n)) {
        switch(n) {
        case 1:
            *(u8 __force *)to = *(u8 *)from;
            return 0;
        case 2:
            *(u16 __force *)to = *(u16 *)from;
            return 0;
        case 4:
            *(u32 __force *)to = *(u32 *)from;
            return 0;

        case 8:
            *(u64 __force *)to = *(u64 *)from;
            return 0;

        default:
            break;
        }
    }

    __builtin_memcpy((void __force *)to, from, n);
    return 0;
}

/**
 * __copy_to_user: - Copy a block of data into user space, with less checking.
 * @to:   Destination address, in user space.
 * @from: Source address, in kernel space.
 * @n:    Number of bytes to copy.
 *
 * Context: User context only. This function may sleep if pagefaults are
 *          enabled.
 *
 * Copy data from kernel space to user space.  Caller must check
 * the specified block with access_ok() before calling this function.
 *
 * Returns number of bytes that could not be copied.
 * On success, this will be zero.
 */
static __always_inline unsigned long __must_check
__copy_to_user(void __user *to, const void *from, unsigned long n)
{
	might_fault();
	return __copy_to_user_inatomic(to, from, n);
}

static __always_inline unsigned long
__copy_from_user_inatomic(void *to, const void __user *from, unsigned long n)
{
	/* Avoid zeroing the tail if the copy fails..
	 * If 'n' is constant and 1, 2, or 4, we do still zero on a failure,
	 * but as the zeroing behaviour is only significant when n is not
	 * constant, that shouldn't be a problem.
	 */
    if (__builtin_constant_p(n)) {
        switch(n) {
        case 1:
            *(u8 *)to = *(u8 __force *)from;
            return 0;
        case 2:
            *(u16 *)to = *(u16 __force *)from;
            return 0;
        case 4:
            *(u32 *)to = *(u32 __force *)from;
            return 0;

        case 8:
            *(u64 *)to = *(u64 __force *)from;
            return 0;

        default:
            break;
        }
    }

    __builtin_memcpy(to, (const void __force *)from, n);
    return 0;
}

/**
 * __copy_from_user: - Copy a block of data from user space, with less checking.
 * @to:   Destination address, in kernel space.
 * @from: Source address, in user space.
 * @n:    Number of bytes to copy.
 *
 * Context: User context only. This function may sleep if pagefaults are
 *          enabled.
 *
 * Copy data from user space to kernel space.  Caller must check
 * the specified block with access_ok() before calling this function.
 *
 * Returns number of bytes that could not be copied.
 * On success, this will be zero.
 *
 * If some data could not be copied, this function will pad the copied
 * data to the requested size using zero bytes.
 *
 * An alternate version - __copy_from_user_inatomic() - may be called from
 * atomic context and will fail rather than sleep.  In this case the
 * uncopied bytes will *NOT* be padded with zeros.  See fs/filemap.h
 * for explanation of why this is needed.
 */
static __always_inline unsigned long
__copy_from_user(void *to, const void __user *from, unsigned long n)
{
	might_fault();
    if (__builtin_constant_p(n)) {
        switch(n) {
        case 1:
            *(u8 *)to = *(u8 __force *)from;
            return 0;
        case 2:
            *(u16 *)to = *(u16 __force *)from;
            return 0;
        case 4:
            *(u32 *)to = *(u32 __force *)from;
            return 0;

        case 8:
            *(u64 *)to = *(u64 __force *)from;
            return 0;

        default:
            break;
        }
    }

    __builtin_memcpy(to, (const void __force *)from, n);
    return 0;
}

static __always_inline unsigned long __copy_from_user_nocache(void *to,
				const void __user *from, unsigned long n)
{
	might_fault();
    if (__builtin_constant_p(n)) {
        switch(n) {
        case 1:
            *(u8 *)to = *(u8 __force *)from;
            return 0;
        case 2:
            *(u16 *)to = *(u16 __force *)from;
            return 0;
        case 4:
            *(u32 *)to = *(u32 __force *)from;
            return 0;
        default:
            break;
        }
    }
    __builtin_memcpy(to, (const void __force *)from, n);
    return 0;
}

static __always_inline unsigned long
__copy_from_user_inatomic_nocache(void *to, const void __user *from,
				  unsigned long n)
{
       return __copy_from_user_inatomic(to, from, n);
}

#endif /* _ASM_X86_UACCESS_32_H */
