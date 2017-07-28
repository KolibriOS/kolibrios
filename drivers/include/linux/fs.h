#ifndef _LINUX_FS_H
#define _LINUX_FS_H
#include <linux/list.h>
#include <linux/bug.h>
#include <linux/mutex.h>
#include <linux/rwsem.h>
#define MAY_EXEC		0x00000001
#define MAY_WRITE		0x00000002
#define MAY_READ		0x00000004
#define MAY_APPEND		0x00000008
#define MAY_ACCESS		0x00000010
#define MAY_OPEN		0x00000020
#define MAY_CHDIR		0x00000040
/* called from RCU mode, don't block */
#define MAY_NOT_BLOCK		0x00000080
/*
 * Attribute flags.  These should be or-ed together to figure out what
 * has been changed!
 */
#define ATTR_MODE	(1 << 0)
#define ATTR_UID	(1 << 1)
#define ATTR_GID	(1 << 2)
#define ATTR_SIZE	(1 << 3)
#define ATTR_ATIME	(1 << 4)
#define ATTR_MTIME	(1 << 5)
#define ATTR_CTIME	(1 << 6)
#define ATTR_ATIME_SET	(1 << 7)
#define ATTR_MTIME_SET	(1 << 8)
#define ATTR_FORCE	(1 << 9) /* Not a change, but a change it */
#define ATTR_ATTR_FLAG	(1 << 10)
#define ATTR_KILL_SUID	(1 << 11)
#define ATTR_KILL_SGID	(1 << 12)
#define ATTR_FILE	(1 << 13)
#define ATTR_KILL_PRIV	(1 << 14)
#define ATTR_OPEN	(1 << 15) /* Truncating from open(O_TRUNC) */
#define ATTR_TIMES_SET	(1 << 16)
/*
 * inode->i_mutex nesting subclasses for the lock validator:
 *
 * 0: the object of the current VFS operation
 * 1: parent
 * 2: child/target
 * 3: xattr
 * 4: second non-directory
 * 5: second parent (when locking independent directories in rename)
 *
 * I_MUTEX_NONDIR2 is for certain operations (such as rename) which lock two
 * non-directories at once.
 *
 * The locking order between these classes is
 * parent[2] -> child -> grandchild -> normal -> xattr -> second non-directory
 */
enum inode_i_mutex_lock_class
{
	I_MUTEX_NORMAL,
	I_MUTEX_PARENT,
	I_MUTEX_CHILD,
	I_MUTEX_XATTR,
	I_MUTEX_NONDIR2,
	I_MUTEX_PARENT2,
};
struct file {

	/*
	 * Protects f_ep_links, f_flags.
	 * Must not be taken from IRQ context.
	 */
	spinlock_t		f_lock;
	atomic_long_t		f_count;
	unsigned int 		f_flags;
	fmode_t			f_mode;

	/* needed for tty driver, and maybe others */
	void			*private_data;

    struct page  **pages;         /* physical memory backend */
    unsigned int   count;
    unsigned int   allocated;
    void           *vma;

} __attribute__((aligned(4)));	/* lest something weird decides that 2 is OK */
#define get_file_rcu(x) atomic_long_inc_not_zero(&(x)->f_count)
#define fput_atomic(x)	atomic_long_add_unless(&(x)->f_count, -1, 1)
#define file_count(x)	atomic_long_read(&(x)->f_count)
#define FL_POSIX	1
#define FL_FLOCK	2
#define FL_DELEG	4	/* NFSv4 delegation */
#define FL_ACCESS	8	/* not trying to lock, just looking */
#define FL_EXISTS	16	/* when unlocking, test for existence */
#define FL_LEASE	32	/* lease held on this file */
#define FL_CLOSE	64	/* unlock on close */
#define FL_SLEEP	128	/* A blocking lock */
#define FL_DOWNGRADE_PENDING	256 /* Lease is being downgraded */
#define FL_UNLOCK_PENDING	512 /* Lease is being broken */
#define FL_OFDLCK	1024	/* lock is "owned" by struct file */
#define FL_LAYOUT	2048	/* outstanding pNFS layout */
struct inode;
#endif /* _LINUX_FS_H */
