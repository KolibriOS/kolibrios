;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2010. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  TCP.INC                                                        ;;
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

; Socket states
TCB_CLOSED		equ 0
TCB_LISTEN		equ 1
TCB_SYN_SENT		equ 2
TCB_SYN_RECEIVED	equ 3
TCB_ESTABLISHED 	equ 4
TCB_CLOSE_WAIT		equ 5
TCB_FIN_WAIT_1		equ 6
TCB_CLOSING		equ 7
TCB_LAST_ACK		equ 8
TCB_FIN_WAIT_2		equ 9
TCB_TIMED_WAIT		equ 10

; Socket Flags
TF_ACKNOW		equ 1 shl 0	; ack peer immediately
TF_DELACK		equ 1 shl 1	; ack, but try to delay it
TF_NODELAY		equ 1 shl 2	; don't delay packets to coalesce
TF_NOOPT		equ 1 shl 3	; don't use tcp options
TF_SENTFIN		equ 1 shl 4	; have sent FIN
TF_REQ_SCALE		equ 1 shl 5	; have/will request window scaling
TF_RCVD_SCALE		equ 1 shl 6	; other side has requested scaling
TF_REQ_TSTMP		equ 1 shl 7	; have/will request timestamps
TF_RCVD_TSTMP		equ 1 shl 8	; a timestamp was received in SYN
TF_SACK_PERMIT		equ 1 shl 9	; other side said I could SACK

; Segment flags
TH_FIN			equ 1 shl 0
TH_SYN			equ 1 shl 1
TH_RST			equ 1 shl 2
TH_PUSH 		equ 1 shl 3
TH_ACK			equ 1 shl 4
TH_URG			equ 1 shl 5

; Segment header options
TCP_OPT_EOL		equ 0		; End of option list.
TCP_OPT_NOP		equ 1		; No-Operation.
TCP_OPT_MAXSEG		equ 2		; Maximum Segment Size.
TCP_OPT_WINDOW		equ 3		; window scale
TCP_OPT_TIMESTAMP	equ 8

struct	TCP_segment
	.SourcePort		dw ?
	.DestinationPort	dw ?
	.SequenceNumber 	dd ?
	.AckNumber		dd ?
	.DataOffset		db ?	; DataOffset[0-3 bits] and Reserved[4-7]
	.Flags			db ?	; Reserved[0-1 bits]|URG|ACK|PSH|RST|SYN|FIN
	.Window 		dw ?
	.Checksum		dw ?
	.UrgentPointer		dw ?
	.Data:				; ..or options
ends

struct	tcp_in_queue_entry
	.data_ptr	dd ?
	.data_size	dd ?
	.offset 	dd ?
	.size:
ends

struct	tcp_out_queue_entry
	.data_ptr	dd ?
	.data_size	dd ?

	.size:
ends

align 4
uglobal
	TCP_segments_tx 	rd IP_MAX_INTERFACES
	TCP_segments_rx 	rd IP_MAX_INTERFACES
	TCP_bytes_rx		rq IP_MAX_INTERFACES
	TCP_bytes_tx		rq IP_MAX_INTERFACES
	TCP_sequence_num	dd ?
endg


;-----------------------------------------------------------------
;
; TCP_init
;
;  This function resets all TCP variables
;
;  IN:  /
;  OUT: /
;
;-----------------------------------------------------------------
align 4
TCP_init:

	xor	eax, eax
	mov	edi, TCP_segments_tx
	mov	ecx, (6*IP_MAX_INTERFACES)
	rep	stosd

	mov	[TCP_sequence_num],1

	ret


;-----------------------------------------------------------------
;
; decrease socket ttls
;
; IN: /
; OUT: /
;
; destroys: eax
;
;-----------------------------------------------------------------
align 4
TCP_timer_1000ms:
; scan through all the active TCP sockets, decrementing active timers

	mov	eax, net_sockets
  .loop:
	mov	eax, [eax + SOCKET.NextPtr]
  .check_only:
	or	eax, eax
	jz	.exit

	cmp	[eax + SOCKET.Type], IP_PROTO_TCP
	jne	.loop

	cmp	[eax + TCP_SOCKET.t_timer], 0
	jne	.decrement_tcb
  ;;;;;;      cmp     [eax + TCP_SOCKET.wndsizeTimer], 0
	jne	.decrement_wnd
	jmp	.loop

  .decrement_tcb:
; decrement it, delete socket if TCB timer = 0 & socket in timewait state
	dec	[eax + TCP_SOCKET.t_timer]
	jnz	.loop

	cmp	[eax + TCP_SOCKET.t_state], TCB_TIMED_WAIT
	jne	.loop

	push	[eax + SOCKET.NextPtr]
	call	SOCKET_free
	pop	eax
	jmp	.check_only

  .decrement_wnd:
;;;;;;        dec     [eax + TCP_SOCKET.wndsizeTimer]

  .exit:
	ret



;----------------------
;
; TCP_500ms
;
;----------------------
align 4
TCP_500ms:

	add	[TCP_sequence_num], 64000

	ret



;----------------------
;
; TCP_10ms
;
;----------------------
align 4
TCP_10ms:

	; todo: decrease timers

	ret




;-----------------------------------------------------------------
;
; TCP_input:
;
;  IN:  [esp] = ptr to buffer
;       [esp+4] = buffer size
;       ebx = ptr to device struct
;       ecx = segment size
;       edx = ptr to TCP segment
;
;       esi = ipv4 source address
;       edi = ipv4 dest   address
;
;  OUT: /
;
;-----------------------------------------------------------------
align 4
TCP_input:

       DEBUGF  1,"TCP_input\n"

; Offset must be greater than or equal to the size of the standard TCP header (20) and less than or equal to the TCP length.

	movzx	eax, [edx + TCP_segment.DataOffset]
	and	eax, 0xf0
	shr	al , 2

	DEBUGF	1,"data offset: %u\n", eax

	cmp	eax, 20
	jl	.drop

	cmp	eax, ecx
	jg	.drop

;-------------------------------
; Now, re-calculate the checksum

	push	eax edx ebx

	push	edi
	push	esi
	mov	esi, edx
	call	TCP_checksum	; this destroys edx, ecx and esi (but not edi! :)

	pop	ebx edx eax

	cmp	[edx + TCP_segment.Checksum], 0
	jnz	.drop

	DEBUGF	1,"Checksum is correct\n"

;-----------------------------------------------------------------------------------------
; Check if this packet has a timestamp option (We do it here so we can process it quickly)

	cmp	eax, 20 + 12					; Timestamp option is 12 bytes
	jl	.no_timestamp
	je	.is_ok

	cmp	byte [edx + TCP_segment.Data + 12], 0		; end of option list
	jne	.no_timestamp

  .is_ok:
	test	[edx + TCP_segment.Flags], TH_SYN		; SYN flag must not be set
	jnz	.no_timestamp

	cmp	dword [edx + TCP_segment.Data], 0x0101080a	; Timestamp header
	jne	.no_timestamp

	DEBUGF	1,"timestamp ok\n"

	; TODO: Parse the options
	; TODO: Set a Bit in the TCP to tell all options are parsed

	ret

  .no_timestamp:

;-------------------------------------------
; Convert Big-endian values to little endian

	ntohld	[edx + TCP_segment.SequenceNumber]
	ntohld	[edx + TCP_segment.AckNumber]

	ntohlw	[edx + TCP_segment.Window]
	ntohlw	[edx + TCP_segment.UrgentPointer]

;------------------------------------------------------------
; Next thing to do is find the TCB (thus, the socket pointer)

; IP Packet TCP Destination Port = local Port
; (IP Packet SenderAddress = Remote IP)  OR  (Remote IP = 0)
; (IP Packet TCP Source Port = remote Port)  OR (remote Port = 0)

	mov	ebx, net_sockets

  .socket_loop:
	mov	ebx, [ebx + SOCKET.NextPtr]
	or	ebx, ebx
	jz	.drop_with_reset

	cmp	[ebx + SOCKET.Type], IP_PROTO_TCP
	jne	.socket_loop

	mov	ax, [edx + TCP_segment.DestinationPort]
	cmp	[ebx + TCP_SOCKET.LocalPort], ax
	jne	.socket_loop

	mov	eax, [ebx + IP_SOCKET.RemoteIP]
	cmp	eax, esi
	je	@f
	test	eax, eax
	jnz	.socket_loop
       @@:

	mov	ax, [ebx + TCP_SOCKET.RemotePort]
	cmp	[edx + TCP_segment.SourcePort] , ax
	je	.found_socket
	test	ax, ax
	jnz	.socket_loop
  .found_socket:
       DEBUGF 1,"Socket ptr: %x\n", ebx

; ebx now contains the pointer to the socket

;----------------------------
; Check if socket isnt closed

	cmp	[TCP_SOCKET.t_state], TCB_CLOSED
	je	.drop

;----------------
; Lock the socket

	add	ebx, SOCKET.lock ; TODO: figure out if we should lock now already
	call	wait_mutex
	sub	ebx, SOCKET.lock

;---------------------------------------
; unscale the window into a 32 bit value  ;;;;;;

	movzx	eax, [edx + TCP_segment.Window]
	xchg	al, ah

	test	[edx + TCP_segment.Flags], TH_SYN
	jnz	.no_syn

	mov	cl , [ebx + TCP_SOCKET.SND_SCALE]
	shl	eax, cl

  .no_syn:

;-----------------------------------
; Is this socket a listening socket?

; If so, create a new socket

	test	[ebx + SOCKET.options], SO_ACCEPTCON
	jz	.no_accept_conn


		; TODO: create a new socket


  .no_accept_conn:

;----------------------------
; Compute window scale factor


; TODO


;-------------------------------------
; Reset idle timer and keepalive timer


; TODO

;-----------------------------------------
; Process TCP options if not in LISTEN state

	test	[ebx + TCP_SOCKET.t_state], TCB_LISTEN
	jz	.dont_do_options

	call	TCP_do_options

  .dont_do_options:

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

	cmp	[TCP_SOCKET.t_state], TCB_ESTABLISHED
	jnz	.not_uni_xfer

	test	[TCP_segment.Flags], TH_SYN + TH_FIN + TH_RST + TH_URG
	jnz	.not_uni_xfer

	test	[TCP_segment.Flags], TH_ACK
	jz	.not_uni_xfer

	mov	eax, [edx + TCP_segment.SequenceNumber]
	cmp	eax, [ebx + TCP_SOCKET.RCV_NXT]
	jne	.not_uni_xfer

	movzx	eax, [edx + TCP_segment.Window] ;;;;;
	cmp	eax, [ebx + TCP_SOCKET.SND_WND]
	jne	.not_uni_xfer

	mov	eax, [ebx + TCP_SOCKET.SND_NXT]
	cmp	eax, [ebx + TCP_SOCKET.SND_MAX]
	jne	.not_uni_xfer

;-------------------------------------------------------------------------------
; If last ACK falls within this segment's sequence number, record the timestamp.

	; TODO: check if it has a timestamp




;---------------------------------------
; check if we are sender in the uni-xfer

; If the following 4 conditions are all true, this segment is a pure ACK.
;
; - The segment contains no data (ti_len is 0).

	movzx	eax, [edx + TCP_segment.DataOffset]
	and	eax, 11110000b
	shr	eax, 2
	sub	ecx, eax
	jnz	.not_sender

; - The acknowledgment field in the segment (ti_ack) is greater than the largest unacknowledged sequence number (snd_una).
;     Since this test is "greater than" and not "greater than or equal to," it is true only if some positive amount of data is acknowledged by the ACK.

	mov	eax, [edx + TCP_segment.AckNumber]
	cmp	eax, [ebx + TCP_SOCKET.SND_UNA]
	jle	.not_uni_xfer

; - The acknowledgment field in the segment (ti_ack) is less than or equal to the maximum sequence number sent (snd_max).

;        mov     eax, [edx + TCP_segment.Ack]
	cmp	eax, [ebx + TCP_SOCKET.SND_MAX]
	jg	.not_uni_xfer

; - The congestion window (snd_cwnd) is greater than or equal to the current send window (snd_wnd).
;     This test is true only if the window is fully open, that is, the connection is not in the middle of slow start or congestion avoidance.

	mov	eax, [ebx + TCP_SOCKET.SND_CWND]
	cmp	eax, [ebx + TCP_SOCKET.SND_WND]
	jl	.not_uni_xfer

	DEBUGF	1,"Header prediction: we are sender\n"

;---------------------------------
; Packet is a pure ACK, process it

; Update RTT estimators

; Delete acknowledged bytes from send buffer

; Stop retransmit timer

; Awaken waiting processes

; Generate more output




	jmp	.drop




;-------------------------------------------------
; maybe we are the receiver in the uni-xfer then..

  .not_sender:
; The amount of data in the segment (ti_len) is greater than 0 (data count is in ecx)


; The acknowledgment field (ti_ack) equals the largest unacknowledged sequence number. This means no data is acknowledged by this segment.
	mov	eax, [edx + TCP_segment.AckNumber]
	cmp	eax, [ebx + TCP_SOCKET.SND_UNA]
	jne	.not_uni_xfer

; The reassembly list of out-of-order segments for the connection is empty (seg_next equals tp).
;;;;
	jnz	.not_uni_xfer

; There is room in the receive buffer for the data in the segment.
;;;;
	jnz	.not_uni_xfer

;-------------------------------------
; Complete processing of received data

	DEBUGF	1,"header prediction: we are receiver\nreceiving %u bytes of data\n", ecx

; The next expected receive sequence number (rcv_nxt) is incremented by the number of bytes of data.

	add	[ebx + TCP_SOCKET.RCV_NXT], ecx

; Add the data to the socket buffer

; The receiving process is awakened (by sorwakeup).

; The delayed-ACK flag is set and the input processing is complete.

	jmp	.drop





;----------------------------------------------------
; Header prediction failed, doing it the slow way..

  .not_uni_xfer:

	DEBUGF	1,"Header prediction failed\n"

;------------------------
; calculate header length                               ;;;;; we already calculated this before!
	movzx	eax, [edx + TCP_segment.DataOffset]
	and	eax, 0xf0
	shr	eax, 2

; Update edx to point to data..
	add	edx, eax
; ..and ecx to give data size
	sub	ecx, eax

;------------------------------
; Calculate receive window size

	;;;;


;-------------------------
; TCP slow input procedure

	DEBUGF	1,"TCP slow input procedure\n"

	cmp	[eax + TCP_SOCKET.t_state], TCB_LISTEN
	je	.LISTEN

	cmp	[eax + TCP_SOCKET.t_state], TCB_SYN_SENT
	je	.SYN_SENT


;--------------------------------------------
; Protection Against Wrapped Sequence Numbers


; First, check timestamp if present

;;;; TODO

; Then, check if at least some bytes of data are within window

;;;; TODO

	jmp	.trim_then_step6

align 4
.LISTEN:

	DEBUGF	1,"TCP state: listen\n"

	test	[edx + TCP_segment.Flags], TH_RST
	jnz	.drop

	test	[edx + TCP_segment.Flags], TH_ACK
	jnz	.drop_with_reset

	test	[edx + TCP_segment.Flags], TH_SYN
	jz	.drop

	; TODO: check if it's a broadcast or multicast, and drop if so

;;; 28.6

	; create a new socket and fill in the nescessary variables

;; Exit if backlog queue is full
;        mov     ax, [ebx + TCP_SOCKET.backlog_cur]
;        cmp     ax, [ebx + TCP_SOCKET.backlog]
;        jae     .exit

; Allocate new socket
	call	SOCKET_alloc
     ;;;   jz      .fail

; Copy structure from current socket to new, (including lock!)
; We start at PID to reserve the socket num, and the 2 pointers at beginning of socket
	lea	esi, [edx + SOCKET.PID]
	lea	edi, [eax + SOCKET.PID]
	mov	ecx, (TCP_SOCKET.end - SOCKET.PID + 3)/4
	rep	movsd

;; Push pointer to new socket to queue
;        movzx   ecx, [ebx + TCP_SOCKET.backlog_cur]
;        inc     [ebx + TCP_SOCKET.backlog_cur]
;        mov     [ebx + TCP_SOCKET.end + ecx*4], eax

	mov	[eax + IP_SOCKET.RemoteIP], esi ; IP source address

	mov	cx, [edx + TCP_segment.SourcePort]
	mov	[eax + TCP_SOCKET.RemotePort], cx

	mov	ecx, [edx + TCP_segment.SequenceNumber]
	mov	[eax + TCP_SOCKET.IRS], ecx

	mov	ecx, [eax + TCP_SOCKET.ISS]
	mov	[eax + TCP_SOCKET.SND_NXT], ecx

	jmp	.trim_then_step6



align 4
.SYN_SENT:

	DEBUGF	1,"TCP state: syn_sent\n"

	test	[edx + TCP_segment.Flags], TH_ACK
	jz	@f

	mov	eax, [edx + TCP_segment.AckNumber]
	cmp	eax, [ebx + TCP_SOCKET.ISS]
	jle	.drop_with_reset

	mov	eax, [edx + TCP_segment.AckNumber]
	cmp	eax, [ebx + TCP_SOCKET.SND_MAX]
	jg	.drop_with_reset
       @@:


	test	[edx + TCP_segment.Flags], TH_RST
	jz	@f

	test	[edx + TCP_segment.Flags], TH_ACK
	jz	.drop

	;tp = tcp_drop(tp, ECONNREFUSED)

	jmp	.drop
       @@:

	test	[edx + TCP_segment.Flags], TH_SYN
	jz	.drop

; now, process received SYN in response to an active open
	test	[edx + TCP_segment.Flags], TH_ACK
	jz	@f

	mov	eax, [edx + TCP_segment.AckNumber]
	mov	[ebx + TCP_SOCKET.SND_UNA], eax

	mov	eax, [ebx + TCP_SOCKET.SND_UNA]
	cmp	eax, [ebx + TCP_SOCKET.SND_NXT]
	jle	@f
	mov	[ebx + TCP_SOCKET.SND_NXT], eax


; TODO: turn off retransmission timer

	mov	eax, [edx + TCP_segment.SequenceNumber]
	mov	[ebx + TCP_SOCKET.IRS], eax

; TODO: set socket state to connected

	mov	[ebx + TCP_SOCKET.t_state], TCB_ESTABLISHED

; TODO: check if we should scale the connection (567-572)
; TODO: update RTT estimators


       @@:

; We have received a syn but no ACK, so we are having a simultaneous open..
	mov	[ebx + TCP_SOCKET.t_state], TCB_SYN_RECEIVED

;-------------------------------------
; Common processing for receipt of SYN

  .trimthenstep6:

	inc	[edx + TCP_segment.SequenceNumber]

	cmp	cx, [ebx + TCP_SOCKET.RCV_WND]
	jle	@f

	movzx	eax, cx
	sub	ax, [ebx + TCP_SOCKET.RCV_WND]
	; TODO: 592
	mov	cx, [ebx + TCP_SOCKET.RCV_WND]
	; TODO...
       @@:
	;;;;;
	;;; jmp     .step6





align 4
.trim_then_step6:

	DEBUGF	1,"Trim, then step 6\n"

;----------------------------
; trim any data not in window

	mov	eax, [ebx + TCP_SOCKET.RCV_NXT]
	sub	eax, [edx + TCP_segment.SequenceNumber]

	test	eax, eax
	jz	.no_drop

	test	[edx + TCP_segment.Flags], TH_SYN
	jz	.no_drop

	and	[edx + TCP_segment.Flags], not (TH_SYN)
	inc	[edx + TCP_segment.SequenceNumber]

	cmp	[edx + TCP_segment.UrgentPointer], 1
	jl	@f

	dec	[edx + TCP_segment.UrgentPointer]

	jmp	.no_drop
       @@:

	and	[edx + TCP_segment.Flags], not (TH_URG)
	dec	eax

  .no_drop:

; eax holds number of bytes to drop


;----------------------------------
; Check for entire duplicate packet

	cmp	eax, ecx
	jge	.duplicate

	;;; TODO: figure 28.30

;;        inc     [TCP_segments_rx]

;;        add     dword [TCP_bytes_rx], ecx
;;        adc     dword [TCP_bytes_rx+4], 0

;------------------------
; Check for duplicate FIN

	test	[edx + TCP_segment.Flags], TH_FIN
	jz	@f
	inc	ecx
	cmp	eax, ecx
	dec	ecx
	jne	@f

	mov	eax, ecx
	and	[edx + TCP_segment.Flags], not TH_FIN
	;;; TODO: set ACKNOW flag

	jmp	.no_duplicate
       @@:

	; Handle the case when a bound socket connects to itself
	; Allow packets with a SYN and an ACKto continue with the processing


;-------------------------------------
; Generate duplicate ACK if nescessary

; This code also handles simultaneous half-open or self-connects

	test	eax, eax
	jnz	.drop_after_ack

	cmp	[edx + TCP_segment.Flags], TH_ACK
	jz	.drop_after_ack

  .duplicate:

;----------------------------------------
; Update statistics for duplicate packets

	;;; TODO

	;;; DROP the packet ??

  .no_duplicate:

;-----------------------------------------------
; Remove duplicate data and update urgent offset

	add	[edx + TCP_segment.SequenceNumber], eax

	;;; TODO

	sub	[edx + TCP_segment.UrgentPointer], ax
	jg	@f

	and	[edx + TCP_segment.Flags], not (TH_URG)
	mov	[edx + TCP_segment.UrgentPointer], 0
       @@:

;--------------------------------------------------
; Handle data that arrives after process terminates

	cmp	[ebx + SOCKET.PID], 0
	jge	@f

	cmp	[ebx + TCP_SOCKET.t_state], TCB_CLOSE_WAIT
	jle	@f

	test	ecx, ecx
	jz	@f

	;;; Close the socket
	;;; update stats

	jmp	.drop_with_reset

       @@:

;----------------------------------------
; Remove data beyond right edge of window

	mov	eax, [edx + TCP_segment.SequenceNumber]
	add	eax, ecx
	sub	eax, [ebx + TCP_SOCKET.RCV_NXT]
	sub	ax, [ebx + TCP_SOCKET.RCV_WND]

	; eax now holds the number of bytes to drop

	jle	.no_excess_data

	;;; TODO: update stats

	cmp	eax, ecx
	jl	.dont_drop_all

;;; TODO 700-736

  .dont_drop_all:

  .no_excess_data:


;-----------------
; Record timestamp

	;;; TODO 737-746

;------------------
; Process RST flags

	test	[edx + TCP_segment.Flags], TH_RST
	jz	.rst_skip

	mov	eax, [ebx + TCP_SOCKET.t_state]
	shl	eax, 2
	jmp	dword [eax + .rst_sw_list]

  .rst_sw_list:
	dd	.rst_skip	;TCB_CLOSED
	dd	.rst_skip	;TCB_LISTEN
	dd	.rst_skip	;TCB_SYN_SENT
	dd	.econnrefused	;TCB_SYN_RECEIVED
	dd	.econnreset	;TCB_ESTABLISHED
	dd	.econnreset	;TCB_CLOSE_WAIT
	dd	.econnreset	;TCB_FIN_WAIT_1
	dd	.rst_close	;TCB_CLOSING
	dd	.rst_close	;TCB_LAST_ACK
	dd	.econnreset	;TCB_FIN_WAIT_2
	dd	.rst_close	;TCB_TIMED_WAIT

  .econnrefused:

	;;; TODO: debug info

	jmp	.close

  .econnreset:

	;;; TODO: debug info
    .close:

	;;; update stats

  .rst_close:

	;;; Close the socket
	jmp	.drop

  .rst_skip:

;--------------------------------------
; handle SYN-full and ACK-less segments

	test	[edx + TCP_segment.Flags], TH_SYN
	jz	@f

	;;; tcp_drop ( ECONNRESET)
	jmp	.drop_with_reset

	test	[edx + TCP_segment.Flags], TH_ACK
	jz	.drop

;----------------
; Process the ACK

	cmp	[ebx + TCP_SOCKET.t_state], TCB_SYN_RECEIVED
	jg	.ack_dup
	jl	.ack_nodup

;        dd      .ack_nodup      ;TCB_CLOSED
;        dd      .ack_nodup      ;TCB_LISTEN
;        dd      .ack_nodup      ;TCB_SYN_SENT
;        dd      .ack_syn_rcvd   ;TCB_SYN_RECEIVED
;        dd      .ack_dup        ;TCB_ESTABLISHED
;        dd      .ack_dup        ;TCB_CLOSE_WAIT
;        dd      .ack_dup        ;TCB_FIN_WAIT_1
;        dd      .ack_dup        ;TCB_CLOSING
;        dd      .ack_dup        ;TCB_LAST_ACK
;        dd      .ack_dup        ;TCB_FIN_WAIT_2
;        dd      .ack_dup        ;TCB_TIMED_WAIT

	;;;;;

  .ack_dup:

	;;;;

  .ack_nodup:

	;;;; 887

;-------------------------------------------------
; If the congestion window was infalted to account
; for the other side's cached packets, retrace it

	;;;; 888 -  902


;------------------------------------------
; RTT measurements and retransmission timer

	;;;;; 903 - 926


;-------------------------------------------
; Open congestion window in response to ACKs

	;;;;


;------------------------------------------
; Remove acknowledged data from send buffer

	;;;; 943 - 956

;---------------------------------------
; Wake up process waiting on send buffer

	;;;;;

	mov	eax, [ebx + TCP_SOCKET.t_state]
	shl	eax, 2
	jmp	dword [eax + .ACK_sw_list]

  .ACK_sw_list:
	dd	.step6		;TCB_CLOSED
	dd	.step6		;TCB_LISTEN
	dd	.step6		;TCB_SYN_SENT
	dd	.step6		;TCB_SYN_RECEIVED
	dd	.step6		;TCB_ESTABLISHED
	dd	.step6		;TCB_CLOSE_WAIT
	dd	._963		;TCB_FIN_WAIT_1
	dd	._958		;TCB_CLOSING
	dd	._999		;TCB_LAST_ACK
	dd	.step6		;TCB_FIN_WAIT_2
	dd	._1010		;TCB_TIMED_WAIT


._963:


	jmp	.step6


._958:

	jmp	.step6

._999:

	jmp	.step6


._1010:

	jmp	.step6



align 4
.step6:

	DEBUGF	1,"step 6\n"

;--------------------------
; update window information

	test	[edx + TCP_segment.Flags], TH_ACK
	jz	.no_window_update

	mov	eax, [ebx + TCP_SOCKET.SND_WL1]
	cmp	eax, [edx + TCP_segment.SequenceNumber]

	;;;; 1021

;----------------------------------
; Keep track of pure window updates

	test	ecx, ecx
	jz	@f

	mov	eax, [ebx + TCP_SOCKET.SND_WL2]
	cmp	eax, [edx + TCP_segment.AckNumber]
	jne	@f

	;; mov eax, tiwin
	cmp	eax, [ebx + TCP_SOCKET.SND_WND]
	jle	@f

	;;; update stats

       @@:

	;; mov eax, incoming window
	cmp	eax, [ebx + TCP_SOCKET.max_sndwnd]
	jle	@f
	mov	[ebx + TCP_SOCKET.max_sndwnd], eax
       @@:
	mov	[ebx + TCP_SOCKET.SND_WND], eax

	mov	eax, [edx + TCP_segment.SequenceNumber]
	mov	[ebx + TCP_SOCKET.SND_WL1], eax

	mov	eax, [edx + TCP_segment.AckNumber]
	mov	[ebx + TCP_SOCKET.SND_WL2], eax

	;;; needoutput = 1

  .no_window_update:


;-----------------
; process URG flag

	test	[edx + TCP_segment.Flags], TH_URG
	jz	.not_urgent

	cmp	[edx + TCP_segment.UrgentPointer], 0
	jz	.not_urgent

	cmp	[ebx + TCP_SOCKET.t_state], TCB_TIMED_WAIT
	je	.not_urgent

; Ignore bogus urgent offsets

	;;; 1040-1050

	movzx	eax, [edx + TCP_segment.UrgentPointer]
	add	eax, [ebx + SOCKET.SO_RCV.SB_CC]
	cmp	eax, SOCKET_MAXDATA
	jle	.not_urgent

	mov	[edx + TCP_segment.UrgentPointer], 0
	and	[edx + TCP_segment.Flags], not (TH_URG)
	jmp	.do_data

  .not_urgent:

;--------------------------------------
; processing of received urgent pointer

	;;; 1051-1093

align 4
.do_data:

	DEBUGF	1,"Do data:\n"

	; process the data in the segment

	test	[edx + TCP_segment.Flags], TH_FIN
	jz	.process_fin

	test	[ebx + TCP_SOCKET.t_state], TCB_FIN_WAIT_1 ;;;;;
	jge	.dont_do_data

	DEBUGF	1,"Processing data in segment\n"

	;;; NOW, process the data

	jmp	.final_processing


  .dont_do_data:


;---------------
; FIN processing

  .process_fin:

	DEBUGF	1,"Processing FIN\n"

	mov	eax, [ebx + TCP_SOCKET.t_state]
	shl	eax, 2
	jmp	dword [eax + .FIN_sw_list]

  .FIN_sw_list:
	dd	.no_fin 	;TCB_CLOSED
	dd	.no_fin 	;TCB_LISTEN
	dd	.no_fin 	;TCB_SYN_SENT
	dd	._1131		;TCB_SYN_RECEIVED
	dd	._1131		;TCB_ESTABLISHED
	dd	.no_fin 	;TCB_CLOSE_WAIT
	dd	._1139		;TCB_FIN_WAIT_1
	dd	.no_fin 	;TCB_CLOSING
	dd	.no_fin 	;TCB_LAST_ACK
	dd	._1147		;TCB_FIN_WAIT_2
	dd	._1156		;TCB_TIMED_WAIT



  ._1131:

  ._1139:

  ._1147:

  ._1156:


  .no_fin:

;-----------------
; Final processing

  .final_processing:

	DEBUGF	1,"Final processing\n"

	;;; if debug enabled, output packet

	;test    ;;;needoutput = 1
	;jnz     .outputnow

	test	[ebx + TCP_SOCKET.t_flags], TF_ACKNOW
	jz	.ret

  .outputnow:
	call	TCP_output

  .ret:
	mov	[ebx + SOCKET.lock], 0

	call	kernel_free
	ret	4

;------------------------------------------
; Generate an ACK, droping incoming segment

align 4
.drop_after_ack:

	DEBUGF	1,"Drop after ACK\n"

	test	[edx + TCP_segment.Flags], TH_RST
	jnz	.drop

	and	[ebx + TCP_SOCKET.t_flags], TF_ACKNOW

	call	TCP_output

	mov	[ebx + SOCKET.lock], 0

	call	kernel_free
	ret	4


;-------------------------------------------
; Generate an RST, dropping incoming segment

align 4
.drop_with_reset:

	DEBUGF	1,"Drop with reset\n"

	test	[edx + TCP_segment.Flags], TH_RST
	jnz	.drop

	;;; if its a multicast/broadcast, also drop

	test	[edx + TCP_segment.Flags], TH_ACK
	jnz	.respond_ack

	test	[edx + TCP_segment.Flags], TH_SYN
	jnz	.respond_syn

	mov	[ebx + SOCKET.lock], 0

	call	kernel_free
	ret	4

  .respond_ack:

	;;;;

	call	TCP_respond

	jmp	.destroy_new_socket


  .respond_syn:

	;;;;

	call	TCP_respond

	jmp	.destroy_new_socket

;-----
; Drop

align 4
.drop:

	DEBUGF	1,"Dropping packet\n"

	;;;; If debugging options are enabled, output the packet somwhere

  .destroy_new_socket:

	;;;; kill the newly created socket

	mov	[ebx + SOCKET.lock], 0

	call	kernel_free
	ret	4



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



;---------------------
;
; TCP_do_options
;
;-------------------

align 4
TCP_do_options:

	DEBUGF	1,"TCP_do_options\n"

	push	eax
	sub	eax, 20
	jz	.no_options

	lea	esi, [edx + TCP_segment.Data]


;-------------------------------------------
; Begin the loop by checking for EOL and NOP

  .loop:

	cmp	byte [esi], TCP_OPT_EOL 	; end of option list?
	jz	.no_options

	cmp	byte [esi], TCP_OPT_NOP 	; nop ?
      ;;;  cmove   edi, 1                          ; if so, set option size to 1
	jz	.continue			; and continue scanning

;------------------
; We have an option

	movzx	edi, byte [esi + 1]		; get the length of this option in edi


;--------------------------------------
; Check for Maximum segment size option

	cmp	byte [esi], TCP_OPT_MAXSEG
	jne	.no_maxseg

	cmp	edi, 4	; option length
	jne	.continue

	test	[edx + TCP_segment.Flags], TH_SYN
	jz	.continue

	; Now parse the option...

	jmp	.continue

  .no_maxseg:

;------------------------
; Check for Window option

	cmp	byte [esi], TCP_OPT_WINDOW
	jne	.no_window

	cmp	edi, 3 ; option length
	jne	.continue

	test	[edx + TCP_segment.Flags], TH_SYN
	jz	.continue

	; ...

	jmp	.continue

  .no_window:

;---------------------------
; Check for Timestamp option

	cmp	byte [esi], TCP_OPT_TIMESTAMP
	jne	.no_timestamp

	cmp	edi, 10 ; option length
	jne	.continue

	; ...


	jmp	.continue

  .no_timestamp:

;----------------------------------
; Future options may be placed here




;------------------------------
; Continue scanning for options

  .continue:
	add	esi, edi
	sub	eax, edi
	jg	.loop

  .no_options:

	pop	eax

	ret




;---------------------------
;
; TCP_pull_out_of_band
;
; IN:  eax =
;      ebx = socket ptr
;      edx = tcp packet ptr
;
; OUT: /
;
;---------------------------

align 4
TCP_pull_out_of_band:

	DEBUGF	1,"TCP_pull_out_of_band\n"

	;;;; 1282-1305

	ret





;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;





;-----------------------------------------------------------------
;
; TCP_output
;
; IN: eax = socket pointer
;;     esi = ptr to data
;;     ecx = number of data bytes
;
; OUT: /
;
;-----------------------------------------------------------------
align 4
TCP_output:

	DEBUGF 1,"TCP_output, socket: %x\n", eax

; We'll detect the length of the data to be transmitted, and flags to be used
; If there is some data, or any critical controls to send (SYN / RST), then transmit
; Otherwise, investigate further

	mov	ebx, [eax + TCP_SOCKET.SND_MAX]
	cmp	ebx, [eax + TCP_SOCKET.SND_UNA]
	jne	.not_idle

	mov	ebx, [eax + TCP_SOCKET.t_idle]
	cmp	ebx, [eax + TCP_SOCKET.t_rxtcur]
	jle	.not_idle

; We have been idle for a while and no ACKS are expected to clock out any data we send..
; Slow start to get ack "clock" running again.

	mov	ebx, [eax + TCP_SOCKET.t_maxseg]
	mov	[eax + TCP_SOCKET.SND_CWND], ebx

  .not_idle:
  .again:
	mov	ebx, [eax + TCP_SOCKET.SND_NXT] 	; calculate offset
	sub	ebx, [eax + TCP_SOCKET.SND_UNA] 	;

	mov	ecx, [eax + TCP_SOCKET.SND_WND] 	; determine window
	cmp	ecx, [eax + TCP_SOCKET.SND_CWND]	;
	jl	@f					;
	mov	ecx, [eax + TCP_SOCKET.SND_CWND]	;
       @@:						;

	call	TCP_outflags

; If in persist timeout with window of 0, send 1 byte.
; Otherwise, if window is small but nonzero, and timer expired,
; we will send what we can and go to transmit state

	test	[eax + TCP_SOCKET.t_force], -1
	jz	.no_persist_timeout

	test	ecx, ecx
	jnz	.no_zero_window

	cmp	ebx, [eax + SOCKET.SO_SND.SB_CC]
	jge	@f

	and	dl, not (TH_FIN)	  ; clear the FIN flag    ??? how can it be set before?

       @@:
	inc	ecx
	jmp	.no_persist_timeout

  .no_zero_window:

;;;        mov     [eax + TCP_SOCKET.t_timer....TCPT_PERSIST], 0
	mov	[eax + TCP_SOCKET.t_rxtshift], 0

  .no_persist_timeout:

;;;106

	mov	esi, [eax + SOCKET.SO_SND.SB_CC]
	cmp	esi, ecx
	jl	@f
	mov	esi, ecx
       @@:
	sub	esi, ebx

	cmp	esi, -1
	jne	.not_minus_one

; If FIN has been set, but not ACKed, and we havent been called to retransmit,
; len (esi) will be -1
; Otherwise, window shrank after we sent into it.
; If window shrank to 0, cancel pending retransmit and pull SND_NXT back to (closed) window
; We will enter persist state below.
; If window didn't close completely, just wait for an ACK

	xor	esi, esi

	test	ecx, ecx
	jnz	@f

;;;        mov     [eax + TCP_SOCKET.t_timer..TCPT_REXMT], 0

	push	[eax + TCP_SOCKET.SND_UNA]
	pop	[eax + TCP_SOCKET.SND_NXT]
       @@:

  .not_minus_one:

;;; 124

	cmp	esi, [eax + TCP_SOCKET.t_maxseg]
	jle	@f

	mov	esi, [eax + TCP_SOCKET.t_maxseg]
	;sendalot = 1

       @@:

;;; 128

	mov	edi, [eax + TCP_SOCKET.SND_NXT]
	add	edi, esi	; len
	sub	edi, [eax + TCP_SOCKET.SND_UNA]
	add	edi, [eax + SOCKET.SO_SND.SB_CC]
	cmp	edi, 0
	jle	@f

	and	dl, not (TH_FIN)	  ; clear the FIN flag

       @@:


;;;; 130 TODO: set window (ecx) to space in send buffer


;------------------------------
; Sender silly window avoidance

	test	esi, esi
	jz	.zero_length


	cmp	esi, [eax + TCP_SOCKET.t_maxseg]
	je	.send

;;; TODO: 144-145

	test	[eax + TCP_SOCKET.t_force], -1
	jnz	.send

;;; TODO: 149..152

  .zero_length:


;----------------------------------------
; Check if a window update should be sent

	cmp	ecx, 0	; window
	jle	.no_window

;;; TODO 154-172

  .no_window:

;--------------------------
; Should a segment be sent?

	test	[ebx + TCP_SOCKET.t_flags], TF_ACKNOW
	jnz	.send

	test	dl, TH_SYN + TH_RST
	jnz	.send

	mov	eax, [ebx + TCP_SOCKET.SND_UP]
	cmp	eax, [ebx + TCP_SOCKET.SND_UNA]
	jg	.send

	test	dl, TH_FIN
	jz	.enter_persist

	test	[ebx + TCP_SOCKET.t_flags], TF_SENTFIN
	jnz	.send

	mov	eax, [ebx + TCP_SOCKET.SND_NXT]
	cmp	eax, [ebx + TCP_SOCKET.SND_UNA]
	je	.send

;--------------------
; Enter persist state

  .enter_persist:

	DEBUGF	1,"Entering pesist state\n"



;--------------------------------------
; No reason to send a segment, just ret

	DEBUGF	1,"No reason to send a segment\n"

	ret





;-----------------------------------------------
;
; Send a segment
;
; ebx = socket pointer
;  dl = flags
;
;-----------------------------------------------

.send:

	DEBUGF	1,"Preparing to send a segment\n"

	xor	edi, edi	; edi will contain the number of header option bytes

;------------------------------------
; Send options with first SYN segment

	test	dl, TH_SYN
	jz	.no_options

	mov	eax, [ebx + TCP_SOCKET.ISS]
	mov	[ebx + TCP_SOCKET.SND_NXT], eax

	test	[ebx + TCP_SOCKET.t_flags], TF_NOOPT
	jnz	.no_options

	mov	eax, TCP_OPT_MAXSEG shl 24 + 4 shl 16
	mov	 ax, 1280 ;;;;;;
	bswap	eax
	push	eax

	mov	di, 4

	test	[ebx + TCP_SOCKET.t_flags], TF_REQ_SCALE
	jz	.no_syn

	test	dl, TH_ACK
	jnz	.scale_opt

	test	[ebx + TCP_SOCKET.t_flags], TF_RCVD_SCALE
	jz	.no_syn

  .scale_opt:

	mov	eax, TCP_OPT_WINDOW shl 24 + 4 shl 16 + TCP_OPT_NOP
	mov	 ah, byte [ebx + TCP_SOCKET.request_r_scale]
	bswap	eax
	push	eax

	add	di, 4

  .no_syn:

;------------------------------------
; Make the timestamp option if needed

	test	[ebx + TCP_SOCKET.t_flags], TF_REQ_TSTMP
	jz	.no_timestamp

	test	dl, TH_RST
	jnz	.no_timestamp

	test	dl, TH_ACK
	jz	.timestamp

	test	[ebx + TCP_SOCKET.t_flags], TF_RCVD_TSTMP
	jz	.no_timestamp

  .timestamp:

	DEBUGF	1,"Creating a timestamp\n"

	push	dword (TCP_OPT_TIMESTAMP shl 8 + 10 + TCP_OPT_NOP shl 16 + TCP_OPT_NOP shl 24)
	pushw	0
	mov	eax, [timer_ticks]
	bswap	eax
	push	eax

	add	di, 10

  .no_timestamp:

	;; TODO: check if we dont exceed the max segment size

  .no_options:
	add	edi, TCP_segment.Data

;-----------------------------------
; Check if we have some data to send

     ;;;   mov     ecx, [huppeldepup]

	test	ecx, ecx
	jz	.no_data

	;;; 278-316

	jmp	.header

  .no_data:

	;;; 317-338


;----------

	push	di dx ebx

	add	ecx, edi	; total TCP segment size

	mov	eax, [ebx + IP_SOCKET.RemoteIP]
	mov	ebx, [ebx + IP_SOCKET.LocalIP]
	mov	di , IP_PROTO_TCP
	call	IPv4_create_packet

