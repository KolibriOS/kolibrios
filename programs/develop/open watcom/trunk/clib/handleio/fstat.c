/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  OS/2 implementation of fstat().
*
****************************************************************************/


#include "variety.h"
#include "widechar.h"
#include <stddef.h>
#include <stdio.h>
#include <io.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <direct.h>
#include "iomode.h"
#include "rtcheck.h"
#include "seterrno.h"
#include "kolibri.h"

typedef struct 
{
  char     *name;
  unsigned int offset;
}__file_handle;


static unsigned short at2mode( int attr )
/***************************************/
    {
        register unsigned short         mode;

        if( attr & 0x10 ) {
            mode = S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
        } else {
            mode = S_IFREG;
        }
        mode |= S_IRUSR | S_IRGRP | S_IROTH;
        if( !(attr & (0x04 | 0x01 ) ) ) {
            mode |= S_IWUSR | S_IWGRP | S_IWOTH;
        }
        return( mode );
    }


#ifdef __WIDECHAR__
_WCRTLINK int _wfstat( int handle, struct _stat *buf )
#else
_WCRTLINK int fstat( int handle, struct stat *buf )
#endif
    {
        __file_handle   *fh;
        FILEINFO        info;
        int err;
        struct tm time;
        
        __handle_check( handle, -1 );

        fh = (__file_handle*) __getOSHandle(handle);
        err = get_fileinfo(fh->name,&info);

        if(err)
          return (-1);

        buf->st_mode = at2mode(info.attr);
        
        time.tm_sec   = info.ctime.sec;
        time.tm_min   = info.ctime.min;
        time.tm_hour  = info.ctime.hour;
        time.tm_mday  = info.cdate.day;
        time.tm_mon   = info.cdate.month;
        time.tm_year  = info.cdate.year - 1900;
        time.tm_isdst = -1;
        buf->st_ctime = mktime(&time);

        time.tm_sec   = info.atime.sec;
        time.tm_min   = info.atime.min;
        time.tm_hour  = info.atime.hour;
        time.tm_mday  = info.adate.day;
        time.tm_mon   = info.adate.month;
        time.tm_year  = info.adate.year - 1900;
        time.tm_isdst = -1;
        buf->st_atime = mktime(&time);
        
        time.tm_sec   = info.mtime.sec;
        time.tm_min   = info.mtime.min;
        time.tm_hour  = info.mtime.hour;
        time.tm_mday  = info.mdate.day;
        time.tm_mon   = info.mdate.month;
        time.tm_year  = info.mdate.year - 1900;
        time.tm_isdst = -1;
        buf->st_mtime = mktime(&time);

        buf->st_size  = info.size;
        buf->st_dev   = buf->st_rdev = 0;
        buf->st_attr  = info.attr;
        buf->st_nlink = 1;
        buf->st_ino   = handle;
        buf->st_uid   = buf->st_gid = 0;

        buf->st_btime = buf->st_mtime;
        buf->st_archivedID = 0;
        buf->st_updatedID = 0;
        buf->st_inheritedRightsMask = 0;
        buf->st_originatingNameSpace = 0;
       
        return( 0 );
};

