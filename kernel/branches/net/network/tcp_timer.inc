;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  Part of the tcp/ip network stack for KolibriOS                 ;;
;;                                                                 ;;
;;   Written by hidnplayr@kolibrios.org                            ;;
;;                                                                 ;;
;;    Based on the code of 4.4BSD                                  ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

;----------------------
; 160 ms timer
;----------------------
macro   TCP_timer_160ms {

local   .loop
local   .exit

        mov     ebx, net_sockets
  .loop:
        mov     ebx, [ebx + SOCKET.NextPtr]
        or      ebx, ebx
        jz      .exit

        cmp     [ebx + SOCKET.Domain], AF_INET4
        jne     .loop

        cmp     [ebx + SOCKET.Protocol], IP_PROTO_TCP
        jne     .loop

        test    [ebx + TCP_SOCKET.t_flags], TF_DELACK
        jz      .loop
        and     [ebx + TCP_SOCKET.t_flags], not (TF_DELACK)

        push    ebx
        mov     cl, TH_ACK
        call    TCP_respond_socket
;        and     [ebx + TCP_SOCKET.t_flags], TF_ACKNOW   ;;
;        mov     eax, ebx                                ;;
;        call    TCP_output                              ;;
        pop     ebx

        jmp     .loop

  .exit:

}


;----------------------
; 640 ms timer
;----------------------
macro   TCP_timer_640ms {

local   .loop
local   .exit

; Update TCP sequence number

        add     [TCP_sequence_num], 64000

; scan through all the active TCP sockets, decrementing ALL timers
; timers do not have the chance to wrap because the keepalive timer will kill the socket when it expires

        mov     eax, net_sockets
  .loop:
        mov     eax, [eax + SOCKET.NextPtr]
  .check_only:
        or      eax, eax
        jz      .exit

        cmp     [eax + SOCKET.Domain], AF_INET4
        jne     .loop

        cmp     [eax + SOCKET.Protocol], IP_PROTO_TCP
        jne     .loop

        inc     [eax + TCP_SOCKET.t_idle]
        dec     [eax + TCP_SOCKET.timer_retransmission]
        jnz     .check_more2

        DEBUGF  1,"socket %x: Retransmission timer expired\n", eax

        push    eax
        call    TCP_output
        pop     eax

  .check_more2:
        dec     [eax + TCP_SOCKET.timer_keepalive]
        jnz     .check_more3

        DEBUGF  1,"socket %x: Keepalive expired\n", eax

        push    eax
        call    TCP_disconnect
        pop     eax
        jmp     .loop

  .check_more3:
        dec     [eax + TCP_SOCKET.timer_timed_wait]
        jnz     .check_more5

        DEBUGF  1,"socket %x: 2MSL timer expired\n", eax

  .check_more5:
        dec     [eax + TCP_SOCKET.timer_persist]
        jnz     .loop

        DEBUGF  1,"socket %x: persist timer expired\n", eax

        call    TCP_set_persist
        mov     [eax + TCP_SOCKET.t_force], 1
        push    eax
        call    TCP_output
        pop     eax
        mov     [eax + TCP_SOCKET.t_force], 0

        jmp     .loop
  .exit:

}



; eax = socket

TCP_cancel_timers:

        push    eax edi

        lea     edi, [eax + TCP_SOCKET.timer_retransmission]
        xor     eax, eax
        stosd
        stosd
        stosd
        stosd
        stosd

        pop     edi eax


        ret