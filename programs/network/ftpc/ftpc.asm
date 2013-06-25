;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2013. All rights reserved.         ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  ftpc.asm - FTP client for KolibriOS                            ;;
;;                                                                 ;;
;;  Written by hidnplayr@kolibrios.org                             ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary as ""

__DEBUG__               = 0
__DEBUG_LEVEL__         = 1
BUFFERSIZE              = 1024

STATUS_CONNECTING       = 0
STATUS_CONNECTED        = 1
STATUS_NEEDPASSWORD     = 2
STATUS_LOGGED_IN        = 3

use32
; standard header
        db      'MENUET01'      ; signature
        dd      1               ; header version
        dd      start           ; entry point
        dd      i_end           ; initialized size
        dd      mem+0x1000      ; required memory
        dd      mem+0x1000      ; stack pointer
        dd      s               ; parameters
        dd      0               ; path

include '../../macros.inc'
purge mov,add,sub
include '../../proc32.inc'
include '../../dll.inc'
include '../../debug-fdo.inc'
include '../../network.inc'

include 'usercommands.inc'
include 'servercommands.inc'

; entry point
start:

        DEBUGF  1, "hello"
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

; Check for parameters
        cmp     byte [s], 0
        jne     resolve

main:
        call    [con_cls]
; Welcome user
        push    str1
        call    [con_write_asciiz]

; write prompt
        push    str2
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

resolve:

; delete terminating '\n'
        mov     esi, s
  @@:
        lodsb
        cmp     al, 0x20
        ja      @r
        mov     byte [esi-1], 0

;        call    [con_cls]
        push    str3
        call    [con_write_asciiz]
        push    s
        call    [con_write_asciiz]

; resolve name
        push    esp     ; reserve stack place
        push    esp     ; fourth parameter
        push    0       ; third parameter
        push    0       ; second parameter
        push    s       ; first parameter
        call    [getaddrinfo]
        pop     esi
; test for error
        test    eax, eax
        jnz     fail

; write results
        push    str8
        call    [con_write_asciiz]
;        mov     edi, esi

; convert IP address to decimal notation
        mov     eax, [esi+addrinfo.ai_addr]
        mov     eax, [eax+sockaddr_in.sin_addr]
        mov     [sockaddr1.ip], eax
        push    eax
        call    [inet_ntoa]
; write result
        push    eax
        call    [con_write_asciiz]
; free allocated memory
        push    esi
        call    [freeaddrinfo]

        push    str9
        call    [con_write_asciiz]

        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        je      fail2
        mov     [socketnum], eax

        push    str11
        call    [con_write_asciiz]

        mcall   connect, [socketnum], sockaddr1, 18

        mcall   40, EVM_STACK

        mov     [status], STATUS_CONNECTING
        mov     [offset], buffer_ptr

        push    str12
        call    [con_write_asciiz]

wait_for_serverdata:
        mcall   10

        call    [con_get_flags]
        test    eax, 0x200                      ; con window closed?
        jnz     exit

; receive socket data
        mcall   recv, [socketnum], [offset], BUFFERSIZE, MSG_DONTWAIT
        inc     eax
        jz      wait_for_serverdata
        dec     eax
        jz      wait_for_serverdata

; extract commands, copy them to "s" buffer
        add     eax, buffer_ptr                 ; eax = end pointer
        mov     esi, buffer_ptr                 ; esi = current pointer
  .nextcommand:
        mov     edi, s
  .byteloop:
        cmp     esi, eax
        jae     wait_for_serverdata
        lodsb
        cmp     al, 10                          ; excellent, we might have a command
        je      .got_command
        cmp     al, 13                          ; just ignore this crap
        je      .byteloop
        stosb
        jmp     .byteloop

; we have a newline check if its a command
  .got_command:
        xor     al, al
        stosb
;        push    esi eax

; print it to the screen
        pushd   s
        call    [con_write_asciiz]
        pushd   str4                            ; newline
        call    [con_write_asciiz]

;        cmp     byte[s+2], " "
;        jne     .not_command

        lea     ecx, [edi - s]
        call    server_parser

;  .not_command:
;        pop     eax esi
;        jmp     .nextcommand




wait_for_usercommand:

        cmp     [status], STATUS_CONNECTED
        je      .connected

        cmp     [status], STATUS_NEEDPASSWORD
        je      .needpass

; write prompt
        push    str2
        call    [con_write_asciiz]
; read string
        mov     esi, s
        push    256
        push    esi
        call    [con_gets]

        call    [con_get_flags]
        test    eax, 0x200                      ; con window closed?
        jnz     exit

        cmp     dword[s], "list"
        je      cmd_list

        cmp     dword[s], "help"
        je      cmd_help

        push    str_unkown
        call    [con_write_asciiz]

        jmp     wait_for_usercommand


  .connected:

        push    str_user
        call    [con_write_asciiz]

        mov     dword[s], "USER"
        mov     byte[s+4], " "

;        mov     [status], STATUS_NEEDPASSWORD
        inc     [status]

        jmp     .send


  .needpass:
        push    str_pass
        call    [con_write_asciiz]

        mov     dword[s], "PASS"
        mov     byte[s+4], " "

;        mov     [status], STATUS_LOGGED_IN
        inc     [status]

  .send:
; read string
        mov     esi, s+5
        push    256
        push    esi
        call    [con_gets]

        mov     edi, s+5
        mov     ecx, 256
        xor     al, al
        repne   scasb
        lea     esi, [edi-s-1]
        mcall   send, [socketnum], s

        jmp     wait_for_usercommand






open_dataconnection:
        cmp     [status], STATUS_LOGGED_IN
        jne     .fail

        mov     dword[s], "PASV"
        mov     byte[s+4], 10
        mcall   send, [socketnum], s, 5

        ret

  .fail:
        push    str6
        call    [con_write_asciiz]

        ret


fail2:
        push    str6
        call    [con_write_asciiz]

        jmp     fail.wait

fail:
        push    str5
        call    [con_write_asciiz]
  .wait:
        push    str10
        call    [con_write_asciiz]
        call    [con_getch2]
        jmp     main

done:
        push    1
        call    [con_exit]
exit:

        mcall   close, [socketnum]
        mcall   -1



; data
title   db 'FTP client',0
str1    db 'FTP client for KolibriOS v0.01',10,10,'Please enter ftp server address.',10,0
str2    db '> ',0
str3    db 'Resolving ',0
str4    db 10,0
str5    db 10,'Name resolution failed.',10,0
str6    db 10,'Socket error.',10,0
str8    db ' (',0
str9    db ')',10,0
str10   db 'Push any key to continue.',0
str11   db 'Connecting',10,0
str12   db 'Connected!',10,0
str_user db "username: ",0
str_pass db "password: ",0
str_unkown db "unkown command",10,0
str_help db "available commands:",10,10
         db "help       list",10,0

str_open db "opening data socket",10,0

sockaddr1:
        dw AF_INET4
.port   dw 0x1500       ; 21
.ip     dd 0
        rb 10

sockaddr2:
        dw AF_INET4
.port   dw 0
.ip     dd 0
        rb 10

include_debug_strings    ; ALWAYS present in data section



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
        con_write_asciiz,'con_write_asciiz',     \
        con_exit,       'con_exit',     \
        con_gets,       'con_gets',\
        con_cls,        'con_cls',\
        con_getch2,     'con_getch2',\
        con_set_cursor_pos, 'con_set_cursor_pos',\
        con_write_string, 'con_write_string',\
        con_get_flags,  'con_get_flags'


i_end:

active_passive  db ?
socketnum       dd ?
datasocket      dd ?
buffer_ptr      rb 2*BUFFERSIZE
status          db ?
offset          dd ?

s       rb      1024

mem:
