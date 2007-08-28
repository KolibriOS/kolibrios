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
#include <io.h>
#include <fcntl.h>
#include <errno.h>
#include "iomode.h"
#include "fileacc.h"
#include "rtcheck.h"
#include "rtdata.h"
#include "seterrno.h"
#include "lseek.h"

typedef struct 
{
  char     *name;
  unsigned int offset;
}__file_handle;


int _stdcall read_file (const char *name,char *buff,unsigned offset, unsigned count,unsigned *reads);

_WCRTLINK int read( int handle, void *buf, unsigned len )
{
    unsigned read_len, total_len;
    unsigned reduce_idx, finish_idx;
    unsigned iomode_flags;
    char *buffer = buf;
    BOOL    rc;
    HANDLE  h;
    unsigned amount_read;
    __file_handle *fh;

    __handle_check( handle, -1 );
    __ChkTTYIOMode( handle );
    iomode_flags = __GetIOMode( handle );
    if( iomode_flags == 0 )
    {
        __set_errno( EBADF );
        return( -1 );
    }
    if( !(iomode_flags & _READ) )
    {
        __set_errno( EACCES );     /* changed from EBADF to EACCES 23-feb-89 */
        return( -1 );
    }


    fh = (__file_handle*) __getOSHandle( handle );
 
    if( iomode_flags & _BINARY )   /* if binary mode */
    {

      if(read_file(fh->name,buffer,fh->offset,len,&amount_read))
      {
        if ( amount_read == 0)
          return (-1);   
      }
      fh->offset+=amount_read;
      total_len = amount_read;
    }
    else
    {
        total_len = 0;
        read_len = len;
        do
        {
          if(read_file(fh->name,buffer,fh->offset,len,&amount_read))
          {
            if( amount_read == 0 )
            {                    /* EOF */
                break;
            }
          }  
          reduce_idx = 0;
          finish_idx = reduce_idx;
          for( ; reduce_idx < amount_read; ++reduce_idx )
          {
            if( buffer[ reduce_idx ] == 0x1a )     /* EOF */
            {
               __lseek( handle,
                           ((long)reduce_idx - (long)amount_read)+1L,
                           SEEK_CUR );
               total_len += finish_idx;
               return( total_len );
            }
            if( buffer[ reduce_idx ] != '\r' )
            {
                buffer[ finish_idx++ ] = buffer[ reduce_idx ];
            };    
          }

          total_len += finish_idx;
          buffer += finish_idx;
          read_len -= finish_idx;
          if( iomode_flags & _ISTTY )
          {
                break;  /* 04-feb-88, FWC */
          }
        } while( read_len != 0 );
    }
    return( total_len );
}

