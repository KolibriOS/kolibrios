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

BUFFERSIZE              = 4096

STATUS_CONNECTING       = 0
STATUS_CONNECTED        = 1
STATUS_NEEDPASSWORD     = 2
STATUS_LOGGED_IN        = 3

OPERATION_LIST          = 0
OPERATION_RETR          = 1
OPERATION_STOR          = 2

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
        invoke  con_write_asciiz, str8          ; ' (',0
        mov     eax, [esi+addrinfo.ai_addr]     ; convert IP address to decimal notation
        mov     eax, [eax+sockaddr_in.sin_addr] ;
        mov     [sockaddr1.ip], eax             ;
        invoke  inet_ntoa, eax                  ;
        invoke  con_write_asciiz, eax           ; print ip
        invoke  freeaddrinfo, esi               ; free allocated memory
        invoke  con_write_asciiz, str9          ; ')',10,0

; open the socket
        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        je      socket_error
        mov     [socketnum], eax

; connect to the server
        invoke  con_write_asciiz, str11
        mcall   connect, [socketnum], sockaddr1, 18
        mov     [status], STATUS_CONNECTING

        invoke  con_write_asciiz, str12         ; 'waiting for welcome'

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
        invoke  con_set_flags, 0x07             ; reset color

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

        invoke  con_write_asciiz, str4          ; newline
        invoke  con_set_flags, 0x07

        cmp     dword[s], "list"
        je      cmd_list

        cmp     dword[s], "help"
        je      cmd_help

        cmp     dword[s], "cwd "
        je      cmd_cwd

        cmp     dword[s], "retr"
        je      cmd_retr

        cmp     dword[s], "pwd" + 10 shl 24
        je      cmd_pwd

        cmp     dword[s], "stor"
        je      cmd_stor

        cmp     dword[s], "dele"
        je      cmd_dele

        cmp     dword[s], "bye" + 10 shl 24
        je      cmd_bye

        cmp     dword[s], "lcwd"
        je      cmd_lcwd

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
        invoke  con_set_flags, 0x00     ; black text on black background for password

  .send:
; read string
        mov     esi, s+5
        invoke  con_gets, esi, 256

; find end of string
        mov     edi, s+5
        mov     ecx, 256
        xor     al, al
        repne   scasb
        lea     esi, [edi-s-1]
; and send it to the server
        mcall   send, [socketnum], s, , 0

        invoke  con_write_asciiz, str4  ; newline
        invoke  con_set_flags, 0x07     ; reset color
        jmp     wait_for_servercommand



open_dataconnection:                    ; only passive for now..
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
str1    db 'FTP client for KolibriOS v0.06',10,10,'Please enter ftp server address.',10,0
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
str_lcwd db "local working directory is now: ",0

str_help db "available commands:",10
         db "help - help",10,10
         db "bye  - close connection",10
         db "cwd  - change working directoy on server",10
         db "dele - delete file from server",10
         db "list - list files and folders in current directory",10
         db "lcwd - change local working directory",10
         db "pwd  - print working directory",10
         db "retr - retreive file from server",10
         db "stor - store file on server",10
         db 10,0

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

status          db ?
active_passive  db ?

socketnum       dd ?
datasocket      dd ?
offset          dd ?
size            dd ?
operation       dd ?

filestruct:
.subfn  dd ?
.offset dd ?
        dd ?
.size   dd ?
.ptr    dd ?
.name   rb 1024

buffer_ptr      rb BUFFERSIZE+1
buffer_ptr2     rb BUFFERSIZE+1

s               rb 1024

mem:
