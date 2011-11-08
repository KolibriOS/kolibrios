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








;-------------------------
;
; TCP_drop
;
;  IN:  eax = socket ptr
;       ebx = error number
;
;  OUT: eax = socket ptr
;
;-------------------------
align 4
TCP_drop:

	DEBUGF	1,"TCP_drop\n"

	cmp	[eax + TCP_SOCKET.t_state], TCPS_SYN_RECEIVED
	jb	.no_syn_received

	mov	[eax + TCP_SOCKET.t_state], TCPS_CLOSED

	call	TCP_output

;;; TODO: update stats

	jmp	TCP_close

  .no_syn_received:

;;; TODO: update stats

;;; TODO: check if error code is "Connection timed out' and handle accordingly

	mov	[eax + SOCKET.errorcode], ebx








;-------------------------
;
; TCP_close
;
;  IN:  eax = socket ptr
;  OUT: eax = socket ptr
;
;-------------------------
align 4
TCP_close:

	DEBUGF	1,"TCP_close\n"

;;; TODO: update RTT and mean deviation
;;; TODO: update slow start threshold
;;; TODO: release connection resources

	call	SOCKET_is_disconnected

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

	db	TH_RST + TH_ACK 	; TCPS_CLOSED
	db	0			; TCPS_LISTEN
	db	TH_SYN			; TCPS_SYN_SENT
	db	TH_SYN + TH_ACK 	; TCPS_SYN_RECEIVED
	db		 TH_ACK 	; TCPS_ESTABLISHED
	db		 TH_ACK 	; TCPS_CLOSE_WAIT
	db	TH_SYN + TH_ACK 	; TCPS_FIN_WAIT_1
	db	TH_SYN + TH_ACK 	; TCPS_CLOSING
	db	TH_SYN + TH_ACK 	; TCPS_LAST_ACK
	db		 TH_ACK 	; TCPS_FIN_WAIT_2
	db		 TH_ACK 	; TCPS_TIMED_WAIT






;---------------------------------------
;
; The fast way to send an ACK/RST/keepalive segment
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
	mov	ecx, sizeof.TCP_header
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
	mov	al, 0x50	; Dataoffset: 20 bytes (TCP_header.DataOffset)
	stosb
	mov	al, cl
	stosb
;        mov     ax, [esi + TCP_SOCKET.RCV_WND]
;        rol     ax, 8
	mov	ax, 0x00a0	;;;;;;; FIXME
	stosw			; window
	xor	eax, eax
	stosd			; checksum + urgentpointer

;---------------------
; Fill in the checksum

  .checksum:
	sub	edi, sizeof.TCP_header
	mov	ecx, sizeof.TCP_header
	xchg	esi, edi
	TCP_checksum (edi + IP_SOCKET.LocalIP), (edi + IP_SOCKET.RemoteIP)
	mov	[esi+TCP_header.Checksum], dx

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
	mov	ebx, [edx - sizeof.IPv4_header + IPv4_header.SourceAddress]	 ;;;; FIXME: and what if ip packet had options?!
	mov	eax, [edx - sizeof.IPv4_header + IPv4_header.DestinationAddress]   ;;;
	mov	ecx, sizeof.TCP_header
	mov	di , IP_PROTO_TCP shl 8 + 128
	call	IPv4_output
	jz	.error
	pop	esi cx

	push	edx eax

;---------------------------------------------------
; Fill in the TCP header by using a received segment

	mov	ax, [esi + TCP_header.DestinationPort]
	rol	ax, 8
	stosw
	mov	ax, [esi + TCP_header.SourcePort]
	rol	ax, 8
	stosw
	mov	eax, [esi + TCP_header.AckNumber]
	bswap	eax
	stosd
	xor	eax, eax
	stosd
	mov	al, 0x50	; Dataoffset: 20 bytes (sizeof.TCP_header)
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
	lea	esi, [edi - sizeof.TCP_header]
	mov	ecx, sizeof.TCP_header
	TCP_checksum (esi - sizeof.IPv4_header + IPv4_header.DestinationAddress),\	; FIXME
		     (esi - sizeof.IPv4_header + IPv4_header.SourceAddress)
	mov	[esi+TCP_header.Checksum], dx

;--------------------
; And send the segment

	call	[ebx + NET_DEVICE.transmit]
	ret

  .error:
	DEBUGF	1,"TCP_respond failed\n"
	add	esp, 2+4

	ret