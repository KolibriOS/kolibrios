;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2012. All rights reserved.    ;;
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

;-----------------------------------------------------------------
;
; TCP_input:
;
;  IN:  [esp] = ptr to buffer
;       [esp+4] = buffer size
;       ebx = ptr to device struct
;       ecx = segment size
;       esi = ptr to TCP segment
;       edi = ptr to ipv4 source address, followed by ipv4 dest address
;
;  OUT: /
;
;-----------------------------------------------------------------

align 4
TCP_input:

        DEBUGF  1,"TCP_input size=%u\n", ecx

; First, re-calculate the checksum

        push    ecx esi
        pushw   [esi + TCP_header.Checksum]
        mov     [esi + TCP_header.Checksum], 0
        TCP_checksum (edi), (edi+4)
        pop     cx              ; previous checksum
        cmp     cx, dx
        pop     edx ecx
        jne     .drop_not_locked

        DEBUGF  1,"Checksum ok\n"

        and     [edx + TCP_header.DataOffset], 0xf0                     ; Calculate TCP segment header size (throwing away unused reserved bits in TCP header)
        shr     [edx + TCP_header.DataOffset], 2
        cmp     [edx + TCP_header.DataOffset], sizeof.TCP_header        ; Now see if it's at least the size of a standard TCP header
        jb      .drop_not_locked                                        ; If not, drop the packet

        movzx   eax, [edx + TCP_header.DataOffset]
        sub     ecx, eax                                                ; substract TCP header size from total segment size
        jb      .drop_not_locked                                        ; If total segment size is less then the advertised header size, drop packet
        DEBUGF  1,"we got %u bytes of data\n", ecx

;-----------------------------------------------------------------------------------------
; Check if this packet has a timestamp option (We do it here so we can process it quickly)

        cmp     eax, sizeof.TCP_header + 12                             ; Timestamp option is 12 bytes
        jb      .no_timestamp
        je      .is_ok

        cmp     byte [edx + sizeof.TCP_header + 12], TCP_OPT_EOL                ; end of option list
        jne     .no_timestamp

  .is_ok:
        test    [edx + TCP_header.Flags], TH_SYN                                ; SYN flag must not be set
        jnz     .no_timestamp

        cmp     dword [edx + sizeof.TCP_header], 0x0101080a                     ; Timestamp header
        jne     .no_timestamp

        DEBUGF  1,"timestamp ok\n"

        ; TODO: Parse the option
        ; TODO: Set a Bit in the TCP to tell all options are parsed

  .no_timestamp:

;-------------------------------------------
; Convert Big-endian values to little endian

        ntohd   [edx + TCP_header.SequenceNumber]
        ntohd   [edx + TCP_header.AckNumber]

        ntohw   [edx + TCP_header.Window]
        ntohw   [edx + TCP_header.UrgentPointer]
        ntohw   [edx + TCP_header.SourcePort]
        ntohw   [edx + TCP_header.DestinationPort]

;------------------------------------------------------------
; Next thing to do is find the TCPS (thus, the socket pointer)

; IP Packet TCP Destination Port = local Port
; (IP Packet SenderAddress = Remote IP)  OR  (Remote IP = 0)
; (IP Packet TCP Source Port = remote Port) OR (remote Port = 0)

        mov     ebx, net_sockets
        mov     si, [edx + TCP_header.DestinationPort]

  .socket_loop:
        mov     ebx, [ebx + SOCKET.NextPtr]
        or      ebx, ebx
        jz      .drop_with_reset_not_locked

        cmp     [ebx + SOCKET.Domain], AF_INET4
        jne     .socket_loop

        cmp     [ebx + SOCKET.Protocol], IP_PROTO_TCP
        jne     .socket_loop

        cmp     [ebx + TCP_SOCKET.LocalPort], si
        jne     .socket_loop

        mov     eax, [ebx + IP_SOCKET.RemoteIP]
        cmp     eax, [edi]                              ; Ipv4 source addres
        je      @f
        test    eax, eax
        jnz     .socket_loop
       @@:

        mov     ax, [ebx + TCP_SOCKET.RemotePort]
        cmp     [edx + TCP_header.SourcePort], ax
        je      .found_socket
        test    ax, ax
        jnz     .socket_loop
  .found_socket:                                        ; ebx now contains the socketpointer
        DEBUGF  1,"Socket ptr: %x\n", ebx


;----------------------------
; Check if socket isnt closed

        cmp     [ebx + TCP_SOCKET.t_state], TCPS_CLOSED
        je      .drop_not_locked

;----------------
; Lock the socket

        pusha
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_lock
        popa

        DEBUGF  1,"Socket locked\n"

;---------------------------------------
; unscale the window into a 32 bit value

        movzx   eax, [edx + TCP_header.Window]
        push    ecx
        mov     cl, [ebx + TCP_SOCKET.SND_SCALE]
        shl     eax, cl
        mov     dword [edx + TCP_header.Window], eax    ; word after window is checksum, we dont need checksum anymore
        pop     ecx

;-----------------------------------
; Is this socket a listening socket?

        test    [ebx + SOCKET.options], SO_ACCEPTCON
        jz      .no_listening_socket

        DEBUGF  1,"Accepting new connection\n"

        pusha
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_unlock
        popa

        push    ecx edx esi edi         ;;;
        call    SOCKET_fork
        pop     edi esi edx ecx

        test    eax, eax
        jz      .drop

        push    dword [edi + 4]                         ; Ipv4 destination addres
        pop     [eax + IP_SOCKET.LocalIP]

        push    [edx + TCP_header.DestinationPort]
        pop     [eax + TCP_SOCKET.LocalPort]

        mov     [eax + TCP_SOCKET.t_state], TCPS_LISTEN

