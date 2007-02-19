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


#include "variety.h"
#include <stdio.h>
#include <stdlib.h>
#include "liballoc.h"
#include <string.h>
#ifdef _M_IX86
 #include <i86.h>
#endif
#include <unistd.h>
#include <errno.h>
#if defined(__OS2__)
#elif defined(__WINDOWS__) || defined(__NT__)
#endif
#include "iomode.h"
#include "fileacc.h"
#include "rtinit.h"
#include "seterrno.h"
#include "handleio.h"

#ifdef DLHEAP

void* _cdecl dlmalloc(size_t);
void  _cdecl dlfree(void*);
void _cdecl mf_init();

#define malloc  dlmalloc
#define free    dlfree
#define realloc dlrealloc

#define lib_malloc   dlmalloc
#define lib_free     dlfree
#define lib_realloc  dlrealloc

#endif


#undef __getOSHandle

extern  unsigned    __NFiles;       // the size of the iomode array
extern  void        __grow_iomode( int num );
        unsigned    __NHandles = 0;

#if defined(__NT__)

HANDLE *__OSHandles = NULL;

unsigned __growPOSIXHandles( unsigned num )
{
    HANDLE *new2;
    unsigned i;

    if( num > __NHandles )
    {
        _AccessFList();
        if( __OSHandles == NULL )
        {
            new2 = lib_malloc( num * sizeof( int ) );
        }
        else
        {
            new2 = lib_realloc( __OSHandles, num * sizeof( int ) );
        }
        if( new2 == NULL )
        {
            __set_errno( ENOMEM );
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
        _ReleaseFList();
    }
    return( __NHandles );
}

int __allocPOSIXHandle( HANDLE hdl )
{
    int i;

    _AccessFList();
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
    _ReleaseFList();
    return( i );
}

void __freePOSIXHandle( int hid )
{
    __OSHandles[ hid ] = NULL_HANDLE;
}


HANDLE __getOSHandle( int hid )
{
    return( __OSHandles[ hid ] );
}

int __setOSHandle( unsigned hid, HANDLE hdl )
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

HANDLE *__FakeHandles = 0;
static int __topFakeHandle = 0;

HANDLE __NTGetFakeHandle( void )
{
    HANDLE os_handle;

    _AccessFList();
    
//    os_handle = CreateEvent( 0, 0, 0, 0 );
    os_handle = 0;
    if( os_handle == NULL )
    {
        // win32s does not support event handles
        static DWORD fakeHandle = 0x80000000L;
        fakeHandle++;
        os_handle = (HANDLE)fakeHandle;
    }
    else
    {
        __FakeHandles = lib_realloc( __FakeHandles, (__topFakeHandle+1) * sizeof( HANDLE ) );
        __FakeHandles[ __topFakeHandle ] = os_handle;
        __topFakeHandle++;
    }
    _ReleaseFList();
    return( os_handle );
}

// called from library startup code

void __initPOSIXHandles( void )
{
    HANDLE h;

    // __OSHandles = NULL;
    // __NHandles = 0;

    __growPOSIXHandles( __NFiles );
    h = 0; //GetStdHandle( STD_INPUT_HANDLE );
    if( h == 0 || h == INVALID_HANDLE_VALUE ) {
        h = (HANDLE)__NTGetFakeHandle();
    }
    __allocPOSIXHandle( h );        // should return 0==STDIN_FILENO
    h = 0; //GetStdHandle( STD_OUTPUT_HANDLE );
    if( h == 0 || h == INVALID_HANDLE_VALUE ) {
        h = (HANDLE)__NTGetFakeHandle();
    }
    __allocPOSIXHandle( h );        // should return 1==STDOUT_FILENO
    h = 0; //GetStdHandle( STD_ERROR_HANDLE );
    if( h == 0 || h == INVALID_HANDLE_VALUE ) {
        h = (HANDLE)__NTGetFakeHandle();
    }
    __allocPOSIXHandle( h );        // should return 3==STDERR_FILENO
}

static void __finiPOSIXHandles( void )
{
    if( __OSHandles != NULL ) {
        lib_free( __OSHandles );
        __OSHandles = NULL;
    }
    if( __FakeHandles != NULL )
    {
        int i;
        for( i = 0 ; i < __topFakeHandle ; i++ )
        {
          //  CloseHandle( __FakeHandles[i] );
        }
        lib_free( __FakeHandles );
        __FakeHandles = 0;
    }
}

AYI( __finiPOSIXHandles, INIT_PRIORITY_LIBRARY-1 )

#endif


void __set_handles( int num )
{
    __NHandles = num;
}

_WCRTLINK int _grow_handles( int num )
{
    if( num > __NHandles )
    {
        #if defined(MSDOS)
        #elif defined( __OS2_286__ )
        #elif defined( __WARP__ )
        #elif defined(__WINDOWS__)
        #elif defined(__NT__)
        {
            num = __growPOSIXHandles( num );
        }
        #elif defined(__NETWARE__)
        #elif defined(__UNIX__)
        #endif

        if( num > __NFiles ) {
            __grow_iomode( num );   // sets new __NFiles if successful
        }
        __NHandles = num;
    }
    return( __NHandles );
}
