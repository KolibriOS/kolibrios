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

;-----------------------------------------------------------------
;
; TCP_input:
;
;       Add a segment to the incoming TCP queue
;
;  IN:  [esp] = ptr to buffer
;       [esp+4] = buffer size (dont care)
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

; record the current time
        mov     eax, [timer_ticks]      ; in 1/100 seconds
        mov     [esp + 4], eax

        push    ebx ecx esi edi         ; mind the order
        mov     esi, esp

        pushf
        cli
        add_to_queue TCP_queue, TCP_QUEUE_SIZE, sizeof.TCP_queue_entry, .fail
        popf

        add     esp, sizeof.TCP_queue_entry

        mov     [check_idle_semaphore], 5
        xor     edx, edx
        mov     eax, [TCP_input_event]
        mov     ebx, [eax + EVENT.id]
        xor     esi, esi
        call    raise_event

        ret

  .fail:
        popf
        DEBUGF  2, "TCP incoming queue is full, discarding packet!\n"

        inc     [TCP_segments_missed]   ; FIXME: use correct interface

        add     esp, sizeof.TCP_queue_entry - 8
        call    kernel_free
        add     esp, 4

        ret




align 4
TCP_process_input:

        xor     esi, esi
        mov     ecx, MANUAL_DESTROY
        call    create_event
        mov     [TCP_input_event], eax

  .wait:
        mov     eax, [TCP_input_event]
        mov     ebx, [eax + EVENT.id]
        call    wait_event

  .loop:
        get_from_queue TCP_queue, TCP_QUEUE_SIZE, sizeof.TCP_queue_entry, .wait

        push    [esi + TCP_queue_entry.timestamp]
        push    [esi + TCP_queue_entry.buffer_ptr]

        mov     ebx, [esi + TCP_queue_entry.device_ptr]
        mov     ecx, [esi + TCP_queue_entry.segment_size]
        mov     edi, [esi + TCP_queue_entry.ip_ptr]                     ; ptr to ipv4 source address, followed by ipv4 destination address
        mov     esi, [esi + TCP_queue_entry.segment_ptr]                ; change esi last

        DEBUGF  1,"TCP_input: size=%u time=%d\n", ecx, [timer_ticks]

        mov     edx, esi

        cmp     ebx, LOOPBACK_DEVICE
        je      .checksum_ok

; re-calculate the checksum (if not already done by hw)
;        test    [ebx + NET_DEVICE.hwacc], HWACC_TCP_IPv4_IN
;        jnz     .checksum_ok

        push    ecx esi
        pushw   [esi + TCP_header.Checksum]
        mov     [esi + TCP_header.Checksum], 0
        TCP_checksum (edi), (edi+4)
        pop     cx                      ; previous checksum
        cmp     cx, dx
        pop     edx ecx
        jne     .drop_no_socket
  .checksum_ok:

; Verify the data offset
        and     [edx + TCP_header.DataOffset], 0xf0                     ; Calculate TCP segment header size (throwing away unused reserved bits in TCP header)
        shr     [edx + TCP_header.DataOffset], 2
        cmp     [edx + TCP_header.DataOffset], sizeof.TCP_header        ; Now see if it's at least the size of a standard TCP header
        jb      .drop_no_socket                                         ; If not, drop the packet

        movzx   eax, [edx + TCP_header.DataOffset]
        sub     ecx, eax                                                ; substract TCP header size from total segment size
        jb      .drop_no_socket                                         ; If total segment size is less then the advertised header size, drop packet
        DEBUGF  1,"TCP_input: %u bytes of data\n", ecx

;-------------------------------------------
; Convert Big-endian values to little endian

        ntohd   [edx + TCP_header.SequenceNumber]
        ntohd   [edx + TCP_header.AckNumber]

        ntohw   [edx + TCP_header.Window]
        ntohw   [edx + TCP_header.UrgentPointer]

;------------------------
; Find the socket pointer

; IP Packet TCP Destination Port = local Port
; (IP Packet SenderAddress = Remote IP)  OR  (Remote IP = 0)
; (IP Packet TCP Source Port = remote Port) OR (remote Port = 0)

  .findpcb:
        mov     ebx, net_sockets
        mov     si, [edx + TCP_header.DestinationPort]

  .socket_loop:
        mov     ebx, [ebx + SOCKET.NextPtr]
        or      ebx, ebx
        jz      .respond_seg_reset

        cmp     [ebx + SOCKET.Domain], AF_INET4
        jne     .socket_loop

        cmp     [ebx + SOCKET.Protocol], IP_PROTO_TCP
        jne     .socket_loop

        cmp     [ebx + TCP_SOCKET.LocalPort], si
        jne     .socket_loop

        mov     eax, [ebx + IP_SOCKET.RemoteIP]
        cmp     eax, [edi]                              ; Ipv4 source address
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
        DEBUGF  1,"TCP_input: socket ptr=%x state=%u flags=%x\n", ebx, [ebx + TCP_SOCKET.t_state], [edx + TCP_header.Flags]:2

;-------------
; update stats

        inc     [TCP_segments_rx]                       ; FIXME: correct interface?

;----------------------------
; Check if socket isnt closed

        cmp     [ebx + TCP_SOCKET.t_state], TCPS_CLOSED
        je      .drop_no_socket

;----------------
; Lock the socket

        pusha
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_lock
        popa

        DEBUGF  1,"TCP_input: socket locked\n"

;---------------------------
; disable all temporary bits

        mov     [ebx + TCP_SOCKET.temp_bits], 0

;---------------------------------------
; unscale the window into a 32 bit value

        movzx   eax, [edx + TCP_header.Window]
        push    ecx
        mov     cl, [ebx + TCP_SOCKET.SND_SCALE]
        shl     eax, cl
        mov     dword [edx + TCP_header.Window], eax    ; word after window is checksum, we dont need checksum anymore
        pop     ecx

;---------------------------------------
; Are we accepting incoming connections?

        test    [ebx + SOCKET.options], SO_ACCEPTCON
        jz      .no_accept

        DEBUGF  1,"TCP_input: Accepting new connection\n"

        pusha
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_unlock
        popa

        push    ecx edx esi edi         ;;;
        call    SOCKET_fork
        pop     edi esi edx ecx

        test    eax, eax
        jz      .drop_no_socket

        mov     ebx, eax

        mov     [ebx + TCP_SOCKET.temp_bits], TCP_BIT_DROPSOCKET        ;;; FIXME: should we take over bits from previous socket?

        push    dword [edi + 4]                         ; Ipv4 destination addres
        pop     [ebx + IP_SOCKET.LocalIP]

        push    [edx + TCP_header.DestinationPort]
        pop     [ebx + TCP_SOCKET.LocalPort]

        mov     [ebx + TCP_SOCKET.t_state], TCPS_LISTEN
  .no_accept:


;-------------------------------------
; Reset idle timer and keepalive timer

        mov     [ebx + TCP_SOCKET.t_idle], 0
        mov     [ebx + TCP_SOCKET.timer_keepalive], TCP_time_keep_idle

;--------------------
; Process TCP options

        push    ecx

        movzx   ecx, [edx + TCP_header.DataOffset]
        cmp     ecx, sizeof.TCP_header                  ; Does header contain any options?
        je      .no_options

        DEBUGF  1,"TCP_input: Segment has options\n"

;;; FIXME: for LISTEN, options should be called after we determined route, we need it for MSS
;;;        cmp     [ebx + TCP_SOCKET.t_state], TCPS_LISTEN ; no options when in listen state
;;;        jz      .not_uni_xfer                           ; also no header prediction

        add     ecx, edx
        lea     esi, [edx + sizeof.TCP_header]

  .opt_loop:
        cmp     esi, ecx                        ; are we scanning outside of header?
        jae     .no_options
        lodsb
        cmp     al, TCP_OPT_EOL                 ; end of option list?
        je      .no_options
        cmp     al, TCP_OPT_NOP
        je      .opt_loop
        cmp     al, TCP_OPT_MAXSEG
        je      .opt_maxseg
        cmp     al, TCP_OPT_WINDOW
        je      .opt_window
        cmp     al, TCP_OPT_SACK_PERMIT
        je      .opt_sack_permit
;        cmp     al, TCP_OPT_SACK
;        je      .opt_sack
        cmp     al, TCP_OPT_TIMESTAMP
        je      .opt_timestamp
        DEBUGF  1,"TCP_input: unknown option:%u\n", al
        jmp     .no_options                     ; If we reach here, some unknown options were received, skip them all!

  .opt_maxseg:
        lodsb
        cmp     al, 4
        jne     .no_options                     ; error occured, ignore all options!

        test    [edx + TCP_header.Flags], TH_SYN
        jz      @f

        lodsw
        rol     ax, 8
        DEBUGF  1,"TCP_input: Maxseg=%u\n", ax
        call    TCP_mss
       @@:
        jmp     .opt_loop


  .opt_window:
        lodsb
        cmp     al, 3
        jne     .no_options

        test    [edx + TCP_header.Flags], TH_SYN
        jz      @f

        DEBUGF  1,"TCP_input: Got window scale option\n"
        or      [ebx + TCP_SOCKET.t_flags], TF_RCVD_SCALE

        lodsb
        mov     [ebx + TCP_SOCKET.SND_SCALE], al
        ;;;;; TODO

       @@:
        jmp     .opt_loop


  .opt_sack_permit:
        lodsb
        cmp     al, 2
        jne     .no_options

        test    [edx + TCP_header.Flags], TH_SYN
        jz      @f

        DEBUGF  1,"TCP_input: Selective Acknowledgement permitted\n"
        or      [ebx + TCP_SOCKET.t_flags], TF_SACK_PERMIT

       @@:
        jmp     .opt_loop


  .opt_timestamp:
        lodsb
        cmp     al, 10                          ; length must be 10
        jne     .no_options

        DEBUGF  1,"TCP_input: Got timestamp option\n"

        test    [edx + TCP_header.Flags], TH_SYN
        jz      @f
        or      [ebx + TCP_SOCKET.t_flags], TF_RCVD_TSTMP
       @@:

        lodsd
        mov     [ebx + TCP_SOCKET.ts_val], eax
        lodsd                                   ; timestamp echo reply
        mov     [ebx + TCP_SOCKET.ts_ecr], eax
        or      [ebx + TCP_SOCKET.temp_bits], TCP_BIT_TIMESTAMP

        ; Since we have a timestamp, lets do the paws test right away!

        test    [edx + TCP_header.Flags], TH_RST
        jnz     .no_paws

        mov     eax, [ebx + TCP_SOCKET.ts_recent]
        test    eax, eax
        jz      .no_paws
        cmp     eax, [ebx + TCP_SOCKET.ts_val]
        jge     .no_paws

        DEBUGF  1,"TCP_input: PAWS: detected an old segment\n"

        mov     eax, [esp+4+4]                          ; tcp_now
        sub     eax, [ebx + TCP_SOCKET.ts_recent_age]

        pop     ecx
        cmp     eax, TCP_PAWS_IDLE
        jle     .drop_after_ack                         ; TODO: update stats
        push    ecx

        mov     [ebx + TCP_SOCKET.ts_recent], 0         ; timestamp was invalid, fix it.
  .no_paws:
        jmp     .opt_loop

  .no_options:

        pop     ecx

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

        DEBUGF  1,"TCP_input: Header prediction: we are sender\n"

;---------------------------------
; Packet is a pure ACK, process it

; Delete acknowledged bytes from send buffer
        pusha
        mov     ecx, eax
        lea     eax, [ebx + STREAM_SOCKET.snd]
        call    SOCKET_ring_free
        popa

; Update RTT estimators

        test    [ebx + TCP_SOCKET.temp_bits], TCP_BIT_TIMESTAMP
        jz      .no_timestamp_rtt
        mov     eax, [esp + 4]                          ; timestamp when this segment was received
        sub     eax, [ebx + TCP_SOCKET.ts_ecr]
        inc     eax
        call    TCP_xmit_timer
        jmp     .rtt_done

  .no_timestamp_rtt:
        cmp     [ebx + TCP_SOCKET.t_rtt], 0
        je      .rtt_done
        mov     eax, [edx + TCP_header.AckNumber]
        cmp     eax, [ebx + TCP_SOCKET.t_rtseq]
        jbe     .rtt_done
        mov     eax, [ebx + TCP_SOCKET.t_rtt]
        call    TCP_xmit_timer

  .rtt_done:

; update window pointers
        mov     eax, [edx + TCP_header.AckNumber]
        mov     [ebx + TCP_SOCKET.SND_UNA], eax

; Stop retransmit timer
        mov     [ebx + TCP_SOCKET.timer_retransmission], 0

; Unlock the socket
        pusha
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_unlock
        popa

; Awaken waiting processes
        mov     eax, ebx
        call    SOCKET_notify

; Generate more output
        call    TCP_output

        jmp     .drop_no_socket

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

;       jnz     .not_uni_xfer

; Complete processing of received data

        DEBUGF  1,"TCP_input: Header prediction: we are receiving %u bytes\n", ecx

        add     [ebx + TCP_SOCKET.RCV_NXT], ecx         ; Update sequence number with number of bytes we have copied

        movzx   esi, [edx + TCP_header.DataOffset]
        add     esi, edx
        lea     eax, [ebx + STREAM_SOCKET.rcv]
        call    SOCKET_ring_write                       ; Add the data to the socket buffer

        mov     eax, ebx
        call    SOCKET_notify

        or      [ebx + TCP_SOCKET.t_flags], TF_DELACK   ; Set delayed ack flag

        jmp     .drop

;--------------------------------------------------
; Header prediction failed, do it the slow way

  .not_uni_xfer:

        DEBUGF  1,"TCP_input: Header prediction failed\n"

; Calculate receive window size

        push    edx
        mov     eax, SOCKETBUFFSIZE
        sub     eax, [ebx + STREAM_SOCKET.rcv.size]
        mov     edx, [ebx + TCP_SOCKET.RCV_ADV]
        sub     edx, [ebx + TCP_SOCKET.RCV_NXT]
        cmp     eax, edx
        jg      @f
        mov     eax, edx
       @@:
        DEBUGF  1,"Receive window size=%d\n", eax
        mov     [ebx + TCP_SOCKET.RCV_WND], eax
        pop     edx

; If we are in listen or syn_sent state, go to that specific code right away

        cmp     [ebx + TCP_SOCKET.t_state], TCPS_LISTEN
        je      .LISTEN

        cmp     [ebx + TCP_SOCKET.t_state], TCPS_SYN_SENT
        je      .SYN_SENT

;----------------------------
; trim any data not in window

; check for duplicate data at beginning of segment (635)

        mov     eax, [ebx + TCP_SOCKET.RCV_NXT]
        sub     eax, [edx + TCP_header.SequenceNumber]
        jle     .no_duplicate

        DEBUGF  1,"TCP_input: %u bytes duplicate data!\n", eax

        test    [edx + TCP_header.Flags], TH_SYN
        jz      .no_dup_syn

        DEBUGF  1,"TCP_input: got duplicate syn\n"

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

; Check for entire duplicate segment (646)
        cmp     eax, ecx                ; eax holds number of bytes to drop, ecx is data size
        jb      .duplicate
        jnz     @f
        test    [edx + TCP_header.Flags], TH_FIN
        jnz     .duplicate
       @@:

; Any valid FIN must be to the left of the window.
; At this point the FIN must be out of sequence or a duplicate, drop it
        and     [edx + TCP_header.Flags], not TH_FIN

; send an ACK and resynchronize and drop any data.
; But keep on processing for RST or ACK
        DEBUGF  1, "616\n"
        or      [ebx + TCP_SOCKET.t_flags], TF_ACKNOW
        mov     eax, ecx
;TODO: update stats

;-----------------------------------------------
; Remove duplicate data and update urgent offset

  .duplicate:
;;; TODO: 677
        add     [edx + TCP_header.SequenceNumber], eax
        sub     ecx, eax

        sub     [edx + TCP_header.UrgentPointer], ax
        jg      @f
        and     [edx + TCP_header.Flags], not (TH_URG)
        mov     [edx + TCP_header.UrgentPointer], 0
       @@:

;--------------------------------------------------
; Handle data that arrives after process terminates (687)

  .no_duplicate:
        cmp     [ebx + SOCKET.PID], 0
        jne     .not_terminated
        cmp     [ebx + TCP_SOCKET.t_state], TCPS_CLOSE_WAIT
        jbe     .not_terminated
        test    ecx, ecx
        jz      .not_terminated

        mov     eax, ebx
        call    TCP_close
;;;TODO: update stats
        jmp     .respond_seg_reset

;----------------------------------------
; Remove data beyond right edge of window (700-736)

  .not_terminated:
        mov     eax, [edx + TCP_header.SequenceNumber]
        add     eax, ecx
        sub     eax, [ebx + TCP_SOCKET.RCV_NXT]
        sub     eax, [ebx + TCP_SOCKET.RCV_WND]         ; eax now holds the number of bytes to drop
        jle     .no_excess_data

        DEBUGF  1,"%d bytes beyond right edge of window\n", eax

;;; TODO: update stats
        cmp     eax, ecx
        jl      .dont_drop_all
; If a new connection request is received while in TIME_WAIT, drop the old connection and start over,
; if the sequence numbers are above the previous ones

        test    [edx + TCP_header.Flags], TH_SYN
        jz      .no_new_request
        cmp     [ebx + TCP_SOCKET.t_state], TCPS_TIMED_WAIT
        jne     .no_new_request
;        mov     edx, [ebx + TCP_SOCKET.RCV_NXT]
;        cmp     edx, [edx + TCP_header.SequenceNumber]
;        add     edx, 64000      ; TCP_ISSINCR   FIXME
        mov     eax, ebx
        call    TCP_close
        jmp     .findpcb        ; FIXME: skip code for unscaling window, ...
  .no_new_request:

; If window is closed can only take segments at window edge, and have to drop data and PUSH from
; incoming segments. Continue processing, but remember to ACK. Otherwise drop segment and ACK

        cmp     [ebx + TCP_SOCKET.RCV_WND], 0
        jne     .drop_after_ack
        mov     eax, [edx + TCP_header.SequenceNumber]
        cmp     eax, [ebx + TCP_SOCKET.RCV_NXT]
        jne     .drop_after_ack

        DEBUGF  1, "690\n"
        or      [ebx + TCP_SOCKET.t_flags], TF_ACKNOW
;;; TODO: update stats
        jmp     .no_excess_data
  .dont_drop_all:
;;; TODO: update stats
;;; TODO: 733

        sub     ecx, eax
        and     [ebx + TCP_SOCKET.t_flags], not (TH_PUSH or TH_FIN)
  .no_excess_data:

;-----------------
; Record timestamp (737-746)

; If last ACK falls within this segments sequence numbers, record its timestamp
        test    [ebx + TCP_SOCKET.temp_bits], TCP_BIT_TIMESTAMP
        jz      .no_timestamp
        mov     eax, [ebx + TCP_SOCKET.last_ack_sent]
        sub     eax, [edx + TCP_header.SequenceNumber]
        jb      .no_timestamp
        test    [ebx + TCP_header.Flags], TH_SYN or TH_FIN      ; syn and fin occupy one byte
        jz      @f
        dec     eax
       @@:
        sub     eax, ecx
        jae     .no_timestamp

        DEBUGF  1,"Recording timestamp\n"

        mov     eax, [esp + 4]                                  ; tcp_now
        mov     [ebx + TCP_SOCKET.ts_recent_age], eax
        mov     eax, [ebx + TCP_SOCKET.ts_val]
        mov     [ebx + TCP_SOCKET.ts_recent], eax
  .no_timestamp:

;------------------
; Process RST flags

        test    [edx + TCP_header.Flags], TH_RST
        jz      .no_rst

        DEBUGF  1,"TCP_input: Got an RST flag\n"

        mov     eax, [ebx + TCP_SOCKET.t_state]
        shl     eax, 2
        jmp     dword [eax + .rst_sw_list]

  .rst_sw_list:
        dd      .no_rst         ; TCPS_CLOSED
        dd      .no_rst         ; TCPS_LISTEN
        dd      .no_rst         ; TCPS_SYN_SENT
        dd      .econnrefused   ; TCPS_SYN_RECEIVED
        dd      .econnreset     ; TCPS_ESTABLISHED
        dd      .econnreset     ; TCPS_CLOSE_WAIT
        dd      .econnreset     ; TCPS_FIN_WAIT_1
        dd      .rst_close      ; TCPS_CLOSING
        dd      .rst_close      ; TCPS_LAST_ACK
        dd      .econnreset     ; TCPS_FIN_WAIT_2
        dd      .rst_close      ; TCPS_TIMED_WAIT

  .econnrefused:
        DEBUGF  1,"TCP_input: Connection refused\n"

        mov     [ebx + SOCKET.errorcode], ECONNREFUSED
        jmp     .close

  .econnreset:
        DEBUGF  1,"TCP_input: Connection reset\n"

        mov     [ebx + SOCKET.errorcode], ECONNRESET

  .close:
        DEBUGF  1,"TCP_input: Closing connection\n"

        mov     [ebx + TCP_SOCKET.t_state], TCPS_CLOSED
