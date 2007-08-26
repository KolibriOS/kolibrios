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
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/


#include "variety.h"
#include <stdio.h>
#include <errno.h>

#include "iomode.h"
#include "fileacc.h"
#include "rtcheck.h"
#include "rtdata.h"
#include "seterrno.h"
#include "qwrite.h"
#include "liballoc.h"


/*
    Use caution when setting the file pointer in a multithreaded
    application. You must synchronize access to shared resources. For
    example, an application whose threads share a file handle, update the
    file pointer, and read from the file must protect this sequence by
    using a critical section object or a mutex object.
 */

typedef struct
{   DWORD    attr;
    DWORD    flags;
    DWORD    cr_time;
    DWORD    cr_date;
    DWORD    acc_time;
    DWORD    acc_date;
    DWORD    mod_time;
    DWORD    mod_date;
    DWORD    size;
    DWORD    size_high; 
} FILEINFO;

typedef struct 
{
  char     *name;
  unsigned int offset;
}__file_handle;

int _stdcall get_fileinfo(const char *name,FILEINFO* pinfo);
int _stdcall write_file(const char *name,const void *buff,unsigned offset,unsigned count,unsigned *writes);
char* getfullpath(const char* path);

int __qwrite( int handle, const void *buffer, unsigned len )
{
    int             atomic;
    __file_handle   *fh;
    unsigned        len_written;
    

    __handle_check( handle, -1 );

    fh = (__file_handle*) __getOSHandle( handle );

    atomic = 0;
    if( __GetIOMode( handle ) & _APPEND )
    {
      FILEINFO info;
        
      _AccessFileH( handle );
      atomic = 1;
      get_fileinfo(fh->name,&info);
      fh->offset = info.size;
    };

    if(write_file(fh->name,buffer,fh->offset,len,&len_written))
    {
      if ( len_written == 0)
      {
        if( atomic == 1 )
          _ReleaseFileH( handle );

        return (-1);
      };   
    };
    
    fh->offset+=len_written;
        

    if( atomic == 1 )
    {
        _ReleaseFileH( handle );
    }
    return( len_written );
}

/********************
int write_once(const char *name, void *buffer, unsigned len)
{   char *path;
    unsigned count;

    path= getfullpath(name);
    write_file(path,buffer,0,len,&count);
    lib_free(path);
    return count;
         
}

*******************/
