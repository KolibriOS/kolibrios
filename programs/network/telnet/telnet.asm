;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2010-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  telnet.asm - Telnet client for KolibriOS                       ;;
;;                                                                 ;;
;;  Written by hidnplayr@kolibrios.org                             ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary as ""

__DEBUG__       = 0
__DEBUG_LEVEL__ = 1
BUFFERSIZE      = 4096

use32
; standard header
        db      'MENUET01'      ; signature
        dd      1               ; header version
        dd      start           ; entry point
        dd      i_end           ; initialized size
        dd      mem+4096        ; required memory
        dd      mem+4096        ; stack pointer
        dd      hostname        ; parameters
        dd      0               ; path

include '../../macros.inc'
purge mov,add,sub
include '../../proc32.inc'
include '../../dll.inc'
include '../../debug-fdo.inc'
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

; Check for parameters
        cmp     byte [hostname], 0
        jne     resolve

main:
        call    [con_cls]
; Welcome user
        push    str1
        call    [con_write_asciiz]

prompt:
; write prompt
        push    str2
        call    [con_write_asciiz]
; read string
        mov     esi, hostname
        push    256
        push    esi
        call    [con_gets]
; check for exit
        test    eax, eax
        jz      done
        cmp     byte [esi], 10
        jz      done

resolve:

        mov     [sockaddr1.port], 23 shl 8

; delete terminating '\n'
        mov     esi, hostname
  @@:
        lodsb
        cmp     al, ':'
        je      .do_port
        cmp     al, 0x20
        ja      @r
        mov     byte [esi-1], 0
        jmp     .done

  .do_port:
        xor     eax, eax
        xor     ebx, ebx
        mov     byte [esi-1], 0
  .portloop:
        lodsb
        cmp     al, 0x20
        jbe     .port_done
        sub     al, '0'
        jb      hostname_error
        cmp     al, 9
        ja      hostname_error
        lea     ebx, [ebx*4 + ebx]
        shl     ebx, 1
        add     ebx, eax
        jmp     .portloop

  .port_done:
        xchg    bl, bh
        mov     [sockaddr1.port], bx

  .done:

; resolve name
        push    esp     ; reserve stack place
        push    esp     ; ptr to result
        push    0       ; addrinfo hints
        push    0       ; servname
        push    hostname; hostname
        call    [getaddrinfo]
        pop     esi
; test for error
        test    eax, eax
        jnz     fail

        call    [con_cls]
        push    str3
        call    [con_write_asciiz]
        push    hostname
        call    [con_write_asciiz]

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
        jz      fail2
        mov     [socketnum], eax

        mcall   connect, [socketnum], sockaddr1, 18

        mcall   40, 1 shl 7 ; + 7
        call    [con_cls]

        mcall   18, 7
        push    eax
        mcall   51, 1, thread, mem - 2048
        pop     ecx
        mcall   18, 3

mainloop:
    DEBUGF  1, 'TELNET: Waiting for events\n'
        mcall   10
    DEBUGF  1, 'TELNET: EVENT %x !\n', eax

        call    [con_get_flags]
        test    eax, 0x200                      ; con window closed?
        jnz     exit

        mcall   recv, [socketnum], buffer_ptr, BUFFERSIZE, 0
        cmp     eax, -1
        je      mainloop

    DEBUGF  1, 'TELNET: got %u bytes of data !\n', eax

        mov     esi, buffer_ptr
        lea     edi, [esi + eax]
        mov     byte [edi], 0

  .scan_cmd:
        cmp     byte [esi], 0xff        ; Interpret As Command
        jne     .no_cmd
        ; TODO: parse options, for now, we will reply with 'WONT' to everything
        mov     byte [esi + 1], 252     ; WONT
        add     esi, 3                  ; a command is always 3 bytes
        jmp     .scan_cmd
  .no_cmd:

        cmp     esi, buffer_ptr
        je      .print_loop

    DEBUGF  1, 'TELNET: sending data\n'

        push    esi edi
        sub     esi, buffer_ptr
        mcall   send, [socketnum], buffer_ptr, , 0
        pop     edi esi

  .print_loop:
    DEBUGF  1, 'TELNET: printloop\n'
        cmp     esi, edi
        jae     mainloop

        cmp     byte [esi], 0x1b        ; escape character
        jne     .print_byte
        inc     esi

        cmp     word [esi], 0x485b      ; move cursor to beginning
        jne     @f
        inc     esi
        inc     esi

    DEBUGF  1, 'TELNET: resetting cursor \n'

        push    0
        push    0
        call    [con_set_cursor_pos]
        jmp     .print_loop

  @@:
        inc     esi
        inc     esi
        jmp     .print_loop

  .print_byte:
        push    dword 1
        push    esi                     ; next string to print
        inc     esi
        call    [con_write_string]
        jmp     .print_loop


fail2:
        push    str6
        call    [con_write_asciiz]

        jmp     prompt

fail:
        push    str5
        call    [con_write_asciiz]

        jmp     prompt

hostname_error:
        push    str11
        call    [con_write_asciiz]
        jmp     prompt

done:
        push    1
        call    [con_exit]
exit:

        mcall   close, [socketnum]
        mcall   -1



thread:
        mcall   40, 0
  .loop:
        call    [con_getch2]
        mov     byte [send_data], al
        mcall   send, [socketnum], send_data, 1

        call    [con_get_flags]
        test    eax, 0x200                      ; con window closed?
        jz      .loop
        mcall   -1

; data
title   db      'Telnet',0
str1    db      'Telnet for KolibriOS',10,10,\
                'Please enter URL of telnet server (for example: towel.blinkenlights.nl:23)',10,10,0
str2    db      '> ',0
str3    db      'Connecting to ',0
str4    db      10,0
str8    db      ' (',0
str9    db      ')',10,0

str5    db      'Name resolution failed.',10,10,0
str6    db      'Could not open socket.',10,10,0
str11   db      'Invalid hostname.',10,10,0

sockaddr1:
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
        con_write_asciiz,       'con_write_asciiz',     \
        con_exit,       'con_exit',     \
        con_gets,       'con_gets',\
        con_cls,        'con_cls',\
        con_getch2,     'con_getch2',\
        con_set_cursor_pos, 'con_set_cursor_pos',\
        con_write_string, 'con_write_string',\
        con_get_flags,  'con_get_flags'


i_end:

socketnum       dd ?
buffer_ptr      rb BUFFERSIZE+1
send_data       rb 1

hostname        rb 1024

mem:
