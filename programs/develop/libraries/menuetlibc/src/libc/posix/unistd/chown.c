/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
 
/* MS-DOS couldn't care less about file ownerships, so we could
   always succeed.  At least fail for non-existent files
   and for devices.  */
 
int chown(const char *path, uid_t owner, gid_t group)
{
    errno = ENOENT;
    return -1;
}