;        mov     [ebx + SOCKET.lock], 0
        mov     ebx, eax

        jmp     .LISTEN

  .no_listening_socket:

;-------------------------------------
; Reset idle timer and keepalive timer

        mov     [ebx + TCP_SOCKET.t_idle], 0
        mov     [ebx + TCP_SOCKET.timer_keepalive], TCP_time_keep_interval

;--------------------
; Process TCP options

        movzx   eax, [edx + TCP_header.DataOffset]
        cmp     eax, sizeof.TCP_header                  ; Does header contain any options?
        je      .no_options

        DEBUGF  1,"Segment has options\n"

        cmp     [ebx + TCP_SOCKET.t_state], TCPS_LISTEN ; no options when in listen state
        jz      .not_uni_xfer                           ; also no header prediction

        add     eax, edx
        lea     esi, [edx + sizeof.TCP_header]

  .opt_loop:
        cmp     esi, eax                        ; are we scanning outside of header?
        jae     .no_options

        cmp     byte [esi], TCP_OPT_EOL         ; end of option list?
        jz      .no_options

        cmp     byte [esi], TCP_OPT_NOP         ; nop ?
        jz      .opt_nop

        cmp     byte [esi], TCP_OPT_MAXSEG
        je      .opt_maxseg

        cmp     byte [esi], TCP_OPT_WINDOW
        je      .opt_window

        cmp     byte [esi], TCP_OPT_TIMESTAMP
        je      .opt_timestamp

        jmp     .no_options     ; If we reach here, some unknown options were received, skip them all!

  .opt_nop:
        inc     edi
        jmp     .opt_loop

  .opt_maxseg:
        cmp     byte [esi+1], 4
        jne     .no_options             ; error occured, ignore all options!

        test    [edx + TCP_header.Flags], TH_SYN
        jz      @f

        movzx   eax, word[esi+2]
        rol     ax, 8
        DEBUGF  1,"Maxseg: %u\n", ax

        mov     [ebx + TCP_SOCKET.t_maxseg], eax

       @@:
        add     edi, 4
        jmp     .opt_loop


  .opt_window:
        cmp     byte [esi+1], 3
        jne     .no_options

        test    [edx + TCP_header.Flags], TH_SYN
        jz      @f

        DEBUGF  1,"Got window option\n"

        ;;;;;
       @@:
        add     edi, 3
        jmp     .opt_loop


  .opt_timestamp:
        cmp     byte [esi+1], 10
        jne     .no_options

        DEBUGF  1,"Got timestamp option\n"

        ;;;;;

        add     esi, 10
        jmp     .opt_loop

  .no_options:






;-----------------------------------------------------------------------
; Time to do some header prediction (Original Principle by Van Jacobson)

; There are two common cases for an uni-directional data transfer.
;
; General rule: the packets has no control flags, is in-sequence,
;   window width didnt change and we're not retransmitting.
;
; Second rules:
;  -  If the length is 0 and the ACK moved forward, we're the sender side of the transfer.
;      In this case we'll free the ACK'ed data and notify higher levels that we have free space in buffer
;
;  -  If the length is not 0 and the ACK didn't move, we're the receiver side of the transfer.
;      If the packets are in order (data queue is empty), add the data to the socket buffer and request a delayed ACK

        cmp     [ebx + TCP_SOCKET.t_state], TCPS_ESTABLISHED
        jnz     .not_uni_xfer

        test    [edx + TCP_header.Flags], TH_SYN + TH_FIN + TH_RST + TH_URG
        jnz     .not_uni_xfer

        test    [edx + TCP_header.Flags], TH_ACK
        jz      .not_uni_xfer

        mov     eax, [edx + TCP_header.SequenceNumber]
        cmp     eax, [ebx + TCP_SOCKET.RCV_NXT]
        jne     .not_uni_xfer

        mov     eax, dword [edx + TCP_header.Window]
        cmp     eax, [ebx + TCP_SOCKET.SND_WND]
        jne     .not_uni_xfer

        mov     eax, [ebx + TCP_SOCKET.SND_NXT]
        cmp     eax, [ebx + TCP_SOCKET.SND_MAX]
        jne     .not_uni_xfer

;---------------------------------------
; check if we are sender in the uni-xfer

; If the following 4 conditions are all true, this segment is a pure ACK.
;
; - The segment contains no data.
        test    ecx, ecx
        jnz     .not_sender

; - The congestion window is greater than or equal to the current send window.
;     This test is true only if the window is fully open, that is, the connection is not in the middle of slow start or congestion avoidance.
        mov     eax, [ebx + TCP_SOCKET.SND_CWND]
        cmp     eax, [ebx + TCP_SOCKET.SND_WND]
        jb      .not_uni_xfer

; - The acknowledgment field in the segment is less than or equal to the maximum sequence number sent.
        mov     eax, [edx + TCP_header.AckNumber]
        cmp     eax, [ebx + TCP_SOCKET.SND_MAX]
        ja      .not_uni_xfer

; - The acknowledgment field in the segment is greater than the largest unacknowledged sequence number.
        sub     eax, [ebx + TCP_SOCKET.SND_UNA]
        jbe     .not_uni_xfer

        DEBUGF  1,"Header prediction: we are sender\n"

;---------------------------------
; Packet is a pure ACK, process it

; Update RTT estimators

;;; TODO

; Delete acknowledged bytes from send buffer
        pusha
        mov     ecx, eax
        lea     eax, [ebx + STREAM_SOCKET.snd]
        call    SOCKET_ring_free
        popa

; update window pointers
        mov     eax, [edx + TCP_header.AckNumber]
        mov     [ebx + TCP_SOCKET.SND_UNA], eax

; Stop retransmit timer
        mov     [ebx + TCP_SOCKET.timer_retransmission], 0

; Awaken waiting processes
        pusha
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_unlock
        popa

        mov     eax, ebx
        call    SOCKET_notify_owner

; Generate more output
        call    TCP_output

        jmp     .drop_not_locked

;-------------------------------------------------
; maybe we are the receiver in the uni-xfer then..

  .not_sender:
; - The amount of data in the segment is greater than 0 (data count is in ecx)

; - The acknowledgment field equals the largest unacknowledged sequence number. This means no data is acknowledged by this segment.
        mov     eax, [edx + TCP_header.AckNumber]
        cmp     eax, [ebx + TCP_SOCKET.SND_UNA]
        jne     .not_uni_xfer

; - The reassembly list of out-of-order segments for the connection is empty (seg_next equals tp).

;;; TODO

        jnz     .not_uni_xfer

; Complete processing of received data

        DEBUGF  1,"header prediction: we are receiver\nreceiving %u bytes of data\n", ecx

        add     [ebx + TCP_SOCKET.RCV_NXT], ecx         ; Update sequence number with number of bytes we have copied

        movzx   esi, [edx + TCP_header.DataOffset]
        add     esi, edx
        lea     eax, [ebx + STREAM_SOCKET.rcv]
        call    SOCKET_ring_write                       ; Add the data to the socket buffer

        mov     eax, ebx
        call    SOCKET_notify_owner

        or      [ebx + TCP_SOCKET.t_flags], TF_DELACK   ; Set delayed ack flag

        jmp     .drop






;--------------------------------------------------
; Header prediction failed, do it the slow way

  .not_uni_xfer:

        DEBUGF  1,"Header prediction failed\n"

; Calculate receive window size

;        mov     eax, [ebx + STREAM_SOCKET.rcv.size]
;        neg     eax
;        add     eax, SOCKETBUFFSIZE
;        mov     edx, [ebx + TCP_SOCKET.RCV_ADV]
;        sub     edx, [ebx + TCP_SOCKET.RCV_NXT]
;        cmp     eax, edx
;        jae     @f
;        mov     eax, edx
;       @@:

        cmp     [ebx + TCP_SOCKET.t_state], TCPS_LISTEN
        je      .LISTEN

        cmp     [ebx + TCP_SOCKET.t_state], TCPS_SYN_SENT
        je      .SYN_SENT

        jmp     .NOT_LISTEN_OR_SYN_SENT



;-------------
; Passive Open

align 4
.LISTEN:

        DEBUGF  1,"TCP state: listen\n"

        test    [edx + TCP_header.Flags], TH_RST        ;;; TODO: kill new socket on error
        jnz     .drop

        test    [edx + TCP_header.Flags], TH_ACK
        jnz     .drop_with_reset

        test    [edx + TCP_header.Flags], TH_SYN
        jz      .drop

;;; TODO: check if it's a broadcast or multicast, and drop if so

        push    dword [edi + 4]                         ; Ipv4 destination addres
        pop     [ebx + IP_SOCKET.RemoteIP]

        push    [edx + TCP_header.SourcePort]
        pop     [ebx + TCP_SOCKET.RemotePort]

        push    [edx + TCP_header.SequenceNumber]
        pop     [ebx + TCP_SOCKET.IRS]

        push    [TCP_sequence_num]                      ;;;;;
        add     [TCP_sequence_num], 64000 / 2
        pop     [ebx + TCP_SOCKET.ISS]

        push    [ebx + TCP_SOCKET.ISS]
        pop     [ebx + TCP_SOCKET.SND_NXT]

        TCP_sendseqinit ebx
        TCP_rcvseqinit ebx

        mov     [ebx + TCP_SOCKET.t_state], TCPS_SYN_RECEIVED
        mov     [ebx + TCP_SOCKET.t_flags], TF_ACKNOW
        mov     [ebx + TCP_SOCKET.timer_keepalive], TCP_time_keep_interval  ;;;; macro

        lea     eax, [ebx + STREAM_SOCKET.snd]
        call    SOCKET_ring_create

        lea     eax, [ebx + STREAM_SOCKET.rcv]
        call    SOCKET_ring_create

;;;        call    SOCKET_notify_owner

        jmp     .trim_then_step6
                                   









;------------
; Active Open

align 4
.SYN_SENT:

        DEBUGF  1,"TCP state: syn_sent\n"

        test    [edx + TCP_header.Flags], TH_ACK
        jz      @f

        mov     eax, [edx + TCP_header.AckNumber]
        cmp     eax, [ebx + TCP_SOCKET.ISS]
        jbe     .drop_with_reset

        cmp     eax, [ebx + TCP_SOCKET.SND_MAX]
        ja      .drop_with_reset
       @@:

        test    [edx + TCP_header.Flags], TH_RST
        jz      @f

        test    [edx + TCP_header.Flags], TH_ACK
        jz      .drop

        mov     eax, ebx
        mov     ebx, ECONNREFUSED
        call    TCP_drop

        jmp     .drop
       @@:

        test    [edx + TCP_header.Flags], TH_SYN
        jz      .drop

; at this point, segment seems to be valid

        test    [edx + TCP_header.Flags], TH_ACK
        jz      .no_syn_ack

; now, process received SYN in response to an active open

        mov     eax, [edx + TCP_header.AckNumber]
        mov     [ebx + TCP_SOCKET.SND_UNA], eax
        cmp     eax, [ebx + TCP_SOCKET.SND_NXT]
        jbe     @f
        mov     [ebx + TCP_SOCKET.SND_NXT], eax
       @@:

  .no_syn_ack:

        mov     [ebx + TCP_SOCKET.timer_retransmission], 0      ; disable retransmission

        push    [edx + TCP_header.SequenceNumber]
        pop     [ebx + TCP_SOCKET.IRS]

        TCP_rcvseqinit ebx

        or      [ebx + TCP_SOCKET.t_flags], TF_ACKNOW

        mov     eax, [ebx + TCP_SOCKET.SND_UNA]
        cmp     eax, [ebx + TCP_SOCKET.ISS]
        jbe     .simultaneous_open

        test    [edx + TCP_header.Flags], TH_ACK
        jz      .simultaneous_open

        DEBUGF  1,"TCP: active open\n"

;;; TODO: update stats

; set socket state to connected

        mov     [ebx + SOCKET.state],1  ;;;; FIXME

        mov     [ebx + TCP_SOCKET.t_state], TCPS_ESTABLISHED

;;; TODO: check if we should scale the connection (567-572)
;;; TODO: update RTT estimators

        jmp     .trim_then_step6

  .simultaneous_open:

        DEBUGF  1,"TCP: simultaneous open\n"
; We have received a syn but no ACK, so we are having a simultaneous open..
        mov     [ebx + TCP_SOCKET.t_state], TCPS_SYN_RECEIVED







;-------------------------------------
; Common processing for receipt of SYN

  .trim_then_step6:

        inc     [edx + TCP_header.SequenceNumber]

;;; TODO: Drop any received data that follows receive window (590)

        mov     eax, [edx + TCP_header.SequenceNumber]
        mov     [ebx + TCP_SOCKET.RCV_UP], eax
        dec     eax
        mov     [ebx + TCP_SOCKET.SND_WL1], eax

        jmp     .ack_processed








  .NOT_LISTEN_OR_SYN_SENT:

        DEBUGF  1,"Slow TCP input: not listen or syn_sent state\n"

;--------------------------------------------
; Protection Against Wrapped Sequence Numbers

; First, check if timestamp is present

;;;; TODO 602

; Then, check if at least some bytes of data are within window

;;;; TODO








;----------------------------
; trim any data not in window

; check for duplicate data at beginning of segment

        mov     eax, [ebx + TCP_SOCKET.RCV_NXT]
        sub     eax, [edx + TCP_header.SequenceNumber]
        jbe     .no_duplicate

        DEBUGF  1,"Uh oh.. %u bytes of duplicate data!\n", eax

        test    [edx + TCP_header.Flags], TH_SYN
        jz      .no_dup_syn

; remove duplicate syn

        and     [edx + TCP_header.Flags], not (TH_SYN)
        inc     [edx + TCP_header.SequenceNumber]

        cmp     [edx + TCP_header.UrgentPointer], 1
        jbe     @f
        dec     [edx + TCP_header.UrgentPointer]
        jmp     .dup_syn
       @@:
        and     [edx + TCP_header.Flags], not (TH_URG)
  .dup_syn:
        dec     eax
  .no_dup_syn:

; eax holds number of bytes to drop

; Check for entire duplicate packet

        cmp     eax, ecx
        jae     .duplicate

        DEBUGF  1,"Going to drop %u out of %u bytes\n", eax, ecx

;;; TODO: apply figure 28.30

; Check for duplicate FIN

        test    [edx + TCP_header.Flags], TH_FIN
        jz      @f
        inc     ecx
        cmp     eax, ecx
        dec     ecx
        jne     @f

        mov     eax, ecx
        and     [edx + TCP_header.Flags], not TH_FIN
        or      [ebx + TCP_SOCKET.t_flags], TF_ACKNOW
        jmp     .no_duplicate
       @@:

; Handle the case when a bound socket connects to itself
; Allow packets with a SYN and an ACKto continue with the processing

;-------------------------------------
; Generate duplicate ACK if nescessary

; This code also handles simultaneous half-open or self-connects

        test    eax, eax
        jnz     .drop_after_ack

        cmp     [edx + TCP_header.Flags], TH_ACK
        jz      .drop_after_ack

  .duplicate:

        DEBUGF  1,"Duplicate received\n"

;----------------------------------------
; Update statistics for duplicate packets

;;; TODO

        jmp     .drop_after_ack

  .no_duplicate:

;-----------------------------------------------
; Remove duplicate data and update urgent offset

        add     [edx + TCP_header.SequenceNumber], eax

;;; TODO

        sub     [edx + TCP_header.UrgentPointer], ax
        ja      @f

        and     [edx + TCP_header.Flags], not (TH_URG)
        mov     [edx + TCP_header.UrgentPointer], 0
       @@:

;--------------------------------------------------
; Handle data that arrives after process terminates

        cmp     [ebx + SOCKET.PID], 0
        ja      @f

        cmp     [ebx + TCP_SOCKET.t_state], TCPS_CLOSE_WAIT
        jbe     @f

        test    ecx, ecx
        jz      @f

;;; Close the socket
;;; update stats

        jmp     .drop_with_reset
       @@:

;----------------------------------------
; Remove data beyond right edge of window

        mov     eax, [edx + TCP_header.SequenceNumber]
        add     eax, ecx
        sub     eax, [ebx + TCP_SOCKET.RCV_NXT]
        sub     ax, [ebx + TCP_SOCKET.RCV_WND]

; eax now holds the number of bytes to drop

        jbe     .no_excess_data

;;; TODO: update stats

        cmp     eax, ecx
        jb      .dont_drop_all

;;; TODO 700-736

  .dont_drop_all:

  .no_excess_data:








;-----------------
; Record timestamp

;;; TODO 737-746





;------------------
; Process RST flags

        test    [edx + TCP_header.Flags], TH_RST
        jz      .rst_skip

        DEBUGF  1,"Got an RST flag"

        mov     eax, [ebx + TCP_SOCKET.t_state]
        shl     eax, 2
        jmp     dword [eax + .rst_sw_list]

  .rst_sw_list:
        dd      .rst_skip       ;TCPS_CLOSED
        dd      .rst_skip       ;TCPS_LISTEN
        dd      .rst_skip       ;TCPS_SYN_SENT
        dd      .econnrefused   ;TCPS_SYN_RECEIVED
        dd      .econnreset     ;TCPS_ESTABLISHED
        dd      .econnreset     ;TCPS_CLOSE_WAIT
        dd      .econnreset     ;TCPS_FIN_WAIT_1
        dd      .rst_close      ;TCPS_CLOSING
        dd      .rst_close      ;TCPS_LAST_ACK
        dd      .econnreset     ;TCPS_FIN_WAIT_2
        dd      .rst_close      ;TCPS_TIMED_WAIT

  .econnrefused:
        DEBUGF  1,"Connection refused"

        mov     [ebx + SOCKET.errorcode], ECONNREFUSED
        jmp     .close

  .econnreset:
        DEBUGF  1,"Connection reset"

        mov     [ebx + SOCKET.errorcode], ECONNRESET

    .close:
        DEBUGF  1,"Closing connection"

        mov     [ebx + TCP_SOCKET.t_state], TCPS_CLOSED
        ;;; TODO: update stats
        mov     eax, ebx
        call    TCP_close
        jmp     .drop

  .rst_close:
        DEBUGF  1,"Closing with reset\n"

        mov     eax, ebx
        call    TCP_close
        jmp     .drop







  .rst_skip:


;--------------------------------------
; handle SYN-full and ACK-less segments

        test    [edx + TCP_header.Flags], TH_SYN
        jz      @f

        mov     eax, ebx
        mov     ebx, ECONNRESET
        call    TCP_drop
        jmp     .drop_with_reset

        test    [edx + TCP_header.Flags], TH_ACK
        jz      .drop
      @@:







;---------------
; ACK processing

        cmp     [ebx + TCP_SOCKET.t_state], TCPS_SYN_RECEIVED
        jnz     .no_syn_rcv

        DEBUGF  1,"TCP state = syn received\n"

        mov     eax, [edx + TCP_header.AckNumber]
        cmp     [ebx + TCP_SOCKET.SND_UNA], eax
        ja      .drop_with_reset
        cmp     eax, [ebx + TCP_SOCKET.SND_MAX]
        ja      .drop_with_reset

;;; TODO: update stats

        mov     eax, ebx
        call    SOCKET_is_connected
        mov     [ebx + TCP_SOCKET.t_state], TCPS_ESTABLISHED

; Do window scaling?

        test    [ebx + TCP_SOCKET.t_flags], TF_RCVD_SCALE
        jz      @f
        test    [ebx + TCP_SOCKET.t_flags], TF_REQ_SCALE
        jz      @f

        push    word [ebx + TCP_SOCKET.requested_s_scale]       ; Set send and receive scale factors to the received values
        pop     word [ebx + TCP_SOCKET.SND_SCALE]
       @@:

;;; TODO: copy the data (if any) into the socket

        mov     eax, [edx + TCP_header.SequenceNumber]
        dec     eax
        mov     [ebx + TCP_SOCKET.SND_WL1], eax
        jmp     .not_dup_ack

  .no_syn_rcv:

; check for duplicate ACK

        mov     eax, [edx + TCP_header.AckNumber]
        cmp     eax, [ebx + TCP_SOCKET.SND_UNA]
        ja      .not_dup_ack

        test    ecx, ecx
        jnz     .reset_dupacks

        mov     eax, dword [edx + TCP_header.Window]
        cmp     eax, [ebx + TCP_SOCKET.SND_WND]
        jne     .reset_dupacks

        DEBUGF  1,"Processing a duplicate ACK..\n"

        cmp     [ebx + TCP_SOCKET.timer_retransmission], 10000 ;;;;  FIXME
        ja      @f

        mov     eax, [edx + TCP_header.AckNumber]
        cmp     eax, [ebx + TCP_SOCKET.SND_UNA]
        je      .dup_ack

       @@:
        mov     [ebx + TCP_SOCKET.t_dupacks], 0
        jmp     .not_dup_ack

  .dup_ack:
        inc     [ebx + TCP_SOCKET.t_dupacks]
        cmp     [ebx + TCP_SOCKET.t_dupacks], TCP_re_xmit_thresh
        jne     .no_re_xmit

        push    [ebx + TCP_SOCKET.SND_NXT]              ; >>>>

        mov     eax, [ebx + TCP_SOCKET.SND_WND]
        cmp     eax, [ebx + TCP_SOCKET.SND_CWND]
        cmovg   eax, [ebx + TCP_SOCKET.SND_CWND]
        shr     eax, 1
        push    edx
        xor     edx, edx
        div     [ebx + TCP_SOCKET.t_maxseg]
        cmp     eax, 2
        jae     @f
        mov     ax, 2
       @@:
        mul     [ebx + TCP_SOCKET.t_maxseg]
        pop     edx
        mov     [ebx + TCP_SOCKET.SND_SSTHRESH], eax

        mov     [ebx + TCP_SOCKET.timer_retransmission], 0      ; turn off retransmission timer
        mov     [ebx + TCP_SOCKET.t_rtt], 0
        mov     eax, [edx + TCP_header.AckNumber]
        mov     [ebx + TCP_SOCKET.SND_NXT], eax
        mov     eax, [ebx + TCP_SOCKET.t_maxseg]
        mov     [ebx + TCP_SOCKET.SND_CWND], eax

        mov     eax, ebx
        call    TCP_output                                      ; retransmit missing segment

        push    edx
        xor     edx, edx
        mov     eax, [ebx + TCP_SOCKET.t_maxseg]
        mul     [ebx + TCP_SOCKET.t_dupacks]
        pop     edx
        add     eax, [ebx + TCP_SOCKET.SND_SSTHRESH]
        mov     [ebx + TCP_SOCKET.SND_CWND], eax

        pop     eax                                     ; <<<<
        cmp     eax, [ebx + TCP_SOCKET.SND_NXT]
        jb      @f
        mov     [ebx + TCP_SOCKET.SND_NXT], eax
       @@:

        jmp     .drop


  .no_re_xmit:
        jbe     .not_dup_ack

        DEBUGF  1,"Increasing congestion window\n"

        mov     eax, [ebx + TCP_SOCKET.t_maxseg]
        add     [ebx + TCP_SOCKET.SND_CWND], eax

        mov     eax, ebx
        call    TCP_output

        jmp     .drop






  .not_dup_ack:

;-------------------------------------------------
; If the congestion window was inflated to account
; for the other side's cached packets, retract it

        mov     eax, [ebx + TCP_SOCKET.SND_SSTHRESH]
        cmp     eax, [ebx + TCP_SOCKET.SND_CWND]
        ja      @f
        cmp     [ebx + TCP_SOCKET.t_dupacks], TCP_re_xmit_thresh
        jbe     @f
        mov     [ebx + TCP_SOCKET.SND_CWND], eax
       @@:

        mov     [ebx + TCP_SOCKET.t_dupacks], 0

        mov     eax, [edx + TCP_header.AckNumber]
        cmp     eax, [ebx + TCP_SOCKET.SND_MAX]
        jbe     @f

        ;;; TODO: update stats
        jmp     .drop_after_ack

       @@:

        mov     edi, [edx + TCP_header.AckNumber]
        sub     edi, [ebx + TCP_SOCKET.SND_UNA]         ; now we got the number of acked bytes in edi

        ;;; TODO: update stats


        DEBUGF  1,"We have an acceptable ACK of %x bytes\n", esi






;------------------------------------------
; RTT measurements and retransmission timer

        ;;;;; 912 - 926

        mov     [ebx + TCP_SOCKET.timer_retransmission], 0

        mov     eax, [ebx + TCP_SOCKET.SND_MAX]
        cmp     eax, [edx + TCP_header.AckNumber]
        je      .all_outstanding
        mov     [ebx + TCP_SOCKET.timer_retransmission], 120 ;;;; TODO: correct this value (use a macro for it)
  .all_outstanding:







;-------------------------------------------
; Open congestion window in response to ACKs

        mov     esi, [ebx + TCP_SOCKET.SND_CWND]
        mov     eax, [ebx + TCP_SOCKET.t_maxseg]

        cmp     esi, [ebx + TCP_SOCKET.SND_SSTHRESH]
        jbe     @f
        push    edx
        push    eax
        mul     eax
        div     esi
        pop     edx
        shr     edx, 3
        add     eax, edx
        pop     edx
       @@:

        add     esi, eax

        push    ecx
        mov     cl, [ebx + TCP_SOCKET.SND_SCALE]
        mov     eax, TCP_max_win
        shl     eax, cl
        pop     ecx

        cmp     esi, eax
        cmovg   esi, eax
        mov     [ebx + TCP_SOCKET.SND_CWND], esi







;------------------------------------------
; Remove acknowledged data from send buffer

        push    ecx edx ebx
        mov     ecx, edi
        lea     eax, [ebx + STREAM_SOCKET.snd]
        call    SOCKET_ring_free
        pop     ebx
        sub     [ebx + TCP_SOCKET.SND_WND], ecx
        pop     edx ecx

; Wake up process waiting on send buffer

        mov     eax, ebx
        call    SOCKET_notify_owner

; Update TCPS

        mov     eax, [edx + TCP_header.AckNumber]
        mov     [ebx + TCP_SOCKET.SND_UNA], eax

        cmp     eax, [ebx + TCP_SOCKET.SND_NXT]
        jb      @f
        mov     [ebx + TCP_SOCKET.SND_NXT], eax
       @@:


        ;; TODO: use zero flag as 'ourfinisacked'




