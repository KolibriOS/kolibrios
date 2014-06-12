#include <libc/stubs.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dos.h>
#include <dir.h>
#include <libc/farptrgs.h>
#include <libc/bss.h>

#include "xstat.h"

int stat(const char *path, struct stat *statbuf)
{
	int nslash=0;
	const char* p;
	char* q;
	struct systree_info2 finf;
	struct bdfe_item attr;

	memset(statbuf,0,sizeof(*statbuf));
// "/<base>/<number>" is special case
	for (p=path;*p;p++) if (*p=='/') ++nslash;
	while (p>path && p[-1]=='/') {--nslash;--p;}
	if (nslash <= 2)
	{
		statbuf->st_mode = S_IFDIR;
		return 0;
	}
	finf.command = 5;
	finf.file_offset_low = 0;
	finf.file_offset_high = 0;
	finf.size = 0;
	finf.data_pointer = (__u32)&attr;
	_fixpath(path,finf.name);
	for (q=finf.name+strlen(finf.name)-1;q>=finf.name && *q=='/';q--) ;
	q[1]=0;
	if (__kolibri__system_tree_access2(&finf))
	{
		errno = EFAULT;
		return -1;
	}
	memset(statbuf,0,sizeof(*statbuf));
	statbuf->st_size = attr.filesize_low;
	if (attr.attr & 0x10)
		statbuf->st_mode = S_IFDIR;
	struct tm time;
	time.tm_sec = attr.atime.seconds;
	time.tm_min = attr.atime.minutes;
	time.tm_hour = attr.atime.hours;
	time.tm_mday = attr.adate.day;
	time.tm_mon = attr.adate.month;
	time.tm_year = attr.adate.year - 1900;
	time.tm_isdst = -1;
	statbuf->st_atime = mktime(&time);
	time.tm_sec = attr.ctime.seconds;
	time.tm_min = attr.ctime.minutes;
	time.tm_hour = attr.ctime.hours;
	time.tm_mday = attr.cdate.day;
	time.tm_mon = attr.cdate.month;
	time.tm_year = attr.cdate.year - 1900;
	time.tm_isdst = -1;
	statbuf->st_ctime = mktime(&time);
	time.tm_sec = attr.mtime.seconds;
	time.tm_min = attr.mtime.minutes;
	time.tm_hour = attr.mtime.hours;
	time.tm_mday = attr.mdate.day;
	time.tm_mon = attr.mdate.month;
	time.tm_year = attr.mdate.year - 1900;
	time.tm_isdst = -1;
	statbuf->st_mtime = mktime(&time);
	return 0;
}
