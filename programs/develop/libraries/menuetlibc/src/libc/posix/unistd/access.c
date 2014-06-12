/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <io.h>
#include <dirent.h>
#include <errno.h>

int access(const char *fn, int flags)
{
  unsigned attr = _chmod(fn, 0);

  if (attr == -1) {
    char fixed_path[FILENAME_MAX];
    const char* p;
    int nums = 0;
    DIR* d;

    /* Root directories on some non-local drives (e.g. CD-ROM)
       might fail `_chmod'.  `findfirst' to the rescue.  */
    _fixpath(fn, fixed_path);
    for (p=fixed_path;*p;p++) if (*p == '/') ++nums;
    if (nums <= 2)
    {
      d = opendir(fn);
      if (d) {closedir(d);return 0;}
    }

    errno = ENOENT;
    return -1;
  }
 
  if (attr & 0x10)		/* directory? */
      return 0;			/* directories always OK */
  if (flags & D_OK)
  {
    errno = EACCES;
    return -1;			/* not a directory */
  }

  if ((flags & W_OK) && (attr & 1))
  {
    errno = EACCES;
    return -1;			/* not writable */
  }

  if (flags & X_OK)
  {
    if (!_is_executable(fn, 0, 0))
    {
      errno = EACCES;
      return -1;
    }
  }

  return 0;			/* everything else is OK */
}