;;; TODO: update stats (tcp drops)
        mov     eax, ebx
        call    TCP_close
        jmp     .drop_no_socket

  .rst_close:
        DEBUGF  1,"TCP_input: Closing with reset\n"

        mov     eax, ebx
        call    TCP_close
        jmp     .drop_no_socket

  .no_rst:

;--------------------------------------
; handle SYN-full and ACK-less segments

        test    [edx + TCP_header.Flags], TH_SYN
        jz      .not_syn_full

        mov     eax, ebx
        mov     ebx, ECONNRESET
        call    TCP_drop
        jmp     .drop_with_reset
  .not_syn_full:

;---------------
; ACK processing

        test    [edx + TCP_header.Flags], TH_ACK
        jz      .drop

        cmp     [ebx + TCP_SOCKET.t_state], TCPS_SYN_RECEIVED
        jb      .ack_processed                                  ; states: closed, listen, syn_sent
        ja      .no_syn_rcv                                     ; established, fin_wait_1, fin_wait_2, close_wait, closing, last_ack, time_wait

        DEBUGF  1,"TCP_input: state=syn_received\n"

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

;;; TODO: call TCP_reassemble

        mov     eax, [edx + TCP_header.SequenceNumber]
        dec     eax
        mov     [ebx + TCP_SOCKET.SND_WL1], eax

  .no_syn_rcv:

;-------------------------
; check for duplicate ACKs

        mov     eax, [edx + TCP_header.AckNumber]
        cmp     eax, [ebx + TCP_SOCKET.SND_UNA]
        ja      .not_dup_ack

        test    ecx, ecx
        jnz     .reset_dupacks

        mov     eax, dword [edx + TCP_header.Window]
        cmp     eax, [ebx + TCP_SOCKET.SND_WND]
        jne     .reset_dupacks

        DEBUGF  1,"TCP_input: Processing duplicate ACK\n"

; If we have outstanding data, other than a window probe, this is a completely duplicate ACK
; (window info didnt change) The ACK is the biggest we've seen and we've seen exactly our rexmt threshold of them,
; assume a packet has been dropped and retransmit it. Kludge snd_nxt & the congestion window so we send only this one packet.

        cmp     [ebx + TCP_SOCKET.timer_retransmission], 0 ;;;;  FIXME
        jg      @f

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
        cmova   eax, [ebx + TCP_SOCKET.SND_CWND]
        shr     eax, 1
        push    edx
        xor     edx, edx
        div     [ebx + TCP_SOCKET.t_maxseg]
        cmp     eax, 2
        ja      @f
        xor     eax, eax
        mov     al, 2
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

; Unlock the socket
        push    ebx
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_unlock

; retransmit missing segment
        mov     eax, [esp]
        call    TCP_output

; Lock the socket again
        mov     ecx, [esp]
        add     ecx, SOCKET.mutex
        call    mutex_lock
        pop     ebx 

; Continue processing
        xor     edx, edx
        mov     eax, [ebx + TCP_SOCKET.t_maxseg]
        mul     [ebx + TCP_SOCKET.t_dupacks]
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

        DEBUGF  1,"TCP_input: Increasing congestion window\n"

        mov     eax, [ebx + TCP_SOCKET.t_maxseg]
        add     [ebx + TCP_SOCKET.SND_CWND], eax

; Unlock the socket
        push    ebx
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_unlock

; retransmit missing segment
        mov     eax, [esp]
        call    TCP_output

; Lock the socket again
        mov     ecx, [esp]
        add     ecx, SOCKET.mutex
        call    mutex_lock
        pop     ebx

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

        DEBUGF  1,"TCP_input: acceptable ACK for %u bytes\n", edi

;------------------------------------------
; RTT measurements and retransmission timer  (912-926)

; If we have a timestamp, update smoothed RTT

        test    [ebx + TCP_SOCKET.temp_bits], TCP_BIT_TIMESTAMP
        jz      .timestamp_not_present
        mov     eax, [esp+4]
        sub     eax, [ebx + TCP_SOCKET.ts_ecr]
        inc     eax
        call    TCP_xmit_timer
        jmp     .rtt_done_

