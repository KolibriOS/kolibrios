;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2010-2014. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  ping.asm - ICMP echo client for KolibriOS                      ;;
;;                                                                 ;;
;;  Written by hidnplayr@kolibrios.org                             ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; TODO: more precise timer, ttl, user selectable size/number of packets

format binary as ""

BUFFERSIZE      = 1500
IDENTIFIER      = 0x1337

use32
        org     0x0

        db      'MENUET01'      ; signature
        dd      1               ; header version
        dd      start           ; entry point
        dd      I_END           ; initialized size
        dd      mem             ; required memory
        dd      mem             ; stack pointer
        dd      s               ; parameters
        dd      0               ; path


; useful includes
include '../../macros.inc'
purge mov,add,sub
include '../../proc32.inc'
include '../../dll.inc'
include '../../network.inc'

include 'icmp.inc'


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
; main loop
        cmp     byte[s], 0
        jne     resolve

        push    str_welcome
        call    [con_write_asciiz]
main:
; write prompt
        push    str_prompt
        call    [con_write_asciiz]
; read string
        mov     esi, s
        push    256
        push    esi
        call    [con_gets]
; check for exit
        test    eax, eax
        jz      exit
        cmp     byte [esi], 10
        jz      exit
; delete terminating '\n'
        push    esi
@@:
        lodsb
        test    al, al
        jnz     @b
        mov     byte [esi-2], al
        pop     esi

; reset stats
        mov     [stats.tx], 0
        mov     [stats.rx], 0
        mov     [stats.time], 0

resolve:
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

; convert IP address to decimal notation
        mov     eax, [esi+addrinfo.ai_addr]
        mov     eax, [eax+sockaddr_in.sin_addr]
        mov     [sockaddr1.ip], eax
        push    eax
        call    [inet_ntoa]
; write result
        mov     [ip_ptr], eax

        push    eax

; free allocated memory
        push    esi
        call    [freeaddrinfo]

        push    str4
        call    [con_write_asciiz]

        mcall   socket, AF_INET4, SOCK_RAW, IPPROTO_ICMP
        cmp     eax, -1
        jz      fail2
        mov     [socketnum], eax

        mcall   connect, [socketnum], sockaddr1, 18

        mcall   40, 1 shl 7 ; + 7
;        call    [con_cls]

        mov     [count], 4

        push    str3
        call    [con_write_asciiz]

        push    [ip_ptr]
        call    [con_write_asciiz]

        push    (icmp_packet.length - ICMP_Packet.Data)
        push    str3b
        call    [con_printf]

mainloop:
        inc     [stats.tx]
        mcall   26,9
        mov     [time_reference], eax
        mcall   send, [socketnum], icmp_packet, icmp_packet.length, 0

        mcall   23, 300 ; 3 seconds time-out
        mcall   26,9
        sub     eax, [time_reference]
        xor     edx, edx
        mov     cx, 10
        mul     cx
        mov     [time_reference], eax

        mcall   recv, [socketnum], buffer_ptr, BUFFERSIZE, MSG_DONTWAIT
        cmp     eax, -1
        je      .no_response

        sub     eax, ICMP_Packet.Data
        jb      .invalid
        mov     [recvd], eax

        cmp     word[buffer_ptr + ICMP_Packet.Identifier], IDENTIFIER
        jne     .invalid

; OK, we have a response, update stats and let the user know
        inc     [stats.rx]
        mov     eax, [time_reference]
        add     [stats.time], eax

        push    str11                   ; TODO: print IP address of packet sender
        call    [con_write_asciiz]

; validate the packet
        lea     esi, [buffer_ptr + ICMP_Packet.Data]
        mov     ecx, [recvd]
        mov     edi, icmp_packet.data
        repe    cmpsb
        jne     .miscomp

; All OK, print to the user!
        push    [time_reference]
        movzx   eax, word[buffer_ptr + ICMP_Packet.SequenceNumber]
        push    eax
        push    [recvd]

        push    str7
        call    [con_printf]

        jmp     .continue

; Error in packet, print it to user
  .miscomp:
        sub     edi, icmp_packet.data
        push    edi
        push    str9
        call    [con_printf]
        jmp     .continue

; Invalid reply
  .invalid:
        push    str10
        call    [con_write_asciiz]
        jmp     .continue

; Timeout!
  .no_response:
        push    str8
        call    [con_write_asciiz]

; Send more ICMP packets ?
  .continue:
        inc     [icmp_packet.seq]

        dec     [count]
        jz      done

        mcall   5, 100  ; wait a second

        jmp     mainloop

; Done..
done:
        cmp     [stats.rx], 0
        jne     @f
        xor     eax, eax
        jmp     .zero
  @@:
        xor     edx, edx
        mov     eax, [stats.time]
        div     [stats.rx]
  .zero:
        push    eax
        push    [stats.rx]
        push    [stats.tx]
        push    str12
        call    [con_printf]
        jmp     main

; DNS error
fail:
        push    str5
        call    [con_write_asciiz]
        jmp     main

; Socket error
fail2:
        push    str6
        call    [con_write_asciiz]
        jmp     main

; Finally.. exit!
exit:
        push    1
        call    [con_exit]

        mcall   -1


; data
title   db      'ICMP echo (ping) client',0
str_welcome db  'Please enter the hostname or IP-address of the host you want to ping,',10
            db  'or just press enter to exit.',10,0
str_prompt  db  10,'> ',0
str3    db      'Pinging to ',0
str3b   db      ' with %u data bytes',10,0

str4    db      10,0
str5    db      'Name resolution failed.',10,0
str6    db      'Could not open socket',10,0

str11   db      'Answer: ',0
str7    db      'bytes=%u seq=%u time=%u ms',10,0
str8    db      'timeout!',10,0
str9    db      'miscompare at offset %u',10,0
str10   db      'reply invalid',10,0

str12   db      10,'Ping stats:',10,'%u packets sent, %u packets received',10,'average response time=%u ms',10,0

sockaddr1:
        dw AF_INET4
.port   dw 0
.ip     dd 0
        rb 10

time_reference  dd ?
ip_ptr          dd ?
count           dd ?
recvd           dd ?    ; received number of bytes in last packet

stats:
        .tx     dd ?
        .rx     dd ?
        .time   dd ?

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
        con_printf,       'con_printf',     \
        con_exit,       'con_exit',     \
        con_gets,       'con_gets',\
        con_cls,        'con_cls',\
        con_getch2,     'con_getch2',\
        con_set_cursor_pos, 'con_set_cursor_pos'

socketnum       dd ?

icmp_packet:    db 8            ; type
                db 0            ; code
                dw 0            ;
 .id            dw IDENTIFIER   ; identifier
 .seq           dw 0x0000       ; sequence number
 .data          db 'abcdefghijklmnopqrstuvwxyz012345'
 .length = $ - icmp_packet

I_END:

buffer_ptr      rb BUFFERSIZE

s               rb 1024
                rb 4096    ; stack
mem:
