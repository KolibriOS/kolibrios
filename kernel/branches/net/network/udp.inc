;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2009. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  UDP.INC                                                        ;;
;;                                                                 ;;
;;  Part of the tcp/ip network stack for KolibriOS                 ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


struct	UDP_Packet
	.SourcePort		dw  ?
	.DestinationPort	dw  ?
	.Length 		dw  ?  ; Length of (UDP Header + Data)
	.Checksum		dw  ?
	.Data:

ends


align 4
uglobal
	UDP_PACKETS_TX		rd  MAX_IP
	UDP_PACKETS_RX		rd  MAX_IP
endg


;-----------------------------------------------------------------
;
; UDP_init
;
;  This function resets all UDP variables
;
;  IN:  /
;  OUT: /
;
;-----------------------------------------------------------------
align 4
UDP_init:

	xor	eax, eax
	mov	edi, UDP_PACKETS_TX
	mov	ecx, 2*MAX_IP
	rep	stosd

	ret



;-----------------------------------------------------------------
;
; UDP_Handler:
;
;  Called by IPv4_handler,
;  this procedure will inject the udp data diagrams in the application sockets.
;
;  IN:  Pointer to buffer in [esp]
;       size of buffer in [esp+4]
;       pointer to device struct in ebx
;       UDP Packet size in ecx
;       pointer to UDP Packet data in edx
;  OUT: /
;
;-----------------------------------------------------------------
align 4
UDP_handler:

	DEBUGF	1,"UDP_Handler\n"

	cmp	[edx + UDP_Packet.Checksum], 0
	jz	.no_checksum
	; First validate, checksum:

	pusha

	push	cx
	rol	word [esp], 8
	push	word IP_PROTO_UDP shl 8
	push	edi
	push	esi

	mov	di, [edx + UDP_Packet.Checksum]
	mov	[edx + UDP_Packet.Checksum], 0

	mov	esi, edx
	xor	edx, edx
	call	checksum_1
	call	checksum_pseudoheader
	call	checksum_2

	cmp	di, dx
	popa
	jne	.checksum_mismatch ;dump


  .no_checksum:

	DEBUGF	1,"UDP Checksum is correct\n"

	; Look for a socket where
	; IP Packet UDP Destination Port = local Port
	; IP Packet SA = Remote IP

	mov	eax, net_sockets
  .try_more:
	mov	si , [edx + UDP_Packet.DestinationPort]   ; get the local port from the IP Packet's UDP header
  .next_socket:
	mov	eax, [eax + SOCKET_head.NextPtr]
	or	eax, eax
	jz	.dump
	cmp	[eax + SOCKET_head.Domain], AF_INET4
	jne	.next_socket
	cmp	[eax + SOCKET_head.Type], IP_PROTO_UDP
	jne	.next_socket
	cmp	[eax + SOCKET_head.end + IPv4_SOCKET.end + UDP_SOCKET.LocalPort], si
	jne	.next_socket

	DEBUGF	1,"found socket with matching domain, type and localport\n"

	; For dhcp, we must allow any remote server to respond.
	; I will accept the first incoming response to be the one
	; I bind to, if the socket is opened with a destination IP address of
	; 255.255.255.255
	cmp	[eax + SOCKET_head.end + IPv4_SOCKET.RemoteIP], 0xffffffff
	je	.ok1

	mov	esi, [esp]
	mov	esi, [ebx + ETH_FRAME.Data + IPv4_Packet.SourceAddress] ; get the Source address from the IP Packet   FIXME
	cmp	[eax + SOCKET_head.end + IPv4_SOCKET.RemoteIP], esi
	jne	.try_more					      ; Quit if the source IP is not valid, check for more sockets with this IP/PORT combination


	DEBUGF	1,"Remote Ip matches\n"
  .ok1:

	cmp	[eax + SOCKET_head.end + IPv4_SOCKET.end + UDP_SOCKET.firstpacket], 0
	jz	.updateport

	mov	si, [edx + UDP_Packet.SourcePort]
	cmp	[eax + SOCKET_head.end + IPv4_SOCKET.end + UDP_SOCKET.RemotePort], si
	jne	.dump

	push	ebx
	lea	ebx, [eax + SOCKET_head.lock]
	call	wait_mutex
	pop	ebx

  .ok2:

	DEBUGF	1,"Found valid UDP packet for socket %x\n", eax
	lea	esi, [edx + UDP_Packet.Data]
	movzx	ecx, [edx + UDP_Packet.Length]
	rol	cx , 8
	sub	cx , UDP_Packet.Data

	inc	[UDP_PACKETS_RX]

	pop	edi
	add	esp, 4

	sub	esi, edi
	xchg	esi, edi
	jmp	socket_internal_receiver


  .updateport:

	push	ebx
	lea	ebx, [eax + SOCKET_head.lock]
	call	wait_mutex
	pop	ebx

	mov	si, [edx + UDP_Packet.SourcePort]
	DEBUGF	1,"Changing remote port to: %x\n", si
	mov	[eax + SOCKET_head.end + IPv4_SOCKET.end + UDP_SOCKET.RemotePort], si
	inc	[eax + SOCKET_head.end + IPv4_SOCKET.end + UDP_SOCKET.firstpacket]

	jmp	.ok2

  .checksum_mismatch:

	DEBUGF	2,"UDP_Handler - checksum mismatch\n"

	mov	esi, [esp]
	mov	ecx, [esp + 4]
       @@:			   ;
	lodsb			   ;
	DEBUGF	2,"%x ", eax:2	   ;
	loop	@r		   ;

  .dump:
	call	kernel_free
	add	esp, 4 ; pop (balance stack)
	DEBUGF	2,"UDP_Handler - dumping\n"

	ret




