/* stat.c -- Get the status of a file.
 *
 * Copyright (c) 1995 Cygnus Support
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */
#include <sys/stat.h>
#include <sys/ksys.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include "glue.h"

/*
 * stat -- Since we have no file system, we just return an error.
 */
int
_DEFUN (stat, (path, buf),
       const char *path _AND
       struct stat *buf)
{
    ksys_file_info_t info;
    struct tm time;

    if (_ksys_file_info(path, &info))
    {
        errno = ENOENT;
        return (-1);
    }

    memset(buf, 0, sizeof (*buf));

    buf->st_size = info.size;

    if (info.attr & (KSYS_FILE_ATTR_DIR | KSYS_FILE_ATTR_VOL_LABEL))
        buf->st_mode = S_IFDIR;
    else
    {
        if (info.attr & (KSYS_FILE_ATTR_SYS | KSYS_FILE_ATTR_HIDDEN | KSYS_FILE_ATTR_RO))
            buf->st_mode = S_IFREG|S_IRUSR|S_IXUSR;
        else
            buf->st_mode = S_IFREG|S_IRUSR|S_IWUSR|S_IXUSR;
    }

    buf->st_blksize = 4096;

    time.tm_sec   = info.atime.sec;
    time.tm_min   = info.atime.min;
    time.tm_hour  = info.atime.hour;
    time.tm_mday  = info.adate.day;
    time.tm_mon   = info.adate.month;
    time.tm_year  = info.adate.year - 1900;
    time.tm_isdst = -1;
    buf->st_atime = mktime(&time);

    time.tm_sec   = info.ctime.sec;
    time.tm_min   = info.ctime.min;
    time.tm_hour  = info.ctime.hour;
    time.tm_mday  = info.cdate.day;
    time.tm_mon   = info.cdate.month;
    time.tm_year  = info.cdate.year - 1900;
    time.tm_isdst = -1;
    buf->st_ctime = mktime(&time);

    time.tm_sec   = info.mtime.sec;
    time.tm_min   = info.mtime.min;
    time.tm_hour  = info.mtime.hour;
    time.tm_mday  = info.mdate.day;
    time.tm_mon   = info.mdate.month;
    time.tm_year  = info.mdate.year - 1900;
    time.tm_isdst = -1;
    buf->st_mtime = mktime(&time);

    return (0);
}


int
_DEFUN (lstat, (path, buf),
       const char *path _AND
       struct stat *buf)
{
    return stat(path, buf);
}
