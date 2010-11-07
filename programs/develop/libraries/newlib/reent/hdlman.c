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
* Description:  Handle manager routines.
*
****************************************************************************/

#include <reent.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#define _NFILES   20
#define _DYNAMIC  0x4000 /* FILE is dynamically allocated   */

#define _READ   0x0001  /* file opened for reading */
#define _WRITE  0x0002  /* file opened for writing */

#define NULL_HANDLE  (int)-1
#define DUMMY_HANDLE (int)-2
#define INVALID_HANDLE_VALUE (int) -1

#define _AccessIOB()
#define _ReleaseIOB()

#undef __getOSHandle

void __ChkTTYIOMode( int handle );

void  __grow_iomode( int num );
int   debugwrite(const char *path,const void *buff,
                 size_t offset, size_t count, size_t *writes);


int _fmode;

#define NUM_STD_STREAMS  3
#define _ISTTY           0x2000  /* is console device */

unsigned __init_mode[_NFILES] = { /* file mode information (flags) */
        _READ,          /* stdin */
        _WRITE,         /* stdout */
        _WRITE          /* stderr */
};

unsigned *__io_mode = __init_mode;      /* initially points to static array */

unsigned  __NFiles   = _NFILES;          /* maximum # of files we can open */

unsigned  __NHandles = 0;

int *__OSHandles = NULL;


static __file_handle
stdin_handle = {
                    NULL,
                    0,
                    NULL
                };

static __file_handle
stdout_handle =
                {
                    NULL,
                    0,
                    debugwrite
                };


static __file_handle
stderr_handle =
                {
                    NULL,
                    0,
                    debugwrite
                };


unsigned __growPOSIXHandles( unsigned num )
{
    int       *new2;
    unsigned   i;

    if( num > __NHandles )
    {
        if( __OSHandles == NULL )
        {
            new2 = malloc( num * sizeof( int ) );
        }
        else
        {
            new2 = realloc( __OSHandles, num * sizeof( int ) );
        }
        if( new2 == NULL )
        {
//            __set_errno( ENOMEM );
            num = __NHandles;
        }
        else
        {
            for( i = __NHandles; i < num; i++ )
            {
                new2[ i ] = NULL_HANDLE;
            }
            __OSHandles = new2;
            __NHandles = num;
        }
    }
    return( __NHandles );
}

int __allocPOSIXHandle( int hdl )
{
    int i;

    for( i = 0; i < __NHandles; i++ )
    {
        if( __OSHandles[i] == NULL_HANDLE ) break;
    }
    if( i >= __NHandles )
    {
                                // 20 -> (20+10+1) -> 31
                                // 31 -> (31+15+1) -> 47
                                // 47 -> (47+23+1) -> 71
        __growPOSIXHandles( i + (i >> 1) + 1 );
        // keep iomode array in sync
        if( __NHandles > __NFiles ) __grow_iomode( __NHandles );
        for( ; i < __NHandles; i++ )
        {
            if( __OSHandles[i] == NULL_HANDLE ) break;
        }
    }
    if( i >= __NHandles )
    {
        i = -1;
    } else {
        __OSHandles[i] = hdl;
    }
    return( i );
}

void __freePOSIXHandle( int hid )
{
    __OSHandles[ hid ] = NULL_HANDLE;
}


int __getOSHandle( int hid )
{
    return( __OSHandles[ hid ] );
}


int __setOSHandle( unsigned hid, int hdl )
{
    // call the Win32 API for a standard file handle
    switch( hid ) {
        case STDIN_FILENO:
//        SetStdHandle( STD_INPUT_HANDLE, hdl );
        break;
    case STDOUT_FILENO:
//        SetStdHandle( STD_OUTPUT_HANDLE, hdl );
        break;
    case STDERR_FILENO:
//        SetStdHandle( STD_ERROR_HANDLE, hdl );
        break;
    }
    if( hid < __NHandles )
    {
        __OSHandles[ hid ] = hdl;
    }
    else
    {
        hid = (unsigned)-1;     // this should never happen
    }
    return( hid );
}

