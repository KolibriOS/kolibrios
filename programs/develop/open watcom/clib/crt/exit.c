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
* Description:  Implementation of exit() and associated functions.
*
****************************************************************************/


#include "variety.h"
#include "rtdata.h"
#include "exitwmsg.h"
#include "initfini.h"
#include "rtinit.h"
#include <stdio.h>
#include <stdlib.h>
//#include "defwin.h"
#include "widechar.h"
#include "initarg.h"

/*
  __int23_exit is used by OS/2 as a general termination routine which unhooks
  exception handlers.  A better name for this variable is __sig_exit.
  __sig_exit should be the system dependent signal termination routine and
  should replace the calls to __int23_exit and __FPE_handler_exit.
  Each OS should define its own __sig_exit and do the appropriate thing (for
  example, DOS version would call __int23_exit and __FPE_handler_exit)
*/


#if defined(__NT__) || defined(__WARP__)
_WCRTLINK extern void (*__process_fini)( unsigned, unsigned );
#endif


#if !defined(__UNIX__) && !defined(__WINDOWS_386__)
static void _null_exit_rtn() {}
void    (*__FPE_handler_exit)() = _null_exit_rtn;
#endif

_WCRTLINK void exit( int status )
{

#if defined(__UNIX__)
#elif defined(__WINDOWS_386__)
#elif defined(__NT__) || defined(__WARP__)
    __FiniRtns( FINI_PRIORITY_EXIT, 255 );
#else
    __FiniRtns( FINI_PRIORITY_EXIT, 255 );
#endif
    _exit( status );
}


#ifndef __NETWARE__

_WCRTLINK void _exit( int status )
{
    __exit( status );
}

#endif