; General ACK handling complete
; Now do the state-specific ones

        mov     eax, [ebx + TCP_SOCKET.t_state]
        jmp     dword [eax*4 + .ACK_sw_list]

  .ACK_sw_list:
        dd      .ack_processed  ;TCPS_CLOSED
        dd      .ack_processed  ;TCPS_LISTEN
        dd      .ack_processed  ;TCPS_SYN_SENT
        dd      .ack_processed  ;TCPS_SYN_RECEIVED
        dd      .ack_processed  ;TCPS_ESTABLISHED
        dd      .ack_processed  ;TCPS_CLOSE_WAIT
        dd      .ack_fw1        ;TCPS_FIN_WAIT_1
        dd      .ack_c          ;TCPS_CLOSING
        dd      .ack_la         ;TCPS_LAST_ACK
        dd      .ack_processed  ;TCPS_FIN_WAIT_2
        dd      .ack_tw         ;TCPS_TIMED_WAIT


  .ack_fw1:
        jz      .ack_processed

        test    [ebx + SOCKET.state], SS_CANTRCVMORE
        jnz     @f
        mov     eax, ebx
        call    SOCKET_is_disconnected
;;;        mov     [ebx + TCP_SOCKET.timer_timed_wait], TCP_time_max_idle
       @@:

        mov     [ebx + TCP_SOCKET.t_state], TCPS_FIN_WAIT_2
        jmp     .ack_processed


  .ack_c:
        jz      .ack_processed

        mov     [ebx + TCP_SOCKET.t_state], TCPS_TIMED_WAIT
        mov     eax, ebx
        call    TCP_cancel_timers
        mov     [ebx + TCP_SOCKET.timer_timed_wait], 2 * TCP_time_MSL
        mov     eax, ebx
        call    SOCKET_is_disconnected
        jmp     .ack_processed


  .ack_la:
        jz      .ack_processed


        mov     eax, ebx
        call    TCP_close
        jmp     .drop


  .ack_tw:
        mov     [ebx + TCP_SOCKET.timer_timed_wait], 2 * TCP_time_MSL
        jmp     .drop_after_ack







  .reset_dupacks:               ; We got a new ACK, reset duplicate ACK counter

        mov     [ebx + TCP_SOCKET.t_dupacks], 0

  .ack_processed:       ; (step 6)

        DEBUGF  1,"ACK processed\n"

;----------------------------------------------
; check if we need to update window information

        test    [edx + TCP_header.Flags], TH_ACK
        jz      .no_window_update

        mov     eax, [ebx + TCP_SOCKET.SND_WL1]
        cmp     eax, [edx + TCP_header.SequenceNumber]
        jb      .update_window
        ja      @f

        mov     eax, [ebx + TCP_SOCKET.SND_WL2]
        cmp     eax, [edx + TCP_header.AckNumber]
        jb      .update_window
        ja      .no_window_update
       @@:

        mov     eax, [ebx + TCP_SOCKET.SND_WL2]
        cmp     eax, [edx + TCP_header.AckNumber]
        jne     .no_window_update

        mov     eax, dword [edx + TCP_header.Window]
        cmp     eax, [ebx + TCP_SOCKET.SND_WND]
        jbe     .no_window_update

  .update_window:

        DEBUGF  1,"Updating window\n"

; Keep track of pure window updates

;        test    ecx, ecx
;        jz      @f
;
;        mov     eax, [ebx + TCP_SOCKET.SND_WL2]
;        cmp     eax, [edx + TCP_header.AckNumber]
;        jne     @f
;
;        ;; mov eax, tiwin
;        cmp     eax, [ebx + TCP_SOCKET.SND_WND]
;        jbe     @f
;
;        ;;; update stats
;
;       @@:

        mov     eax, dword [edx + TCP_header.Window]
        cmp     eax, [ebx + TCP_SOCKET.max_sndwnd]
        jbe     @f
        mov     [ebx + TCP_SOCKET.max_sndwnd], eax
       @@:
        mov     [ebx + TCP_SOCKET.SND_WND], eax

        push    [edx + TCP_header.SequenceNumber]
        pop     [ebx + TCP_SOCKET.SND_WL1]

        push    [edx + TCP_header.AckNumber]
        pop     [ebx + TCP_SOCKET.SND_WL2]

        ;;; needoutput = 1

  .no_window_update:







;-----------------
; process URG flag

        test    [edx + TCP_header.Flags], TH_URG
        jz      .not_urgent

        cmp     [edx + TCP_header.UrgentPointer], 0
        jz      .not_urgent

        cmp     [ebx + TCP_SOCKET.t_state], TCPS_TIMED_WAIT
        je      .not_urgent

; Ignore bogus urgent offsets

        ;;; 1040-1050

        movzx   eax, [edx + TCP_header.UrgentPointer]
        add     eax, [ebx + STREAM_SOCKET.rcv.size]
        cmp     eax, SOCKET_MAXDATA
        jbe     .not_urgent

        mov     [edx + TCP_header.UrgentPointer], 0
        and     [edx + TCP_header.Flags], not (TH_URG)
        jmp     .do_data

  .not_urgent:

; processing of received urgent pointer

        ;;; TODO (1051-1093)








;--------------------------------
; process the data in the segment

  .do_data:

        DEBUGF  1,"TCP: do data (%u)\n", ecx

        test    [edx + TCP_header.Flags], TH_FIN
        jnz     .process_fin

        cmp     [ebx + TCP_SOCKET.t_state], TCPS_FIN_WAIT_1
        jae     .dont_do_data

        test    ecx, ecx
        jz      .final_processing

        DEBUGF  1,"Processing data in segment\n"

