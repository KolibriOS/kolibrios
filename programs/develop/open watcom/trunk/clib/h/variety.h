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
* Description:  Configuration for clib builds.
*
****************************************************************************/


#ifndef _VARIETY_H_INCLUDED
#define _VARIETY_H_INCLUDED

//
// Note: for the DLL versions of the runtime libraries, this file must be
//       included before any of the runtime header files.
//

#ifndef _COMDEF_H_INCLUDED
     #include <_comdef.h>
#endif

// specialized data reference macro
#define _HUGEDATA       _WCDATA

// memory model macros
#if defined(__SMALL__)
    #define __SMALL_DATA__
    #define __SMALL_CODE__
#elif defined(__FLAT__)
    #define __SMALL_DATA__
    #define __SMALL_CODE__
#elif defined(__MEDIUM__)
    #define __SMALL_DATA__
    #define __BIG_CODE__
#elif defined(__COMPACT__)
    #define __BIG_DATA__
    #define __SMALL_CODE__
#elif defined(__LARGE__)
    #define __BIG_DATA__
    #define __BIG_CODE__
#elif defined(__HUGE__)
    #define __BIG_DATA__
    #define __BIG_CODE__
#elif defined(__AXP__) || defined(__PPC__) || defined(__MIPS__)
    // these effectively use near data references
    #define __SMALL_DATA__
    #define __SMALL_CODE__
#else
    #error unable to configure memory model
#endif

// operating system and processor macros
#if defined(__GENERIC__)
    #if defined(__386__)
        #define __PROTECT_MODE__
        #define __GENERIC_386__
    #elif defined(M_I86)
        #define __REAL_MODE__
        #define __GENERIC_086__
    #else
        #error unrecognized processor for GENERIC
    #endif
#elif defined(__OS2__)
    #if defined(M_I86)
        #define __REAL_MODE__
        #define __OS2_286__
    #elif defined(__386__)
        #define __PROTECT_MODE__
        #define __OS2_386__
        #define __WARP__
    #elif defined(__PPC__)
        #define __PROTECT_MODE__
        #define __OS2_PPC__
        #define __WARP__
    #else
        #error unrecognized processor for OS2
    #endif
#elif defined(__NT__)
    #if !defined(WIN32_LEAN_AND_MEAN) && !defined(WIN32_NICE_AND_FAT)
        #define WIN32_LEAN_AND_MEAN
    #endif
    #define __PROTECT_MODE__
    #if defined(__386__)
        #define __NT_386__
    #elif defined(__AXP__)
        #define __NT_AXP__
    #elif defined(__PPC__)
        #define __NT_PPC__
    #else
        #error unrecognized processor for NT
    #endif
#elif defined(__WINDOWS__) || defined(__WINDOWS_386__)
    #define __PROTECT_MODE__
    #if defined(__386__)
        #define __WINDOWS__
    #elif defined(M_I86)
        #define __WINDOWS_286__
    #else
        #error unrecognized processor for WINDOWS
    #endif
#elif defined(__DOS__)
    #if defined(__386__)
        #define __PROTECT_MODE__
        #define __DOS_386__
    #elif defined(M_I86)
        #define __REAL_MODE__
        #define __DOS_086__
    #else
        #error unrecognized processor for DOS
    #endif
#elif defined(__OSI__)
    #if defined(__386__)
        #define __PROTECT_MODE__
        #define __OSI_386__
    #else
        #error unrecognized processor for OSI
    #endif
#elif defined(__QNX__)
    #define __PROTECT_MODE__
    #define __UNIX__
    #if defined(__386__)
        #define __QNX_386__
    #elif defined(M_I86)
        #define __QNX_286__
    #else
        #error unrecognized processor for QNX
    #endif
#elif defined(__LINUX__)
    #define __PROTECT_MODE__
    #define __UNIX__
    #if defined(__386__)
        #define __LINUX_386__
    #elif defined(__PPC__)
        #define __LINUX_PPC__
    #elif defined(__MIPS__)
        #define __LINUX_MIPS__
    #else
        #error unrecognized processor for Linux
    #endif
#elif defined(__NETWARE__)
    #define __PROTECT_MODE__
    #if defined(__386__)
        #define __NETWARE_386__
    #else
        #error unrecognized processor for NETWARE
    #endif
#else
    #error unable to configure operating system and processor
#endif

