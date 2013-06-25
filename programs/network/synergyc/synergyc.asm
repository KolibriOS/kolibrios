;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2010-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  Synergyc.asm - Synergy client for KolibriOS                    ;;
;;                                                                 ;;
;;  Written by hidnplayr@kolibrios.org                             ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary as ""

BUFFERSIZE      = 1024
DEFAULTPORT     = 24800

use32
        org     0x0

        db      'MENUET01'      ; signature
        dd      1               ; header version
        dd      start           ; entry point
        dd      i_end           ; initialized size
        dd      mem+0x1000      ; required memory
        dd      mem+0x1000      ; stack pointer
        dd      0               ; parameters
        dd      path            ; path

include '../../macros.inc'
purge mov,add,sub
include '../../proc32.inc'
include '../../dll.inc'

include '../../network.inc'

start:

        cld
        mov     edi, path       ; Calculate the length of zero-terminated string
        xor     al, al
        mov     ecx, 1024
        repne   scasb
        dec     edi
        mov     esi, filename   ; append the path with '.ini'
        movsd
        movsb

        mcall   68, 11

        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     exit

        push    1
        call    [con_start]

        push    title
        push    25
        push    80
        push    25
        push    80
        call    [con_init]

        push    str0
        call    [con_write_asciiz]

        invoke  ini.get_str, path, str_remote, str_ip, buffer_ptr, 16, 0
        test    eax, eax
        jnz     error

        invoke  ini.get_int, path, str_remote, str_port, DEFAULTPORT
        xchg    al, ah
        mov     [sockaddr1.port], ax

        push    str1
        call    [con_write_asciiz]

        push    buffer_ptr
        call    [con_write_asciiz]

        push    newline
        call    [con_write_asciiz]

        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        je      error
        mov     [socketnum], eax

; resolve name
        push    esp             ; reserve stack place
        push    esp             ; ptr to result
        push    0               ; addrinfo hints
        push    0               ; servname
        push    buffer_ptr      ; hostname
        call    [getaddrinfo]
        pop     esi
; test for error
        test    eax, eax
        jnz     error

        mov     eax, [esi+addrinfo.ai_addr]
        mov     eax, [eax+sockaddr_in.sin_addr]
        mov     [sockaddr1.ip], eax

        mcall   connect, [socketnum], sockaddr1, 18

        push    str7
        call    [con_write_asciiz]

        mcall   40, EVM_STACK

login:
        call    wait_for_data

        push    buffer_ptr + 4
        call    [con_write_asciiz]

        cmp     dword [buffer_ptr], 11 shl 24
        jne     login
        cmp     dword [buffer_ptr + 4], 'Syne'
        jne     login
        cmp     word [buffer_ptr + 8], 'rg'
        jne     login
        cmp     byte [buffer_ptr + 10], 'y'
        jne     login

        push    str2
        call    [con_write_asciiz]

        lea     edi, [buffer_ptr + 11 + 4 + 4]
        invoke  ini.get_str, path, str_local, str_name, edi, 255, 0
        xor     al , al
        mov     ecx, 256
        repne   scasb
        sub     edi, buffer_ptr + 1 + 4
        mov     esi, edi
        bswap   edi
        mov     dword [buffer_ptr], edi
        mov     edi, esi
        sub     edi, 11 + 4
        bswap   edi
        mov     dword [buffer_ptr + 11 + 4], edi
        add     esi, 4

        mcall   send, [socketnum], buffer_ptr, , 0

mainloop:
        call    wait_for_data
        mov     edi, buffer_ptr

  .command:
        push    eax edi

        cmp     dword [edi + 4], 'QINF' ; query screen info
        je      .qinf

        cmp     dword [edi + 4], 'CALV' ; alive ?
        je      .calv

        cmp     dword [edi + 4], 'CINN' ; mouse moved into screen
        je      .cinn

        cmp     dword [edi + 4], 'DCLP' ; Clipboard event
        je      .dclp

        cmp     dword [edi + 4], 'DMMV' ; Mouse moved
        je      .dmmv

        cmp     dword [edi + 4], 'COUT' ; leave screen
        je      .cout

        cmp     dword [edi + 4], 'DMDN' ; mouse button down
        je      .dmdn

        cmp     dword [edi + 4], 'DMUP' ; mouse button released
        je      .dmup

        cmp     dword [edi + 4], 'CNOP' ; no operation
        je      .next

        cmp     dword [edi + 4], 'CIAK' ; resolution changed?
        je      .ciak

        push    str3
        call    [con_write_asciiz]

        mov     byte[edi+8],0
        add     edi, 4
        push    edi
        call    [con_write_asciiz]

        push    newline
        call    [con_write_asciiz]

  .next:
        pop     edi eax

        mov     ecx, dword [edi]
        bswap   ecx
        add     ecx, 4
        sub     eax, ecx
        jle     mainloop
        add     edi, ecx
        jmp     .command


  .qinf:
        mcall   14      ; get screen info
        add     eax, 0x00010001
        bswap   eax
        mov     dword [screeninfo.size], eax
        mcall   send, [socketnum], screeninfo, screeninfo.length, 0     ; send client name
        jmp     .next


  .calv:
        mcall   send, [socketnum], calv, calv.length, 0     ; looks like ping-pong event
        jmp     .next


  .cinn:
        mov     edx, [edi + 8]
        bswap   edx
        mcall   18, 19, 4
        ; ignore sequence number and modify key mask for now
        push    str6
        call    [con_write_asciiz]
        jmp     .next

  .dclp:

        jmp     .next

  .dmmv:
        mov     edx, [edi + 8]
        bswap   edx
        mcall   18, 19, 4
        mcall   send, [socketnum], cnop, cnop.length, 0     ; reply with NOP
        push    str4
        call    [con_write_asciiz]
        jmp     .next

  .cout:
        jmp     .next

  .dmdn:
        movzx   eax, byte [edi + 8]
        or      [mousestate], eax
        mcall   18, 19, 5, [mousestate]
        mcall   send, [socketnum], cnop, cnop.length, 0     ; reply with NOP
        push    str5
        call    [con_write_asciiz]
        jmp     .next

  .dmup:
        movzx   eax, byte [edi + 8]
        not     eax
        and     [mousestate], eax
        mcall   18, 19, 5, [mousestate]
        mcall   send, [socketnum], cnop, cnop.length, 0     ; reply with NOP
        push    str5
        call    [con_write_asciiz]
        jmp     .next

  .ciak:
        jmp     .next

error:
        push    str_err
        call    [con_write_asciiz]

;        call    [con_gets]

        call    [con_getch2]

        mcall   close, [socketnum]

        push    1
        call    [con_exit]
exit:

        mcall   -1


wait_for_data:
        mcall   recv, [socketnum], buffer_ptr, BUFFERSIZE, 0
        cmp     eax, -1
        je      wait_for_data

        cmp     eax, 8
        jb      wait_for_data   ; FIXME

        ret



; data
title   db      'Synergy client',0
str0    db      'Welcome to the software KM switch for KolibriOS.',10,10,0
str1    db      'Connecting to: ',0
str7    db      'Connected!',10,0
str2    db      10,'Handshake received',10,0
str3    db      'Unsupported command: ',0
newline db      10,0
str4    db      'mouse moved',10,0
str5    db      'mouse buttons changed',10,0
str6    db      'Enter screen',10,0
str_err db      'Uh-Oh.. some error occured !',10,'Press any key to quit.',0

screeninfo:
        dd (screeninfo.length - 4) shl 24
        db 'DINF'
        dw 0    ; coordinate of leftmost pixel
        dw 0    ; coordiante of topmost pixel
  .size:
        dw 0    ; width
        dw 0    ; height

        dw 0    ; size of warp zone

        dw 0xb88b        ; x position of the mouse on the secondary screen  (no idea what it means)
        dw 0xbcfb        ; y position of the mouse on the secondary screen
  .length = $ - screeninfo

calv:
        dd (4) shl 24
        db 'CALV'
  .length = $ - calv + 8 ; also send cnop

cnop:
        dd (4) shl 24
        db 'CNOP'
  .length = $ - cnop

mousestate      dd 0


sockaddr1:
        dw AF_INET4
.port   dw 0
.ip     dd 192 + 168 shl 8 + 1 shl 16 + 115 shl 24
        rb 10

filename        db      '.ini', 0
str_local       db      'local', 0
str_name        db      'name', 0
str_remote      db      'remote', 0
str_port        db      'port', 0
str_ip          db      'ip', 0

; import
align 16
@IMPORT:

library console,        'console.obj',\
        network,        'network.obj',\
        libini,         'libini.obj'

import  network,\
        getaddrinfo,    'getaddrinfo'

import  console,                \
        con_start,              'START',\
        con_init,               'con_init',\
        con_write_asciiz,       'con_write_asciiz',\
        con_exit,               'con_exit',\
        con_gets,               'con_gets',\
        con_cls,                'con_cls',\
        con_getch2,             'con_getch2',\
        con_set_cursor_pos,     'con_set_cursor_pos'

import  libini,\
        ini.get_str,    'ini_get_str',\
        ini.get_int,    'ini_get_int'

align   4
i_end:
socketnum       dd ?
buffer_ptr      rb BUFFERSIZE
path            rb 4096    ; stack
mem: