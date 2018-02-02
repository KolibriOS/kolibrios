/*
 * sysfs.h - definitions for the device driver filesystem
 *
 * Copyright (c) 2001,2002 Patrick Mochel
 * Copyright (c) 2004 Silicon Graphics, Inc.
 * Copyright (c) 2007 SUSE Linux Products GmbH
 * Copyright (c) 2007 Tejun Heo <teheo@suse.de>
 *
 * Please see Documentation/filesystems/sysfs.txt for more information.
 */

#ifndef _SYSFS_H_
#define _SYSFS_H_
#include <linux/compiler.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/lockdep.h>
#include <linux/atomic.h>

struct kobject;
struct module;
struct bin_attribute;
enum kobj_ns_type;

struct attribute {
	const char		*name;
	umode_t			mode;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	bool			ignore_lockdep:1;
	struct lock_class_key	*key;
	struct lock_class_key	skey;
#endif
};
struct attribute_group {
	const char		*name;
	umode_t			(*is_visible)(struct kobject *,
					      struct attribute *, int);
	umode_t			(*is_bin_visible)(struct kobject *,
						  struct bin_attribute *, int);
	struct attribute	**attrs;
	struct bin_attribute	**bin_attrs;
};
#ifdef CONFIG_SYSFS

int __must_check sysfs_create_dir_ns(struct kobject *kobj, const void *ns);
void sysfs_remove_dir(struct kobject *kobj);
int __must_check sysfs_rename_dir_ns(struct kobject *kobj, const char *new_name,
				     const void *new_ns);
int __must_check sysfs_move_dir_ns(struct kobject *kobj,
				   struct kobject *new_parent_kobj,
				   const void *new_ns);
int __must_check sysfs_create_mount_point(struct kobject *parent_kobj,
					  const char *name);
void sysfs_remove_mount_point(struct kobject *parent_kobj,
			      const char *name);

int __must_check sysfs_create_file_ns(struct kobject *kobj,
				      const struct attribute *attr,
				      const void *ns);
int __must_check sysfs_create_files(struct kobject *kobj,
				   const struct attribute **attr);
int __must_check sysfs_chmod_file(struct kobject *kobj,
				  const struct attribute *attr, umode_t mode);
void sysfs_remove_file_ns(struct kobject *kobj, const struct attribute *attr,
			  const void *ns);
bool sysfs_remove_file_self(struct kobject *kobj, const struct attribute *attr);
void sysfs_remove_files(struct kobject *kobj, const struct attribute **attr);

int __must_check sysfs_create_bin_file(struct kobject *kobj,
				       const struct bin_attribute *attr);
void sysfs_remove_bin_file(struct kobject *kobj,
			   const struct bin_attribute *attr);

int __must_check sysfs_create_link(struct kobject *kobj, struct kobject *target,
				   const char *name);
int __must_check sysfs_create_link_nowarn(struct kobject *kobj,
					  struct kobject *target,
					  const char *name);
void sysfs_remove_link(struct kobject *kobj, const char *name);

int sysfs_rename_link_ns(struct kobject *kobj, struct kobject *target,
			 const char *old_name, const char *new_name,
			 const void *new_ns);

void sysfs_delete_link(struct kobject *dir, struct kobject *targ,
			const char *name);

int __must_check sysfs_create_group(struct kobject *kobj,
				    const struct attribute_group *grp);
int __must_check sysfs_create_groups(struct kobject *kobj,
				     const struct attribute_group **groups);
int sysfs_update_group(struct kobject *kobj,
		       const struct attribute_group *grp);
void sysfs_remove_group(struct kobject *kobj,
			const struct attribute_group *grp);
void sysfs_remove_groups(struct kobject *kobj,
			 const struct attribute_group **groups);
int sysfs_add_file_to_group(struct kobject *kobj,
			const struct attribute *attr, const char *group);
void sysfs_remove_file_from_group(struct kobject *kobj,
			const struct attribute *attr, const char *group);
int sysfs_merge_group(struct kobject *kobj,
		       const struct attribute_group *grp);
void sysfs_unmerge_group(struct kobject *kobj,
		       const struct attribute_group *grp);
int sysfs_add_link_to_group(struct kobject *kobj, const char *group_name,
			    struct kobject *target, const char *link_name);
void sysfs_remove_link_from_group(struct kobject *kobj, const char *group_name,
				  const char *link_name);
int __compat_only_sysfs_link_entry_to_kobj(struct kobject *kobj,
				      struct kobject *target_kobj,
				      const char *target_name);

