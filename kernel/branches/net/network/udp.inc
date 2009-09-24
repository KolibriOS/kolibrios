;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved.    ;;
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

$Revision: 983 $


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

UDP_Handler:

	DEBUGF 1,"UDP_Handler\n"
	; TODO: First validate the header & checksum. Discard buffer if error

	; Look for a socket where
	; IP Packet UDP Destination Port = local Port
	; IP Packet SA = Remote IP

	mov	esi, net_sockets
  .try_more:
	mov	ax , [edx + UDP_Packet.DestinationPort]   ; get the local port from the IP Packet's UDP header
	rol	ax , 8
  .next_socket:
	mov	esi, [esi + SOCKET.NextPtr]
	or	esi, esi
	jz	.dump
	cmp	[esi + SOCKET.Type], IP_PROTO_UDP
	jne	.next_socket
	cmp	[esi + SOCKET.LocalPort], ax
	jne	.next_socket

	; For dhcp, we must allow any remote server to respond.
	; I will accept the first incoming response to be the one
	; I bind to, if the socket is opened with a destination IP address of
	; 255.255.255.255
	cmp	[esi + SOCKET.RemoteIP], 0xffffffff
	je	@f

	mov	eax, [esp]
	mov	eax, [eax + ETH_FRAME.Data + IPv4_Packet.SourceAddress] ; get the Source address from the IP Packet
	cmp	[esi + SOCKET.RemoteIP], eax
	jne	.try_more					      ; Quit if the source IP is not valid, check for more sockets with this IP/PORT combination

    @@:
	DEBUGF 1,"Found valid UDP packet for socket %x\n", esi


;        sub     ecx, UDP_Packet.Data                    ; get # of bytes in ecx
;        mov     eax, ecx

	movzx	ecx, [edx + UDP_Packet.Length]
	xchg	cl , ch

;        cmp     ecx, eax                                ; If UDP packet size is bigger then IP packet told us,
;        jg      .error                                  ; Something must went wrong!

	lea	ebx, [esi + SOCKET.lock]
	call	wait_mutex

	; OK - we have a valid UDP Packet for this socket.
	; First, update the sockets remote port number with the incoming msg
	; - it will have changed
	; from the original ( 69 normally ) to allow further connects
	mov	ax, [edx + UDP_Packet.SourcePort]	   ; get the UDP source port
	xchg	al, ah
	mov	[esi + SOCKET.RemotePort], ax

	; Now, copy data to socket. We have socket address as [eax + sockets].
	; We have IP Packet in edx

	add	edx, UDP_Packet.Data
	mov	eax, [esi + SOCKET.rxDataCount] 	; get # of bytes already in buffer
	DEBUGF 1,"bytes in socket: %u ", eax
	lea	edi, [ecx + eax]			; check for buffer overflow
	cmp	edi, SOCKETBUFFSIZE - SOCKETHEADERSIZE	;
	jg	.dump					;
	add	[esi + SOCKET.rxDataCount], ecx 	; increment the count of bytes in buffer
	DEBUGF 1,"adding %u bytes\n", ecx

	; ecx has count, edx points to data

	lea	edi, [esi + eax + SOCKETHEADERSIZE]
	push	esi
	push	ecx
	mov	esi, edx
	shr	ecx, 2
	rep	movsd	       ; copy the data across
	pop	ecx
	and	ecx, 3
	rep	movsb
	pop	esi

	DEBUGF 1,"UDP socket updated\n"

	mov	[esi + SOCKET.lock], 0

	; flag an event to the application
	mov	eax, [esi + SOCKET.PID] 		; get socket owner PID
	mov	ecx, 1
	mov	esi, TASK_DATA + TASKDATA.pid

       .next_pid:
	cmp	[esi], eax
	je	.found_pid
	inc	ecx
	add	esi, 0x20
	cmp	ecx, [TASK_COUNT]
	jbe	.next_pid

	jmp	.dump

       .found_pid:
	shl	ecx, 8
	or	[ecx + SLOT_BASE + APPDATA.event_mask], EVENT_NETWORK ; stack event

	mov	[check_idle_semaphore], 200

       .dump:
	DEBUGF 1,"UDP_handler - dumping\n"

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
;     edx = remote port shl 16 + local port
;     esi = data offset
;
;-----------------------------------------------------------------

UDP_create_Packet:

	DEBUGF 1,"Create UDP Packet\n"

	push	edx esi

	add	ecx, UDP_Packet.Data
	mov	di , IP_PROTO_UDP

;       dx  = fragment id

	call	IPv4_create_Packet				; TODO: figure out a way to choose between IPv4 and IPv6
	cmp	edi, -1
	je	.exit

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
	bswap	ecx						; convert little endian - big endian
	rol	ecx, 16 					;
	mov	dword [edi + UDP_Packet.SourcePort], ecx	; notice: we write both port's at once


	mov	[edi + UDP_Packet.Checksum], 0

	; TODO: calculate checksum using Pseudo-header  (However, using a 0 as checksum shouldnt generate any errors :)

	push	ebx eax 		     ; TODO: make this work on other protocols besides ethernet
	mov	ebx,edx 		     ;
	DEBUGF 1,"Sending UDP Packet to device %x\n", ebx      ;
	jmp	ETH_Sender		     ;

  .exit:
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
