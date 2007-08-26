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
*    governing rights and limitations under the License.*
*  ========================================================================
*
* Description:  low level lseek without file extend for Windows NT
*
****************************************************************************/


#include "variety.h"
#include <stdio.h>
#include <unistd.h>
#include "iomode.h"
#include "rtcheck.h"
#include "seterrno.h"
#include "lseek.h"
#include "handleio.h"

/*
    DWORD SetFilePointer(
      HANDLE hFile,                // handle to file
      LONG lDistanceToMove,        // bytes to move pointer
      PLONG lpDistanceToMoveHigh,  // bytes to move pointer
      DWORD dwMoveMethod           // starting point
    );
 */

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER 0xFFFFFFFF
#endif

typedef struct 
{
  char     *name;
  unsigned int offset;
}__file_handle;

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

int _stdcall get_fileinfo(const char *name,FILEINFO* pinfo);

_WCRTLINK long __lseek( int hid, long offset, int origin )
{
    __file_handle *fh;
    long rc;
    
    __handle_check( hid, -1 );
    fh = (__file_handle*) __getOSHandle( hid );

    switch(origin)
    {
      case SEEK_SET:
        rc = offset;
        break;
      case SEEK_CUR:  
        rc = fh->offset + offset;
        break;
      case SEEK_END:
      {
        FILEINFO info;
        get_fileinfo(fh->name,&info);
        rc = offset + info.size;
        break;
      }    
      default:
        return -1;
    };
    
    fh->offset = rc;

    return( rc );
}