;; TODO: check if data is in sequence !

        movzx   esi, [edx + TCP_header.DataOffset]
        add     esi, edx

        or      [ebx + TCP_SOCKET.t_flags], TF_DELACK
        add     [ebx + TCP_SOCKET.RCV_NXT], ecx

        lea     eax, [ebx + STREAM_SOCKET.rcv]
        call    SOCKET_ring_write

        mov     eax, ebx
        call    SOCKET_notify_owner

        jmp     .final_processing


  .dont_do_data:







;---------------
; FIN processing

  .process_fin:

        DEBUGF  1,"Processing FIN\n"

        cmp     [ebx + TCP_SOCKET.t_state], TCPS_CLOSE_WAIT
        je      .not_first_fin
        cmp     [ebx + TCP_SOCKET.t_state], TCPS_CLOSING
        je      .not_first_fin
        cmp     [ebx + TCP_SOCKET.t_state], TCPS_FIN_WAIT_2
        je      .not_first_fin

        DEBUGF  1,"First FIN for this connection\n"

        mov     eax, ebx
        call    SOCKET_cant_recv_more

        mov     [ebx + TCP_SOCKET.t_flags], TF_ACKNOW
        inc     [ebx + TCP_SOCKET.RCV_NXT]

  .not_first_fin:
        mov     eax, [ebx + TCP_SOCKET.t_state]
        shl     eax, 2
        jmp     dword [eax + .FIN_sw_list]

  .FIN_sw_list:
        dd      .no_fin         ;TCPS_CLOSED
        dd      .no_fin         ;TCPS_LISTEN
        dd      .no_fin         ;TCPS_SYN_SENT
        dd      .fin_syn_est    ;TCPS_SYN_RECEIVED
        dd      .fin_syn_est    ;TCPS_ESTABLISHED
        dd      .no_fin         ;TCPS_CLOSE_WAIT
        dd      .fin_wait1      ;TCPS_FIN_WAIT_1
        dd      .no_fin         ;TCPS_CLOSING
        dd      .no_fin         ;TCPS_LAST_ACK
        dd      .fin_wait2      ;TCPS_FIN_WAIT_2
        dd      .fin_timed      ;TCPS_TIMED_WAIT

  .fin_syn_est:

        mov     [ebx + TCP_SOCKET.t_state], TCPS_CLOSE_WAIT
        jmp     .no_fin

  .fin_wait1:

        mov     [ebx + TCP_SOCKET.t_state], TCPS_CLOSING
        jmp     .no_fin

  .fin_wait2:

        mov     [ebx + TCP_SOCKET.t_state], TCPS_TIMED_WAIT
        mov     eax, ebx
        call    TCP_cancel_timers
        mov     [ebx + TCP_SOCKET.timer_timed_wait], 2 * TCP_time_MSL
        call    SOCKET_is_disconnected
        jmp     .no_fin

  .fin_timed:
        mov     [ebx + TCP_SOCKET.timer_timed_wait], 2 * TCP_time_MSL
        jmp     .no_fin

  .no_fin:








;-----------------
; Final processing

  .final_processing:

        DEBUGF  1,"Final processing\n"

        ;;; if debug enabled, output packet

        ;test    needoutput, needoutput
        ;jz      .dumpit

        test    [ebx + TCP_SOCKET.t_flags], TF_ACKNOW
        jz      .dumpit

        DEBUGF  1,"ACK now!\n"

        push    ebx
        mov     eax, ebx
        call    TCP_output
        pop     ebx

  .dumpit:

        pusha
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_unlock
        popa

        call    kernel_free
        add     esp, 4
        ret







;------------------------------------------
; Generate an ACK, droping incoming segment

align 4
.drop_after_ack:

        DEBUGF  1,"Drop after ACK\n"

        test    [edx + TCP_header.Flags], TH_RST
        jnz     .drop

        and     [ebx + TCP_SOCKET.t_flags], TF_ACKNOW

        pusha
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_unlock
        popa

        push    ebx
;        mov     cl, TH_ACK
;        call    TCP_respond_socket
        mov     eax, ebx
        call    TCP_output
        pop     ebx

        call    kernel_free
        add     esp, 4
        ret








;-------------------------------------------
; Generate an RST, dropping incoming segment

align 4
.drop_with_reset:

        pusha
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_unlock
        popa

.drop_with_reset_not_locked:

        DEBUGF  1,"Drop with reset\n"

        test    [edx + TCP_header.Flags], TH_RST
        jnz     .drop

        ;;; if its a multicast/broadcast, also drop

        test    [edx + TCP_header.Flags], TH_ACK
        jnz     .respond_ack

        test    [edx + TCP_header.Flags], TH_SYN
        jnz     .respond_syn

        call    kernel_free
        add     esp, 4
        ret

  .respond_ack:

        push    ebx
        mov     cl, TH_RST
        call    TCP_respond_socket
        pop     ebx

        jmp     .destroy_new_socket


  .respond_syn:

        push    ebx
        mov     cl, TH_RST + TH_ACK
        call    TCP_respond_socket
        pop     ebx

        jmp     .destroy_new_socket







;-----
; Drop

align 4
.drop:

        pusha
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_unlock
        popa

.drop_not_locked:

        DEBUGF  1,"Dropping packet\n"

        ;;;; If debugging options are enabled, output the packet somwhere

  .destroy_new_socket:

        ;;;; kill the newly created socket

        call    kernel_free
        add     esp, 4
        ret