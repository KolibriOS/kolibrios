;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2012. All rights reserved.    ;;
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

        DEBUGF  1,"TCP_usrclosed: %x\n", eax

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
; TCP_disconnect
;
;  IN:  eax = socket ptr
;  OUT: eax = socket ptr
;
;-------------------------
align 4
TCP_disconnect:

        DEBUGF  1,"TCP_disconnect: %x\n", eax

        cmp     [eax + TCP_SOCKET.t_state], TCPS_ESTABLISHED
        jb      TCP_close


; TODO: implement LINGER ?

        call    SOCKET_is_disconnecting
        call    TCP_usrclosed
        call    TCP_output

        ret