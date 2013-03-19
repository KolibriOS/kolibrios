#ifndef __SHMEM_FS_H
#define __SHMEM_FS_H

#include <kernel.h>

struct file *shmem_file_setup(const char *name, loff_t size, unsigned long flags);
struct page *shmem_read_mapping_page_gfp(struct file *filep,
                                         pgoff_t index, gfp_t gfp);


#endif
