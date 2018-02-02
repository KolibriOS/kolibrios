/*
 * kernfs.h - pseudo filesystem decoupled from vfs locking
 *
 * This file is released under the GPLv2.
 */

#ifndef __LINUX_KERNFS_H
#define __LINUX_KERNFS_H

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/idr.h>
#include <linux/lockdep.h>
#include <linux/rbtree.h>
#include <linux/atomic.h>
#include <linux/wait.h>

struct file;
struct dentry;
struct iattr;
struct seq_file;
struct vm_area_struct;
struct super_block;
struct file_system_type;

struct kernfs_open_node;
struct kernfs_iattrs;

enum kernfs_node_type {
	KERNFS_DIR		= 0x0001,
	KERNFS_FILE		= 0x0002,
	KERNFS_LINK		= 0x0004,
};

#define KERNFS_TYPE_MASK	0x000f
#define KERNFS_FLAG_MASK	~KERNFS_TYPE_MASK

enum kernfs_node_flag {
	KERNFS_ACTIVATED	= 0x0010,
	KERNFS_NS		= 0x0020,
	KERNFS_HAS_SEQ_SHOW	= 0x0040,
	KERNFS_HAS_MMAP		= 0x0080,
	KERNFS_LOCKDEP		= 0x0100,
	KERNFS_SUICIDAL		= 0x0400,
	KERNFS_SUICIDED		= 0x0800,
	KERNFS_EMPTY_DIR	= 0x1000,
};

/* @flags for kernfs_create_root() */
enum kernfs_root_flag {
	/*
	 * kernfs_nodes are created in the deactivated state and invisible.
	 * They require explicit kernfs_activate() to become visible.  This
	 * can be used to make related nodes become visible atomically
	 * after all nodes are created successfully.
	 */
	KERNFS_ROOT_CREATE_DEACTIVATED		= 0x0001,

	/*
	 * For regular flies, if the opener has CAP_DAC_OVERRIDE, open(2)
	 * succeeds regardless of the RW permissions.  sysfs had an extra
	 * layer of enforcement where open(2) fails with -EACCES regardless
	 * of CAP_DAC_OVERRIDE if the permission doesn't have the
	 * respective read or write access at all (none of S_IRUGO or
	 * S_IWUGO) or the respective operation isn't implemented.  The
	 * following flag enables that behavior.
	 */
	KERNFS_ROOT_EXTRA_OPEN_PERM_CHECK	= 0x0002,
};

/* type-specific structures for kernfs_node union members */
struct kernfs_elem_dir {
	unsigned long		subdirs;
	/* children rbtree starts here and goes through kn->rb */
	struct rb_root		children;

	/*
	 * The kernfs hierarchy this directory belongs to.  This fits
	 * better directly in kernfs_node but is here to save space.
	 */
	struct kernfs_root	*root;
};

struct kernfs_elem_symlink {
	struct kernfs_node	*target_kn;
};

struct kernfs_elem_attr {
	const struct kernfs_ops	*ops;
	struct kernfs_open_node	*open;
	loff_t			size;
	struct kernfs_node	*notify_next;	/* for kernfs_notify() */
};

/*
 * kernfs_node - the building block of kernfs hierarchy.  Each and every
 * kernfs node is represented by single kernfs_node.  Most fields are
 * private to kernfs and shouldn't be accessed directly by kernfs users.
 *
 * As long as s_count reference is held, the kernfs_node itself is
 * accessible.  Dereferencing elem or any other outer entity requires
 * active reference.
 */
struct kernfs_node {
	atomic_t		count;
	atomic_t		active;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map	dep_map;
#endif
	/*
	 * Use kernfs_get_parent() and kernfs_name/path() instead of
	 * accessing the following two fields directly.  If the node is
	 * never moved to a different parent, it is safe to access the
	 * parent directly.
	 */
	struct kernfs_node	*parent;
	const char		*name;

	struct rb_node		rb;

	const void		*ns;	/* namespace tag */
	unsigned int		hash;	/* ns + name hash */
	union {
		struct kernfs_elem_dir		dir;
		struct kernfs_elem_symlink	symlink;
		struct kernfs_elem_attr		attr;
	};

	void			*priv;

	unsigned short		flags;
	umode_t			mode;
	unsigned int		ino;
	struct kernfs_iattrs	*iattr;
};


#endif	/* __LINUX_KERNFS_H */
