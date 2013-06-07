;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2010-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  socketdbg.asm - socket debug utility for KolibriOS             ;;
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
include '../../struct.inc'

include 'socket.inc'

; entry point
start:
        mcall   40, 0                   ; we dont want any events
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
        mcall   75, 255, 0, socket_list ; get current socket list

        call    [con_cls]

        mov     esi, socket_list
  .loop:
        lodsd
        test    eax, eax
        jz      .done

        mov     ecx, eax
        mcall   75, 255, , socket_buf

        pushd   [socket_buf + SOCKET.state]
        pushd   [socket_buf + SOCKET.PID]
        pushd   [socket_buf + SOCKET.Number]
        push    str_sock
        call    [con_printf]
        add     esp, 4

        jmp     .loop

  .done:

        mcall   23, 50

        jmp     main


        push    0
        call    [con_exit]
exit:
        mcall   -1

; data
title           db 'Socket debugger', 0

str_sock        db 'Socket=%d PID=%d state=%d', 10, 0

; import
align 4
@IMPORT:

library console, 'console.obj'

import  console,        \
        con_start,      'START',        \
        con_init,       'con_init',     \
        con_cls,        'con_cls',      \
        con_exit,       'con_exit',     \
        con_printf,     'con_printf'
i_end:

socket_list     rd 4096
socket_buf      rd 4096

align   4
rb      4096    ; stack
mem:
