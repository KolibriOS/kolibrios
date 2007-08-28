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
* Description:  Routines to create/destroy clib copy of OS environment.
*
****************************************************************************/


#include "variety.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <env.h>
#include "liballoc.h"
#include <string.h>
#include "filestr.h"
#include "rtdata.h"

extern char _WCI86FAR *_Envptr;

static char *_free_ep;

#define allocate lib_malloc

void __setenvp( void )
{
#if defined(__NETWARE__)
    // no environment support
#elif defined(__LINUX__)
#else
    char        _WCI86FAR *startp;
    char        _WCI86FAR *p;
    char    *ep;
    char    *my_env_mask;
    char    **my_environ;
    int     count;
    size_t  ep_size;
    size_t  env_size;

    /* if we are already initialized, then return */
    if( _RWD_environ != NULL ) return;           /* 10-jun-90 */

    // env_size = sys_get_environ(0,0)+1;
    // _Envptr = lib_malloc(env_size);
    // sys_get_environ(_Envptr, env_size)
    
    startp = _Envptr;
    
    count = 0;
    p = startp;
    while( *p ) {
        while( *++p );
        ++count;
        ++p;
    }
    ep_size = p - startp;
    if( ep_size == 0 ) {
        ep_size = 1;
    }
    ep = (char *)allocate( ep_size );
    if( ep ) {
        env_size = (count + 1) * sizeof(char *) + count * sizeof(char);
        my_environ = (char **)allocate( env_size );
        if( my_environ ) {
            _RWD_environ = my_environ;
            p = startp;
            _free_ep = ep;
            while( *p ) {
                *my_environ++ = ep;
                while( *ep++ = *p++ )
                    ;
            }
            *my_environ++ = NULL;
            _RWD_env_mask = my_env_mask = (char *) my_environ;
            for( ; count; count-- )
                *my_env_mask++ = 0;
        } else {
            lib_free( ep );
        }
    }

    /*** Handle the C_FILE_INFO entry ***/
    #ifdef __USE_POSIX_HANDLE_STRINGS
 //       __ParsePosixHandleStr();
    #endif
#endif
}

#if !defined(__NETWARE__) && !defined(__LINUX__)

void __freeenvp( void )
{
    clearenv();
    if( _RWD_environ ) {
        lib_free( _RWD_environ );
        _RWD_environ = NULL;
    }
    if( _free_ep ) {
        lib_free( _free_ep );
        _free_ep = NULL;
    }
}

#endif
