;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2018. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;   Written by hidnplayr@kolibrios.org                            ;;
;;                                                                 ;;
;;         GNU GENERAL PUBLIC LICENSE                              ;;
;;          Version 2, June 1991                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


socket_connect:

; ignore if status is not "disconnected"
        cmp     [status], STATUS_DISCONNECTED
        jne     .reconnect

        if TIMESTAMP
        call    print_timestamp
        end if
        mov     esi, str_connecting
        call    print_asciiz

        mov     esi, irc_server_name
        call    print_asciiz

        mov     al, 10
        call    print_char

; update status
        inc     [status]        ; was STATUS_DISCONNECTED, now STATUS_RESOLVING

; resolve name
        push    esp     ; reserve stack place
        push    esp     ; fourth parameter
        push    0       ; third parameter
        push    0       ; second parameter
        push    irc_server_name
        call    [getaddrinfo]
        pop     esi
; test for error
        test    eax, eax
        jnz     .fail_dns

; fill in ip in sockstruct
        mov     eax, [esi + addrinfo.ai_addr]
        mov     eax, [eax + sockaddr_in.sin_addr]
        mov     [sockaddr1.ip], eax

; free allocated memory
        push    esi
        call    [freeaddrinfo]

; update status
        inc     [status]

; connect
        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        jz      .fail
        mov     [socketnum], eax

        mcall   connect, [socketnum], sockaddr1, 18
        cmp     eax, -1
        jz      .fail_refused

        ret

  .fail:
        mov     [status], STATUS_DISCONNECTED

        if TIMESTAMP
        call    print_timestamp
        end if
        mov     esi, str_sockerr
        call    print_asciiz

        ret

  .fail_dns:
        mov     [status], STATUS_DISCONNECTED

        if TIMESTAMP
        call    print_timestamp
        end if
        mov     esi, str_dnserr
        call    print_asciiz

        ret

  .fail_refused:
        mov     [status], STATUS_DISCONNECTED

        if TIMESTAMP
        call    print_timestamp
        end if
        mov     esi, str_refused
        call    print_asciiz

        ret

  .reconnect:
        if TIMESTAMP
        call    print_timestamp
        end if
        mov     esi, str_reconnect
        call    print_asciiz

        mov     esi, quit_msg
        call    quit_server

        jmp     socket_connect


socket_write_userinfo:

; create packet in packetbuf
        mov     edi, packetbuf

        mov     eax, 'NICK'
        stosd
        mov     al, ' '
        stosb
        mov     esi, user_nick
        mov     ecx, MAX_NICK_LEN
  .loop:
        lodsb
        test    al, al
        jz      .done
        stosb
        dec     ecx
        jnz     .loop
  .done:
        mov     ax, 0x0a0d
        stosw

        mov     eax, 'USER'
        stosd
        mov     al, ' '
        stosb
        mov     esi, user_nick
        mov     ecx, MAX_NICK_LEN
  .loop2:
        lodsb
        test    al, al
        jz      .done2
        stosb
        dec     ecx
        jnz     .loop2
  .done2:
        mov     eax, ' 8 *'
        stosd
        mov     ax, ' :'
        stosw
        mov     al, ' '
        stosb
        mov     esi, user_real_name
        mov     ecx, MAX_REAL_LEN
  .loop3:
        lodsb
        test    al, al
        jz      .done3
        stosb
        dec     ecx
        jnz     .loop3
  .done3:
        mov     ax, 0x0a0d
        stosw

        lea     esi, [edi - packetbuf]
        mcall   send, [socketnum], packetbuf, , 0

        ret




process_network_event:
; values for status: 0, 1, 2, 3
        mov     eax, [status]
        dec     eax
; 0 = STATUS_DISCONNECTED - do nothing
; (ignore network events if we are disconnected from network)
        js      .nothing
; 1 = STATUS_RESOLVING
        jz      .nothing
; 2 = STATUS_CONNECTING
        dec     eax
        jz      .connecting
; 3 = STATUS_CONNECTED
        jmp     .connected

  .nothing:
        ret

  .connecting:
        call    socket_write_userinfo

; The connection has been established, change status from "connecting" to "connected".
        inc     [status]

  .connected:
        call    socket_receive
        ret


socket_receive:

        pusha

; FIXME: make this a proper stream!

  .nextpacket:
        mcall   recv, [socketnum], packetbuf, PACKETBUF_SIZE, MSG_DONTWAIT    ; read a packet
        inc     eax                                                           ; check if we got any data
        jz      .done                                                         ; TODO: handle errors!
        dec     eax
        jz      .disconnected

; ok we have data, now feed it to the command splicer

        mov     ecx, eax
        mov     esi, packetbuf                          ; esi = start pointer
  .nextcommand:
        mov     edi, servercommand
  .byteloop:
        lodsb
        cmp     al, 10
        je      .got_command
        cmp     al, 13
        je      .got_command
        stosb
        dec     ecx
        jnz     .byteloop
        ;;; FIXME
        jmp     .nextpacket

; we have a command, call the serverparser

  .got_command:
        mov     byte[edi], 0                            ; mark the end of the command
        push    esi ecx
        call    server_parser
        pop     ecx esi
        test    ecx, ecx
        jnz     .nextcommand

  .done:
        popa
        ret


  .disconnected:
        if TIMESTAMP
        call    print_timestamp
        end if
        mov     esi, str_srv_disconnected
        call    print_asciiz

        mov     [status], STATUS_DISCONNECTED
        mcall   close, [socketnum]

        popa
        ret