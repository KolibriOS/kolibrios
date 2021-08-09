;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2010-2021. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  tracert.asm - Trace network route for KolibriOS                ;;
;;                                                                 ;;
;;  Written by hidnplayr@kolibrios.org                             ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary as ""

BUFFERSIZE      = 1500
IDENTIFIER      = 0x1337

__DEBUG__               = 1             ; enable/disable
__DEBUG_LEVEL__         = 2             ; 1 = all, 2 = errors

use32
        org     0x0

        db      'MENUET01'      ; signature
        dd      1               ; header version
        dd      START           ; entry point
        dd      I_END           ; initialized size
        dd      IM_END+0x1000   ; required memory
        dd      IM_END+0x1000   ; stack pointer
        dd      params          ; parameters
        dd      0               ; path

include '../../proc32.inc'
include '../../macros.inc'
purge mov,add,sub
include '../../dll.inc'
include '../../struct.inc'
include '../../debug-fdo.inc'
include '../../network.inc'

include '../icmp.inc'
include '../ip.inc'


START:
; init heap
        mcall   68, 11
        test    eax, eax
        jz      exit
; load libraries
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     exit
; initialize console
        invoke  con_start, 1
        invoke  con_init, 80, 25, 80, 250, title
; main loop
        cmp     byte[params], 0
        jne     parse_param

        invoke  con_write_asciiz, str_welcome
main:
; write prompt
        invoke  con_write_asciiz, str_prompt
; read string
        mov     esi, params
        invoke  con_gets, esi, 1024
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
        mov     [esi-2], al
        pop     esi

parse_param:
; Check if any additional parameters were given

        DEBUGF  2, "parse parameters\n"
        mov     esi, params
        mov     ecx, 1024
  .addrloop:
        lodsb
        test    al, al
        jz      .resolve
        cmp     al, ' '
        jne     .addrloop
        mov     byte[esi-1], 0
        jmp     .param

  .param_loop:
        lodsb
        test    al, al
        jz      .resolve
        cmp     al, ' '
        jne     .invalid
  .param:
        lodsb
        cmp     al, '-'
        jne     .invalid
        lodsb
        ; implement more parameters here
  .invalid:
        invoke  con_write_asciiz, str13
        jmp     main

  .resolve:
        DEBUGF  2, "resolve\n"
; resolve name
        push    esp     ; reserve stack place
        invoke  getaddrinfo, params, 0, 0, esp
        pop     esi
; test for error
        test    eax, eax
        jnz     fail

; convert IP address to decimal notation
        mov     eax, [esi+addrinfo.ai_addr]
        mov     eax, [eax+sockaddr_in.sin_addr]
        mov     [sockaddr1.ip], eax
        invoke  inet_ntoa
; write result
        mov     [ip_ptr], eax

        push    eax

; free allocated memory
        invoke  freeaddrinfo, esi

        invoke  con_write_asciiz, str4

        mcall   socket, AF_INET4, SOCK_RAW, IPPROTO_ICMP
        cmp     eax, -1
        jz      fail2
        mov     [icmp_socket], eax

        mcall   socket, AF_INET4, SOCK_DGRAM, 0
        cmp     eax, -1
        jz      fail2
        mov     [udp_socket], eax

        mcall   connect, [udp_socket], sockaddr1, 18
        cmp     eax, -1
        je      fail2

        mcall   40, EVM_STACK

        invoke  con_write_asciiz, str3
        invoke  con_write_asciiz, [ip_ptr]
        invoke  con_write_asciiz, str4

        mov     [ttl], 1

 ;;       mcall   send, [udp_socket], udp_packet, 5, 0    ; dummy send

        mcall   recv, [icmp_socket], buffer_ptr, BUFFERSIZE, MSG_DONTWAIT ;; dummy read

mainloop:
        invoke  con_get_flags
        test    eax, 0x200                      ; con window closed?
        jnz     exit_now

        invoke  con_kbhit
        test    eax, eax
        jz      .nokey
        invoke  con_getch2
        cmp     ax, 0x1E03      ; Ctrl+C
        je      main
  .nokey:

        pushd   [ttl]
        invoke  con_printf, str9
        add     esp, 2*4

        DEBUGF  2, "Setsockopt\n"

        pushd   [ttl]
        pushd   4                               ; length of option
        pushd   IP_TTL
        pushd   IPPROTO_IP
        mcall   setsockopt, [udp_socket], esp
        add     esp, 16
        cmp     eax, -1
        je      fail2

        DEBUGF  2, "Sending\n"

        mcall   26, 10                          ; Get high precision timer count
        mov     [time_reference], eax
        mcall   send, [udp_socket], udp_packet, 5, 0
        cmp     eax, -1
        je      fail2

        DEBUGF  2, "Packet sent\n", str_ini_int

   .receive:
        mcall   23, [timeout]

        mcall   26, 10                          ; Get high precision timer count
        sub     eax, [time_reference]
        jz      @f
        xor     edx, edx
        mov     ebx, 100000
        div     ebx
        cmp     edx, 50000
        jb      @f
        inc     eax
  @@:
        mov     [time_reference], eax

; Receive reply
        mcall   recv, [icmp_socket], buffer_ptr, BUFFERSIZE, MSG_DONTWAIT
        cmp     eax, -1
        je      .timeout
        test    eax, eax
        jz      fail2

        DEBUGF  2, "Answer of %u bytes\n", eax

; IP header length
        movzx   esi, byte[buffer_ptr]
        and     esi, 0xf
        shl     esi, 2

; Check packet length
        sub     eax, esi
        sub     eax, sizeof.ICMP_header
        jb      .invalid
        mov     [recvd], eax

        DEBUGF  2, "Packet length OK\n", eax

; make esi point to ICMP packet header
        add     esi, buffer_ptr

; Verify packet
;;        movzx   eax, [esi + sizeof.ICMP_header + IPv4_header.TimeToLive]
;;        cmp     eax, [ttl]
;;        jne     .receive

; What kind of response is it?
        DEBUGF  2, "Response Type: %u Code: %u\n", [esi + ICMP_header.Type], [esi + ICMP_header.Code]
        cmp     [esi + ICMP_header.Type], ICMP_UNREACH_PORT
        je      .last
        cmp     [esi + ICMP_header.Type], ICMP_TIMXCEED
        jne     .invalid
        call    .print
        jmp     .continue

  .last:
        call    .print
        jmp     main

  .print:
        DEBUGF  2, "Valid response\n"
; we have a response, print a line
        mov     eax, [time_reference]
        xor     edx, edx
        mov     ebx, 10
        div     ebx
        push    edx
        push    eax

        invoke  con_printf, str1
        add     esp, 3*4

        mov     ebx, [buffer_ptr + IPv4_header.SourceAddress]
        push    ebx
        call    reverse_dns_lookup

        pop     eax
        rol     eax, 16
        movzx   ebx, ah
        push    ebx
        movzx   ebx, al
        push    ebx
        shr     eax, 16
        movzx   ebx, ah
        push    ebx
        movzx   ebx, al
        push    ebx

        invoke  con_printf, str2
        add     esp, 5*4

        ret


; Invalid reply
  .invalid:
        DEBUGF  2, "Invalid response\n"
        invoke  con_write_asciiz, str10
        jmp     main    ;.continue

; Timeout!
  .timeout:
        DEBUGF  2, "Timeout\n", eax
        invoke  con_write_asciiz, str8

; Send more ICMP packets ?
  .continue:
        inc     [ttl]

; wait a second before sending next request
        mcall   5, 100
        jmp     mainloop

; DNS error
fail:
        invoke  con_write_asciiz, str5
        jmp     main

; Socket error
fail2:
        invoke  con_write_asciiz, str6
        jmp     main

; Finally.. exit!
exit:
        invoke  con_exit, 1
exit_now:
        mcall   -1


ascii_to_dec:

        lodsb
        cmp     al, ' '
        jne     .fail

        xor     eax, eax
        xor     ebx, ebx
  .loop:
        lodsb
        test    al, al
        jz      .done
        cmp     al, ' '
        je      .done
        sub     al, '0'
        jb      .fail
        cmp     al, 9
        ja      .fail
        lea     ebx, [ebx*4+ebx]
        lea     ebx, [ebx*2+eax]
        jmp     .loop
  .fail:
        xor     ebx, ebx
  .done:
        dec     esi
        ret


