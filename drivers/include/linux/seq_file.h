#ifndef _LINUX_SEQ_FILE_H
#define _LINUX_SEQ_FILE_H

#include <linux/types.h>
#include <linux/string.h>
#include <linux/bug.h>
#include <linux/mutex.h>
struct file;
struct path;
struct inode;
struct dentry;

struct seq_file {
	char *buf;
	size_t size;
	size_t from;
	size_t count;
	size_t pad_until;
	loff_t index;
	loff_t read_pos;
	u64 version;
	void *private;
};

int seq_puts(struct seq_file *m, const char *s);

__printf(2, 3) int seq_printf(struct seq_file *, const char *, ...);


#endif
