;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2009. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  ICMP.INC                                                       ;;
;;                                                                 ;;
;;  Part of the tcp/ip network stack for KolibriOS                 ;;
;;                                                                 ;;
;;  Based on the work of [Johnny_B] and [smb]                      ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


$Revision$

; ICMP types & codes

ICMP_ECHOREPLY		equ	0		; echo reply message

ICMP_UNREACH		equ	3
	ICMP_UNREACH_NET	equ	0		; bad net
	ICMP_UNREACH_HOST	equ	1		; bad host
	ICMP_UNREACH_PROTOCOL	equ	2		; bad protocol
	ICMP_UNREACH_PORT	equ	3		; bad port
	ICMP_UNREACH_NEEDFRAG	equ	4		; IP_DF caused drop
	ICMP_UNREACH_SRCFAIL	equ	5		; src route failed
	ICMP_UNREACH_NET_UNKNOWN equ	6		; unknown net
	ICMP_UNREACH_HOST_UNKNOWN equ	7		; unknown host
	ICMP_UNREACH_ISOLATED	equ	8		; src host isolated
	ICMP_UNREACH_NET_PROHIB equ	9		; prohibited access
	ICMP_UNREACH_HOST_PROHIB equ	10		; ditto
	ICMP_UNREACH_TOSNET	equ	11		; bad tos for net
	ICMP_UNREACH_TOSHOST	equ	12		; bad tos for host
	ICMP_UNREACH_FILTER_PROHIB equ	13		; admin prohib
	ICMP_UNREACH_HOST_PRECEDENCE equ 14		; host prec vio.
	ICMP_UNREACH_PRECEDENCE_CUTOFF equ 15		; prec cutoff

ICMP_SOURCEQUENCH	equ	4		; Packet lost, slow down

ICMP_REDIRECT		equ	5		; shorter route, codes:
	ICMP_REDIRECT_NET	equ	0		; for network
	ICMP_REDIRECT_HOST	equ	1		; for host
	ICMP_REDIRECT_TOSNET	equ	2		; for tos and net
	ICMP_REDIRECT_TOSHOST	equ	3		; for tos and host

ICMP_ALTHOSTADDR	equ	6		; alternate host address
ICMP_ECHO		equ	8		; echo service
ICMP_ROUTERADVERT	equ	9		; router advertisement
	ICMP_ROUTERADVERT_NORMAL equ 0			; normal advertisement
	ICMP_ROUTERADVERT_NOROUTE_COMMON equ 16 	; selective routing

ICMP_ROUTERSOLICIT	equ	10		; router solicitation
ICMP_TIMXCEED		equ	11		; time exceeded, code:
    ICMP_TIMXCEED_INTRANS	equ	0		; ttl==0 in transit
    ICMP_TIMXCEED_REASS equ	1		; ttl==0 in reass

ICMP_PARAMPROB		  equ  12		; ip header bad
    ICMP_PARAMPROB_ERRATPTR   equ  0		; error at param ptr
    ICMP_PARAMPROB_OPTABSENT  equ  1		; req. opt. absent
    ICMP_PARAMPROB_LENGTH     equ  2		; bad length

ICMP_TSTAMP		equ	13		; timestamp request
ICMP_TSTAMPREPLY	equ	14		; timestamp reply
ICMP_IREQ		equ	15		; information request
ICMP_IREQREPLY		equ	16		; information reply
ICMP_MASKREQ		equ	17		; address mask request
ICMP_MASKREPLY		equ	18		; address mask reply
ICMP_TRACEROUTE 	equ	30		; traceroute
ICMP_DATACONVERR	equ	31		; data conversion error
ICMP_MOBILE_REDIRECT	equ	32		; mobile host redirect
ICMP_IPV6_WHEREAREYOU	equ	33		; IPv6 where-are-you
 ICMP_IPV6_IAMHERE	equ	34		; IPv6 i-am-here
ICMP_MOBILE_REGREQUEST	equ	35		; mobile registration req
ICMP_MOBILE_REGREPLY	equ	36		; mobile registreation reply
ICMP_SKIP		equ	39		; SKIP

ICMP_PHOTURIS		equ	40		; Photuris
    ICMP_PHOTURIS_UNKNOWN_INDEX   equ  1		; unknown sec index
    ICMP_PHOTURIS_AUTH_FAILED	  equ  2		; auth failed
    ICMP_PHOTURIS_DECRYPT_FAILED  equ  3		; decrypt failed



struct	ICMP_Packet
	.Type		db   ?
	.Code		db   ?
	.Checksum	dw   ?
	.Identifier	dw   ?
	.SequenceNumber dw   ?
	.Data:
ends


align 4
uglobal
	ICMP_PACKETS_TX 	rd  MAX_IP
	ICMP_PACKETS_RX 	rd  MAX_IP
endg



;-----------------------------------------------------------------
;
; ICMP_init
;
;  This function resets all ICMP variables
;
;  IN:  /
;  OUT: /
;
;-----------------------------------------------------------------
align 4
ICMP_init:

	xor	eax, eax
	mov	edi, ICMP_PACKETS_TX
	mov	ecx, 2*MAX_IP
	rep	stosd

	ret
												    
													     


;-----------------------------------------------------------------
;
; ICMP_Handler:
;
;  this procedure will send reply's to ICMP echo's
;  and insert packets into sockets when needed                         ;;; TODO: update this to work with fragmented packets too!
;
;  IN:  Pointer to buffer in [esp]
;       size of buffer in [esp+4]
;       pointer to device struct in ebx
;       ICMP Packet size in ecx
;       pointer to ICMP Packet data in edx
;  OUT: /
;
;-----------------------------------------------------------------
align 4
ICMP_handler:	;TODO: works only on pure ethernet right now !

	DEBUGF	1,"ICMP_Handler - buf:%x size:%x dev:%x, size:%x, buf:%x\n", [esp], [esp+4], ebx, ecx, edx
	cmp	byte [edx + ICMP_Packet.Type], ICMP_ECHO		    ; Is this an echo request?
	jne	.check_sockets

;;; TODO: check checksum!

	DEBUGF	1,"ICMP_Handler - is echo request, through device:%x\n", ebx

	mov	byte [edx + ICMP_Packet.Type], ICMP_ECHOREPLY		    ; Change Packet type to reply
	mov	word [edx + ICMP_Packet.Checksum], 0			    ; Set checksum to 0, needed to calculate new checksum

	call	ETH_struc2dev
	cmp	edi,-1
	je	.dump
	inc	[ICMP_PACKETS_RX+4*edi]
	inc	[ICMP_PACKETS_TX+4*edi]

; exchange dest and source address in IP header
; exchange dest and source MAC in ETH header
	mov	esi, [esp]

	mov	eax, dword [esi + ETH_FRAME.DstMAC]
	mov	ecx, dword [esi + ETH_FRAME.SrcMAC]
	mov	dword [esi + ETH_FRAME.SrcMAC], eax
	mov	dword [esi + ETH_FRAME.DstMAC], ecx

	mov	ax, word [esi + ETH_FRAME.DstMAC + 4]
	mov	cx, word [esi + ETH_FRAME.SrcMAC + 4]
	mov	word [esi + ETH_FRAME.SrcMAC + 4], ax
	mov	word [esi + ETH_FRAME.DstMAC + 4], cx

	mov	eax, dword [esi + ETH_FRAME.Data + IPv4_Packet.SourceAddress]
	mov	ecx, dword [esi + ETH_FRAME.Data + IPv4_Packet.DestinationAddress]
	mov	dword [esi + ETH_FRAME.Data + IPv4_Packet.DestinationAddress], eax
	mov	dword [esi + ETH_FRAME.Data + IPv4_Packet.SourceAddress], ecx

; Recalculate ip header checksum
	add	esi, ETH_FRAME.Data					    ; Point esi to start of IP Packet
	movzx	ecx, byte [esi + IPv4_Packet.VersionAndIHL]		    ; Calculate IP Header length by using IHL field
	and	ecx, 0x0000000F 					    ;
	shl	cx , 2
	push	ebx edx ecx esi
	xor	edx, edx
	call	checksum_1
	call	checksum_2
	pop	esi
	mov	word [esi + IPv4_Packet.HeaderChecksum], dx		    ; Store it in the IP Packet header

; Recalculate ICMP CheckSum
	movzx	eax, word[esi + IPv4_Packet.TotalLength]		    ; Find length of IP Packet
	xchg	ah , al 						    ;
	sub	eax, [esp]						     ; Now we know the length of ICMP data in eax
	mov	ecx, eax
	mov	esi, [esp + 4]
	xor	edx, edx
	call	checksum_1
	call	checksum_2
	mov	ax , dx
	pop	ecx edx ebx
	mov	word [edx + ICMP_Packet.Checksum], ax

	jmp	ETH_sender						    ; Send the reply





       .check_sockets:
	; TODO: validate the header & checksum.

	; Look for an open ICMP socket

	mov	esi, net_sockets
  .try_more:
	mov	ax , [edx + ICMP_Packet.Identifier]
  .next_socket:
	mov	esi, [esi + SOCKET_head.NextPtr]
	or	esi, esi
	jz	.dump
	cmp	[esi + SOCKET_head.Type], IP_PROTO_ICMP
	jne	.next_socket
	cmp	[esi + SOCKET_head.end + IPv4_SOCKET.end + ICMP_SOCKET.Identifier], ax
	jne	.next_socket

	call	IPv4_dest_to_dev
	cmp	edi,-1
	je	.dump
	inc	[ICMP_PACKETS_RX+4*edi]

	DEBUGF 1,"Found valid ICMP packet for socket %x\n", esi

	lea	ebx, [esi + SOCKET_head.lock]
	call	wait_mutex

	; Now, assign data to socket. We have socket address in esi.
	; We have ICMP Packet in edx
	; number of bytes in ecx

	mov	eax, esi
	pop	esi
	add	esp, 4
	sub	edx, esi
	mov	edi, edx
	jmp	socket_internal_receiver

       .dump:
	DEBUGF	1,"ICMP_Handler - dumping\n"

	call	kernel_free
	add	esp, 4 ; pop (balance stack)

	ret


;-----------------------------------------------------------------
;
; ICMP_Handler_fragments:
;
;  Called by IP_handler,
;  this procedure will send reply's to ICMP echo's etc
;
;  IN:  Pointer to buffer in [esp]
;       size of buffer in [esp+4]
;       pointer to device struct in ebx
;       ICMP Packet size in ecx
;       pointer to ICMP Packet data in edx
;  OUT: /
;
;-----------------------------------------------------------------
align 4
ICMP_handler_fragments:   ; works only on pure ethernet right now !

	DEBUGF	1,"ICMP_Handler_fragments - start\n"

	cmp	ecx, 65500
	jg	.dump

	cmp	byte [edx + ICMP_Packet.Type], ICMP_ECHO		    ; Is this an echo request? discard if not
	jne	.dump

	mov	esi, [esp]

	sub	ecx, ICMP_Packet.Data
	mov	eax, [esi + IPv4_Packet.SourceAddress]
	mov	ebx, [esi + IPv4_Packet.DestinationAddress]
	push	word [esi + IPv4_Packet.Identification]

	mov	di , [edx + ICMP_Packet.Identifier]
	shl	edi, 16
	mov	di , [edx + ICMP_Packet.SequenceNumber]

	mov	esi, edx
	add	esi, ICMP_Packet.Data
	pop	dx
	shl	edx, 16
	mov	dx , ICMP_ECHOREPLY shl 8 + 0				    ; Type + Code

	call	ICMP_create_packet

       .dump:
	DEBUGF	1,"ICMP_Handler_fragments - end\n"

	call	kernel_free
	add	esp, 4 ; pop (balance stack)
	ret


;-----------------------------------------------------------------
;
; Note: ICMP only works on top of IP protocol :)
;
; inputs:
;
; eax = dest ip
; ebx = source ip
; ecx = data length
; dh = type
; dl = code
; high 16 bits of edx = fragment id (for IP header)
; esi = data offset
; edi = identifier shl 16 + sequence number
;
;-----------------------------------------------------------------
align 4
ICMP_create_packet:

	DEBUGF 1,"Create ICMP Packet\n"

	push	esi edi edx

	add	ecx, ICMP_Packet.Data
	mov	di , IP_PROTO_ICMP
	shr	edx, 16

	call	IPv4_create_packet

	cmp	edi, -1
	je	.exit

	DEBUGF 1,"full icmp packet size: %u\n", edx

	pop	eax
	mov	word [edi + ICMP_Packet.Type], ax	; Write both type and code bytes at once
	pop	eax
	mov	[edi + ICMP_Packet.SequenceNumber], ax
	shr	eax, 16
	mov	[edi + ICMP_Packet.Identifier], ax
	mov	[edi + ICMP_Packet.Checksum], 0

	push	eax ebx ecx edx
	mov	esi, edi
	xor	edx, edx
	call	checksum_1
	call	checksum_2
	mov	[edi + ICMP_Packet.Checksum], dx
	pop	edx ecx ebx eax esi

	sub	ecx, ICMP_Packet.Data
	add	edi, ICMP_Packet.Data
	push	cx
	shr	cx , 2
	rep	movsd
	pop	cx
	and	cx , 3
	rep	movsb

	sub	edi, edx  ;; TODO: find a better way to remember start of packet
	mov	ecx, [ebx + ETH_DEVICE.transmit]
	push	edx edi ecx
	DEBUGF 1,"Sending ICMP Packet\n"
	ret						; Send the packet (create_packet routine outputs pointer to routine to send packet in eax)

  .exit:
	DEBUGF 1,"Creating ICMP Packet failed\n"
	add	esp, 3*4
	ret




;-----------------------------------------------------------------
;
; ICMP_API
;
; This function is called by system function 75
;
; IN:  subfunction number in bl
;      device number in bh
;      ecx, edx, .. depends on subfunction
;
; OUT:
;
;-----------------------------------------------------------------
align 4
ICMP_API:

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
	add	eax, ICMP_PACKETS_TX
	mov	eax, [eax]
	ret

.packets_rx:
	add	eax, ICMP_PACKETS_RX
	mov	eax, [eax]
	ret
