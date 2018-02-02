/*
 * Wrapper functions for accessing the file_struct fd array.
 */

#ifndef __LINUX_FILE_H
#define __LINUX_FILE_H

#include <linux/compiler.h>
#include <linux/types.h>
#include <linux/posix_types.h>

struct file;

extern void fput(struct file *);

struct file_operations;
struct vfsmount;
struct dentry;
struct path;
struct fd {
	struct file *file;
	unsigned int flags;
};
#define FDPUT_FPUT       1
#define FDPUT_POS_UNLOCK 2
extern struct file *fget(unsigned int fd);
#endif /* __LINUX_FILE_H */
