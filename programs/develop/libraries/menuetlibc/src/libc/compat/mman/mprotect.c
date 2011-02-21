/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <assert.h>

int mprotect(void *addr, size_t len, int prot)
{
 return -1;
}
