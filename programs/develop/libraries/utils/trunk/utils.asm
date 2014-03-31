;-----------------------------------------------------------------------------+
; Library "utils" (c) Sergei Steshin (Akyltist)                               |
;-----------------------------------------------------------------------------+
; Charset:DOS-866 Font:Courier New Size:9pt                                   |
; compiler:     FASM 1.69.31                                                  |
; version:      0.1.0                                                         |
; last update:  31/03/2014                                                    |
; e-mail:       dr.steshin@gmail.com                                          |
; license:      BSD                                                           |
;-----------------------------------------------------------------------------+

format MS COFF

public EXPORTS

section '.flat' code readable writable align 16

include '../../../../proc32.inc'
include '_ftoa.inc'
include '_atof.inc'
include '_rand.inc'


;-----------------------------------------------------------------------------+
; float to ascii string                                                       |
;-----------------------------------------------------------------------------+
ftoa:                                     ;
    mov     ebx, dword [esp+4]            ; out string
    mov     eax, dword [esp+8]            ; in  value
    stdcall FloatToString,eax,ebx         ;
    ret 8                                 ;


;-----------------------------------------------------------------------------+
; ascii string to float                                                       |
;-----------------------------------------------------------------------------+
atof:                                     ;
    mov     ebx, dword [esp+4]            ; out <- value
    mov     eax, dword [esp+8]            ; in  -> string
    stdcall string2float,eax,ebx          ;
    ret 8                                 ;


;-----------------------------------------------------------------------------+
; returns a random integer in the range [ 0...99999 ]                         |
;-----------------------------------------------------------------------------+
random:                                   ;
    call   _random                        ; out <- eax random
    ret                                   ;

;=============================================================================;
align 16
EXPORTS:
    dd  szFtoa   ,  ftoa
    dd  szAtof   ,  atof
    dd  szRandom ,  random
    dd  0        ,  0

    szFtoa    db 'ftoa'     ,0
    szAtof    db 'atof'     ,0
    szRandom  db 'random'   ,0

section '.data' data readable writable align 16

