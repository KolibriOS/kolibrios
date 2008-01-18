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
* Description:  C Runtime write() and _lwrite() implementation.
*
****************************************************************************/


#include "variety.h"
#include "int64.h"
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <malloc.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include "iomode.h"
#include "fileacc.h"
#include "rtcheck.h"
#include "rtdata.h"
#include "seterrno.h"
#include "lseek.h"

/*
    The _lwrite function writes data to the specified file.

    Note: This function is provided only for compatibility with 16-bit
    versions of Windows. Win32-based applications should use the WriteFile
    function.

    UINT _lwrite(
      HFILE hFile,  // handle to file
      LPCSTR lpBuffer,  // write data buffer
      UINT uBytes   // number of bytes to write
    );
 */

extern  void    __STKOVERFLOW();

/*
    Win32 Note:
    Note that it is not an error to set the file pointer to a position
    beyond the end of the file. The size of the file does not increase
    until you call the SetEndOfFile, WriteFile, or WriteFileEx function. A
    write operation increases the size of the file to the file pointer
    position plus the size of the buffer written, leaving the intervening
    bytes uninitialized.  Hence, the reason for zero-padding the file
    to achieve POSIX conformance.
*/

/*
    POSIX Note:
    When writing to a file that does not support seek operations (pipe,
    device, etc.), the O_APPEND flag is effectively ignored. POSIX does
    not explicitly state this, but it is implied. Also, ESPIPE (illegal
    seek error) is not listed for write(), only pwrite(). Hence we must
    either not attempt to seek on such devices, or ignore the failures.
*/

#define PAD_SIZE 512

typedef union {
    unsigned __int64    _64;
    long                _32[2];
} __i64;

static int zero_pad( int handle )           /* 09-jan-95 */
/*******************************/
{
    int         rc;
    long        curPos, eodPos;
    long        bytesToWrite;
    unsigned    writeAmt;
    char        zeroBuf[PAD_SIZE];

    // Pad with zeros due to lseek() past EOF (POSIX)
    curPos = __lseek( handle, 0L, SEEK_CUR );   /* current offset */
    if( curPos == -1 )
        return( -1 );
    eodPos = __lseek( handle, 0L, SEEK_END );   /* end of data offset */
    if( eodPos == -1 )
        return( -1 );

    if( curPos > eodPos ) {
        bytesToWrite = curPos - eodPos;         /* amount to pad by */

        if( bytesToWrite > 0 ) {                /* only write if needed */
            memset( zeroBuf, 0x00, PAD_SIZE );  /* zero out a buffer */
            do {                                /* loop until done */
                if( bytesToWrite > PAD_SIZE )
                    writeAmt = 512;
                else
                    writeAmt = (unsigned)bytesToWrite;
                rc = write( handle, zeroBuf, writeAmt );
                if( rc < 0 )
                    return( rc );
                bytesToWrite -= writeAmt;       /* more bytes written */
            } while( bytesToWrite != 0 );
        }
    } else {
        curPos = __lseek( handle, curPos, SEEK_SET );
        if( curPos == -1 ) {
            return( -1 );
        }
    }

    return( 0 );                /* return success code */
}

/*
    The os_write function returns 0 meaning no error, -1 meaning error, or
    ENOSPC meaning no space left on device.
*/

typedef struct 
{
  char     *name;
  unsigned int offset;
}__file_handle;

int _stdcall write_file (const char *name,const void* buff,unsigned offset, unsigned count,unsigned *reads);

static int os_write( int handle, const void *buffer, unsigned len, unsigned *amt )
/********************************************************************************/
{
    __file_handle *fh;
    int         rc;

    rc = 0;
    
    fh = (__file_handle*) __getOSHandle( handle );
      
    if(write_file(fh->name,buffer,fh->offset,len,amt))
    {
       rc = __set_errno_nt();
    };


    if( *amt != len )
    {
        rc = ENOSPC;
        __set_errno( rc );
    }
    return( rc );
}

  _WCRTLINK int write( int handle, const void *buffer, unsigned len )
/**********************************************************************/
{
    unsigned    iomode_flags;
    char        *buf;
    unsigned    buf_size;
    unsigned    len_written, i, j;
    int         rc2;

    __file_handle *fh;

    __handle_check( handle, -1 );
    iomode_flags = __GetIOMode( handle );
    if( iomode_flags == 0 )
    {
        __set_errno( EBADF );
        return( -1 );
    }
    
    if( !(iomode_flags & _WRITE) ) {
        __set_errno( EACCES );     /* changed from EBADF to EACCES 23-feb-89 */
        return( -1 );
    }

    fh = (__file_handle*) __getOSHandle( handle );

    // put a semaphore around our writes

    _AccessFileH( handle );
    if( (iomode_flags & _APPEND) && !(iomode_flags & _ISTTY) )
    {
      fh->offset = __lseek( handle, 0L, SEEK_END );   /* end of data offset */
    }

    len_written = 0;
    rc2 = 0;

    // Pad the file with zeros if necessary
    if( iomode_flags & _FILEEXT ) {
        // turn off file extended flag
        __SetIOMode_nogrow( handle, iomode_flags&(~_FILEEXT) );

        // It is not required to pad a file with zeroes on an NTFS file system;
        // unfortunately it is required on FAT (and probably FAT32). (JBS)
        rc2 = zero_pad( handle );
    }

    if( rc2 == 0 ) {
        if( iomode_flags & _BINARY ) {  /* if binary mode */
            rc2 = os_write( handle, buffer, len, &len_written );
            /* end of binary mode part */
        } else {    /* text mode */
            i = stackavail();
            if( i < 0x00b0 ) {
                __STKOVERFLOW();    /* not enough stack space */
            }
            buf_size = 512;
            if( i < (512 + 48) ) {
                buf_size = 128;
            }

            buf = __alloca( buf_size );
            
            j = 0;
            for( i = 0; i < len; )
            {
                if( ((const char*)buffer)[i] == '\n' )
                {
                    buf[j] = '\r';
                    ++j;
                    if( j == buf_size )
                    {
                        rc2 = os_write( handle, buf, buf_size, &j );
                        if( rc2 == -1 )
                            break;
                        len_written += j;
                        if( rc2 == ENOSPC )
                            break;
                        len_written = i;
                        j = 0;
                    }
                }
                buf[j] = ((const char*)buffer)[i];
                ++i;
                ++j;
                if( j == buf_size ) {
                    rc2 = os_write( handle, buf, buf_size, &j );
                    if( rc2 == -1 )
                        break;
                    len_written += j;
                    if( rc2 == ENOSPC )
                        break;
                    len_written = i;
                    j = 0;
                }
            }
            if( j ) {
                rc2 = os_write( handle, buf, j, &i );
                if( rc2 == ENOSPC ) {
                    len_written += i;
                } else {
                    len_written = len;
                }
            }
            /* end of text mode part */
        }
    }
    _ReleaseFileH( handle );
    if( rc2 == -1 ) {
        return( rc2 );
    } else {
        return( len_written );
    }
}


