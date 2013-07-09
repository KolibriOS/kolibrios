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
include '../../network.inc'

include 'usercommands.inc'
include 'servercommands.inc'

; entry point
start:

        mcall   40, 0
; load libraries
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     exit
; initialize console
        invoke  con_start, 1
        invoke  con_init, 80, 25, 80, 250, title

; Check for parameters
        cmp     byte [s], 0
        jne     resolve

main:
        invoke  con_cls
; Welcome user
        invoke  con_write_asciiz, str1

; write prompt
        invoke  con_set_flags, 0x0a
        invoke  con_write_asciiz, str2
; read string
        mov     esi, s
        invoke  con_gets, esi, 256
        invoke  con_write_asciiz, str4  ; newline
        invoke  con_set_flags, 0x07
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

        invoke  con_write_asciiz, str3
        invoke  con_write_asciiz, s

; resolve name
        push    esp     ; reserve stack place
        invoke  getaddrinfo, s, 0, 0, esp
        pop     esi
; test for error
        test    eax, eax
        jnz     fail

; write results
        invoke  con_write_asciiz, str8
;        mov     edi, esi

; convert IP address to decimal notation
        mov     eax, [esi+addrinfo.ai_addr]
        mov     eax, [eax+sockaddr_in.sin_addr]
        mov     [sockaddr1.ip], eax

        invoke  inet_ntoa, eax
; write result
        invoke  con_write_asciiz, eax
; free allocated memory
        invoke  freeaddrinfo, esi

        invoke  con_write_asciiz, str9
        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        je      socket_error
        mov     [socketnum], eax

        invoke  con_write_asciiz, str11
        mcall   connect, [socketnum], sockaddr1, 18
        mov     [status], STATUS_CONNECTING

        invoke  con_write_asciiz, str12

        mov     [offset], 0

wait_for_servercommand:

        cmp     [offset], 0
        je      .receive
        mov     esi, [offset]
        mov     edi, s
        mov     ecx, [size]
        add     ecx, esi
        jmp     .byteloop

; receive socket data
  .receive:
        mcall   recv, [socketnum], buffer_ptr, BUFFERSIZE, 0
        inc     eax
        jz      socket_error
        dec     eax
        jz      wait_for_servercommand

        mov     [offset], 0

; extract commands, copy them to "s" buffer
        lea     ecx, [eax + buffer_ptr]         ; ecx = end pointer
        mov     esi, buffer_ptr                 ; esi = current pointer
        mov     edi, s
  .byteloop:
        cmp     esi, ecx
        jae     wait_for_servercommand
        lodsb
        cmp     al, 10                          ; excellent, we might have a command
        je      .got_command
        cmp     al, 13                          ; just ignore this byte
        je      .byteloop
        stosb
        jmp     .byteloop
  .got_command:                                 ; we have a newline check if its a command
        cmp     esi, ecx
        je      .no_more_data
        mov     [offset], esi
        sub     ecx, esi
        mov     [size], ecx
        jmp     .go_cmd
  .no_more_data:
        mov     [offset], 0
  .go_cmd:
        xor     al, al
        stosb

        invoke  con_set_flags, 0x03             ; change color
        invoke  con_write_asciiz, s             ; print servercommand
        invoke  con_write_asciiz, str4          ; newline
        invoke  con_set_flags, 0x07

        jmp     server_parser                   ; parse command

wait_for_usercommand:

        invoke  con_set_flags, 0x0a

        cmp     [status], STATUS_CONNECTED
        je      .connected

        cmp     [status], STATUS_NEEDPASSWORD
        je      .needpass

; write prompt
        invoke  con_write_asciiz, str2
; read string
        mov     esi, s
        invoke  con_gets, esi, 256
        invoke  con_set_flags, 0x07

        cmp     dword[s], "list"
        je      cmd_list

        cmp     dword[s], "help"
        je      cmd_help

        cmp     dword[s], "cwd "
        je      cmd_cwd

        invoke  con_write_asciiz, str_unknown
        jmp     wait_for_usercommand


  .connected:

        invoke  con_write_asciiz, str_user
        mov     dword[s], "USER"
        mov     byte[s+4], " "
        jmp     .send


  .needpass:

        invoke  con_write_asciiz, str_pass
        mov     dword[s], "PASS"
        mov     byte[s+4], " "

  .send:
; read string
        mov     esi, s+5
        invoke  con_gets, esi, 256

        mov     edi, s+5
        mov     ecx, 256
        xor     al, al
        repne   scasb
        lea     esi, [edi-s-1]
        mcall   send, [socketnum], s, , 0

        invoke  con_write_asciiz, str4  ; newline
        invoke  con_set_flags, 0x07
        jmp     wait_for_servercommand



open_dataconnection:
        cmp     [status], STATUS_LOGGED_IN
        jne     .fail

        mov     dword[s], "PASV"
        mov     byte[s+4], 10
        mcall   send, [socketnum], s, 5, 0
        ret

  .fail:
        invoke  con_write_asciiz, str6
        ret



socket_error:
        invoke  con_write_asciiz, str6
        jmp     fail.wait

fail:
        invoke  con_write_asciiz, str5
  .wait:
        invoke  con_write_asciiz, str10
        invoke  con_getch2
        jmp     main

done:
        invoke  con_exit, 1

exit:
        mcall   close, [socketnum]
        mcall   -1



; data
title   db 'FTP client',0
str1    db 'FTP client for KolibriOS v0.03',10,10,'Please enter ftp server address.',10,0
str2    db '> ',0
str3    db 'Resolving ',0
str4    db 10,0
str5    db 10,'Name resolution failed.',10,0
str6    db 10,'Socket error.',10,0
str8    db ' (',0
str9    db ')',10,0
str10   db 'Push any key to continue.',0
str11   db 'Connecting...',10,0
str12   db 'Waiting for welcome message.',10,0
str_user db "username: ",0
str_pass db "password: ",0
str_unknown db "unknown command",10,0
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
        con_get_flags,  'con_get_flags', \
        con_set_flags,  'con_set_flags'


i_end:

active_passive  db ?
socketnum       dd ?
datasocket      dd ?
buffer_ptr      rb BUFFERSIZE
buffer_ptr2     rb BUFFERSIZE
status          db ?
offset          dd ?
size            dd ?

s               rb 1024

mem:
