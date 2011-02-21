/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include "dirstruc.h"

struct dirent *
readdir(DIR *dir)
{
	dir->fileinfo.file_offset_low++;
	if (__kolibri__system_tree_access2(&dir->fileinfo))
		return NULL;
	dir->entry.d_namlen = strlen(dir->bdfename);
	strcpy(dir->entry.d_name, dir->bdfename);
	return &dir->entry;
}
