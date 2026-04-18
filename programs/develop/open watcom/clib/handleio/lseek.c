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
* Description:  lseek wrapper with positive -> extend file check
*
****************************************************************************/


#include "variety.h"
#include <stdio.h>
#include <unistd.h>
#ifdef __NT__
#include <windows.h>
#endif
#include "iomode.h"
#include "rtcheck.h"
#include "seterrno.h"
#include "lseek.h"

_WCRTLINK long lseek( int handle, long offset, int origin )
{
    unsigned            iomode_flags;

    __handle_check( handle, -1 );

    /*** Set the _FILEEXT iomode_flags bit if positive offset ***/
    iomode_flags = __GetIOMode( handle );

    if( offset > 0 && !(iomode_flags & _APPEND) )
        __SetIOMode( handle, iomode_flags | _FILEEXT );
    return( __lseek( handle, offset, origin ) );
}