// called from library startup code


void __initPOSIXHandles( void )
{
    int h;

    _fmode = O_BINARY;

    __growPOSIXHandles( __NFiles );

    h = (int)&stdin_handle;
    __allocPOSIXHandle( h );        // should return 0==STDIN_FILENO
    h = (int)&stdout_handle;
    __allocPOSIXHandle( h );        // should return 1==STDOUT_FILENO
    h = (int)&stderr_handle;
    __allocPOSIXHandle( h );        // should return 3==STDERR_FILENO
}

/*
static void __finiPOSIXHandles( void )
{
    if( __OSHandles != NULL ) {
        free( __OSHandles );
        __OSHandles = NULL;
    }
    if( __FakeHandles != NULL )
    {
        int i;
        for( i = 0 ; i < __topFakeHandle ; i++ )
        {
          //  CloseHandle( __FakeHandles[i] );
        }
        free( __FakeHandles );
        __FakeHandles = 0;
    }
}
*/


void __set_handles( int num )
{
    __NHandles = num;
}

int _grow_handles( int num )
{
    if( num > __NHandles )
    {
        num = __growPOSIXHandles( num );

        if( num > __NFiles ) {
            __grow_iomode( num );   // sets new __NFiles if successful
        }
        __NHandles = num;
    }
    return( __NHandles );
}



static  unsigned _init_NFiles;          // original __NFiles value;

void __grow_iomode( int num )
{
    unsigned    *new;

    _AccessIOB();
    if( __io_mode == __init_mode )
    {
        _init_NFiles = __NFiles;
        new = (unsigned *) malloc( num * sizeof( unsigned ) );
        if( new != NULL ) {
            memcpy( new, __init_mode, __NFiles * sizeof(unsigned) );
        }
    }
    else
    {
        new = (unsigned *) realloc( __io_mode, num * sizeof( unsigned ) );
    }
    if( new == NULL )
    {
//        __set_errno( ENOMEM );
    }
    else
    {
        memset( &new[__NFiles], 0, (num-__NFiles)*sizeof(unsigned) );
        __io_mode = new;
        __NFiles = num;
    }
    _ReleaseIOB();
}

void __shrink_iomode( void )
{
    _AccessIOB();
    // free any malloc'd iomode array
    if( __io_mode != __init_mode )
    {
        free( __io_mode );
        __io_mode = __init_mode;
        __NFiles = _init_NFiles;
    }
    _ReleaseIOB();
}

#define _INITIALIZED    _DYNAMIC

signed __SetIOMode( int handle, unsigned value )
{
    int         i;

    if( handle >= __NFiles )
    {
        i = __NFiles;           // 20 -> (20+10+1) -> 31
                                // 31 -> (31+15+1) -> 47
                                // 47 -> (47+23+1) -> 71
        __grow_iomode( i + (i > 1) + 1 );
    }
    if( handle >= __NFiles )
    {
        // return an error indication (errno should be set to ENOMEM)
        return( -1 );
    }
    else
    {
        if( value != 0 )
        {
            __ChkTTYIOMode( handle );
            __io_mode[handle] = value | _INITIALIZED;
        }
        else
        {
            __io_mode[handle] = value;    /* we're closing it; smite _INITIALIZED */
        }
        return( handle );
    }
}

int _isatty( int hid )
{
    return( 0 );
}

void __ChkTTYIOMode( int handle )
{
    if( handle < NUM_STD_STREAMS && !(__io_mode[handle] & _INITIALIZED) )
    {
        __io_mode[handle] |= _INITIALIZED;
        if( _isatty( handle ) )
        {
            __io_mode[handle] |= _ISTTY;
        }
    }
}

unsigned __GetIOMode( int handle )
{
    if( handle >= __NFiles )
    {
        return( 0 );
    }
    return( __io_mode[handle] );
};

void __SetIOMode_nogrow( int handle, unsigned value )
{
    if( handle < __NFiles )
    {
        __io_mode[handle] = value;    /* we're closing it; smite _INITIALIZED */
    }
}