void sysfs_notify(struct kobject *kobj, const char *dir, const char *attr);

int __must_check sysfs_init(void);

static inline void sysfs_enable_ns(struct kernfs_node *kn)
{
	return kernfs_enable_ns(kn);
}

#else /* CONFIG_SYSFS */

static inline int sysfs_create_dir_ns(struct kobject *kobj, const void *ns)
{
	return 0;
}

static inline void sysfs_remove_dir(struct kobject *kobj)
{
}

static inline int sysfs_rename_dir_ns(struct kobject *kobj,
				      const char *new_name, const void *new_ns)
{
	return 0;
}

static inline int sysfs_move_dir_ns(struct kobject *kobj,
				    struct kobject *new_parent_kobj,
				    const void *new_ns)
{
	return 0;
}

static inline int sysfs_create_mount_point(struct kobject *parent_kobj,
					   const char *name)
{
	return 0;
}

static inline void sysfs_remove_mount_point(struct kobject *parent_kobj,
					    const char *name)
{
}

static inline int sysfs_create_file_ns(struct kobject *kobj,
				       const struct attribute *attr,
				       const void *ns)
{
	return 0;
}

static inline int sysfs_create_files(struct kobject *kobj,
				    const struct attribute **attr)
{
	return 0;
}

static inline int sysfs_chmod_file(struct kobject *kobj,
				   const struct attribute *attr, umode_t mode)
{
	return 0;
}

static inline void sysfs_remove_file_ns(struct kobject *kobj,
					const struct attribute *attr,
					const void *ns)
{
}

static inline bool sysfs_remove_file_self(struct kobject *kobj,
					  const struct attribute *attr)
{
	return false;
}

static inline void sysfs_remove_files(struct kobject *kobj,
				     const struct attribute **attr)
{
}

static inline int sysfs_create_bin_file(struct kobject *kobj,
					const struct bin_attribute *attr)
{
	return 0;
}

static inline void sysfs_remove_bin_file(struct kobject *kobj,
					 const struct bin_attribute *attr)
{
}

static inline int sysfs_create_link(struct kobject *kobj,
				    struct kobject *target, const char *name)
{
	return 0;
}

static inline int sysfs_create_link_nowarn(struct kobject *kobj,
					   struct kobject *target,
					   const char *name)
{
	return 0;
}

static inline void sysfs_remove_link(struct kobject *kobj, const char *name)
{
}

static inline int sysfs_rename_link_ns(struct kobject *k, struct kobject *t,
				       const char *old_name,
				       const char *new_name, const void *ns)
{
	return 0;
}

static inline void sysfs_delete_link(struct kobject *k, struct kobject *t,
				     const char *name)
{
}

static inline int sysfs_create_group(struct kobject *kobj,
				     const struct attribute_group *grp)
{
	return 0;
}

static inline int sysfs_create_groups(struct kobject *kobj,
				      const struct attribute_group **groups)
{
	return 0;
}

static inline int sysfs_update_group(struct kobject *kobj,
				const struct attribute_group *grp)
{
	return 0;
}

static inline void sysfs_remove_group(struct kobject *kobj,
				      const struct attribute_group *grp)
{
}

static inline void sysfs_remove_groups(struct kobject *kobj,
				       const struct attribute_group **groups)
{
}

static inline int sysfs_add_file_to_group(struct kobject *kobj,
		const struct attribute *attr, const char *group)
{
	return 0;
}

static inline void sysfs_remove_file_from_group(struct kobject *kobj,
		const struct attribute *attr, const char *group)
{
}

static inline int sysfs_merge_group(struct kobject *kobj,
		       const struct attribute_group *grp)
{
	return 0;
}

static inline void sysfs_unmerge_group(struct kobject *kobj,
		       const struct attribute_group *grp)
{
}

static inline int sysfs_add_link_to_group(struct kobject *kobj,
		const char *group_name, struct kobject *target,
		const char *link_name)
{
	return 0;
}

static inline void sysfs_remove_link_from_group(struct kobject *kobj,
		const char *group_name, const char *link_name)
{
}

static inline int __compat_only_sysfs_link_entry_to_kobj(
	struct kobject *kobj,
	struct kobject *target_kobj,
	const char *target_name)
{
	return 0;
}

static inline void sysfs_notify(struct kobject *kobj, const char *dir,
				const char *attr)
{
}

static inline int __must_check sysfs_init(void)
{
	return 0;
}

#endif /* CONFIG_SYSFS */
#endif /* _SYSFS_H_ */
