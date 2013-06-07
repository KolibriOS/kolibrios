;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2010-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  tcpserv.asm - TCP demo program for KolibriOS                   ;;
;;                                                                 ;;
;;  Written by hidnplayr@kolibrios.org                             ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary as ""

BUFFERSIZE      = 1500

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
        push    25
        push    80
        push    25
        push    80
        call    [con_init]

        mcall   40, 1 shl 7     ; we only want network events

        push    str1
        call    [con_write_asciiz]

        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        je      sock_err

        mov     [socketnum], eax

;;        mcall   setsockopt, [socketnum], SOL_SOCKET, SO_REUSEADDR, &yes,
;;        cmp     eax, -1
;;        je      opt_err

        mcall   bind, [socketnum], sockaddr1, sockaddr1.length
        cmp     eax, -1
        je      bind_err

        mcall   listen, [socketnum], 10 ; Backlog = 10
        cmp     eax, -1
        je      listen_err

        push    str2
        call    [con_write_asciiz]

        mcall   10

        mcall   accept, [socketnum], sockaddr1, sockaddr1.length
        cmp     eax, -1
        je      acpt_err

        mov     [socketnum2], eax

;;        mcall   close, [socketnum]

        mcall   send, [socketnum2], hello, hello.length

  .loop:
        mcall   10

        mcall   recv, [socketnum2], buffer, buffer.length, 0
        cmp     eax, -1
        je      .loop

        mov     byte [buffer + eax], 0

        push    buffer
        call    [con_write_asciiz]

        jmp     .loop

acpt_err:
        push    str8
        call    [con_write_asciiz]
        jmp     done

listen_err:
        push    str3
        call    [con_write_asciiz]
        jmp     done

bind_err:
        push    str4
        call    [con_write_asciiz]
        jmp     done

sock_err:
        push    str6
        call    [con_write_asciiz]
        jmp     done

done:
        call    [con_getch2]
        push    1
        call    [con_exit]
exit:
        mcall   -1



; data
title   db      'TCP stream server - test',0
str1    db      'Opening socket',10, 0
str2    db      'Listening for incoming connections...',10,0
str3    db      'Listen error',10,10,0
str4    db      'Bind error',10,10,0
str5    db      'Setsockopt error.',10,10,0
str6    db      'Could not open socket',10,10,0
str7    db      'Got data!',10,10,0
str8    db      'Error accepting connection',10,10,0

hello   db      'Hello world!',0
.length = $ - hello

sockaddr1:
        dw AF_INET4
.port   dw 0x1700       ; 23
.ip     dd 0
        rb 10
.length = $ - sockaddr1

; import
align 4
@IMPORT:

library console, 'console.obj'

import  console,        \
        con_start,      'START',        \
        con_init,       'con_init',     \
        con_write_asciiz,       'con_write_asciiz',     \
        con_exit,       'con_exit',     \
        con_gets,       'con_gets',\
        con_cls,        'con_cls',\
        con_printf,     'con_printf',\
        con_getch2,     'con_getch2',\
        con_set_cursor_pos, 'con_set_cursor_pos'
i_end:

socketnum       dd ?
socketnum2      dd ?
buffer         rb BUFFERSIZE
.length = BUFFERSIZE

align   4
rb      4096    ; stack
mem:
