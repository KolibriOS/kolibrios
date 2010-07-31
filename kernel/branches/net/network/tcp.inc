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

; Fundamental timer values
TCP_time_MSL		equ 47		; max segment lifetime (30s)
TCP_time_re_min 	equ 2		; min retransmission (1,28s)
TCP_time_re_max 	equ 100 	; max retransmission (64s)
TCP_time_pers_min	equ 8		; min persist (5,12s)
TCP_time_pers_max	equ 94		; max persist (60,16s)
TCP_time_keep_init	equ 118 	; connectione stablishment (75,52s)
TCP_time_keep_idle	equ 4608	; idle time before 1st probe (2h)
TCP_time_keep_interval	equ 118 	; between probes when no response (75,52s)
TCP_time_rtt_default	equ 5		; default Round Trip Time (3,2s)

; timer constants
TCP_max_rxtshift	equ 12		; max retransmissions waiting for ACK
TCP_max_keepcnt 	equ 8		; max keepalive probes

;
TCP_max_winshift	equ 14
TCP_max_win		equ 65535

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
;-----------------------------------------------------------------
macro	TCP_init {

	xor	eax, eax
	mov	edi, TCP_segments_tx
	mov	ecx, (6*IP_MAX_INTERFACES)
	rep	stosd

	pseudo_random	eax
	mov	[TCP_sequence_num], eax

}


;----------------------
;
;
;----------------------
macro	TCP_timer_160ms {

local	.loop
local	.exit

	mov	eax, net_sockets
  .loop:
	mov	eax, [eax + SOCKET.NextPtr]
	or	eax, eax
	jz	.exit

	cmp	[eax + SOCKET.Protocol], IP_PROTO_TCP		;;; We should also check if family is AF_INET
	jne	.loop

	dec	[eax + TCP_SOCKET.timer_ack]
	jnz	.loop

	DEBUGF	1,"TCP ack for socket %x expired, time to piggyback!\n", eax

	push	eax
	call	TCP_respond_socket
	pop	eax

	jmp	.loop

  .exit:

}


;-----------------------------------------------------------------
;
;
;-----------------------------------------------------------------
macro	TCP_timer_640ms {

local	.loop
local	.exit

; Update TCP sequence number

	add	[TCP_sequence_num], 64000

; scan through all the active TCP sockets, decrementing ALL timers
; timers do not have the chance to wrap because of the keepalive timer will kill the socket when it expires

	mov	eax, net_sockets
  .loop:
	mov	eax, [eax + SOCKET.NextPtr]
  .check_only:
	or	eax, eax
	jz	.exit

	cmp	[eax + SOCKET.Protocol], IP_PROTO_TCP	  ;;; We should also check if family is AF_INET
	jne	.loop

	inc	[eax + TCP_SOCKET.t_idle]
	dec	[eax + TCP_SOCKET.timer_retransmission]
	jnz	.check_more2

	DEBUGF	1,"socket %x: Retransmission timer expired\n", eax

	push	eax
	call	TCP_output
	pop	eax

  .check_more2:
	dec	[eax + TCP_SOCKET.timer_keepalive]
	jnz	.check_more3

	DEBUGF	1,"socket %x: Keepalive expired\n", eax

	;;; TODO: check socket state and handle accordingly

  .check_more3:
	dec	[eax + TCP_SOCKET.timer_timed_wait]
	jnz	.check_more5

	DEBUGF	1,"socket %x: 2MSL timer expired\n", eax

  .check_more5:
	dec	[eax + TCP_SOCKET.timer_persist]
	jnz	.loop

	DEBUGF	1,"socket %x: persist timer expired\n", eax

	jmp	.loop
  .exit:
}




macro	TCP_checksum IP1, IP2 {

;-------------
; Pseudoheader

	; protocol type
	mov	edx, IP_PROTO_TCP

	; source address
	add	dl, byte [IP1+1]
	adc	dh, byte [IP1+0]
	adc	dl, byte [IP1+3]
	adc	dh, byte [IP1+2]

	; destination address
	adc	dl, byte [IP2+1]
	adc	dh, byte [IP2+0]
	adc	dl, byte [IP2+3]
	adc	dh, byte [IP2+2]

	; size
	adc	dl, cl
	adc	dh, ch

;---------------------
; Real header and data

	push	esi
	call	checksum_1
	call	checksum_2
	pop	esi

}	; returns in dx only


macro	TCP_sendseqinit ptr {

	push	edi			;;;; i dont like this static use of edi
	mov	edi, [ptr + TCP_SOCKET.ISS]
	mov	[ptr + TCP_SOCKET.SND_UP], edi
	mov	[ptr + TCP_SOCKET.SND_MAX], edi
	mov	[ptr + TCP_SOCKET.SND_NXT], edi
	mov	[ptr + TCP_SOCKET.SND_UNA], edi
	pop	edi

}

macro	TCP_rcvseqinit ptr {

	push	edi
	mov	edi, [ptr + TCP_SOCKET.IRS]
	inc	edi
	mov	[ptr + TCP_SOCKET.RCV_NXT], edi
	mov	[ptr + TCP_SOCKET.RCV_ADV], edi
	pop	edi

}



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

       DEBUGF  1,"TCP_input size=%u\n", ecx
; Offset must be greater than or equal to the size of the standard TCP header (20) and less than or equal to the TCP length.

	movzx	eax, [edx + TCP_segment.DataOffset]
	and	eax, 0xf0
	shr	al, 2

	DEBUGF	1,"headersize=%u\n", eax

	cmp	eax, 20
	jl	.drop

;-------------------------------
; Now, re-calculate the checksum

	push	eax ecx edx
	pushw	[edx + TCP_segment.Checksum]
	mov	[edx + TCP_segment.Checksum], 0
	push	esi edi
	mov	esi, edx
	TCP_checksum (esp), (esp+4)
	pop	esi edi ; yes, swap them (we dont need dest addr)
	pop	cx	; previous checksum
	cmp	cx, dx
	pop	edx ecx esi
	jnz	.drop

	DEBUGF	1,"Checksum is correct\n"

	sub	ecx, esi	; update packet size
	jl	.drop
	DEBUGF	1,"we got %u bytes of data\n", ecx

;-----------------------------------------------------------------------------------------
; Check if this packet has a timestamp option (We do it here so we can process it quickly)

	cmp	esi, 20 + 12					; Timestamp option is 12 bytes
	jl	.no_timestamp
	je	.is_ok

	cmp	byte [edx + TCP_segment.Data + 12], TCP_OPT_EOL ; end of option list
	jne	.no_timestamp

  .is_ok:
	test	[edx + TCP_segment.Flags], TH_SYN		; SYN flag must not be set
	jnz	.no_timestamp

	cmp	dword [edx + TCP_segment.Data], 0x0101080a	; Timestamp header
	jne	.no_timestamp

	DEBUGF	1,"timestamp ok\n"

	; TODO: Parse the option
	; TODO: Set a Bit in the TCP to tell all options are parsed

  .no_timestamp:

;-------------------------------------------
; Convert Big-endian values to little endian

	ntohd	[edx + TCP_segment.SequenceNumber]
	ntohd	[edx + TCP_segment.AckNumber]

	ntohw	[edx + TCP_segment.Window]
	ntohw	[edx + TCP_segment.UrgentPointer]
	ntohw	[edx + TCP_segment.SourcePort]
	ntohw	[edx + TCP_segment.DestinationPort]

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

	cmp	[ebx + SOCKET.Domain], AF_INET4
	jne	.socket_loop

	cmp	[ebx + SOCKET.Protocol], IP_PROTO_TCP
	jne	.socket_loop

	mov	ax, [edx + TCP_segment.DestinationPort]
	cmp	[ebx + TCP_SOCKET.LocalPort], ax
	jne	.socket_loop

	mov	eax, [ebx + IP_SOCKET.RemoteIP]
	cmp	eax, edi			; edi is source ip from packet
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
	DEBUGF	1,"Socket ptr: %x\n", ebx

; ebx now contains the pointer to the socket

;----------------------------
; Check if socket isnt closed

	cmp	[ebx + TCP_SOCKET.t_state], TCB_CLOSED
	je	.drop

;----------------
; Lock the socket

;;        add     ebx, SOCKET.lock ; TODO: figure out if we should lock now already
;;        call    wait_mutex
;;        sub     ebx, SOCKET.lock

	DEBUGF	1,"Socket locked\n"

;---------------------------------------
; unscale the window into a 32 bit value

	movzx	eax, [edx + TCP_segment.Window]
	push	ecx
	mov	cl, [ebx + TCP_SOCKET.SND_SCALE]
	shl	eax, cl
	mov	dword [edx + TCP_segment.Window], eax	; word after window is checksum, we dont need checksum anymore
	pop	ecx

;-----------------------------------
; Is this socket a listening socket?

	test	[ebx + SOCKET.options], SO_ACCEPTCON
	jz	.no_listening_socket

	call	SOCKET_fork
	jz	.drop

	push	[edx + TCP_segment.DestinationPort]
	pop	[eax + TCP_SOCKET.LocalPort]

	push	[edx - IPv4_Packet.DataOrOptional + IPv4_Packet.DestinationAddress]	;;; FIXME
	pop	[eax + IP_SOCKET.LocalIP]

	push	[edx - IPv4_Packet.DataOrOptional + IPv4_Packet.SourceAddress]	   ;;; FIXME
	pop	[eax + IP_SOCKET.RemoteIP]

	mov	[eax + TCP_SOCKET.t_state], TCB_LISTEN

	jmp	.not_uni_xfer

  .no_listening_socket:

;-------------------------------------
; Reset idle timer and keepalive timer

	mov	[ebx + TCP_SOCKET.t_idle], 0
	mov	[ebx + TCP_SOCKET.timer_keepalive], TCP_time_keep_interval

