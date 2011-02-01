;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved.    ;;
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
	mov	ebx, [eax + TCP_SOCKET.SND_NXT] 	; calculate offset (71)
	sub	ebx, [eax + TCP_SOCKET.SND_UNA] 	;

	mov	ecx, [eax + TCP_SOCKET.SND_WND] 	; determine window
	cmp	ecx, [eax + TCP_SOCKET.SND_CWND]	;
	jl	@f					;
	mov	ecx, [eax + TCP_SOCKET.SND_CWND]	;
       @@:						;

	call	TCP_outflags				; flags in dl

;------------------------
; data being forced out ?

; If in persist timeout with window of 0, send 1 byte.
; Otherwise, if window is small but nonzero, and timer expired,
; we will send what we can and go to transmit state

	test	[eax + TCP_SOCKET.t_force], -1
	jz	.no_force

	test	ecx, ecx
	jnz	.no_zero_window

	cmp	ebx, [eax + STREAM_SOCKET.snd + RING_BUFFER.size]
	jge	@f

	and	dl, not (TH_FIN)	  ; clear the FIN flag    ??? how can it be set before?

       @@:
	inc	ecx
	jmp	.no_force

  .no_zero_window:
	mov	[eax + TCP_SOCKET.timer_persist], 0
	mov	[eax + TCP_SOCKET.t_rxtshift], 0

  .no_force:

;--------------------------------
; Calculate how much data to send (106)

	mov	esi, [eax + STREAM_SOCKET.snd + RING_BUFFER.size]
	cmp	esi, ecx
	jl	@f
	mov	esi, ecx
       @@:
	sub	esi, ebx

;------------------------
; check for window shrink (107)

; If FIN has been set, but not ACKed, but we havent been called to retransmit, esi will be -1
; Otherwise, window shrank after we sent into it.

	jns	.not_negative

; enter persist state
	xor	esi, esi

; If window shrank to 0
	test	ecx, ecx
	jnz	@f

; cancel pending retransmit
	mov	[eax + TCP_SOCKET.timer_retransmission], 0

; pull SND_NXT back to (closed) window, We will enter persist state below.
	push	[eax + TCP_SOCKET.SND_UNA]
	pop	[eax + TCP_SOCKET.SND_NXT]

       @@:

; If window didn't close completely, just wait for an ACK

  .not_negative:

;---------------------------
; Send one segment at a time (124)

	cmp	esi, [eax + TCP_SOCKET.t_maxseg]
	jle	@f

	mov	esi, [eax + TCP_SOCKET.t_maxseg]

;;; sendalot = 1

       @@:

;--------------------------------------------
; Turn of FIN flag if send buffer not emptied (128)

	mov	edi, [eax + TCP_SOCKET.SND_NXT]
	add	edi, esi
	sub	edi, [eax + TCP_SOCKET.SND_UNA]
	sub	edi, [eax + STREAM_SOCKET.snd + RING_BUFFER.size]
	jns	@f

	and	dl, not (TH_FIN)

       @@:

;-------------------------------
; calculate window advertisement (130)

	mov	ecx, SOCKET_MAXDATA
	sub	ecx, [eax + STREAM_SOCKET.rcv + RING_BUFFER.size]

;------------------------------
; Sender silly window avoidance (131)

	test	esi, esi
	jz	.len_zero

	cmp	esi, [eax + TCP_SOCKET.t_maxseg]
	je	.send

;;; if (idle or TF_NODELAY) && (esi + ebx >= so_snd.sb_cc), send

	test	[eax + TCP_SOCKET.t_force], -1	;;;
	jnz	.send

	mov	ebx, [eax + TCP_SOCKET.max_sndwnd]
	shr	ebx, 1
	cmp	esi, ebx
	jge	.send

	mov	ebx, [eax + TCP_SOCKET.SND_NXT]
	cmp	ebx, [eax + TCP_SOCKET.SND_MAX]
	jl	.send

  .len_zero:

;----------------------------------------
; Check if a window update should be sent (154)

	test	ecx, ecx
	jz	.no_window

;;; TODO 167-172

  .no_window:

;--------------------------
; Should a segment be sent? (174)

	test	[eax + TCP_SOCKET.t_flags], TF_ACKNOW	; we need to ACK
	jnz	.send

	test	dl, TH_SYN + TH_RST			; we need to send a SYN or RST
	jnz	.send

	mov	ebx, [eax + TCP_SOCKET.SND_UP]		; when urgent pointer is beyond start of send bufer
	cmp	ebx, [eax + TCP_SOCKET.SND_UNA]
	jg	.send

	test	dl, TH_FIN
	jz	.enter_persist	; no reason to send, enter persist state

; FIN was set, only send if not already sent, or on retransmit

	test	[eax + TCP_SOCKET.t_flags], TF_SENTFIN
	jnz	.send

	mov	ebx, [eax + TCP_SOCKET.SND_NXT]
	cmp	ebx, [eax + TCP_SOCKET.SND_UNA]
	je	.send

