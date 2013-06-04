;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  Part of the TCP/IP network stack for KolibriOS                 ;;
;;                                                                 ;;
;;   Written by hidnplayr@kolibrios.org                            ;;
;;                                                                 ;;
;;    Based on the code of 4.4BSD                                  ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 3143 $

timer_flag_retransmission       = 1 shl 0
timer_flag_keepalive            = 1 shl 1
timer_flag_2msl                 = 1 shl 2
timer_flag_persist              = 1 shl 3
timer_flag_wait                 = 1 shl 4


;----------------------
; 160 ms timer
;----------------------
macro   TCP_timer_160ms {

local   .loop
local   .exit

        mov     ebx, net_sockets
  .loop:
        mov     ebx, [ebx + SOCKET.NextPtr]
        test    ebx, ebx
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
        call    TCP_respond
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
macro   TCP_timer_640ms {                       ; TODO: implement timed wait timer!

local   .loop
local   .exit

; Update TCP sequence number

        add     [TCP_sequence_num], 64000

; scan through all the active TCP sockets, decrementing ALL timers
; When a timer reaches zero, we'll check wheter it was active or not

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
        test    [eax + TCP_SOCKET.timer_flags], timer_flag_retransmission
        jz      .check_more2

        DEBUGF  DEBUG_NETWORK_VERBOSE, "socket %x: Retransmission timer expired\n", eax

        push    eax
        call    TCP_output
        pop     eax

  .check_more2:
        dec     [eax + TCP_SOCKET.timer_keepalive]
        jnz     .check_more3
        test    [eax + TCP_SOCKET.timer_flags], timer_flag_keepalive
        jz      .check_more3

        DEBUGF  DEBUG_NETWORK_VERBOSE, "socket %x: Keepalive expired\n", eax

        cmp     [eax + TCP_SOCKET.state], TCPS_ESTABLISHED
        ja      .dont_kill

        push    eax
        call    TCP_disconnect
        pop     eax
        jmp     .loop

  .dont_kill:
        test    [eax + SOCKET.options], SO_KEEPALIVE
        jz      .reset_keepalive

        push    eax
        mov     ebx, eax
        xor     cl, cl
        call    TCP_respond     ; send keepalive
        pop     eax
        mov     [eax + TCP_SOCKET.timer_keepalive], TCP_time_keep_interval
        jmp     .check_more3

  .reset_keepalive:
        mov     [eax + TCP_SOCKET.timer_keepalive], TCP_time_keep_idle

  .check_more3:
        dec     [eax + TCP_SOCKET.timer_timed_wait]
        jnz     .check_more5
        test    [eax + TCP_SOCKET.timer_flags], timer_flag_2msl
        jz      .check_more5

        DEBUGF  DEBUG_NETWORK_VERBOSE, "socket %x: 2MSL timer expired\n", eax

  .check_more5:
        dec     [eax + TCP_SOCKET.timer_persist]
        jnz     .loop
        test    [eax + TCP_SOCKET.timer_flags], timer_flag_persist
        jz      .loop

        DEBUGF  DEBUG_NETWORK_VERBOSE, "socket %x: persist timer expired\n", eax

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

        mov     [eax + TCP_SOCKET.timer_flags], 0

        ret