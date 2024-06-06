;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved.    ;;
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


;-----------------------------------------------------------------;
;                                                                 ;
; tcp_usrclosed                                                   ;
;                                                                 ;
;  IN:  eax = socket ptr                                          ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
tcp_usrclosed:

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
        call    tcp_close
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
        call    socket_is_disconnected
  .ret:
        pop     ebx
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; tcp_connect                                                     ;
;                                                                 ;
;  IN:  eax = socket ptr                                          ;
;                                                                 ;
;  OUT: eax = 0 on success                                        ;
;       eax = -1 on error                                         ;
;       ebx = error code on error                                 ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
tcp_connect:

        test    [eax + SOCKET.state], SS_ISCONNECTED
        jnz     .eisconn

        push    eax edx
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_lock

        mov     ebx, eax
        lea     eax, [ebx + STREAM_SOCKET.snd]
        call    socket_ring_create
        test    eax, eax
        jz      .nomem

        lea     eax, [ebx + STREAM_SOCKET.rcv]
        call    socket_ring_create
        test    eax, eax
        jz      .nomem
        pop     edx eax

; Fill in remote port and IP
        pushw   [edx + 2]
        pop     [eax + TCP_SOCKET.RemotePort]

        pushd   [edx + 4]
        pop     [eax + TCP_SOCKET.RemoteIP]

; Find route to host
        pusha
        push    eax
        mov     ebx, [eax + TCP_SOCKET.device]
        mov     edx, [eax + TCP_SOCKET.LocalIP]
        mov     eax, [eax + TCP_SOCKET.RemoteIP]
        call    ipv4_route
        test    eax, eax
        jz      .enoroute
        pop     eax
        mov     ebx, [net_device_list + edi]
        mov     [eax + TCP_SOCKET.device], ebx
        mov     [eax + TCP_SOCKET.LocalIP], edx
        popa

; Find a local port, if user didnt define one
        cmp     [eax + TCP_SOCKET.LocalPort], 0
        jne     @f
        call    socket_find_port
       @@:

; Compute window scaling factor
        push    ecx
        xor     ecx, ecx
        mov     ebx, TCP_max_win
  @@:
        cmp     ebx, SOCKET_BUFFER_SIZE
        ja      @f
        shl     ebx, 1
        inc     ecx
        cmp     ecx, TCP_max_winshift
        jb      @r
  @@:
        mov     [eax + TCP_SOCKET.request_r_scale], cl
        pop     ecx

        call    socket_is_connecting
        inc     [TCPS_connattempt]

        mov     [eax + TCP_SOCKET.timer_persist], 0
        mov     [eax + TCP_SOCKET.t_state], TCPS_SYN_SENT

        mov     [eax + TCP_SOCKET.timer_keepalive], TCP_time_keep_init

        push    [TCP_sequence_num]
        add     [TCP_sequence_num], TCP_ISSINCR/2
        pop     [eax + TCP_SOCKET.ISS]

        tcp_sendseqinit eax

        push    eax
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_unlock
        pop     eax

; Now send the SYN packet to remote end
        push    eax
        call    tcp_output
        pop     eax

        test    [eax + SOCKET.options], SO_NONBLOCK
        jz      .waitforit

        xor     eax, eax
        dec     eax
        mov     ebx, EINPROGRESS
        ret

  .nomem:
        pop     edx eax
        xor     eax, eax
        dec     eax
        mov     ebx, ENOMEM
        ret

  .eisconn:
        xor     eax, eax
        dec     eax
        mov     ebx, EISCONN
        ret

  .enoroute:
        pop     eax
        popa
        xor     eax, eax
        dec     eax
        mov     ebx, EADDRNOTAVAIL
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

        call    socket_block
        jmp     .loop

  .timeout:
        mov     eax, [esp+4]
        mov     [eax + SOCKET.errorcode], ETIMEDOUT
        and     [eax + SOCKET.state], not SS_ISCONNECTING
        call    socket_notify
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
