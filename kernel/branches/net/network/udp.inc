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
	; TODO: First validate the header & checksum!

	; Look for a socket where
	; IP Packet UDP Destination Port = local Port
	; IP Packet SA = Remote IP

	mov	eax, net_sockets
  .try_more:
	mov	bx , [edx + UDP_Packet.DestinationPort]   ; get the local port from the IP Packet's UDP header
  .next_socket:
	mov	eax, [eax + SOCKET.NextPtr]
	or	eax, eax
	jz	.dump
	cmp	[eax + SOCKET.Domain], AF_INET4
	jne	.next_socket
	cmp	[eax + SOCKET.Type], IP_PROTO_UDP
	jne	.next_socket
	cmp	[eax + SOCKET.LocalPort], bx
	jne	.next_socket

	; For dhcp, we must allow any remote server to respond.
	; I will accept the first incoming response to be the one
	; I bind to, if the socket is opened with a destination IP address of
	; 255.255.255.255
	cmp	[eax + SOCKET.RemoteIP], 0xffffffff
	je	.ok1

	mov	ebx, [esp]
	mov	ebx, [ebx + ETH_FRAME.Data + IPv4_Packet.SourceAddress] ; get the Source address from the IP Packet
	cmp	[eax + SOCKET.RemoteIP], eax
	jne	.try_more					      ; Quit if the source IP is not valid, check for more sockets with this IP/PORT combination

  .ok1:

	mov	bx, [edx + UDP_Packet.SourcePort]		      ; Remote port must be 0, or equal to sourceport of packet

	cmp	[eax + SOCKET.RemotePort],0
	je	.ok2

	cmp	[eax + SOCKET.RemotePort], bx
	jne	.dump

  .ok2:

	DEBUGF 1,"Found valid UDP packet for socket %x\n", eax
	lea	esi, [edx + UDP_Packet.Data]
	movzx	ecx, [edx + UDP_Packet.Length]
	rol	cx , 8
	sub	cx , UDP_Packet.Data
	mov	dx , bx

	call	socket_internal_receiver

	inc	[UDP_PACKETS_RX]

  .dump:
	DEBUGF 1,"Dumping UDP packet\n"
	call	kernel_free
	add	esp, 4 ; pop (balance stack)

	ret




;-----------------------------------------------------------------
;
; Note: UDP works only on top of IP protocol :)
;
; IN: eax = dest ip
;     ebx = source ip
;     ecx = data length
;     edx = remote port shl 16 + local port (both in INET order)
;     esi = data offset
;
;-----------------------------------------------------------------

UDP_create_packet:

	DEBUGF 1,"Create UDP Packet (size=%u)\n",ecx

	push	edx esi

	add	ecx, UDP_Packet.Data
	mov	di , IP_PROTO_UDP

;       dx  = fragment id

	call	IPv4_create_packet				; TODO: figure out a way to choose between IPv4 and IPv6
	cmp	edi, -1
	je	.fail

	mov	byte[edi + UDP_Packet.Length], ch
	mov	byte[edi + UDP_Packet.Length+1], cl
	sub	ecx , UDP_Packet.Data

	pop	esi
	push	edi
	add	edi, UDP_Packet.Data
	push	cx
	shr	ecx, 2
	rep	movsd
	pop	cx
	and	cx , 3
	rep	movsb
	pop	edi

	pop	ecx
	mov	dword [edi + UDP_Packet.SourcePort], ecx	; notice: we write both port's at once

	mov	[edi + UDP_Packet.Checksum], 0

	; TODO: calculate checksum using Pseudo-header  (However, using a 0 as checksum shouldnt generate any errors :)

	inc	[UDP_PACKETS_TX]

	push	edx eax 		     ; TODO: make this work on other protocols besides ethernet
	DEBUGF 1,"Sending UDP Packet to device %x\n", ebx      ;
	jmp	ETH_Sender		     ;

  .exit:
	ret

  .fail:
	; todo: queue the packet
	add	esp, 8
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