;--------------------
; Process TCP options

	cmp	esi, 20 			; esi is headersize
	je	.no_options

	DEBUGF	1,"Segment has options\n"

	cmp	[ebx + TCP_SOCKET.t_state], TCB_LISTEN		; no options when in listen state
	jz	.not_uni_xfer					; also no header prediction

	lea	edi, [edx + TCP_segment.Data]
	lea	eax, [edx + esi]

  .opt_loop:
	cmp	edi, eax
	jge	.no_options

	cmp	byte [edi], TCP_OPT_EOL 	; end of option list?
	jz	.no_options

	cmp	byte [edi], TCP_OPT_NOP 	; nop ?
	jz	.opt_nop

	cmp	byte [edi], TCP_OPT_MAXSEG
	je	.opt_maxseg

	cmp	byte [edi], TCP_OPT_WINDOW
	je	.opt_window

	cmp	byte [edi], TCP_OPT_TIMESTAMP
	je	.opt_timestamp

	jmp	.no_options	; If we reach here, some unknown options were received, skip them all!

  .opt_nop:
	inc	edi
	jmp	.opt_loop

  .opt_maxseg:
	cmp	byte [edi+1], 4
	jne	.no_options		; error occured, ignore all options!

	test	[edx + TCP_segment.Flags], TH_SYN
	jz	@f

	movzx	eax, word[edi+2]
	rol	ax, 8
	DEBUGF	1,"Maxseg: %u\n", ax

	mov	[ebx + TCP_SOCKET.t_maxseg], eax

       @@:
	add	edi, 4
	jmp	.opt_loop


  .opt_window:
	cmp	byte [edi+1], 3
	jne	.no_options

	test	[edx + TCP_segment.Flags], TH_SYN
	jz	@f

	DEBUGF	1,"Got window option\n"

	;;;;;
       @@:
	add	edi, 3
	jmp	.opt_loop


  .opt_timestamp:
	cmp	byte [edi+1], 10
	jne	.no_options

	DEBUGF	1,"Got timestamp option\n"

	;;;;;

	add	edi, 10
	jmp	.opt_loop

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

	cmp	[ebx + TCP_SOCKET.t_state], TCB_ESTABLISHED
	jnz	.not_uni_xfer

	test	[edx + TCP_segment.Flags], TH_SYN + TH_FIN + TH_RST + TH_URG
	jnz	.not_uni_xfer

	test	[edx + TCP_segment.Flags], TH_ACK
	jz	.not_uni_xfer

	mov	eax, [edx + TCP_segment.SequenceNumber]
	cmp	eax, [ebx + TCP_SOCKET.RCV_NXT]
	jne	.not_uni_xfer

	mov	eax, dword [edx + TCP_segment.Window]
	cmp	eax, [ebx + TCP_SOCKET.SND_WND]
	jne	.not_uni_xfer

	mov	eax, [ebx + TCP_SOCKET.SND_NXT]
	cmp	eax, [ebx + TCP_SOCKET.SND_MAX]
	jne	.not_uni_xfer

;---------------------------------------
; check if we are sender in the uni-xfer

; If the following 4 conditions are all true, this segment is a pure ACK.
;
; - The segment contains no data.
	test	ecx, ecx
	jnz	.not_sender

; - The congestion window is greater than or equal to the current send window.
;     This test is true only if the window is fully open, that is, the connection is not in the middle of slow start or congestion avoidance.
	mov	eax, [ebx + TCP_SOCKET.SND_CWND]
	cmp	eax, [ebx + TCP_SOCKET.SND_WND]
	jl	.not_uni_xfer

; - The acknowledgment field in the segment is less than or equal to the maximum sequence number sent.
	mov	eax, [edx + TCP_segment.AckNumber]
	cmp	eax, [ebx + TCP_SOCKET.SND_MAX]
	jg	.not_uni_xfer

; - The acknowledgment field in the segment is greater than the largest unacknowledged sequence number.
	sub	eax, [ebx + TCP_SOCKET.SND_UNA]
	jle	.not_uni_xfer

	DEBUGF	1,"Header prediction: we are sender\n"

;---------------------------------
; Packet is a pure ACK, process it

; Update RTT estimators

; Delete acknowledged bytes from send buffer
	mov	ecx, eax
	lea	eax, [ebx + STREAM_SOCKET.snd]
	call	SOCKET_ring_free

; update window pointers
	mov	eax, [edx + TCP_segment.AckNumber]
	dec	eax
	mov	[ebx + TCP_SOCKET.SND_WL1], eax

; Stop retransmit timer
	mov	[ebx + TCP_SOCKET.timer_ack], 0

; Awaken waiting processes
	mov	eax, ebx
	call	SOCKET_notify_owner

;; Generate more output
;;        mov     eax, ebx
;;        call    TCP_output
;;
;;        jmp     .drop
	jmp	.step6

;-------------------------------------------------
; maybe we are the receiver in the uni-xfer then..

  .not_sender:
; - The amount of data in the segment is greater than 0 (data count is in ecx)

; - The acknowledgment field equals the largest unacknowledged sequence number. This means no data is acknowledged by this segment.
	mov	eax, [edx + TCP_segment.AckNumber]
	cmp	eax, [ebx + TCP_SOCKET.SND_UNA]
	jne	.not_uni_xfer

; - The reassembly list of out-of-order segments for the connection is empty (seg_next equals tp). ;;;;;;;

	jnz	.not_uni_xfer

;-------------------------------------
; Complete processing of received data

	DEBUGF	1,"header prediction: we are receiver\nreceiving %u bytes of data\n", ecx

	add	esi, edx
	lea	eax, [ebx + STREAM_SOCKET.rcv]
	call	SOCKET_ring_write			; Add the data to the socket buffer

	mov	eax, ebx
	call	SOCKET_notify_owner

	add	[ebx + TCP_SOCKET.RCV_NXT], ecx 	; Update sequence number with number of bytes we have copied
	or	[ebx + TCP_SOCKET.t_flags], TF_DELACK	; Set delayed ack flag

	jmp	.drop

;----------------------------------------------------
; Header prediction failed, doing it the slow way..     ;;;;; current implementation of header prediction destroys some regs (ecx) !!

  .not_uni_xfer:

	DEBUGF	1,"Header prediction failed\n"		; time to do it the "slow" way :)

;------------------------------
; Calculate receive window size

	;;;;

	cmp	[ebx + TCP_SOCKET.t_state], TCB_LISTEN
	je	.LISTEN

	cmp	[ebx + TCP_SOCKET.t_state], TCB_SYN_SENT
	je	.SYN_SENT

;--------------------------------------------
; Protection Against Wrapped Sequence Numbers

; First, check if timestamp is present

;;;; TODO

; Then, check if at least some bytes of data are within window

;;;; TODO

	jmp	.trim_then_step6

;-------------
; Passive Open

align 4
.LISTEN:

	DEBUGF	1,"TCP state: listen\n"

	test	[edx + TCP_segment.Flags], TH_RST	;;; TODO: kill new socket on error
	jnz	.drop

	test	[edx + TCP_segment.Flags], TH_ACK
	jnz	.drop_with_reset

	test	[edx + TCP_segment.Flags], TH_SYN
	jz	.drop


	; TODO: check if it's a broadcast or multicast, and drop if so

;-----------------------
; Fill in some variables

	add	[TCP_sequence_num], 64000

	push	[edx + TCP_segment.SourcePort]
	pop	[eax + TCP_SOCKET.RemotePort]

	push	[edx + TCP_segment.SequenceNumber]
	pop	[eax + TCP_SOCKET.IRS]

	push	[eax + TCP_SOCKET.ISS]
	pop	[eax + TCP_SOCKET.SND_NXT]

	TCP_sendseqinit eax
	TCP_rcvseqinit eax

	mov	[eax + TCP_SOCKET.t_state], TCB_SYN_RECEIVED
	mov	[eax + TCP_SOCKET.t_flags], TF_ACKNOW
	mov	[eax + TCP_SOCKET.timer_keepalive], TCP_time_keep_interval  ;;;; macro

	add	eax, STREAM_SOCKET.snd
	call	SOCKET_ring_create

	add	eax, STREAM_SOCKET.rcv - STREAM_SOCKET.snd
	call	SOCKET_ring_create

	sub	eax, STREAM_SOCKET.rcv

	mov	[eax + SOCKET.lock], 0
	mov	ebx, eax	; if there is data, it must arrive in this new socket!
	jmp	.trim_then_step6


;------------
; Active Open

align 4
.SYN_SENT:

	DEBUGF	1,"TCP state: syn_sent\n"

	test	[edx + TCP_segment.Flags], TH_ACK
	jz	@f

	mov	eax, [edx + TCP_segment.AckNumber]
	cmp	eax, [ebx + TCP_SOCKET.ISS]
	jle	.drop_with_reset

;        mov     eax, [edx + TCP_segment.AckNumber]
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

; at this point, segment seems to be valid

	test	[edx + TCP_segment.Flags], TH_ACK
	jz	.no_syn_ack

; now, process received SYN in response to an active open

	mov	eax, [edx + TCP_segment.AckNumber]
	mov	[ebx + TCP_SOCKET.SND_UNA], eax
	cmp	eax, [ebx + TCP_SOCKET.SND_NXT]
	jle	@f
	mov	[ebx + TCP_SOCKET.SND_NXT], eax
       @@:

  .no_syn_ack:

	mov	[ebx + TCP_SOCKET.timer_retransmission], 0	; disable retransmission

	push	[edx + TCP_segment.SequenceNumber]
	pop	[ebx + TCP_SOCKET.IRS]

	TCP_rcvseqinit ebx

	mov	[ebx + TCP_SOCKET.t_flags], TF_ACKNOW

	mov	eax, [ebx + TCP_SOCKET.SND_UNA]
	cmp	eax, [ebx + TCP_SOCKET.ISS]
	jle	.simultaneous_open

	test	[edx + TCP_segment.Flags], TH_ACK
	jz	.simultaneous_open

	DEBUGF	1,"TCP: active open\n"

; TODO: update stats
; TODO: set general socket state to connected

	mov	[ebx + TCP_SOCKET.t_state], TCB_ESTABLISHED

; TODO: check if we should scale the connection (567-572)
; TODO: update RTT estimators

	jmp	.trimthenstep6

  .simultaneous_open:

	DEBUGF	1,"TCP: simultaneous open\n"
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
	jmp	.step6


  .trim_then_step6:

;----------------------------
; trim any data not in window

	DEBUGF	1,"Trimming window\n"

	mov	eax, [ebx + TCP_SOCKET.RCV_NXT]
	sub	eax, [edx + TCP_segment.SequenceNumber]

	test	eax, eax
	jz	.no_duplicate

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
	jz	.no_duplicate
  .no_drop:

	DEBUGF	1,"Going to drop %u out of %u bytes\n", eax, ecx

; eax holds number of bytes to drop

;----------------------------------
; Check for entire duplicate packet

	cmp	eax, ecx
	jge	.duplicate

	;;; TODO: figure 28.30

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
	or	[ebx + TCP_SOCKET.t_flags], TF_ACKNOW
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

	DEBUGF	1,"Duplicate received\n"

;----------------------------------------
; Update statistics for duplicate packets

	;;; TODO

	jmp	.drop	       ;;; DROP the packet ??

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
	jg	@f

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

	DEBUGF	1,"Got an RST flag"

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

	DEBUGF	1,"Connection refused"

	;;; TODO: debug info

	jmp	.close

  .econnreset:

	DEBUGF	1,"Connection reset"

	;;; TODO: debug info
  .close:

	DEBUGF	1,"Closing connection"

	;;; update stats

  .rst_close:

	DEBUGF	1,"Closing with reset\n"

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
      @@:
;----------------
; Process the ACK

	cmp	[ebx + TCP_SOCKET.t_state], TCB_SYN_RECEIVED
	jg	.ack_dup
	jl	.ack_nodup

	DEBUGF	1,"TCP state = syn received\n"

	;;;;;

  .ack_nodup:

	DEBUGF	1,"New ACK\n"



  .ack_dup:

	;;;;

;-------------------------------------------------
; If the congestion window was inflated to account
; for the other side's cached packets, retrace it

	;;;; 888 -  902


;------------------------------------------
; RTT measurements and retransmission timer

	;;;;; 903 - 926

	mov	[ebx + TCP_SOCKET.timer_retransmission], 0

	mov	eax, [ebx + TCP_SOCKET.SND_MAX]
	cmp	eax, [edx + TCP_segment.AckNumber]
	je	.all_outstanding
	mov	[ebx + TCP_SOCKET.timer_retransmission], 120 ;;;; TODO: correct this value (use a macro for it)
  .all_outstanding:

;-------------------------------------------
; Open congestion window in response to ACKs

	;;;;


;------------------------------------------
; Remove acknowledged data from send buffer

	pusha
; Delete acknowledged bytes from send buffer
	mov	ecx, [edx + TCP_segment.AckNumber]
	sub	ecx, [ebx + TCP_SOCKET.SND_UNA]
	lea	eax, [ebx + STREAM_SOCKET.snd]
	call	SOCKET_ring_free
	popa


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  code missing (943?)



	mov	eax, [edx + TCP_segment.AckNumber]
	mov	[ebx + TCP_SOCKET.SND_UNA], eax

	cmp	eax, [ebx + TCP_SOCKET.SND_NXT]
	jl	@f
	mov	[ebx + TCP_SOCKET.SND_NXT], eax
       @@:


;---------------------------------------
; Wake up process waiting on send buffer

	mov	eax, ebx
	call	SOCKET_notify_owner

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


  .step6:

	DEBUGF	1,"step 6\n"

;----------------------------------------------
; check if we need to update window information

	test	[edx + TCP_segment.Flags], TH_ACK
	jz	.no_window_update

	mov	eax, [ebx + TCP_SOCKET.SND_WL1]
	cmp	eax, [edx + TCP_segment.SequenceNumber]
	jl	.update_window
	jg	@f

	mov	eax, [ebx + TCP_SOCKET.SND_WL2]
	cmp	eax, [edx + TCP_segment.AckNumber]
	jl	.update_window
	jg	.no_window_update
       @@:

	mov	eax, [ebx + TCP_SOCKET.SND_WL2]
	cmp	eax, [edx + TCP_segment.AckNumber]
	jne	.no_window_update

	movzx	eax, [edx + TCP_segment.Window]
	cmp	eax, [ebx + TCP_SOCKET.SND_WND]
	jle	.no_window_update

  .update_window:

	DEBUGF	1,"Updating window\n"

;----------------------------------
; Keep track of pure window updates

;        test    ecx, ecx
;        jz      @f
;
;        mov     eax, [ebx + TCP_SOCKET.SND_WL2]
;        cmp     eax, [edx + TCP_segment.AckNumber]
;        jne     @f
;
;        ;; mov eax, tiwin
;        cmp     eax, [ebx + TCP_SOCKET.SND_WND]
;        jle     @f
;
;        ;;; update stats
;
;       @@:

	mov	eax, dword [edx + TCP_segment.Window]
	cmp	eax, [ebx + TCP_SOCKET.max_sndwnd]
	jle	@f
	mov	[ebx + TCP_SOCKET.max_sndwnd], eax
       @@:
	mov	[ebx + TCP_SOCKET.SND_WND], eax

	push	[edx + TCP_segment.SequenceNumber]
	pop	[ebx + TCP_SOCKET.SND_WL1]

	push	[edx + TCP_segment.AckNumber]
	pop	[ebx + TCP_SOCKET.SND_WL2]

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
	add	eax, [ebx + STREAM_SOCKET.rcv + RING_BUFFER.size]
	cmp	eax, SOCKET_MAXDATA
	jle	.not_urgent

	mov	[edx + TCP_segment.UrgentPointer], 0
	and	[edx + TCP_segment.Flags], not (TH_URG)
	jmp	.do_data

  .not_urgent:

;--------------------------------------
; processing of received urgent pointer

	;;; TODO (1051-1093)

;--------------------------------
; process the data in the segment

  .do_data:

	DEBUGF	1,"TCP: do data (%u)\n", ecx

	test	[edx + TCP_segment.Flags], TH_FIN
	jnz	.process_fin

	cmp	[ebx + TCP_SOCKET.t_state], TCB_FIN_WAIT_1
	jge	.dont_do_data

	test	ecx, ecx
	jz	.final_processing

	DEBUGF	1,"Processing data in segment\n"

;; TODO: check if data is in sequence !

	movzx	eax, [edx + TCP_segment.DataOffset]		;;; todo: remember this in.. edi ?
	and	eax, 0xf0
	shr	al, 2

	lea	esi, [edx + eax]

	or	[ebx + TCP_SOCKET.t_flags], TF_DELACK
	add	[ebx + TCP_SOCKET.RCV_NXT], ecx

	lea	eax, [ebx + STREAM_SOCKET.rcv]
	call	SOCKET_ring_write

	mov	eax, ebx
	call	SOCKET_notify_owner

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
	dd	.fin_syn_est	;TCB_SYN_RECEIVED
	dd	.fin_syn_est	;TCB_ESTABLISHED
	dd	.no_fin 	;TCB_CLOSE_WAIT
	dd	.fin_wait1	;TCB_FIN_WAIT_1
	dd	.no_fin 	;TCB_CLOSING
	dd	.no_fin 	;TCB_LAST_ACK
	dd	.fin_wait2	;TCB_FIN_WAIT_2
	dd	.fin_timed	;TCB_TIMED_WAIT



  .fin_syn_est:

	jmp	.final_processing

  .fin_wait1:

	jmp	.final_processing

  .fin_wait2:

	jmp	.final_processing

  .fin_timed:

	jmp	.final_processing

  .no_fin:

;-----------------
; Final processing

  .final_processing:

	DEBUGF	1,"Final processing\n"

	mov	[ebx + SOCKET.lock], 0

	;;; if debug enabled, output packet

	;test    ;;;needoutput = 1
	;jnz     .outputnow

	test	[ebx + TCP_SOCKET.t_flags], TF_ACKNOW
	jnz	.ack_now

	call	kernel_free
	add	esp, 4
	ret

  .ack_now:

	DEBUGF	1,"ACK now!\n"

	push	ebx
	mov	eax, ebx
	call	TCP_output
	pop	ebx

	call	kernel_free
	add	esp, 4
	ret

;------------------------------------------
; Generate an ACK, droping incoming segment

align 4
.drop_after_ack:

	DEBUGF	1,"Drop after ACK\n"

	test	[edx + TCP_segment.Flags], TH_RST
	jnz	.drop

	and	[ebx + TCP_SOCKET.t_flags], TF_ACKNOW

	push	ebx
	mov	eax, ebx
	call	TCP_output
	pop	ebx

	call	kernel_free
	add	esp, 4
	ret


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

	call	kernel_free
	add	esp, 4
	ret

  .respond_ack:

	mov	dl, TH_RST

	push	ebx
	call	TCP_respond_segment
	pop	ebx

	jmp	.destroy_new_socket


  .respond_syn:

	mov	dl, TH_RST + TH_ACK

	push	ebx
	call	TCP_respond_socket
	pop	ebx

	jmp	.destroy_new_socket

;-----
; Drop

align 4
.drop:

	DEBUGF	1,"Dropping packet\n"

	;;;; If debugging options are enabled, output the packet somwhere

  .destroy_new_socket:

	;;;; kill the newly created socket

	call	kernel_free
	add	esp, 4
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



;-----------------------------------------------------------------
;
; TCP_output
;
; IN:  eax = socket pointer
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

	call	TCP_outflags	; in dl

; If in persist timeout with window of 0, send 1 byte.
; Otherwise, if window is small but nonzero, and timer expired,
; we will send what we can and go to transmit state

	test	[eax + TCP_SOCKET.t_force], -1
	jz	.no_persist_timeout

	test	ecx, ecx
	jnz	.no_zero_window

	cmp	ebx, [eax + STREAM_SOCKET.snd + RING_BUFFER.size]
	jge	@f

	and	dl, not (TH_FIN)	  ; clear the FIN flag    ??? how can it be set before?

       @@:
	inc	ecx
	jmp	.no_persist_timeout

  .no_zero_window:

	mov	[eax + TCP_SOCKET.timer_persist], 0
	mov	[eax + TCP_SOCKET.t_rxtshift], 0

  .no_persist_timeout:

;;;106

	mov	esi, [eax + STREAM_SOCKET.snd + RING_BUFFER.size]
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

	mov	[eax + TCP_SOCKET.timer_retransmission], 0   ; cancel retransmit

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
	add	edi, [eax + STREAM_SOCKET.snd + RING_BUFFER.size]
	cmp	edi, 0
	jle	@f

	and	dl, not (TH_FIN)	  ; clear the FIN flag

       @@:


; set ecx to space available in receive buffer
; From now on, ecx will be the window we advertise to the other end

	mov	ecx, SOCKET_MAXDATA
	sub	ecx, [eax + STREAM_SOCKET.rcv + RING_BUFFER.size]

;------------------------------
; Sender silly window avoidance

	cmp	ecx, [eax + TCP_SOCKET.t_maxseg]
	je	.send

;;; TODO: 144-145

	test	[eax + TCP_SOCKET.t_force], -1
	jnz	.send

	mov	ebx, [eax + TCP_SOCKET.max_sndwnd]
	shr	ebx, 1
	cmp	ecx, ebx
	jge	.send

	mov	ebx, [eax + TCP_SOCKET.SND_NXT]
	cmp	ebx, [eax + TCP_SOCKET.SND_MAX]
	jl	.send

;----------------------------------------
; Check if a window update should be sent

	test	ecx, ecx	; window
	jz	.no_window

;;; TODO 154-172

  .no_window:

;--------------------------
; Should a segment be sent?

	test	[eax + TCP_SOCKET.t_flags], TF_ACKNOW
	jnz	.send

	test	dl, TH_SYN + TH_RST
	jnz	.send

	mov	ebx, [eax + TCP_SOCKET.SND_UP]
	cmp	ebx, [eax + TCP_SOCKET.SND_UNA]
	jg	.send

	test	dl, TH_FIN
	jz	.enter_persist

	test	[eax + TCP_SOCKET.t_flags], TF_SENTFIN
	jnz	.send

	mov	ebx, [eax + TCP_SOCKET.SND_NXT]
	cmp	ebx, [eax + TCP_SOCKET.SND_UNA]
	je	.send

;--------------------
; Enter persist state

  .enter_persist:

	DEBUGF	1,"Entering persist state\n"

;--------------------------------------
; No reason to send a segment, just ret

	DEBUGF	1,"No reason to send a segment\n"

	ret


;-----------------------------------------------
;
; Send a segment
;
; eax = socket pointer
;  dl = flags
;
;-----------------------------------------------

  .send:

	DEBUGF	1,"Preparing to send a segment\n"

	mov	edi, TCP_segment.Data	; edi will contain headersize

	sub	esp, 8			; create some space on stack
	push	eax			; save this too..

;------------------------------------
; Send options with first SYN segment

	test	dl, TH_SYN
	jz	.no_options

	push	[eax + TCP_SOCKET.ISS]
	pop	[eax + TCP_SOCKET.SND_NXT]

	test	[eax + TCP_SOCKET.t_flags], TF_NOOPT
	jnz	.no_options

	mov	ecx, 1460
	or	ecx, TCP_OPT_MAXSEG shl 24 + 4 shl 16
	bswap	ecx
	push	ecx
	add	di, 4

	test	[eax + TCP_SOCKET.t_flags], TF_REQ_SCALE
	jz	.no_syn

	test	dl, TH_ACK
	jnz	.scale_opt

	test	[eax + TCP_SOCKET.t_flags], TF_RCVD_SCALE
	jz	.no_syn

  .scale_opt:
	movzx	ecx, byte [eax + TCP_SOCKET.request_r_scale]
	or	ecx, TCP_OPT_WINDOW shl 24 + 4 shl 16 + TCP_OPT_NOP shl 8
	bswap	ecx
	pushd	ecx
	add	di, 4

  .no_syn:

;------------------------------------
; Make the timestamp option if needed

	test	[eax + TCP_SOCKET.t_flags], TF_REQ_TSTMP
	jz	.no_timestamp

	test	dl, TH_RST
	jnz	.no_timestamp

	test	dl, TH_ACK
	jz	.timestamp

	test	[eax + TCP_SOCKET.t_flags], TF_RCVD_TSTMP
	jz	.no_timestamp

  .timestamp:
	mov	esi, [timer_ticks]
	bswap	esi
	push	esi
	pushw	0
	pushd	TCP_OPT_TIMESTAMP + 10 shl 8 + TCP_OPT_NOP shl 16 + TCP_OPT_NOP shl 24
	add	di, 10

  .no_timestamp:
	;; TODO: check if we dont exceed the max segment size

  .no_options:
	; eax = socket ptr
	; edx = flags
	; ecx = data size
	; edi = header size
	; esi = snd ring buff ptr

	mov	ecx, [eax + STREAM_SOCKET.snd + RING_BUFFER.size]
	cmp	ecx, [eax + TCP_SOCKET.t_maxseg]			;;; right?
	jle	@f
	mov	ecx, [eax + TCP_SOCKET.t_maxseg]
       @@:
	add	ecx, edi	; total TCP segment size

; Start by pushing all TCP header values in reverse order on stack
; (essentially, creating the tcp header!)

	pushw	0	;        .UrgentPointer          dw ?
	pushw	0	;        .Checksum               dw ?
	pushw	0x00a0	;        .Window                 dw ?    ;;;;;;;
	shl	edi, 2	;        .DataOffset             db ?  only 4 left-most bits
	shl	dx, 8
	or	dx, di	;        .Flags                  db ?
	pushw	dx
	shr	edi, 2	;        .DataOffset             db ? ;;;;

	push	[eax + TCP_SOCKET.RCV_NXT]	;        .AckNumber              dd ?
	ntohd	[esp]

	push	[eax + TCP_SOCKET.SND_NXT]	;        .SequenceNumber         dd ?
	ntohd	[esp]

	push	[eax + TCP_SOCKET.RemotePort]	;        .DestinationPort        dw ?
	ntohw	[esp]

	push	[eax + TCP_SOCKET.LocalPort]	;        .SourcePort             dw ?
	ntohw	[esp]

	push	edi		; header size

; Create the IP packet
	mov	ebx, [eax + IP_SOCKET.LocalIP]	; source ip
	mov	eax, [eax + IP_SOCKET.RemoteIP] ; dest ip
	mov	di, IP_PROTO_TCP shl 8 + 128
	call	IPv4_output
	jz	.fail

;-----------------------------------------
; Move TCP header from stack to TCP packet

	push	ecx
	mov	ecx, [esp+4]
	lea	esi, [esp+4+4]
	shr	ecx, 2
	rep	movsd
	pop	ecx		; full TCP packet size

	pop	esi		; headersize
	add	esp, esi

	mov	[esp + 4], eax		; packet ptr
	mov	[esp + 4+4], edx	; packet size

	mov	edx, edi		; begin of data
	sub	edx, esi		; begin of packet (edi = begin of data)
	push	ecx
	sub	ecx, esi		; data size

;--------------
; Copy the data

; eax = ptr to ring struct
; ecx = buffer size
; edi = ptr to buffer

;        test    ecx, ecx
	mov	eax, [esp+4]		  ; socket ptr
	add	[eax + TCP_SOCKET.SND_NXT], ecx
	add	eax, STREAM_SOCKET.snd
	push	edx
	call	SOCKET_ring_read
	pop	esi
	pop	ecx
	pop	eax

	test	[esi + TCP_segment.Flags], TH_SYN + TH_FIN
	jz	@f
	inc	[eax + TCP_SOCKET.SND_NXT]
	;;; TODO: update sentfin flag
       @@:

	mov	edx, [eax + TCP_SOCKET.SND_NXT]
	cmp	edx, [eax + TCP_SOCKET.SND_MAX]
	jle	@f
	mov	[eax + TCP_SOCKET.SND_MAX], edx

	;;;; TODO: time transmission (420)
       @@:

	;;; TODO: set retransmission timer

;--------------------
; Create the checksum

	DEBUGF	1,"checksum: ptr=%x size=%u\n", esi, ecx

	TCP_checksum (eax + IP_SOCKET.LocalIP), (eax + IP_SOCKET.RemoteIP)
	mov	[esi+TCP_segment.Checksum], dx

;----------------
; Send the packet

	DEBUGF	1,"Sending TCP Packet to device %x\n", ebx
	call	[ebx + NET_DEVICE.transmit]
	ret


  .fail:
	pop	ecx
	add	esp, ecx
	add	esp, 4+8
	DEBUGF 1,"TCP_output: failed\n"
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
; The easy way to send an ACK/RST/keepalive segment
;
; TCP_respond_socket:
;
;  IN:  ebx = socket ptr
;        cl = flags
;
;--------------------------------------
align 4
TCP_respond_socket:

	DEBUGF	1,"TCP_respond_socket\n"

;---------------------
; Create the IP packet

	push	cx ebx
	mov	eax, [ebx + IP_SOCKET.RemoteIP]
	mov	ebx, [ebx + IP_SOCKET.LocalIP]
	mov	ecx, TCP_segment.Data
	mov	di , IP_PROTO_TCP shl 8 + 128
	call	IPv4_output
	test	edi, edi
	jz	.error
	pop	esi cx
	push	edx eax

;-----------------------------------------------
; Fill in the TCP header by using the socket ptr

	mov	ax, [esi + TCP_SOCKET.LocalPort]
	rol	ax, 8
	stosw
	mov	ax, [esi + TCP_SOCKET.RemotePort]
	rol	ax, 8
	stosw
	mov	eax, [esi + TCP_SOCKET.SND_NXT]
	bswap	eax
	stosd
	mov	eax, [esi + TCP_SOCKET.RCV_NXT]
	bswap	eax
	stosd
	mov	al, 0x50	; Dataoffset: 20 bytes
	stosb
	mov	al, cl
	stosb
	mov	ax, [esi + TCP_SOCKET.RCV_WND]
	rol	ax, 8
	stosw			; window
	xor	eax, eax
	stosd			; checksum + urgentpointer

;---------------------
; Fill in the checksum

  .checksum:
	sub	edi, TCP_segment.Data
	mov	ecx, TCP_segment.Data
	xchg	esi, edi
	TCP_checksum (edi + IP_SOCKET.LocalIP), (esi + IP_SOCKET.RemoteIP)
	mov	[esi+TCP_segment.Checksum], dx

;--------------------
; And send the segment

	call	[ebx + NET_DEVICE.transmit]
	ret

  .error:
	DEBUGF	1,"TCP_respond failed\n"
	add	esp, 2+4

	ret



;-------------------------
; TCP_respond.segment:
;
;  IN:  edx = segment ptr (a previously received segment)
;        cl = flags

align 4
TCP_respond_segment:

	DEBUGF	1,"TCP_respond_segment\n"

;---------------------
; Create the IP packet

	push	cx edx
	mov	ebx, [edx - 20 + IPv4_Packet.SourceAddress]	 ;;;; and what if ip packet had options?!
	mov	eax, [edx - 20 + IPv4_Packet.DestinationAddress]   ;;;
	mov	ecx, TCP_segment.Data
	mov	di , IP_PROTO_TCP shl 8 + 128
	call	IPv4_output
	jz	.error
	pop	esi cx

	push	edx eax

;---------------------------------------------------
; Fill in the TCP header by using a received segment

	mov	ax, [esi + TCP_segment.DestinationPort]
	rol	ax, 8
	stosw
	mov	ax, [esi + TCP_segment.SourcePort]
	rol	ax, 8
	stosw
	mov	eax, [esi + TCP_segment.AckNumber]
	bswap	eax
	stosd
	xor	eax, eax
	stosd
	mov	al, 0x50	; Dataoffset: 20 bytes
	stosb
	mov	al, cl
	stosb
	mov	ax, 1280
	rol	ax, 8
	stosw			; window
	xor	eax, eax
	stosd			; checksum + urgentpointer

;---------------------
; Fill in the checksum

  .checksum:
	lea	esi, [edi - TCP_segment.Data]
	mov	ecx, TCP_segment.Data
	TCP_checksum (esi - 20 + IPv4_Packet.DestinationAddress), (esi - 20 + IPv4_Packet.DestinationAddress)
	mov	[esi+TCP_segment.Checksum], dx

;--------------------
; And send the segment

	call	[ebx + NET_DEVICE.transmit]
	ret

  .error:
	DEBUGF	1,"TCP_respond failed\n"
	add	esp, 2+4

	ret




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