;--------------------
; Enter persist state (191)

  .enter_persist:

	DEBUGF	1,"Entering persist state\n"


;;; 213 - 217

;----------------------------
; No reason to send a segment (219)

	DEBUGF	1,"No reason to send a segment\n"

	mov	[eax + SOCKET.lock], 0

	ret









;-----------------------------------------------
;
; Send a segment (222)
;
; eax = socket pointer
; esi = data len
;  dl = flags
;
;-----------------------------------------------

  .send:

	DEBUGF	1,"Preparing to send a segment\n"

	mov	edi, TCP_segment.Data	; edi will contain headersize

	sub	esp, 8			; create some space on stack
	push	eax			; save socket pointer

;------------------------------------
; Send options with first SYN segment

	test	dl, TH_SYN
	jz	.options_done

	push	[eax + TCP_SOCKET.ISS]
	pop	[eax + TCP_SOCKET.SND_NXT]

	test	[eax + TCP_SOCKET.t_flags], TF_NOOPT
	jnz	.options_done

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
	mov	ebx, [timer_ticks]
	bswap	ebx
	push	ebx
	pushw	0
	pushd	TCP_OPT_TIMESTAMP + 10 shl 8 + TCP_OPT_NOP shl 16 + TCP_OPT_NOP shl 24
	add	di, 10

  .no_timestamp:

	; <Add additional options here>








  .options_done:

; eax = socket ptr
; edx = flags
; edi = header size
; esi = data len

;---------------------------------------------
; check if we dont exceed the max segment size (270)

	add	esi, edi			; total TCP segment size
	cmp	esi, [eax + TCP_SOCKET.t_maxseg]
	jle	.no_overflow

	mov	esi, [eax + TCP_SOCKET.t_maxseg]

;;; sendalot = 1

  .no_overflow:

;-----------------------------------------------------------------
; Start by pushing all TCP header values in reverse order on stack
; (essentially, creating the tcp header on the stack!)

	pushw	0	;        .UrgentPointer          dw ?
	pushw	0	;        .Checksum               dw ?
	pushw	0x00a0	;        .Window                 dw ?    ;;;;;;; FIXME
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

;---------------------
; Create the IP packet

	mov	ecx, esi

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

	mov	eax, [esp+4]			; get socket ptr

	add	[eax + TCP_SOCKET.SND_NXT], ecx ; update sequence number

	add	eax, STREAM_SOCKET.snd
	push	edx
	call	SOCKET_ring_read
	pop	esi				; begin of data
	pop	ecx				; full packet size
	pop	eax				; socket ptr

;----------------------------------
; update sequence number and timers  (400)

	test	[esi + TCP_segment.Flags], TH_SYN + TH_FIN
	jz	@f
	inc	[eax + TCP_SOCKET.SND_NXT]	; syn and fin take a sequence number
	test	[esi + TCP_segment.Flags], TH_FIN
	jz	@f
	or	[eax + TCP_SOCKET.t_flags], TF_SENTFIN	; if we sent a fin, set the sentfin flag
       @@:

	mov	edx, [eax + TCP_SOCKET.SND_NXT]
	cmp	edx, [eax + TCP_SOCKET.SND_MAX]
	jle	@f
	mov	[eax + TCP_SOCKET.SND_MAX], edx

	;;;; TODO: time transmission (420)

       @@:

; set retransmission timer if not already set, and not doing an ACK or keepalive probe

	cmp	[eax + TCP_SOCKET.timer_retransmission], 1000 ;;;;
	jl	.retransmit_set

	cmp	edx, [eax + TCP_SOCKET.SND_UNA] 	; edx = [eax + TCP_SOCKET.SND_NXT]
	je	.retransmit_set

	mov	edx, [eax + TCP_SOCKET.t_rxtcur]
	mov	[eax + TCP_SOCKET.timer_retransmission], dx

	mov	[eax + TCP_SOCKET.timer_persist], 0
	mov	[eax + TCP_SOCKET.t_rxtshift], 0	;;; TODO: only do this if timer_persist was set


  .retransmit_set:

;--------------------
; Create the checksum

	DEBUGF	1,"checksum: ptr=%x size=%u\n", esi, ecx

	TCP_checksum (eax + IP_SOCKET.LocalIP), (eax + IP_SOCKET.RemoteIP)
	mov	[esi+TCP_segment.Checksum], dx

; unlock socket

	mov	[eax + SOCKET.lock], 0

;----------------
; Send the packet

	DEBUGF	1,"Sending TCP Packet to device %x\n", ebx
	call	[ebx + NET_DEVICE.transmit]
	ret


  .fail:
	pop	ecx
	add	esp, ecx
	add	esp, 4+8
	mov	[eax + SOCKET.lock], 0
	DEBUGF 1,"TCP_output: failed\n"
	ret