; ebx = ip
reverse_dns_lookup:

        push    ebx
        mcall   socket, AF_INET4, SOCK_DGRAM, 0
        pop     ebx
        cmp     eax, -1
        je      .fail
        mov     [dns_socket], eax

        push    ebx
        mcall   connect, [dns_socket], sockaddr2, 18
        pop     ebx
        cmp     eax, -1
        je      .fail

        mov     edi, dns_pkt.name
        rol     ebx, 8
        movzx   eax, bl
        call    byte_to_ascii
        rol     ebx, 8
        movzx   eax, bl
        call    byte_to_ascii
        rol     ebx, 8
        movzx   eax, bl
        call    byte_to_ascii
        rol     ebx, 8
        movzx   eax, bl
        call    byte_to_ascii

        mov     esi, dns_tr
        mov     ecx, dns_tr.length
        rep movsb

        sub     edi, dns_pkt
        mov     esi, edi

        mcall   send, [dns_socket], dns_pkt, , 0
        cmp     eax, -1
        je      .fail

        push    esi
        mcall   recv, [dns_socket], buffer_ptr, BUFFERSIZE, 0
        pop     esi

        mcall   close, [dns_socket]

        cmp     word[buffer_ptr+6], 0   ; answers
        je      .fail

        add     esi, buffer_ptr+12
        mov     edi, buffer_ptr
        xor     ecx, ecx
        lodsb
        test    al, al
        jz      @f
        movzx   ecx, al
  @@:
        rep movsb
        lodsb
        test    al, al
        jz      @f
        movzx   ecx, al
        mov     al, '.'
        stosb
        jmp     @r
  @@:
        stosb

        invoke  con_write_asciiz, buffer_ptr
        invoke  con_write_asciiz, str7
        ret

  .fail:
        ret



; input: eax - number
;        edi - ptr
byte_to_ascii:

        push    ebx ecx edx

        xor     edx, edx        ; result
        xor     ecx, ecx        ; byte count
        inc     ecx
        mov     bl, 10          ; divisor

        div     bl
        mov     dl, ah
        add     dl, '0'
        and     ax, 0x00ff
        jz      .ok

        inc     ecx
        shl     edx, 8

        div     bl
        mov     dl, ah
        add     dl, '0'
        and     ax, 0x00ff
        jz      .ok

        inc     ecx
        shl     edx, 8

        mov     dl, al
        add     dl, '0'

  .ok:
        shl     edx, 8
        mov     dl, cl
        mov     [edi], edx
        add     edi, ecx
        inc     edi

        pop     edx ecx ebx
        ret


; data
title   db      'Trace route',0
str_welcome db  'Please enter the hostname or IP-address of the host you want to trace,',10
            db  'or just press enter to exit.',10,10,0
str_prompt  db  10,'> ',0
str3    db      'Tracing route to ',0

str4    db      10,0
str7    db      ' ', 0
str5    db      'Name resolution failed.',10,0
str6    db      'Socket error.',10,0
str13   db      'Invalid parameter(s)',10,0

str9    db      '%u ',0
str1    db      '%u.%u ms ',0
str2    db      '[%u.%u.%u.%u]',10,0
str10   db      'Invalid reply',10,0
str8    db      'Timeout!',10,0


sockaddr1:
        dw AF_INET4
.port   dw 666
.ip     dd 0
        rb 10

sockaddr2:
        dw AF_INET4
.port   dw 53 shl 8     ; DNS port
.ip     dd 0x08080808   ; Google DNS
        rb 10

time_reference  dd ?
ip_ptr          dd ?
ttl             dd ?
timeout         dd 500
recvd           dd ?    ; received number of bytes in last packet

; import
align 4
@IMPORT:

library console, 'console.obj', \
        network, 'network.obj'

import  console,        \
        con_start,      'START',        \
        con_init,       'con_init',     \
        con_write_asciiz,       'con_write_asciiz',     \
        con_printf,       'con_printf',     \
        con_exit,       'con_exit',     \
        con_gets,       'con_gets',\
        con_cls,        'con_cls',\
        con_getch2,     'con_getch2',\
        con_set_cursor_pos, 'con_set_cursor_pos',\
        con_get_flags,  'con_get_flags',\
        con_kbhit,      'con_kbhit'

import  network,        \
        getaddrinfo,    'getaddrinfo',  \
        freeaddrinfo,   'freeaddrinfo', \
        inet_ntoa,      'inet_ntoa'

include_debug_strings

icmp_socket     dd ?
udp_socket      dd ?
dns_socket      dd ?

udp_packet      db 'hello!'

dns_tr:
        db  7,'in-addr',4,'arpa',0
        dw  0x0C00      ; Qtype: PTR
        dw  0x0100      ; Class: IN

  .length = $ - dns_tr

dns_pkt:
        dw  0x9A02      ; Transaction ID
        dw  0x0001      ; Flags: Recursive desired
        dw  0x0100      ; Questions
        dw  0x0000      ; Answers
        dw  0x0000      ; Authority RR
        dw  0x0000      ; Additional RR
  .name rb  512

I_END:

params          rb 1024
buffer_ptr:     rb BUFFERSIZE

IM_END:
