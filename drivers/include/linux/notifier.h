/*
 *	Routines to manage notifier chains for passing status changes to any
 *	interested routines. We need this instead of hard coded call lists so
 *	that modules can poke their nose into the innards. The network devices
 *	needed them so here they are for the rest of you.
 *
 *				Alan Cox <Alan.Cox@linux.org>
 */
 
#ifndef _LINUX_NOTIFIER_H
#define _LINUX_NOTIFIER_H
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/rwsem.h>
/*
 * Notifier chains are of four types:
 *
 *	Atomic notifier chains: Chain callbacks run in interrupt/atomic
 *		context. Callouts are not allowed to block.
 *	Blocking notifier chains: Chain callbacks run in process context.
 *		Callouts are allowed to block.
 *	Raw notifier chains: There are no restrictions on callbacks,
 *		registration, or unregistration.  All locking and protection
 *		must be provided by the caller.
 *	SRCU notifier chains: A variant of blocking notifier chains, with
 *		the same restrictions.
 *
 * atomic_notifier_chain_register() may be called from an atomic context,
 * but blocking_notifier_chain_register() and srcu_notifier_chain_register()
 * must be called from a process context.  Ditto for the corresponding
 * _unregister() routines.
 *
 * atomic_notifier_chain_unregister(), blocking_notifier_chain_unregister(),
 * and srcu_notifier_chain_unregister() _must not_ be called from within
 * the call chain.
 *
 * SRCU notifier chains are an alternative form of blocking notifier chains.
 * They use SRCU (Sleepable Read-Copy Update) instead of rw-semaphores for
 * protection of the chain links.  This means there is _very_ low overhead
 * in srcu_notifier_call_chain(): no cache bounces and no memory barriers.
 * As compensation, srcu_notifier_chain_unregister() is rather expensive.
 * SRCU notifier chains should be used when the chain will be called very
 * often but notifier_blocks will seldom be removed.  Also, SRCU notifier
 * chains are slightly more difficult to use because they require special
 * runtime initialization.
 */

struct notifier_block;

typedef	int (*notifier_fn_t)(struct notifier_block *nb,
			unsigned long action, void *data);

struct notifier_block {
	notifier_fn_t notifier_call;
	struct notifier_block __rcu *next;
	int priority;
};

/* Console keyboard events.
 * Note: KBD_KEYCODE is always sent before KBD_UNBOUND_KEYCODE, KBD_UNICODE and
 * KBD_KEYSYM. */
#define KBD_KEYCODE		0x0001 /* Keyboard keycode, called before any other */
#define KBD_UNBOUND_KEYCODE	0x0002 /* Keyboard keycode which is not bound to any other */
#define KBD_UNICODE		0x0003 /* Keyboard unicode */
#define KBD_KEYSYM		0x0004 /* Keyboard keysym */
#define KBD_POST_KEYSYM		0x0005 /* Called after keyboard keysym interpretation */


#endif /* _LINUX_NOTIFIER_H */
