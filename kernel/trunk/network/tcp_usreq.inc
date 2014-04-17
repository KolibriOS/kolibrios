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

$Revision$


;-------------------------
;
; TCP_usrclose
;
; Move connection to next state, based on process close.
;
;  IN:  eax = socket ptr
;
;-------------------------
align 4
TCP_usrclosed:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_usrclosed: %x\n", eax

        push    ebx
        mov     ebx, [eax + TCP_SOCKET.t_state]
        mov     ebx, dword [.switch + ebx*4]
        jmp     ebx

  .switch:

        dd      .close                  ; TCPS_CLOSED
        dd      .close                  ; TCPS_LISTEN
        dd      .close                  ; TCPS_SYN_SENT
        dd      .wait1                  ; TCPS_SYN_RECEIVED
        dd      .wait1                  ; TCPS_ESTABLISHED
        dd      .last_ack               ; TCPS_CLOSE_WAIT
        dd      .ret                    ; TCPS_FIN_WAIT_1
        dd      .ret                    ; TCPS_CLOSING
        dd      .ret                    ; TCPS_LAST_ACK
        dd      .disc                   ; TCPS_FIN_WAIT_2
        dd      .disc                   ; TCPS_TIMED_WAIT


  .close:
        mov     [eax + TCP_SOCKET.t_state], TCPS_CLOSED
        call    TCP_close
        pop     ebx
        ret

  .wait1:
        mov     [eax + TCP_SOCKET.t_state], TCPS_FIN_WAIT_1
        pop     ebx
        ret

  .last_ack:
        mov     [eax + TCP_SOCKET.t_state], TCPS_LAST_ACK
        pop     ebx
        ret

  .disc:
        call    SOCKET_is_disconnected
  .ret:
        pop     ebx
        ret


;-------------------------
;
; TCP_connect
;
;  IN:  eax = socket ptr
;  OUT: eax = 0 ok / -1 error
;       ebx = error code
;
;-------------------------
align 4
TCP_connect:

        test    [eax + SOCKET.state], SS_ISCONNECTED
        jnz     .eisconn

        push    eax edx
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_lock
        pop     edx eax

; Fill in local IP
        cmp     [eax + IP_SOCKET.LocalIP], 0
        jne     @f
        push    [IP_LIST + 4]                                   ; FIXME: use correct local IP
        pop     [eax + IP_SOCKET.LocalIP]

; Fill in remote port and IP
        pushw   [edx + 2]
        pop     [eax + TCP_SOCKET.RemotePort]

        pushd   [edx + 4]
        pop     [eax + IP_SOCKET.RemoteIP]

; Find a local port, if user didnt define one
        cmp     [eax + TCP_SOCKET.LocalPort], 0
        jne     @f
        call    SOCKET_find_port
       @@:

; Start the TCP sequence
        mov     [eax + TCP_SOCKET.timer_persist], 0
        mov     [eax + TCP_SOCKET.t_state], TCPS_SYN_SENT

        push    [TCP_sequence_num]
        add     [TCP_sequence_num], 6400
        pop     [eax + TCP_SOCKET.ISS]
        mov     [eax + TCP_SOCKET.timer_keepalive], TCP_time_keep_init

        TCP_sendseqinit eax

        mov     ebx, eax
        lea     eax, [ebx + STREAM_SOCKET.snd]
        call    SOCKET_ring_create
        test    eax, eax
        jz      .nomem

        lea     eax, [ebx + STREAM_SOCKET.rcv]
        call    SOCKET_ring_create
        test    eax, eax
        jz      .nomem

        push    ebx
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_unlock
        pop     eax

        call    SOCKET_is_connecting

; Now send the SYN packet to remote end
        push    eax
        call    TCP_output
        pop     eax

  .block:
        test    [eax + SOCKET.options], SO_NONBLOCK
        jz      .waitforit

        xor     eax, eax
        dec     eax
        mov     ebx, EINPROGRESS
        ret

  .nomem:
        xor     eax, eax
        dec     eax
        mov     ebx, ENOMEM
        ret

  .eisconn:
        xor     eax, eax
        dec     eax
        mov     ebx, EISCONN
        ret

  .waitforit:
        push    eax
        stdcall timer_hs, TCP_time_connect, 0, .timeout, eax
        pop     ebx
        mov     [ebx + TCP_SOCKET.timer_connect], eax
        mov     eax, ebx

  .loop:
        cmp     [eax + SOCKET.errorcode], 0
        jne     .fail
        cmp     [eax + TCP_SOCKET.t_state], TCPS_ESTABLISHED
        je      .established

        call    SOCKET_block
        jmp     .loop

  .timeout:
        mov     eax, [esp+4]
        mov     [eax + SOCKET.errorcode], ETIMEDOUT
        and     [eax + SOCKET.state], not SS_ISCONNECTING
        call    SOCKET_notify
        ret     4

  .fail:
        mov     ebx, [eax + SOCKET.errorcode]
        mov     [eax + SOCKET.errorcode], 0                     ; Clear the error, we only need to send it to the caller once
        xor     eax, eax
        dec     eax
        ret

  .established:
        stdcall cancel_timer_hs, [eax + TCP_SOCKET.timer_connect]

        xor     eax, eax
        ret