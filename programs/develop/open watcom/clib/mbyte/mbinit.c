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

#include <string.h>
#include <mbstring.h>
#include "mbchar.h"


unsigned int __MBCodePage = 0;              /* default code page */

static void set_dbcs_table( int low, int high )
{
    memset( __MBCSIsTable + low + 1, _MB_LEAD, high - low + 1 );
}

static void clear_dbcs_table( void )
{
    __IsDBCS = 0;                           /* SBCS for now */
    __MBCodePage = 0;
    memset( __MBCSIsTable, 0, 257 );
}

/****
***** Initialize a multi-byte character set.  Returns 0 on success.
****/

int __mbinit( int codepage )
{

    /*** Handle values from _setmbcp ***/
    if( codepage == _MBINIT_CP_ANSI )
    {
      codepage = 0;
    }
    else
    if( codepage == _MBINIT_CP_OEM )
    {
      codepage = 0;
    }
    else
    if( codepage == _MBINIT_CP_SBCS )
    {
        clear_dbcs_table();
        return( 0 );
    }
    else
    if( codepage == _MBINIT_CP_932 )
    {
        clear_dbcs_table();
        set_dbcs_table( 0x81, 0x9F );
        set_dbcs_table( 0xE0, 0xFC );
        __IsDBCS = 1;
        __MBCodePage = 932;
        return( 0 );
    }
    return( 0 );                                /* return success code */
}



/****
***** Query DOS to find the valid lead byte ranges.
****/

#if defined(__DOS__) && !defined(__OSI__)
#ifndef __386__

// for some unknown reason NT DPMI returns for DOS service 6300h
// Carry=0, odd SI value and DS stay unchanged
// this case is also tested as wrong int 21h result
#if 1
#pragma aux             dos_get_dbcs_lead_table = \
        "push ds"       \
        "xor ax,ax"     \
        "mov ds,ax"     \
        "mov ah,63h"    /* get DBCS vector table */ \
        "int 21h"       \
        "mov di,ds"     \
        "jnc label1"    \
        "xor di,di"     \
        "label1:"       \
        "test di,di"    \
        "jnz exit1"     \
        "mov si,di"     \
        "exit1:"        \
        "pop ds"        \
        value           [di si] \
        modify          [ax bx cx dx si di es];
#else
unsigned short _WCFAR *dos_get_dbcs_lead_table( void )
/****************************************************/
{
    union REGS        regs;
    struct SREGS      sregs;

    regs.w.ax = 0x6300;                     /* get lead byte table code */
    sregs.ds = 0;
    sregs.es = 0;
    intdosx( &regs, &regs, &sregs );        /* call DOS */
    if( regs.w.cflag || ( sregs.ds == 0 ))  /* ensure function succeeded */
        return( NULL );
    return( MK_FP( sregs.ds, regs.w.si ) ); /* return pointer to table */
}
#endif

#if 0
unsigned short dos_get_code_page( void )
/**************************************/
{
    union REGS          regs;
    struct SREGS        sregs;
    unsigned char       buf[7];

    regs.w.ax = 0x6501;                     /* get international info */
    regs.w.bx = 0xFFFF;                     /* global code page */
    regs.w.cx = 7;                          /* buffer size */
    regs.w.dx = 0xFFFF;                     /* current country */
    regs.w.di = FP_OFF( (void __far*)buf ); /* buffer offset */
    sregs.es = FP_SEG( (void __far*)buf );  /* buffer segment */
    sregs.ds = 0;                           /* in protected mode (dos16m) DS must be initialized */
    intdosx( &regs, &regs, &sregs );        /* call DOS */
    if( regs.w.cflag )  return( 0 );        /* ensure function succeeded */
    return( * (unsigned short*)(buf+5) );   /* return code page */
}
#else
#pragma aux dos_get_code_page = \
        "push ds"       \
        "push bp"       \
        "mov bp,sp"     \
        "sub sp,8"      \
        "xor ax,ax"     \
        "mov ds,ax"     \
        "mov ax,6501h"  /* get international info */ \
        "mov bx,0ffffh" /* global code page */ \
        "mov cx,0007h"  /* buffer size */ \
        "mov dx,0ffffh" /* current country */ \
        "lea di,[bp-8]" /* buffer offset */ \
        "push ss"       \
        "pop es"        /* buffer segment */ \
        "int 21h"       /* call DOS */ \
        "mov ax,[bp-8+5]" /* code page */ \
        "jnc NoError"   \
        "xor ax,ax"     \
        "NoError:"      \
        "mov sp,bp"     \
        "pop bp"        \
        "pop ds"        \
        value           [ax] \
        modify          [ax bx cx dx di es];
#endif

#else


