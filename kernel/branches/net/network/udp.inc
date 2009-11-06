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

	DEBUGF 1,"UDP_Handler\n"
	; First validate, checksum:

	DEBUGF 1,"Real UDP checksum: %x\n", [edx + UDP_Packet.Checksum]:4
	mov    [edx + UDP_Packet.Checksum], 0

	pusha

	rol	cx, 8
	push	cx
	rol	cx, 8
	push	word IP_PROTO_UDP shl 8
	push	edi
	push	esi

	mov	esi, edx
	xor	edx, edx
	call	checksum_1
; Checksum for pseudoheader
	mov	ecx, 12
	mov	esi, esp
	call	checksum_1
	add	esp, 12
	call	checksum_2

	popa



	; Look for a socket where
	; IP Packet UDP Destination Port = local Port
	; IP Packet SA = Remote IP

	mov	eax, net_sockets
  .try_more:
	mov	bx , [edx + UDP_Packet.DestinationPort]   ; get the local port from the IP Packet's UDP header
  .next_socket:
	mov	eax, [eax + SOCKET_head.NextPtr]
	or	eax, eax
	jz	.dump
	cmp	[eax + SOCKET_head.Domain], AF_INET4
	jne	.next_socket
	cmp	[eax + SOCKET_head.Type], IP_PROTO_UDP
	jne	.next_socket
	cmp	[eax + SOCKET_head.end + IPv4_SOCKET.end + UDP_SOCKET.LocalPort], bx
	jne	.next_socket

	DEBUGF 1,"found socket with matching domain, type and localport\n"

	; For dhcp, we must allow any remote server to respond.
	; I will accept the first incoming response to be the one
	; I bind to, if the socket is opened with a destination IP address of
	; 255.255.255.255
	cmp	[eax + SOCKET_head.end + IPv4_SOCKET.RemoteIP], 0xffffffff
	je	.ok1

	mov	ebx, [esp]
	mov	ebx, [ebx + ETH_FRAME.Data + IPv4_Packet.SourceAddress] ; get the Source address from the IP Packet   FIXME
	cmp	[eax + SOCKET_head.end + IPv4_SOCKET.RemoteIP], ebx
	jne	.try_more					      ; Quit if the source IP is not valid, check for more sockets with this IP/PORT combination


	DEBUGF 1,"Remote Ip matches\n"
  .ok1:

	mov	bx, [edx + UDP_Packet.SourcePort]		      ; Remote port must be 0, or equal to sourceport of packet

	cmp	[eax + SOCKET_head.end + IPv4_SOCKET.end + UDP_SOCKET.RemotePort], 0
	je	.ok2

	cmp	[eax + SOCKET_head.end + IPv4_SOCKET.end + UDP_SOCKET.RemotePort], bx
	jne	.dump

  .ok2:

	DEBUGF 1,"Found valid UDP packet for socket %x\n", eax
	lea	esi, [edx + UDP_Packet.Data]
	movzx	ecx, [edx + UDP_Packet.Length]
	rol	cx , 8
	sub	cx , UDP_Packet.Data
	mov	dx , bx


	lea	ebx, [eax + SOCKET_head.lock]
	call	wait_mutex
	mov	[eax + SOCKET_head.end + IPv4_SOCKET.end + UDP_SOCKET.RemotePort], dx	       ; update remote port number
	mov	[eax + SOCKET_head.end + IPv4_SOCKET.RemoteIP], edi
	inc	[UDP_PACKETS_RX]

	pop	edi
	add	esp, 4

	sub	esi, edi
	xchg	esi, edi
	jmp	socket_internal_receiver


  .dump:
	DEBUGF 1,"Dumping UDP packet\n"
	call	kernel_free
	add	esp, 4 ; pop (balance stack)

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

	DEBUGF 1,"Create UDP Packet (size=%u)\n",ecx

	mov	di , IP_PROTO_UDP

	sub	esp, 8						; reserve some place in stack for later

; Create the pseudoheader in stack,
; (now that we still have all the variables that are needed.)
	push	dword IP_PROTO_UDP shl 8
	push	eax
	push	ebx

	add	ecx, UDP_Packet.Data

; TODO: fill in:   dx  = fragment id

	push	edx esi
	call	IPv4_create_packet				; TODO: figure out a way to choose between IPv4 and IPv6
	cmp	edi, -1
	je	.fail

	mov	[esp + 8 + 12], eax				; pointer to buffer start
	mov	[esp + 8 + 12 + 4], edx 			; buffer size

	rol	cx, 8
	mov	[edi + UDP_Packet.Length], cx
	mov	[esp + 8 + 10], cx
	ror	cx, 8

	pop	esi
	push	edi ecx
	sub	ecx, UDP_Packet.Data
	add	edi, UDP_Packet.Data
	shr	ecx, 2
	rep	movsd
	mov	ecx, [esp]
	and	cx , 3
	rep	movsb
	pop	ecx edi

	pop	dword [edi + UDP_Packet.SourcePort]		      ; fill in both portnumbers
	mov	[edi + UDP_Packet.Checksum], 0			; set it to zero, to calculate checksum

; Checksum for UDP header + data
	xor	edx, edx
	mov	esi, edi
	call	checksum_1
; Checksum for pseudoheader
	mov	ecx, 12
	mov	esi, esp
	call	checksum_1
	add	esp, 12 					; remove the pseudoheader from stack
; Now create the final checksum and store it in UDP header
	call	checksum_2
	mov	[edi + UDP_Packet.Checksum], dx

	inc	[UDP_PACKETS_TX]

	DEBUGF 1,"Sending UDP Packet to device %x\n", ebx      ;
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
