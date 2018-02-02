#ifndef __LINUX_UACCESS_H__
#define __LINUX_UACCESS_H__

#include <linux/sched.h>
#include <asm/uaccess.h>
/*
 * These routines enable/disable the pagefault handler. If disabled, it will
 * not take any locks and go straight to the fixup table.
 *
 * User access methods will not sleep when called from a pagefault_disabled()
 * environment.
 */
static inline void pagefault_disable(void)
{
}

static inline void pagefault_enable(void)
{
}
#ifndef ARCH_HAS_NOCACHE_UACCESS

static inline unsigned long __copy_from_user_inatomic_nocache(void *to,
				const void __user *from, unsigned long n)
{
	return __copy_from_user_inatomic(to, from, n);
}

static inline unsigned long __copy_from_user_nocache(void *to,
				const void __user *from, unsigned long n)
{
	return __copy_from_user(to, from, n);
}

#endif		/* ARCH_HAS_NOCACHE_UACCESS */

#endif		/* __LINUX_UACCESS_H__ */