; If no timestamp but transmit timer is running and timed sequence number was acked,
; update smoothed RTT. Since we now have an RTT measurement, cancel the timer backoff
; (Phil Karn's retransmit algo)
; Recompute the initial retransmit timer

  .timestamp_not_present:
        mov     eax, [edx + TCP_header.AckNumber]
        cmp     eax, [ebx + TCP_SOCKET.t_rtseq]
        jbe     .rtt_done_
        mov     eax, [ebx + TCP_SOCKET.t_rtt]
        test    eax, eax
        jz      .rtt_done_
        call    TCP_xmit_timer

  .rtt_done_:

; If all outstanding data is acked, stop retransmit timer and remember to restart (more output or persist)
; If there is more data to be acked, restart retransmit timer, using current (possible backed-off) value.

        mov     eax, [ebx + TCP_SOCKET.SND_MAX]
        cmp     eax, [edx + TCP_header.AckNumber]
        jne     .more_data
        mov     [ebx + TCP_SOCKET.timer_retransmission], 0
        or      [ebx + TCP_SOCKET.temp_bits], TCP_BIT_NEEDOUTPUT
        jmp     .no_restart
  .more_data:
        cmp     [ebx + TCP_SOCKET.timer_persist], 0
        jne     .no_restart

        mov     eax, [ebx + TCP_SOCKET.t_rxtcur]
        mov     [ebx + TCP_SOCKET.timer_retransmission], eax

  .no_restart:


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
        cmova   esi, eax
        mov     [ebx + TCP_SOCKET.SND_CWND], esi

;------------------------------------------
; Remove acknowledged data from send buffer

        cmp     edi, [ebx + STREAM_SOCKET.snd.size]
        jbe     .finiacked

        push    ecx edx ebx
        mov     ecx, [ebx + STREAM_SOCKET.snd.size]
        lea     eax, [ebx + STREAM_SOCKET.snd]
        sub     [ebx + TCP_SOCKET.SND_WND], ecx
        call    SOCKET_ring_free
        pop     ebx edx ecx

        DEBUGF  1,"TCP_input: our FIN is acked\n"
        stc

        jmp     .wakeup

  .finiacked:

        push    ecx edx ebx
        mov     ecx, edi
        lea     eax, [ebx + STREAM_SOCKET.snd]
        call    SOCKET_ring_free
        pop     ebx
        sub     [ebx + TCP_SOCKET.SND_WND], ecx
        pop     edx ecx

        DEBUGF  1,"TCP_input: our FIN is not acked\n"
        clc

;----------------------------------------
; Wake up process waiting on send buffer

  .wakeup:

        pushf                   ; Keep the flags (Carry flag)
        mov     eax, ebx
        call    SOCKET_notify

; Update TCPS

        mov     eax, [edx + TCP_header.AckNumber]
        mov     [ebx + TCP_SOCKET.SND_UNA], eax
        cmp     eax, [ebx + TCP_SOCKET.SND_NXT]
        jb      @f
        mov     [ebx + TCP_SOCKET.SND_NXT], eax
       @@:

        popf

; General ACK handling complete
; Now do the state-specific ones
; Carry flag is set when our FIN is acked

        mov     eax, [ebx + TCP_SOCKET.t_state]
        jmp     dword [eax*4 + .ACK_sw_list]

  .ACK_sw_list:
        dd      .ack_processed  ; TCPS_CLOSED
        dd      .ack_processed  ; TCPS_LISTEN
        dd      .ack_processed  ; TCPS_SYN_SENT
        dd      .ack_processed  ; TCPS_SYN_RECEIVED
        dd      .ack_processed  ; TCPS_ESTABLISHED
        dd      .ack_processed  ; TCPS_CLOSE_WAIT
        dd      .ack_fw1        ; TCPS_FIN_WAIT_1
        dd      .ack_c          ; TCPS_CLOSING
        dd      .ack_la         ; TCPS_LAST_ACK
        dd      .ack_processed  ; TCPS_FIN_WAIT_2
        dd      .ack_tw         ; TCPS_TIMED_WAIT


  .ack_fw1:
        jnc     .ack_processed

        test    [ebx + SOCKET.state], SS_CANTRCVMORE
        jnz     @f
        mov     eax, ebx
        call    SOCKET_is_disconnected
        mov     [ebx + TCP_SOCKET.timer_timed_wait], TCP_time_max_idle
       @@:
        mov     [ebx + TCP_SOCKET.t_state], TCPS_FIN_WAIT_2
        jmp     .ack_processed

  .ack_c:
        jnc     .ack_processed

        mov     [ebx + TCP_SOCKET.t_state], TCPS_TIMED_WAIT
        mov     eax, ebx
        call    TCP_cancel_timers
        mov     [ebx + TCP_SOCKET.timer_timed_wait], 2 * TCP_time_MSL
        mov     eax, ebx
        call    SOCKET_is_disconnected
        jmp     .ack_processed

  .ack_la:
        jnc     .ack_processed

        mov     eax, ebx
        call    TCP_disconnect
        jmp     .drop

  .ack_tw:
        mov     [ebx + TCP_SOCKET.timer_timed_wait], 2 * TCP_time_MSL
        jmp     .drop_after_ack

  .reset_dupacks:               ; We got a new ACK, reset duplicate ACK counter
        mov     [ebx + TCP_SOCKET.t_dupacks], 0
        jmp     .ack_processed

;-------
; LISTEN

align 4
  .LISTEN:

        DEBUGF  1,"TCP_input: state=listen\n"

        test    [edx + TCP_header.Flags], TH_RST
        jnz     .drop

        test    [edx + TCP_header.Flags], TH_ACK
        jnz     .drop_with_reset

        test    [edx + TCP_header.Flags], TH_SYN
        jz      .drop

;;; TODO: check if it's a broadcast or multicast, and drop if so

        push    dword [edi]                             ; Ipv4 source addres
        pop     [ebx + IP_SOCKET.RemoteIP]

        push    [edx + TCP_header.SourcePort]
        pop     [ebx + TCP_SOCKET.RemotePort]

        push    [edx + TCP_header.SequenceNumber]
        pop     [ebx + TCP_SOCKET.IRS]

        mov     eax, [TCP_sequence_num]
        add     [TCP_sequence_num], 64000 / 2
        mov     [ebx + TCP_SOCKET.ISS], eax
        mov     [ebx + TCP_SOCKET.SND_NXT], eax

        TCP_sendseqinit ebx
        TCP_rcvseqinit ebx

        mov     [ebx + TCP_SOCKET.t_state], TCPS_SYN_RECEIVED
        mov     [ebx + TCP_SOCKET.t_flags], TF_ACKNOW
        mov     [ebx + TCP_SOCKET.timer_keepalive], TCP_time_keep_interval  ;;;; macro

        lea     eax, [ebx + STREAM_SOCKET.snd]
        call    SOCKET_ring_create

        lea     eax, [ebx + STREAM_SOCKET.rcv]
        call    SOCKET_ring_create

        and     [ebx + TCP_SOCKET.temp_bits], not TCP_BIT_DROPSOCKET

;;;        call    SOCKET_notify_owner

        jmp     .trim_then_step6

;------------
; Active Open

align 4
  .SYN_SENT:

        DEBUGF  1,"TCP_input: state=syn_sent\n"

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

        DEBUGF  1,"TCP_input: active open\n"

;;; TODO: update stats

; set socket state to connected
        mov     [ebx + SOCKET.state], SS_ISCONNECTED
        mov     [ebx + TCP_SOCKET.t_state], TCPS_ESTABLISHED

; Do window scaling on this connection ?
        mov     eax, [ebx + TCP_SOCKET.t_flags]
        and     eax, TF_REQ_SCALE or TF_RCVD_SCALE
        cmp     eax, TF_REQ_SCALE or TF_RCVD_SCALE
        jne     .no_scaling

        mov     ax, word [ebx + TCP_SOCKET.requested_s_scale]
        mov     word [ebx + TCP_SOCKET.SND_SCALE], ax
  .no_scaling:

;;; TODO: reassemble packets queue

        mov     eax, [ebx + TCP_SOCKET.t_rtt]
        test    eax, eax
        je      .trim_then_step6
        call    TCP_xmit_timer
        jmp     .trim_then_step6

  .simultaneous_open:

        DEBUGF  1,"TCP_input: simultaneous open\n"
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

;-------
; step 6

  .ack_processed:

        DEBUGF  1,"TCP_input: ACK processed\n"

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

        mov     eax, dword [edx + TCP_header.Window]
        cmp     eax, [ebx + TCP_SOCKET.SND_WND]
        jbe     .no_window_update

  .update_window:

;;; TODO: update stats (Keep track of pure window updates)

        mov     eax, dword [edx + TCP_header.Window]
        cmp     eax, [ebx + TCP_SOCKET.max_sndwnd]
        jbe     @f
        mov     [ebx + TCP_SOCKET.max_sndwnd], eax
       @@:
        mov     [ebx + TCP_SOCKET.SND_WND], eax

        DEBUGF  1,"TCP_input: Updating window to %u\n", eax

        push    [edx + TCP_header.SequenceNumber]
        pop     [ebx + TCP_SOCKET.SND_WL1]

        push    [edx + TCP_header.AckNumber]
        pop     [ebx + TCP_SOCKET.SND_WL2]

        or      [ebx + TCP_SOCKET.temp_bits], TCP_BIT_NEEDOUTPUT

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


;---------------------------------------
; process the data in the segment (1094)

  .do_data:

        cmp     [ebx + TCP_SOCKET.t_state], TCPS_TIMED_WAIT
        jae     .final_processing

        test    [edx + TCP_header.Flags], TH_FIN
        jnz     @f

        test    ecx, ecx
        jnz     .final_processing
       @@:


; The segment is in order?
        mov     eax, [edx + TCP_header.SequenceNumber]
        cmp     eax, [ebx + TCP_SOCKET.RCV_NXT]
        jne     .out_of_order

; The reassembly queue is empty?
        cmp     [ebx + TCP_SOCKET.seg_next], 0
        jne     .out_of_order

; The connection is established?
        cmp     [ebx + TCP_SOCKET.t_state], TCPS_ESTABLISHED
        jne     .out_of_order

; Ok, lets do this..  Set delayed ACK flag and copy data into socket buffer
        or      [ebx + TCP_SOCKET.t_flags], TF_DELACK

        pusha
        movzx   esi, [edx + TCP_header.DataOffset]
        add     esi, edx
        lea     eax, [ebx + STREAM_SOCKET.rcv]
        call    SOCKET_ring_write                       ; Add the data to the socket buffer
        add     [ebx + TCP_SOCKET.RCV_NXT], ecx         ; Update sequence number with number of bytes we have copied
        popa

; Wake up the sleeping process
        mov     eax, ebx
        call    SOCKET_notify

        jmp     .data_done

  .out_of_order:

; Uh-oh, some data is out of order, lets call TCP reassemble for help

        call    TCP_reassemble

        DEBUGF  1, "1470\n"
        or      [ebx + TCP_SOCKET.t_flags], TF_ACKNOW

  .data_done:

;---------------
; FIN processing

        test    [edx + TCP_header.Flags], TH_FIN
        jz      .final_processing

        DEBUGF  1,"TCP_input: Processing FIN\n"

        cmp     [ebx + TCP_SOCKET.t_state], TCPS_TIMED_WAIT
        jae     .not_first_fin

        DEBUGF  1,"TCP_input: First FIN for this connection\n"

        mov     eax, ebx
        call    SOCKET_cant_recv_more

        mov     [ebx + TCP_SOCKET.t_flags], TF_ACKNOW
        inc     [ebx + TCP_SOCKET.RCV_NXT]

  .not_first_fin:
        mov     eax, [ebx + TCP_SOCKET.t_state]
        shl     eax, 2
        jmp     dword [eax + .FIN_sw_list]

  .FIN_sw_list:
        dd      .final_processing       ; TCPS_CLOSED
        dd      .final_processing       ; TCPS_LISTEN
        dd      .final_processing       ; TCPS_SYN_SENT
        dd      .fin_syn_est            ; TCPS_SYN_RECEIVED
        dd      .fin_syn_est            ; TCPS_ESTABLISHED
        dd      .final_processing       ; TCPS_CLOSE_WAIT
        dd      .fin_wait1              ; TCPS_FIN_WAIT_1
        dd      .final_processing       ; TCPS_CLOSING
        dd      .final_processing       ; TCPS_LAST_ACK
        dd      .fin_wait2              ; TCPS_FIN_WAIT_2
        dd      .fin_timed              ; TCPS_TIMED_WAIT

  .fin_syn_est:

        mov     [ebx + TCP_SOCKET.t_state], TCPS_CLOSE_WAIT
        jmp     .final_processing

  .fin_wait1:

        mov     [ebx + TCP_SOCKET.t_state], TCPS_CLOSING
        jmp     .final_processing

  .fin_wait2:

        mov     [ebx + TCP_SOCKET.t_state], TCPS_TIMED_WAIT
        mov     eax, ebx
        call    TCP_cancel_timers
        mov     [ebx + TCP_SOCKET.timer_timed_wait], 2 * TCP_time_MSL
        call    SOCKET_is_disconnected
        jmp     .final_processing

  .fin_timed:
        mov     [ebx + TCP_SOCKET.timer_timed_wait], 2 * TCP_time_MSL
        jmp     .final_processing


  .drop_after_ack:
        DEBUGF  1,"TCP_input: Drop after ACK\n"

        push    edx ebx
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_unlock
        pop     eax edx

        test    [edx + TCP_header.Flags], TH_RST
        jnz     .dumpit

        or      [eax + TCP_SOCKET.t_flags], TF_ACKNOW
        jmp     .need_output

  .drop_with_reset:
        DEBUGF  1,"TCP_input: Drop with reset\n"

        push    ebx edx
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_unlock
        pop     edx ebx

        test    [edx + TCP_header.Flags], TH_RST
        jnz     .dumpit

        ;;; if its a multicast/broadcast, also drop

        test    [edx + TCP_header.Flags], TH_ACK
        jnz     .respond_ack

        test    [edx + TCP_header.Flags], TH_SYN
        jnz     .respond_syn
        jmp     .dumpit

;-----------------
; Final processing

  .final_processing:
        DEBUGF  1,"TCP_input: Final processing\n"

        push    ebx
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_unlock
        pop     eax

        test    [eax + TCP_SOCKET.temp_bits], TCP_BIT_NEEDOUTPUT
        jnz     .need_output

        test    [eax + TCP_SOCKET.t_flags], TF_ACKNOW
        jz      .dumpit
        DEBUGF  1,"TCP_input: ACK now!\n"

  .need_output:
        DEBUGF  1,"TCP_input: need output\n"
        call    TCP_output

  .dumpit:
        DEBUGF  1,"TCP_input: dumping\n"

        call    kernel_free
        add     esp, 4
        jmp     .loop

;---------
; Respond

  .respond_ack:
        push    ebx
        mov     cl, TH_RST
        call    TCP_respond
        pop     ebx
        jmp     .destroy_new_socket

  .respond_syn:
        push    ebx
        mov     cl, TH_RST + TH_ACK
        call    TCP_respond
        pop     ebx
        jmp     .destroy_new_socket

  .respond_seg_reset:
        test    [edx + TCP_header.Flags], TH_RST
        jnz     .drop_no_socket

        ;;; TODO: if its a multicast/broadcast, also drop

        test    [edx + TCP_header.Flags], TH_ACK
        jnz     .respond_seg_ack

        test    [edx + TCP_header.Flags], TH_SYN
        jnz     .respond_seg_syn

        jmp     .drop_no_socket

  .respond_seg_ack:
        mov     cl, TH_RST
        call    TCP_respond_segment
        jmp     .drop_no_socket

  .respond_seg_syn:
        mov     cl, TH_RST + TH_ACK
        call    TCP_respond_segment
        jmp     .drop_no_socket

;-----
; Drop

  .drop:
        DEBUGF  1,"TCP_input: Dropping segment\n"

        pusha
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_unlock
        popa

  .destroy_new_socket:
        test    [ebx + TCP_SOCKET.temp_bits], TCP_BIT_DROPSOCKET
        jz      .drop_no_socket

        mov     eax, ebx
        call    SOCKET_free

  .drop_no_socket:
        DEBUGF  1,"TCP_input: Drop (no socket)\n"

        call    kernel_free
        add     esp, 4
        jmp     .loop
