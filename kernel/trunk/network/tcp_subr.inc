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


align 4
iglobal
        TCP_backoff     db 0,1,2,3,4,5,6,6,6,6,6,6,6
endg

macro   tcp_checksum IP1, IP2 {

;-------------
; Pseudoheader

        ; protocol type
        mov     edx, IP_PROTO_TCP

        ; source address
        add     dl, byte [IP1+1]
        adc     dh, byte [IP1+0]
        adc     dl, byte [IP1+3]
        adc     dh, byte [IP1+2]

        ; destination address
        adc     dl, byte [IP2+1]
        adc     dh, byte [IP2+0]
        adc     dl, byte [IP2+3]
        adc     dh, byte [IP2+2]

        ; size
        adc     dl, cl
        adc     dh, ch

        adc     edx, 0

;---------------------
; Real header and data

        push    esi
        call    checksum_1
        call    checksum_2
        pop     esi

}       ; returns in dx only




macro   tcp_sendseqinit ptr {

        push    edi                     ;;;; FIXME: I don't like this static use of edi
        mov     edi, [ptr + TCP_SOCKET.ISS]
        mov     [ptr + TCP_SOCKET.SND_UP], edi
        mov     [ptr + TCP_SOCKET.SND_MAX], edi
        mov     [ptr + TCP_SOCKET.SND_NXT], edi
        mov     [ptr + TCP_SOCKET.SND_UNA], edi
        pop     edi

}



macro   tcp_rcvseqinit ptr {

        push    edi
        mov     edi, [ptr + TCP_SOCKET.IRS]
        inc     edi                             ; SYN occupies a sequence number
        mov     [ptr + TCP_SOCKET.RCV_NXT], edi
        mov     [ptr + TCP_SOCKET.RCV_ADV], edi
        pop     edi

}



macro   tcp_init_socket socket {

; new tcp control block

        mov     [socket + TCP_SOCKET.t_maxseg], TCP_mss_default
        mov     [socket + TCP_SOCKET.t_flags], TF_REQ_SCALE or TF_REQ_TSTMP

        mov     [socket + TCP_SOCKET.t_srtt], TCP_time_srtt_default
        mov     [socket + TCP_SOCKET.t_rttvar], TCP_time_rtt_default * 4
        mov     [socket + TCP_SOCKET.t_rttmin], TCP_time_re_min
;;; TODO: TCP_time_rangeset

        mov     [socket + TCP_SOCKET.SND_CWND], TCP_max_win shl TCP_max_winshift
        mov     [socket + TCP_SOCKET.SND_SSTHRESH], TCP_max_win shl TCP_max_winshift

        mov     [socket + TCP_SOCKET.RCV_SCALE], 0
        mov     [socket + TCP_SOCKET.SND_SCALE], 0

}


;-----------------------------------------------------------------;
;                                                                 ;
; tcp_pull_out_of_band                                            ;
;                                                                 ;
;  IN:  eax = ?                                                   ;
;       ebx = socket ptr                                          ;
;       edx = tcp packet ptr                                      ;
;                                                                 ;
; OUT:  /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
tcp_pull_out_of_band:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "tcp_pull_out_of_band\n"

        ;;;; 1282-1305

        ret



;-----------------------------------------------------------------;
;                                                                 ;
; tcp_drop                                                        ;
;                                                                 ;
;  IN:  eax = socket ptr                                          ;
;       ebx = error number                                        ;
;                                                                 ;
;  OUT: eax = socket ptr                                          ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
tcp_drop:

;;; TODO: check if error code is "Connection timed out' and handle accordingly

        DEBUGF  DEBUG_NETWORK_VERBOSE, "tcp_drop: %x\n", eax

        cmp     [eax + TCP_SOCKET.t_state], TCPS_SYN_RECEIVED
        jb      .no_syn_received

        mov     [eax + TCP_SOCKET.t_state], TCPS_CLOSED

        push    eax
        call    tcp_output
        pop     eax

        inc     [TCPS_drops]

        mov     [eax + SOCKET.errorcode], ebx
        jmp     tcp_close

  .no_syn_received:
        inc     [TCPS_conndrops]

        mov     [eax + SOCKET.errorcode], ebx
        jmp     tcp_close


;-----------------------------------------------------------------;
;                                                                 ;
; tcp_disconnect                                                  ;
;                                                                 ;
;  IN:  eax = socket ptr                                          ;
;                                                                 ;
;  OUT: eax = socket ptr / 0                                      ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
tcp_disconnect:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_disconnect: %x\n", eax

        cmp     [eax + TCP_SOCKET.t_state], TCPS_ESTABLISHED
        jb      tcp_close       ; Connection not yet synchronized, just get rid of the socket

        test    [eax + SOCKET.options], SO_LINGER
        jz      .nolinger

; TODO: implement LINGER
;        cmp     [eax + SOCKET.so_linger], 0
;        je     TCP_drop

  .nolinger:
        call    socket_is_disconnecting

        push    eax
        add     eax, STREAM_SOCKET.rcv
        mov     ecx, [eax + RING_BUFFER.size]
        call    socket_ring_free
        pop     eax

        call    tcp_usrclosed

        test    eax, eax
        jz      @f
        push    eax
        call    tcp_output
        pop     eax
  @@:
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; tcp_close                                                       ;
;                                                                 ;
;  IN:  eax = socket ptr                                          ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
tcp_close:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_close: %x\n", eax

;;; TODO: update RTT and mean deviation
;;; TODO: update slow start threshold

        call    socket_is_disconnected
        call    socket_free

        inc     [TCPS_closed]

        xor     eax, eax
        ret



;-----------------------------------------------------------------;
;                                                                 ;
; tcp_outflags                                                    ;
;                                                                 ;
;  IN:  eax = socket ptr                                          ;
;                                                                 ;
;  OUT: edx = flags                                               ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
tcp_outflags:

        mov     edx, [eax + TCP_SOCKET.t_state]
        movzx   edx, byte[edx + .flaglist]

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_outflags: socket=%x flags=%x\n", eax, dl

        ret

  .flaglist:
        db      TH_RST + TH_ACK         ; TCPS_CLOSED
        db      0                       ; TCPS_LISTEN
        db      TH_SYN                  ; TCPS_SYN_SENT
        db      TH_SYN + TH_ACK         ; TCPS_SYN_RECEIVED
        db               TH_ACK         ; TCPS_ESTABLISHED
        db               TH_ACK         ; TCPS_CLOSE_WAIT
        db      TH_FIN + TH_ACK         ; TCPS_FIN_WAIT_1
        db      TH_FIN + TH_ACK         ; TCPS_CLOSING
        db      TH_FIN + TH_ACK         ; TCPS_LAST_ACK
        db               TH_ACK         ; TCPS_FIN_WAIT_2
        db               TH_ACK         ; TCPS_TIME_WAIT


;-----------------------------------------------------------------;
;                                                                 ;
; TCP_respond: Fast way to send an ACK/RST/keepalive segment.     ;
;                                                                 ;
;  IN:  ebx = socket ptr                                          ;
;        cl = flags                                               ;
;                                                                 ;
; OUT:  /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
tcp_respond:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_respond_socket: socket=%x flags=%x\n", ebx, cl

;---------------------
; Create the IP packet

        push    cx ebx
        mov     edx, [ebx + IP_SOCKET.LocalIP]
        mov     edi, [ebx + IP_SOCKET.RemoteIP]
        mov     al, [ebx + IP_SOCKET.ttl]
        mov     ah, IP_PROTO_TCP
        mov     ecx, sizeof.TCP_header
        mov     ebx, [ebx + IP_SOCKET.device]
        call    ipv4_output
        jz      .error
        pop     esi cx
        push    eax

;-----------------------------------------------
; Fill in the TCP header by using the socket ptr

        mov     ax, [esi + TCP_SOCKET.LocalPort]
        stosw
        mov     ax, [esi + TCP_SOCKET.RemotePort]
        stosw
        mov     eax, [esi + TCP_SOCKET.SND_NXT]
        bswap   eax
        stosd
        mov     eax, [esi + TCP_SOCKET.RCV_NXT]
        bswap   eax
        stosd
        mov     al, 0x50        ; Dataoffset: 20 bytes (TCP_header.DataOffset)
        stosb
        mov     al, cl
        stosb
        mov     eax, SOCKET_BUFFER_SIZE
        sub     eax, [esi + STREAM_SOCKET.rcv.size]
        cmp     eax, TCP_max_win
        jbe     .lessthanmax
        mov     eax, TCP_max_win
  .lessthanmax:
        mov     cl, [esi + TCP_SOCKET.RCV_SCALE]
        shr     eax, cl

        xchg    al, ah
        stosw                   ; window
        xor     eax, eax
        stosd                   ; checksum + urgentpointer

;---------------------
; Fill in the checksum

  .checksum:
        sub     edi, sizeof.TCP_header
        mov     ecx, sizeof.TCP_header
        xchg    esi, edi
        tcp_checksum (edi + IP_SOCKET.LocalIP), (edi + IP_SOCKET.RemoteIP)
        mov     [esi+TCP_header.Checksum], dx

;--------------------
; And send the segment

        call    [ebx + NET_DEVICE.transmit]
        test    eax, eax
        jnz     @f
        call    net_ptr_to_num4
        inc     [TCP_segments_tx + edi]
       @@:
        ret

  .error:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_respond_socket: failed\n"
        add     esp, 2 + 4

        ret


;-----------------------------------------------------------------;
;                                                                 ;
; tcp_respond_segment                                             ;
;                                                                 ;
;  IN:  ebx = device ptr                                          ;
;       edx = segment ptr (a previously received segment)         ;
;       edi = ptr to IPv4 header                                  ;
;        cl = flags                                               ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
tcp_respond_segment:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_respond_segment: frame=%x flags=%x\n", edx, cl

;---------------------
; Create the IP packet

        push    cx edx
        mov     edx, [edi + IPv4_header.DestinationAddress]
        mov     edi, [edi + IPv4_header.SourceAddress]
        mov     ecx, sizeof.TCP_header
        mov     ax, IP_PROTO_TCP shl 8 + 128
        call    ipv4_output
        jz      .error
        pop     esi cx

        push    eax

;---------------------------------------------------
; Fill in the TCP header by using a received segment

        mov     ax, [esi + TCP_header.DestinationPort]
        stosw
        mov     ax, [esi + TCP_header.SourcePort]
        stosw
        mov     eax, [esi + TCP_header.AckNumber]
        bswap   eax
        stosd
        xor     eax, eax
        stosd
        mov     al, 0x50        ; Dataoffset: 20 bytes (sizeof.TCP_header/4 shl 4)
        stosb
        mov     al, cl
        stosb
        mov     ax, 1280
        rol     ax, 8
        stosw                   ; window
        xor     eax, eax
        stosd                   ; checksum + urgentpointer

;---------------------
; Fill in the checksum

        lea     esi, [edi - sizeof.TCP_header]
        mov     ecx, sizeof.TCP_header
        tcp_checksum (esi - sizeof.IPv4_header + IPv4_header.DestinationAddress),\      ; FIXME
                     (esi - sizeof.IPv4_header + IPv4_header.SourceAddress)
        mov     [esi + TCP_header.Checksum], dx

;--------------------
; And send the segment

        call    [ebx + NET_DEVICE.transmit]
        test    eax, eax
        jnz     @f
        call    net_ptr_to_num4
        inc     [TCP_segments_tx + edi]
       @@:
        ret

  .error:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_respond_segment: failed\n"
        add     esp, 2+4

        ret


macro   tcpt_rangeset   timer, value, min, max {

local   .min
local   .max
local   .done

        cmp     value, min
        jb      .min
        cmp     value, max
        ja      .max
        mov     timer, value
        jmp     .done
  .min:
        mov     timer, min
        jmp     .done
  .max:
        mov     timer, max
  .done:
}

;-----------------------------------------------------------------;
;                                                                 ;
; tcp_set_persist                                                 ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
tcp_set_persist:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_set_persist\n"

; First, check if retransmit timer is not set, retransmit and persist are mutually exclusive

        test    [eax + TCP_SOCKET.timer_flags], timer_flag_retransmission
        jnz     .exit

; calculate RTO
        push    ebx
        mov     ebx, [eax + TCP_SOCKET.t_srtt]
        shr     ebx, 2
        add     ebx, [eax + TCP_SOCKET.t_rttvar]
        shr     ebx, 1

        mov     cl, [eax + TCP_SOCKET.t_rxtshift]
        shl     ebx, cl

; Start/restart persistance timer.

        tcpt_rangeset [eax + TCP_SOCKET.timer_persist], ebx, TCP_time_pers_min, TCP_time_pers_max
        or      [ebx + TCP_SOCKET.timer_flags], timer_flag_persist
        pop     ebx

        cmp     [eax + TCP_SOCKET.t_rxtshift], TCP_max_rxtshift
        jae     @f
        inc     [eax + TCP_SOCKET.t_rxtshift]
      @@:
  .exit:

        ret



;-----------------------------------------------------------------;
;                                                                 ;
; tcp_xmit_timer: Calculate new smoothed RTT.                     ;
;                                                                 ;
;   IN: eax = rtt                                                 ;
;       ebx = socket ptr                                          ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
tcp_xmit_timer:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_xmit_timer: socket=0x%x rtt=%d0ms\n", ebx, eax

        inc     [TCPS_rttupdated]

        cmp     [ebx + TCP_SOCKET.t_rtt], 0
        je      .no_rtt_yet

; srtt is stored as a fixed point with 3 bits after the binary point.
; The following magic is equivalent of the smoothing algorithm in rfc793 with an alpha of .875
; (srtt = rtt/8 + srtt*7/8 in fixed point)
; Adjust rtt to origin 0.

        push    ecx
        mov     ecx, [ebx + TCP_SOCKET.t_srtt]
        shr     ecx, TCP_RTT_SHIFT
        sub     eax, ecx
        dec     eax
        pop     ecx

        add     [ebx + TCP_SOCKET.t_srtt], eax
        ja      @f
        mov     [ebx + TCP_SOCKET.t_srtt], 1
  @@:

; We accumulate a smoothed rtt variance (actually, a smoothed mean difference),
; then set the retransmit timer to smoothed rtt + 4 times the smoothed variance.
; rttvar is stored as fixed point with 2 bits after the binary point.
; The following is equivalent to rfc793 smoothing with an alpha of .75
; (rttvar = rttvar*3/4 + delta/4) (delta = eax)

; get abs(eax)
        push    edx
        cdq
        xor     eax, edx
        sub     eax, edx

        mov     edx, [ebx + TCP_SOCKET.t_rttvar]
        shr     edx, TCP_RTTVAR_SHIFT
        sub     eax, edx
        pop     edx

        add     [ebx + TCP_SOCKET.t_rttvar], eax
        ja      @f
        mov     [ebx + TCP_SOCKET.t_rttvar], 1
  @@:
        ret


  .no_rtt_yet:
        push    ecx
        mov     ecx, eax
        shl     ecx, TCP_RTT_SHIFT
        mov     [ebx + TCP_SOCKET.t_srtt], ecx

        shl     eax, TCP_RTTVAR_SHIFT - 1
        mov     [ebx + TCP_SOCKET.t_rttvar], eax
        pop     ecx

        ret


;-----------------------------------------------------------------;
;                                                                 ;
; tcp_mss: Update maximum segment size                            ;
;                                                                 ;
;  IN:  eax = max segment size                                    ;
;       ebx = socket ptr                                          ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
tcp_mss:

        cmp     eax, 1420       ; FIXME
        jbe     @f
        mov     eax, 1420
  @@:
        mov     [ebx + TCP_SOCKET.t_maxseg], eax


        ret



;-----------------------------------------------------------------;
;                                                                 ;
; tcp_reassemble                                                  ;
;                                                                 ;
;   IN: ebx = socket ptr                                          ;
;       edx = segment ptr                                         ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
tcp_reassemble:

        ;;;;; TODO

        ret

