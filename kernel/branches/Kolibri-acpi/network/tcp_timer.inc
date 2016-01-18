;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved.    ;;
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

$Revision: 6011 $

timer_flag_retransmission       = 1 shl 0
timer_flag_keepalive            = 1 shl 1
timer_flag_2msl                 = 1 shl 2
timer_flag_persist              = 1 shl 3
timer_flag_wait                 = 1 shl 4


macro   tcp_timer_160ms {

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
        call    tcp_respond
        pop     ebx

        inc     [TCPS_delack]                   ; update stats

        jmp     .loop

  .exit:

}


align 4
proc tcp_timer_640ms

        xor     esi, esi
        mov     ecx, MANUAL_DESTROY
        call    create_event
        mov     [TCP_timer1_event], eax

  .wait:
        mov     eax, [TCP_timer1_event]
        mov     ebx, [eax + EVENT.id]
        call    wait_event

; Update TCP sequence number

        add     [TCP_sequence_num], 64000

; Scan through all the active TCP sockets, decrementing all active timers
; When a timer reaches zero, run its handler.

        mov     eax, net_sockets
  .loop:
        mov     eax, [eax + SOCKET.NextPtr]
  .check_only:
        or      eax, eax
        jz      .wait

        cmp     [eax + SOCKET.Domain], AF_INET4
        jne     .loop

        cmp     [eax + SOCKET.Protocol], IP_PROTO_TCP
        jne     .loop

        inc     [eax + TCP_SOCKET.t_idle]

        test    [eax + TCP_SOCKET.timer_flags], timer_flag_retransmission
        jz      .check_more2
        dec     [eax + TCP_SOCKET.timer_retransmission]
        jnz     .check_more2

        DEBUGF  DEBUG_NETWORK_VERBOSE, "socket %x: Retransmission timer expired\n", eax

        push    eax
        call    tcp_output
        pop     eax

  .check_more2:
        test    [eax + TCP_SOCKET.timer_flags], timer_flag_keepalive
        jz      .check_more3
        dec     [eax + TCP_SOCKET.timer_keepalive]
        jnz     .check_more3

        DEBUGF  DEBUG_NETWORK_VERBOSE, "socket %x: Keepalive expired\n", eax

        cmp     [eax + TCP_SOCKET.state], TCPS_ESTABLISHED
        ja      .dont_kill

        push    eax
        call    tcp_disconnect
        pop     eax
        jmp     .loop

  .dont_kill:
        test    [eax + SOCKET.options], SO_KEEPALIVE
        jz      .reset_keepalive

        push    eax
        mov     ebx, eax
        xor     cl, cl
        call    tcp_respond                     ; send keepalive
        pop     eax
        mov     [eax + TCP_SOCKET.timer_keepalive], TCP_time_keep_interval
        jmp     .check_more3

  .reset_keepalive:
        mov     [eax + TCP_SOCKET.timer_keepalive], TCP_time_keep_idle

  .check_more3:
        test    [eax + TCP_SOCKET.timer_flags], timer_flag_2msl
        jz      .check_more5
        dec     [eax + TCP_SOCKET.timer_timed_wait]
        jnz     .check_more5

        DEBUGF  DEBUG_NETWORK_VERBOSE, "socket %x: 2MSL timer expired\n", eax

  .check_more5:
        test    [eax + TCP_SOCKET.timer_flags], timer_flag_persist
        jz      .check_more6
        dec     [eax + TCP_SOCKET.timer_persist]
        jnz     .check_more6

        DEBUGF  DEBUG_NETWORK_VERBOSE, "socket %x: persist timer expired\n", eax

        call    tcp_set_persist
        mov     [eax + TCP_SOCKET.t_force], 1
        push    eax
        call    tcp_output
        pop     eax
        mov     [eax + TCP_SOCKET.t_force], 0

  .check_more6:
        test    [eax + TCP_SOCKET.timer_flags], timer_flag_wait
        jz      .loop
        dec     [eax + TCP_SOCKET.timer_timed_wait]
        jnz     .loop

        DEBUGF  DEBUG_NETWORK_VERBOSE, "socket %x: timed wait timer expired\n", eax

        push    [eax + SOCKET.NextPtr]
        call    tcp_close
        pop     eax

        jmp     .check_only

endp


;-----------------------------------------------------------------;
;                                                                 ;
; TCP_cancel_timers                                               ;
;                                                                 ;
;   IN: eax = socket                                              ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
tcp_cancel_timers:

        mov     [eax + TCP_SOCKET.timer_flags], 0

        ret