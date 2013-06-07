;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2010-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;; nslookup.asm - name service lookup (DNS)) program for KolibriOS ;;
;;                                                                 ;;
;;  Written by hidnplayr@kolibrios.org                             ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary as ""

use32
; standard header
        db      'MENUET01'      ; signature
        dd      1               ; header version
        dd      start           ; entry point
        dd      i_end           ; initialized size
        dd      mem             ; required memory
        dd      mem             ; stack pointer
        dd      0               ; parameters
        dd      0               ; path

; useful includes
include '../../macros.inc'
purge mov,add,sub
include '../../proc32.inc'
include '../../dll.inc'

include '../../network.inc'

; entry point
start:
; load libraries
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     exit
; initialize console
        push    1
        call    [con_start]
        push    title
        push    -1
        push    -1
        push    -1
        push    -1
        call    [con_init]
; main loop
main:
; write prompt
        push    str1
        call    [con_write_asciiz]
; read string
        mov     esi, s
        push    256
        push    esi
        call    [con_gets]
; check for exit
        test    eax, eax
        jz      done
        cmp     byte [esi], 10
        jz      done
; delete terminating '\n'
        push    esi
@@:
        lodsb
        test    al, al
        jnz     @b
        mov     byte [esi-2], al
        pop     esi
; resolve name
        push    esp     ; reserve stack place
        push    esp     ; fourth parameter
        push    0       ; third parameter
        push    0       ; second parameter
        push    esi     ; first parameter
        call    [getaddrinfo]
        pop     esi
; test for error
        test    eax, eax
        jnz     fail
; write results
        push    str2
        call    [con_write_asciiz]
        mov     edi, esi
addrloop:
; before all subsequent addresses print comma
        cmp     edi, esi
        jz      @f
        push    str3
        call    [con_write_asciiz]
@@:
; convert IP address to decimal notation
        mov     eax, [edi+addrinfo.ai_addr]
        pushd   [eax+sockaddr_in.sin_addr]
        call    [inet_ntoa]
; write result
        push    eax
        call    [con_write_asciiz]
; advance to next item
        mov     edi, [edi+addrinfo.ai_next]
        test    edi, edi
        jnz     addrloop
; free allocated memory
        push    esi
        call    [freeaddrinfo]
; write newline and continue main loop
        push    str4
@@:
        call    [con_write_asciiz]
        jmp     main
fail:
        push    str5
        jmp     @b
done:
        push    1
        call    [con_exit]
exit:
        mcall   -1

; data
title   db      'Names resolver',0
str1    db      'Host name to resolve: ',0
str2    db      'IP address(es): ',0
str3    db      ', ',0
str4    db      10,0
str5    db      'Name resolution failed.',10,0
; import
align 4
@IMPORT:

library network, 'network.obj', console, 'console.obj'
import  network,        \
        getaddrinfo,    'getaddrinfo',  \
        freeaddrinfo,   'freeaddrinfo', \
        inet_ntoa,      'inet_ntoa'
import  console,        \
        con_start,      'START',        \
        con_init,       'con_init',     \
        con_write_asciiz,       'con_write_asciiz',     \
        con_exit,       'con_exit',     \
        con_gets,       'con_gets'
i_end:
s       rb      256
align   4
rb      4096    ; stack
mem:
