; SPDX-License-Identifier: GPL-2.0-only
;
; xml.obj - AsmXml parser as a KolibriOS library
; Copyright (C) KolibriOS team 2026
;
; asm-xml.asm and LICENSE.txt are the unmodified AsmXml 1.4 release
; by Marc Kerbiquet (BSD 3-clause), https://tibleiz.net/asm-xml/

format MS COFF

public EXPORTS

section '.flat' code readable align 16

; AsmXml calls malloc/free as cdecl, the heap functions given to lib_init
; are stdcall: the caller pops the argument they have already taken
lib_init:
        mov     [mem.alloc], eax
        mov     [mem.free], ebx
        mov     eax, malloc
        mov     ebx, free
        jmp     initialize

malloc:
        push    dword [esp + 4]
        call    [mem.alloc]
        ret

free:
        push    dword [esp + 4]
        call    [mem.free]
        ret

align 4
mem.alloc       dd ?
mem.free        dd ?

align 16
EXPORTS:
        dd      sz_lib_init,                    lib_init
        dd      sz_initializeParser,            _initializeParser
        dd      sz_releaseParser,               _releaseParser
        dd      sz_parse,                       _parse
        dd      sz_initializeClassParser,       _initializeClassParser
        dd      sz_releaseClassParser,          _releaseClassParser
        dd      sz_classFromElement,            _classFromElement
        dd      sz_classFromString,             _classFromString
        dd      0, 0

sz_lib_init                     db 'lib_init', 0
sz_initializeParser             db 'ax_initializeParser', 0
sz_releaseParser                db 'ax_releaseParser', 0
sz_parse                        db 'ax_parse', 0
sz_initializeClassParser        db 'ax_initializeClassParser', 0
sz_releaseClassParser           db 'ax_releaseClassParser', 0
sz_classFromElement             db 'ax_classFromElement', 0
sz_classFromString              db 'ax_classFromString', 0

include 'asm-xml.asm'
