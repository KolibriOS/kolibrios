;*****************************************************************************
;*
;*                            Open Watcom Project
;*
;*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
;*
;*  ========================================================================
;*
;*    This file contains Original Code and/or Modifications of Original
;*    Code as defined in and that are subject to the Sybase Open Watcom
;*    Public License version 1.0 (the 'License'). You may not use this file
;*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
;*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
;*    provided with the Original Code and Modifications, and is also
;*    available at www.sybase.com/developer/opensource.
;*
;*    The Original Code and all software distributed under the License are
;*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
;*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
;*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
;*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
;*    NON-INFRINGEMENT. Please see the License for the specific language
;*    governing rights and limitations under the License.
;*
;*  ========================================================================
;*
;* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
;*               DESCRIBE IT HERE!
;*
;*****************************************************************************


;
; segment definitions for WATCOM C/C++32
;
include langenv.inc

        name    segdefns
.386p

        assume  nothing

        extrn   _cstart_         : near
        extrn   __STACKLOW       : near
        extrn   __STACKTOP       : near

  
DGROUP group _NULL,_AFTERNULL,CONST,_DATA,DATA,TIB,TI,TIE,XIB,XI,XIE,YIB,YI,YIE,_IEND,_BSS,STACK,MEMSIZE

; this guarantees that no function pointer will equal NULL
; (WLINK will keep segment 'BEGTEXT' in front)
; This segment must be at least 4 bytes in size to avoid confusing the
; signal function.

BEGTEXT segment use32 word public 'CODE'
        assume  cs:BEGTEXT
forever label   near

        db 'MENUET01'
        dd 0x0001
        dd offset _cstart_
        dd offset ___iend
        dd offset MEMSIZE
        dd offset MEMSIZE
        dd offset ___cmdline
        dd offset ___pgmname
        
        int     3h
        jmp     short forever
        ; NOTE that __begtext needs to be at offset 3
        ; don't move it.  i.e. don't change any code before here.
___begtext label byte
        nop
        nop
        nop
        nop
        public ___begtext
        assume  cs:nothing
BEGTEXT ends

        assume  ds:DGROUP

_NULL   segment para public 'BEGDATA'
__nullarea label word
        db      01h,01h,01h,00h
        public  __nullarea
_NULL   ends

_AFTERNULL segment word public 'BEGDATA'
_AFTERNULL ends

CONST   segment word public 'DATA'
CONST   ends

if COMP_CFG_COFF eq 0
TIB     segment byte public 'DATA'
TIB     ends
TI      segment byte public 'DATA'
TI      ends
TIE     segment byte public 'DATA'
TIE     ends

XIB     segment word public 'DATA'
_Start_XI label byte
        public  "C",_Start_XI
XIB     ends
XI      segment word public 'DATA'
XI      ends
XIE     segment word public 'DATA'
_End_XI label byte
        public  "C",_End_XI
XIE     ends

YIB     segment word public 'DATA'
_Start_YI label byte
        public  "C",_Start_YI
YIB     ends
YI      segment word public 'DATA'
YI      ends
YIE     segment word public 'DATA'
_End_YI label byte
        public  "C",_End_YI
YIE     ends
endif

_DATA    segment word public 'DATA'
_DATA    ends

DATA    segment word public 'DATA'
DATA    ends

_IEND   segment word public 'IEND'
___iend  label byte
_IEND   ends

_BSS    segment word public 'BSS'

        public ___cmdline
        public ___pgmname
        
___cmdline   db 256 dup(?)          ; pointer to raw command line
___pgmname   db 1024 dup (?)        ; pointer to program name (for argv[0])

_BSS    ends

STACK   segment para stack 'STACK'

___stack_low label byte
        public ___stack_low

STACK   ends

MEMSIZE   segment para stack 'STACK'
MEMSIZE   ends


_TEXT   segment use32 word public 'CODE'
_TEXT   ends

        end
