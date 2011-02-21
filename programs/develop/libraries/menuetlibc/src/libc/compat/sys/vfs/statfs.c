/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <errno.h>
#include <sys/vfs.h>
#include <ctype.h>

int statfs(const char *path, struct statfs *buf)
{
  buf->f_bavail = 0xFFFFFFFF;
  buf->f_bfree = 0xFFFFFFFF;
  buf->f_blocks = 0xFFFF;
  buf->f_bsize = 512;
  buf->f_ffree = 0xFFFF;
  buf->f_files = 0xFFFF;
  buf->f_type = 0;
  buf->f_fsid[0] = 0;
  buf->f_fsid[1] = MOUNT_UFS;
  buf->f_magic = FS_MAGIC;
  return 0;
}