;-----------------------------------------------------------------
;
; UDP_socket_send
;
; IN: eax = socket pointer
;     ecx = number of bytes to send
;     esi = pointer to data
;
;-----------------------------------------------------------------

align 4
UDP_socket_send:

	mov	edx, dword [eax + SOCKET_head.end + IPv4_SOCKET.end + UDP_SOCKET.LocalPort] ; load local port and remote port at once
	DEBUGF	1,"local port: %x, remote port: %x\n",\
	[eax + SOCKET_head.end + IPv4_SOCKET.end + UDP_SOCKET.LocalPort]:4,\
	[eax + SOCKET_head.end + IPv4_SOCKET.end + UDP_SOCKET.RemotePort]:4
	mov	ebx, [eax + SOCKET_head.end + IPv4_SOCKET.LocalIP]
	mov	eax, [eax + SOCKET_head.end + IPv4_SOCKET.RemoteIP]

	DEBUGF	1,"Create UDP Packet (size=%u)\n",ecx

	mov	di , IP_PROTO_UDP
	sub	esp, 8						; reserve some place in stack for later
								; Create a part of the pseudoheader in stack,
	push	dword IP_PROTO_UDP shl 8
	add	ecx, UDP_Packet.Data

; TODO: fill in:   dx  = fragment id

	push	edx esi
	call	IPv4_create_packet				; TODO: figure out a way to choose between IPv4 and IPv6
	cmp	edi, -1
	je	.fail

	mov	[esp + 8 + 4], eax				; pointer to buffer start
	mov	[esp + 8 + 4 + 4], edx				; buffer size

	rol	cx, 8
	mov	[edi + UDP_Packet.Length], cx
	mov	[esp + 8 + 2], cx
	ror	cx, 8

	pop	esi
	push	edi ecx
	sub	ecx, UDP_Packet.Data
	add	edi, UDP_Packet.Data
	shr	ecx, 2
	rep	movsd
	mov	ecx, [esp]
	and	ecx, 3
	rep	movsb
	pop	ecx edi

	pop	dword [edi + UDP_Packet.SourcePort]		; fill in both portnumbers
	mov	[edi + UDP_Packet.Checksum], 0			; set it to zero, to calculate checksum

; Checksum for UDP header + data
	xor	edx, edx
	mov	esi, edi
	call	checksum_1
; Checksum for pseudoheader
	pushd	[edi-4] ; destination address           ; TODO: fix this, IPv4 packet could have options..
	pushd	[edi-8] ; source address
	call	checksum_pseudoheader
; Now create the final checksum and store it in UDP header
	call	checksum_2
	mov	[edi + UDP_Packet.Checksum], dx

	inc	[UDP_PACKETS_TX]

	DEBUGF	1,"Sending UDP Packet to device %x\n", ebx	;
	jmp	ETH_sender				       ;

  .fail:
	; todo: queue the packet
	add	esp, 8+12+8
	ret




;---------------------------------------------------------------------------
;
; UDP_API
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
UDP_API:

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
	add	eax, UDP_PACKETS_TX
	mov	eax, [eax]
	ret

.packets_rx:
	add	eax, UDP_PACKETS_RX
	mov	eax, [eax]
	ret