#pragma pack(__push,1);
typedef struct {
    unsigned short  int_num;
    unsigned short  real_ds;
    unsigned short  real_es;
    unsigned short  real_fs;
    unsigned short  real_gs;
    unsigned long   real_eax;
    unsigned long   real_edx;
} PHARLAP_block;
#pragma pack(__pop);

unsigned short _WCFAR *dos_get_dbcs_lead_table( void )
/****************************************************/
{
    union REGPACK       regs;

    if( _IsPharLap() ) {
        PHARLAP_block   pblock;

        memset( &pblock, 0, sizeof( pblock ) );
        memset( &regs, 0, sizeof( regs ) );
        pblock.real_eax = 0x6300;           /* get DBCS vector table */
        pblock.int_num = 0x21;              /* DOS call */
        regs.x.eax = 0x2511;                /* issue real-mode interrupt */
        regs.x.edx = FP_OFF( &pblock );     /* DS:EDX -> parameter block */
        regs.w.ds = FP_SEG( &pblock );
        intr( 0x21, &regs );
        if( pblock.real_ds != 0xFFFF ) {    /* weird OS/2 value */
            return( MK_FP( _ExtenderRealModeSelector,
                           (((unsigned)pblock.real_ds)<<4) + regs.w.si ) );
        }
    } else if( _IsRational() ) {
        rm_call_struct  dblock;

        memset( &dblock, 0, sizeof( dblock ) );
        dblock.eax = 0x6300;                /* get DBCS vector table */
        DPMISimulateRealModeInterrupt( 0x21, 0, 0, &dblock );
        if( (dblock.flags & 1) == 0 ) {
            return( MK_FP( _ExtenderRealModeSelector,
                           (((unsigned)dblock.ds)<<4) + dblock.esi ) );
        }
    }
    return( NULL );
}

unsigned short dos_get_code_page( void )
/**************************************/
{
    union REGPACK           regs;
    unsigned short __far  * temp;
    unsigned short          real_seg;
    unsigned short          codepage = 0;


    /*** Get the code page ***/
    if( _IsPharLap() ) {
        union REGS      r;
        PHARLAP_block   pblock;

        /*** Alloc DOS Memory under Phar Lap ***/
        memset( &r, 0, sizeof( r ) );
        r.x.ebx = 1;
        r.x.eax = 0x25c0;
        intdos( &r, &r );
        real_seg = r.w.ax;

        memset( &pblock, 0, sizeof( pblock ) );
        memset( &regs, 0, sizeof( regs ) );
        pblock.real_eax = 0x6501;           /* get international info */
        pblock.real_edx = 0xFFFF;           /* current country */
        pblock.real_es = real_seg;          /* buffer segment */
        regs.x.ebx = 0xFFFF;                /* global code page */
        regs.x.ecx = 7;                     /* buffer size */
        regs.x.edi = 0;                     /* buffer offset */
        pblock.int_num = 0x21;              /* DOS call */
        regs.x.eax = 0x2511;                /* issue real-mode interrupt */
        regs.x.edx = FP_OFF( &pblock );     /* DS:EDX -> parameter block */
        regs.w.ds = FP_SEG( &pblock );
        intr( 0x21, &regs );
        if( pblock.real_ds != 0xFFFF ) {    /* weird OS/2 value */
            temp = MK_FP( _ExtenderRealModeSelector, (real_seg<<4) + 5 );
            codepage = *temp;
        }

        /*** Free DOS Memory under Phar Lap ***/
        r.x.ecx = real_seg;
        r.x.eax = 0x25c1;
        intdos( &r, &r );
    } else if( _IsRational() ) {
        unsigned long       dpmi_rc;
        unsigned short      selector;
        rm_call_struct      dblock;

        /*** Allocate some DOS memory with DPMI ***/
        dpmi_rc = DPMIAllocateDOSMemoryBlock( 1 );      /* one paragraph is enough */
        real_seg = (unsigned short) dpmi_rc;
        selector = (unsigned short) (dpmi_rc>>16);

        memset( &dblock, 0, sizeof( dblock ) );
        dblock.eax = 0x6501;                /* get international info */
        dblock.ebx = 0xFFFF;                /* global code page */
        dblock.ecx = 7;                     /* buffer size */
        dblock.edx = 0xFFFF;                /* current country */
        dblock.edi = 0;                     /* buffer offset */
        dblock.es = real_seg;               /* buffer segment */
        DPMISimulateRealModeInterrupt( 0x21, 0, 0, &dblock );
        if( (dblock.flags & 1) == 0 ) {
            temp = MK_FP( _ExtenderRealModeSelector, (real_seg<<4) + 5 );
            codepage = *temp;
        }
        /*** Free DOS memory with DPMI ***/
        DPMIFreeDOSMemoryBlock( selector );
    }

    return( codepage );
}


#endif
#endif
