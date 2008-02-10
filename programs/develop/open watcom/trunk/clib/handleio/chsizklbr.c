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
#include <windows.h>
#include <unistd.h>
#include "iomode.h"
#include "fileacc.h"
#include "rtcheck.h"
#include "seterrno.h"
#include "lseek.h"
#include "kolibri.h"

typedef struct 
{
  char     *name;
  unsigned int offset;
}__file_handle;

_WCRTLINK int chsize( int hid, long size )
{
    long        curOffset;
    long        rc;
    __file_handle *fh;

    __handle_check( hid, -1 );
    fh = (__file_handle*) __getOSHandle( hid );

    _AccessFileH( hid );
    
    curOffset = __lseek( hid, 0L, SEEK_CUR );     /* get current offset */

    rc = set_file_size(fh->name, size);
     
    if(rc !=0 )
    {
      _ReleaseFileH( hid );
      if(rc==8)
        __set_errno( ENOSPC );
      else
        __set_errno( EACCES );
      return -1;
    };     

    if( curOffset > size ) curOffset = size;
    curOffset = __lseek( hid, curOffset, SEEK_SET );
    _ReleaseFileH( hid );
    if( curOffset == -1 ) {
        __set_errno(ENOSPC);
        return( -1 );
    }
    return( 0 );
}