;;;;        jz      .fail

	push	edx eax
	call	NET_send
	ret

;----------------


;-------------------------------
; Now, create the 20-byte header

  .header:

;-----------------------
; Fill in the TCP header
	pop	esi

	push	[esi + TCP_SOCKET.SND_NXT]
	rol	word [esp], 8
	rol	dword [esp], 16
	pop	[edi + TCP_segment.SequenceNumber]

	push	[esi + TCP_SOCKET.RCV_NXT]
	rol	word [esp], 8
	rol	dword [esp], 16
	pop	[edi + TCP_segment.AckNumber]

	push	[esi + TCP_SOCKET.LocalPort]
	rol	word [esp], 8
	pop	[edi + TCP_segment.SourcePort]

	push	[esi + TCP_SOCKET.RemotePort]
	rol	word [esp], 8
	pop	[edi + TCP_segment.DestinationPort]


	mov	[edi + TCP_segment.Window], 0x0005
	; 1280 bytes
	mov	[edi + TCP_segment.UrgentPointer], 0

	mov	[edi + TCP_segment.DataOffset], 0x50

	mov	[edi + TCP_segment.Flags], cl

	mov	[edi + TCP_segment.Checksum], 0

;-----


;--------------
; Copy the data

	pop	esi
	push	edi
	add	edi, TCP_segment.Data  ;;
	sub	ecx, TCP_segment.Data  ;;;

	shr	ecx, 1
	jnc	.nb
	movsb
  .nb:
	shr	ecx, 1
	jnc	.nw
	movsw
  .nw:
	test	ecx, ecx
	jz	.nd
	rep	movsd
  .nd:
	pop	edi

;--------------------
; Create the checksum

	push	[ebx + IP_SOCKET.LocalIP]
	push	[ebx + IP_SOCKET.RemoteIP]
	call	TCP_checksum

;----------------
; Send the packet

	;;;;;


	DEBUGF 1,"Sending TCP Packet to device %x\n", ebx
	mov	esi, [ebx + ETH_DEVICE.transmit]

	ret



;-------------------------
;
; TCP_outflags
;
;  IN:  eax = socket ptr
;
;  OUT: edx = flags
;
;-------------------------
align 4
TCP_outflags:

	mov	edx, [eax + TCP_SOCKET.t_state]
	movzx	edx, byte [edx + .flaglist]

	DEBUGF	1,"TCP_outflags, socket: %x, flags: %x\n", eax, dl

	ret

  .flaglist:

	db	TH_RST + TH_ACK 	; TCB_CLOSED
	db	0			; TCB_LISTEN
	db	TH_SYN			; TCB_SYN_SENT
	db	TH_SYN + TH_ACK 	; TCB_SYN_RECEIVED
	db		 TH_ACK 	; TCB_ESTABLISHED
	db		 TH_ACK 	; TCB_CLOSE_WAIT
	db	TH_SYN + TH_ACK 	; TCB_FIN_WAIT_1
	db	TH_SYN + TH_ACK 	; TCB_CLOSING
	db	TH_SYN + TH_ACK 	; TCB_LAST_ACK
	db		 TH_ACK 	; TCB_FIN_WAIT_2
	db		 TH_ACK 	; TCB_TIMED_WAIT


;-------------------------
;
; TCP_drop
;
;  IN:  eax = socket ptr
;
;  OUT: /
;
;-------------------------
align 4
TCP_drop:

	DEBUGF	1,"TCP_drop\n"

;        cmp     [eax + TCP_SOCKET.t_state], TCB_SYN_RECEIVED
;        jl      .no_syn_received

	mov	[eax + TCP_SOCKET.t_state], TCB_CLOSED

	call	TCP_output

;  .no_syn_received:

	ret





;---------------------------------------
;
; TCP_respond
;
; The easy way to send a RST/ACK segment
;
;  IN:  eax = socket ptr
;
;  OUT: /
;
;---------------------------------------
align 4
TCP_respond:

	DEBUGF	1,"TCP_respond\n"

	ret



;-----------------------------------------------------------------
;
; TCP_checksum
;
; This is the fast procedure to create or check a UDP header
;  - To create a new checksum, the checksum field must be set to 0 before computation
;  - To check an existing checksum, leave the checksum as is,
;     and it will be 0 after this procedure, if it was correct
;
;  IN:  push source ip
;       push dest ip
;
;       esi = packet ptr
;
;  OUT: checksum is filled in in packet! (but also in dx)
;
;-----------------------------------------------------------------
align 4
TCP_checksum:

;-------------
; Pseudoheader

	; protocol type
	mov	edx, IP_PROTO_TCP		; NO shl 8 here ! (it took me ages to figure this one out)

	; source address
	add	dl, [esp+1+4]
	adc	dh, [esp+0+4]
	adc	dl, [esp+3+4]
	adc	dh, [esp+2+4]

	; destination address
	adc	dl, [esp+1+8]
	adc	dh, [esp+0+8]
	adc	dl, [esp+3+8]
	adc	dh, [esp+2+8]

	; size
	adc	dl, cl
	adc	dh, ch

;---------------------
; Real header and data

	push	esi
	call	checksum_1
	call	checksum_2
	pop	esi

	neg	[esi+UDP_Packet.Checksum]	    ; zero will stay zero so we just get the checksum
	add	[esi+UDP_Packet.Checksum], dx	    ;  , else we will get (new checksum - old checksum) in the end, wich should be 0 :)

	ret	8			; Remove the IPs from stack




;---------------------------------------------------------------------------
;
; TCP_API
;
; This function is called by system function 75
;
; IN:  subfunction number in bl
;      device number in bh
;      ecx, edx, .. depends on subfunction
;
; OUT:
;
;---------------------------------------------------------------------------
align 4
TCP_API:

	movzx	eax, bh
	shl	eax, 2

	test	bl, bl
	jz	.packets_tx	; 0
	dec	bl
	jz	.packets_rx	; 1

.error:
	mov	eax, -1
	ret

.packets_tx:
	add	eax, TCP_segments_tx
	mov	eax, [eax]
	ret

.packets_rx:
	add	eax, TCP_segments_rx
	mov	eax, [eax]
	ret