// handle building dll's with appropriate linkage
#if !defined(__SW_BR) && (defined(__WARP__) || defined(__NT__))
    #if defined(__MAKE_DLL_CLIB)
        #undef _WCRTLINK
        #undef _WCIRTLINK
        #undef _WCRTLINKD
        #undef _WMRTLINK
        #undef _WMIRTLINK
        #undef _WMRTLINKD
        #undef _WPRTLINK
        #undef _WPIRTLINK
        #undef _WPRTLINKD
        #if defined(__NT__)
            #define _WCRTLINK __declspec(dllexport) _WRTLCALL
            #define _WCIRTLINK __declspec(dllexport) _WRTLCALL
            #define _WCRTLINKD __declspec(dllexport)
            #define _WMRTLINK __declspec(dllimport) _WRTLCALL
            #define _WMIRTLINK __declspec(dllimport) _WRTLCALL
            #define _WMRTLINKD __declspec(dllimport)
            #define _WPRTLINK __declspec(dllimport) _WRTLCALL
            #define _WPIRTLINK __declspec(dllimport) _WRTLCALL
            #define _WPRTLINKD __declspec(dllimport)
        #elif defined(__WARP__)
            #define _WCRTLINK __declspec(dllexport) _WRTLCALL
            #define _WCIRTLINK __declspec(dllexport) _WRTLCALL
            #define _WCRTLINKD __declspec(dllexport)
            #define _WMRTLINK _WRTLCALL
            #define _WMIRTLINK _WRTLCALL
            #define _WMRTLINKD
            #define _WPRTLINK _WRTLCALL
            #define _WPIRTLINK _WRTLCALL
            #define _WPRTLINKD
        #endif
    #elif defined(__MAKE_DLL_MATHLIB)
        #define _RTDLL
        #undef _WCRTLINK
        #undef _WCIRTLINK
        #undef _WCRTLINKD
        #undef _WMRTLINK
        #undef _WMIRTLINK
        #undef _WMRTLINKD
        #undef _WPRTLINK
        #undef _WPIRTLINK
        #undef _WPRTLINKD
        #if defined(__NT__)
            #define _WCRTLINK __declspec(dllimport) _WRTLCALL
            #define _WCIRTLINK __declspec(dllimport) _WRTLCALL
            #define _WCRTLINKD __declspec(dllimport)
            #define _WMRTLINK __declspec(dllexport) _WRTLCALL
            #define _WMIRTLINK __declspec(dllexport) _WRTLCALL
            #define _WMRTLINKD __declspec(dllexport)
            #define _WPRTLINK __declspec(dllimport) _WRTLCALL
            #define _WPIRTLINK __declspec(dllimport) _WRTLCALL
            #define _WPRTLINKD __declspec(dllimport)
        #elif defined(__WARP__)
            #define _WCRTLINK _WRTLCALL
            #define _WCIRTLINK _WRTLCALL
            #define _WCRTLINKD
            #define _WMRTLINK __declspec(dllexport) _WRTLCALL
            #define _WMIRTLINK __declspec(dllexport) _WRTLCALL
            #define _WMRTLINKD __declspec(dllexport)
            #define _WPRTLINK _WRTLCALL
            #define _WPIRTLINK _WRTLCALL
            #define _WPRTLINKD
        #endif
    #elif defined(__MAKE_DLL_CPPLIB)
        #define _RTDLL
        #undef _WCRTLINK
        #undef _WCIRTLINK
        #undef _WCRTLINKD
        #undef _WMRTLINK
        #undef _WMIRTLINK
        #undef _WMRTLINKD
        #undef _WPRTLINK
        #undef _WPIRTLINK
        #undef _WPRTLINKD
        #if defined(__NT__)
            #define _WCRTLINK __declspec(dllimport) _WRTLCALL
            #define _WCIRTLINK __declspec(dllimport) _WRTLCALL
            #define _WCRTLINKD __declspec(dllimport)
            #define _WMRTLINK __declspec(dllimport) _WRTLCALL
            #define _WMIRTLINK __declspec(dllimport) _WRTLCALL
            #define _WMRTLINKD __declspec(dllimport)
            #define _WPRTLINK __declspec(dllexport) _WRTLCALL
            #define _WPIRTLINK __declspec(dllexport) _WRTLCALL
            #define _WPRTLINKD __declspec(dllexport)
        #elif defined(__WARP__)
            #define _WCRTLINK _WRTLCALL
            #define _WCIRTLINK _WRTLCALL
            #define _WCRTLINKD
            #define _WMRTLINK _WRTLCALL
            #define _WMIRTLINK _WRTLCALL
            #define _WMRTLINKD
            #define _WPRTLINK __declspec(dllexport) _WRTLCALL
            #define _WPIRTLINK __declspec(dllexport) _WRTLCALL
            #define _WPRTLINKD __declspec(dllexport)
        #endif
    #endif
#endif

#define __ptr_check( p, a )
#define __null_check( p, a )
#define __stream_check( s, a )

#define __ROUND_UP_SIZE( __x, __amt ) (((__x)+((__amt)-1))&(~((__amt)-1)))
///
/// This doesn't work for far pointer's
///
///#define __ROUND_UP_PTR( __x, __amt )  ((void *)__ROUND_UP_SIZE((unsigned)(__x),__amt))
#if defined(M_I86)
    #define __ALIGN_SIZE( __x ) __ROUND_UP_SIZE( __x, 2 )
//    #define __ALIGN_PTR( __x )  __ROUND_UP_PTR( __x, 2 )
#elif defined(__386__)
    #define __ALIGN_SIZE( __x ) __ROUND_UP_SIZE( __x, 4 )
///    #define __ALIGN_PTR( __x )  __ROUND_UP_PTR( __x, 4 )
#else
    #define __ALIGN_SIZE( __x ) __ROUND_UP_SIZE( __x, 8 )
//    #define __ALIGN_PTR( __x )  __ROUND_UP_PTR( __x, 8 )
#endif

#endif
