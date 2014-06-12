/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include "dirstruc.h"

DIR *
opendir(const char *name)
{
	int res;
	DIR* dir = (DIR*)malloc(sizeof(DIR));
	if (!dir)
		return NULL;
	_fixpath(name, dir->fileinfo.name);
	struct bdfe_item attr;
	dir->fileinfo.command = 5;
	dir->fileinfo.file_offset_low = 0;
	dir->fileinfo.file_offset_high = 0;
	dir->fileinfo.size = 0;
	dir->fileinfo.data_pointer = (__u32)&attr;
	res = __kolibri__system_tree_access2(&dir->fileinfo);
	if (res!=0 && res!=2)
	{
		free(dir);
		return NULL;
	}
	if (res==0 && (attr.attr & 0x10)==0)
	{
		free(dir);
		return NULL;
	}
	dir->fileinfo.command = 1;
	dir->fileinfo.size = 1;
	dir->fileinfo.data_pointer = (__u32)dir->bdfeheader;
	dir->fileinfo.file_offset_low = (__u32)-1;
	return dir;
}
