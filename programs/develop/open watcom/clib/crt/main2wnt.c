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
* Description:  Implementation of __NTMain().
*
****************************************************************************/


#include "variety.h"
#include "widechar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <windows.h>

#include "kolibri.h"

//#include "sigtab.h"
#include "initfini.h"
#include "initarg.h"

int  __appcwdlen;
char* __appcwd;
extern char *LpCmdLine;
extern char *LpPgmName;

_WCRTLINK void (*__process_fini)(unsigned,unsigned) = 0;

#ifdef __SW_BR
    _WCRTLINK extern    void    (*__process_fini)( unsigned, unsigned );
    extern      void    __CommonInit( void );
    extern      int     wmain( int, wchar_t ** );
    extern      int     main( int, char ** );
#else
    extern      void            __NTMainInit( void *, void * );
    #ifdef __WIDECHAR__
        extern  void            __wCMain( void );
        #if defined(_M_IX86)
            #pragma aux __wCMain  "*"
        #endif
    #else
        extern  void            __CMain( void );
        #if defined(_M_IX86)
            #pragma aux __CMain  "*"
        #endif
    #endif
    extern      unsigned        __ThreadDataSize;
#endif

void __F_NAME(__NTMain,__wNTMain)( void )
/***************************************/
{

   init_heap();

    __process_fini = &__FiniRtns;
   __InitRtns( 255 );
   __CommonInit();
   __initPOSIXHandles();
   __appcwdlen = strrchr(_LpPgmName, '/') - _LpPgmName + 1;
   __appcwdlen = __appcwdlen > 512 ? 512 : __appcwdlen;
   __appcwd= (char*)malloc(__appcwdlen);
   strncpy(__appcwd, _LpPgmName, __appcwdlen);
   __appcwd[__appcwdlen] = 0;
   ___Argv[0] = _LpPgmName;
   if( *_LpCmdLine != 0)
   {
      ___Argc = 2;
      ___Argv[1] = _LpCmdLine;
   } else ___Argc = 1;

   #ifdef __WIDECHAR__
      exit( wmain( ___wArgc, ___wArgv ) );
   #else
      exit( main( ___Argc, ___Argv ) );
   #endif
}

#ifdef __WIDECHAR__
    #if defined(_M_IX86)
        #pragma aux __wNTMain "*"
    #endif
#else
    #if defined(_M_IX86)
        #pragma aux __NTMain "*"
    #endif
#endif

#pragma aux __exit aborts;

_WCRTLINK void __exit( unsigned ret_code )
{
  __FiniRtns( 0, FINI_PRIORITY_EXIT-1 );
  _asm
  {
    mov eax, -1
    int 0x40
  }
}


