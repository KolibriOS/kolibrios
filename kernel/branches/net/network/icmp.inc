;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2010. All rights reserved.    ;;
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

ICMP_ECHOREPLY			equ 0		    ; echo reply message

ICMP_UNREACH			equ 3
ICMP_UNREACH_NET		equ  0		     ; bad net
ICMP_UNREACH_HOST		equ  1		     ; bad host
ICMP_UNREACH_PROTOCOL		equ  2		     ; bad protocol
ICMP_UNREACH_PORT		equ  3		     ; bad port
ICMP_UNREACH_NEEDFRAG		equ  4		     ; IP_DF caused drop
ICMP_UNREACH_SRCFAIL		equ  5		     ; src route failed
ICMP_UNREACH_NET_UNKNOWN	equ  6		     ; unknown net
ICMP_UNREACH_HOST_UNKNOWN	equ  7		     ; unknown host
ICMP_UNREACH_ISOLATED		equ  8		     ; src host isolated
ICMP_UNREACH_NET_PROHIB 	equ  9		     ; prohibited access
ICMP_UNREACH_HOST_PROHIB	equ 10		    ; ditto
ICMP_UNREACH_TOSNET		equ 11		    ; bad tos for net
ICMP_UNREACH_TOSHOST		equ 12		    ; bad tos for host
ICMP_UNREACH_FILTER_PROHIB	equ 13		    ; admin prohib
ICMP_UNREACH_HOST_PRECEDENCE	equ 14		   ; host prec vio.
ICMP_UNREACH_PRECEDENCE_CUTOFF	equ 15		 ; prec cutoff

ICMP_SOURCEQUENCH		equ 4		    ; Packet lost, slow down

ICMP_REDIRECT			equ 5		    ; shorter route, codes:
ICMP_REDIRECT_NET		equ  0		     ; for network
ICMP_REDIRECT_HOST		equ  1		     ; for host
ICMP_REDIRECT_TOSNET		equ  2		     ; for tos and net
ICMP_REDIRECT_TOSHOST		equ  3		     ; for tos and host

ICMP_ALTHOSTADDR		equ 6		    ; alternate host address
ICMP_ECHO			equ  8		     ; echo service
ICMP_ROUTERADVERT		equ  9		     ; router advertisement
ICMP_ROUTERADVERT_NORMAL	equ  0			; normal advertisement
ICMP_ROUTERADVERT_NOROUTE_COMMON equ 16 	; selective routing

ICMP_ROUTERSOLICIT		equ 10		    ; router solicitation
ICMP_TIMXCEED			equ 11		    ; time exceeded, code:
ICMP_TIMXCEED_INTRANS		equ 0		    ; ttl==0 in transit
ICMP_TIMXCEED_REASS		equ 1		    ; ttl==0 in reass

ICMP_PARAMPROB			equ 12		     ; ip header bad
ICMP_PARAMPROB_ERRATPTR 	equ 0		 ; error at param ptr
ICMP_PARAMPROB_OPTABSENT	equ 1		 ; req. opt. absent
ICMP_PARAMPROB_LENGTH		equ 2		 ; bad length

ICMP_TSTAMP			equ 13		    ; timestamp request
ICMP_TSTAMPREPLY		equ 14		    ; timestamp reply
ICMP_IREQ			equ 15		    ; information request
ICMP_IREQREPLY			equ 16		    ; information reply
ICMP_MASKREQ			equ 17		    ; address mask request
ICMP_MASKREPLY			equ 18		    ; address mask reply
ICMP_TRACEROUTE 		equ 30		    ; traceroute
ICMP_DATACONVERR		equ 31		    ; data conversion error
ICMP_MOBILE_REDIRECT		equ 32		    ; mobile host redirect
ICMP_IPV6_WHEREAREYOU		equ 33		    ; IPv6 where-are-you
ICMP_IPV6_IAMHERE		equ 34		    ; IPv6 i-am-here
ICMP_MOBILE_REGREQUEST		equ 35		    ; mobile registration req
ICMP_MOBILE_REGREPLY		equ 36		    ; mobile registreation reply
ICMP_SKIP			equ 39		    ; SKIP

ICMP_PHOTURIS			equ 40		    ; Photuris
ICMP_PHOTURIS_UNKNOWN_INDEX	equ 1		     ; unknown sec index
ICMP_PHOTURIS_AUTH_FAILED	equ 2		     ; auth failed
ICMP_PHOTURIS_DECRYPT_FAILED	equ 3		     ; decrypt failed



struct	ICMP_header

	Type			db   ?
	Code			db   ?
	Checksum		dw   ?
	Identifier		dw   ?
	SequenceNumber		dw   ?

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
;-----------------------------------------------------------------

macro ICMP_init {

	xor	eax, eax
	mov	edi, ICMP_PACKETS_TX
	mov	ecx, 2*MAX_IP
	rep	stosd

}


;-----------------------------------------------------------------
;
; ICMP_input:
;
;  This procedure will send reply's to ICMP echo's
;   and insert packets into sockets when needed
;
;  IN:  Pointer to buffer in [esp]
;       size of buffer in [esp+4]
;       ebx = pointer to device struct
;       ecx = ICMP Packet size
;       edx = ptr to ICMP Packet data
;       esi = ipv4 source address
;       edi = ipv4 dest   address
;  OUT: /
;
;-----------------------------------------------------------------
align 4
ICMP_input:

	DEBUGF	1,"ICMP_input - start\n"

; First, check the checksum (altough some implementations ignore it)

	push	edx esi ecx
	push	[edx + ICMP_header.Checksum]
	mov	[edx + ICMP_header.Checksum], 0
	mov	esi, edx
	xor	edx, edx
	call	checksum_1
	call	checksum_2
	pop	si
	cmp	dx, si
	pop	ecx esi edx
	jne	.checksum_mismatch

	cmp	[edx + ICMP_header.Type], ICMP_ECHO		; Is this an echo request?
	jne	.check_sockets

; We well re-use the packet sow e can create the response as fast as possible
; Notice: this only works on pure ethernet (however, IP packet options are not a problem this time :)

	DEBUGF	1,"ICMP_input - echo request\n"
	mov	[edx + ICMP_header.Type], ICMP_ECHOREPLY	; Change Packet type to reply

; Update stats (and validate device ptr)
	call	NET_ptr_to_num
	cmp	edi,-1
	je	.dump
	inc	[ICMP_PACKETS_RX+4*edi]
	inc	[ICMP_PACKETS_TX+4*edi]

; exchange dest and source address in IP header
; exchange dest and source MAC in ETH header
	mov	esi, [esp]					; Start of buffer
	push	dword [esi + ETH_header.DstMAC]
	push	dword [esi + ETH_header.SrcMAC]
	pop	dword [esi + ETH_header.DstMAC]
	pop	dword [esi + ETH_header.SrcMAC]
	push	word [esi + ETH_header.DstMAC + 4]
	push	word [esi + ETH_header.SrcMAC + 4]
	pop	word [esi + ETH_header.DstMAC + 4]
	pop	word [esi + ETH_header.SrcMAC + 4]

	add	esi, sizeof.ETH_header
	push	[esi + IPv4_header.SourceAddress]
	push	[esi + IPv4_header.DestinationAddress]
	pop	[esi + IPv4_header.SourceAddress]
	pop	[esi + IPv4_header.DestinationAddress]

; Recalculate ip header checksum
	movzx	ecx, [esi + IPv4_header.VersionAndIHL]		; Calculate IP Header length by using IHL field
	and	ecx, 0x0f
	shl	cx, 2
	mov	edi, ecx					; IP header length
	mov	eax, edx					; ICMP packet start addr

	push	esi						; Calculate the IP checksum
	xor	edx, edx					;
	call	checksum_1					;
	call	checksum_2					;
	pop	esi						;
	mov	[esi + IPv4_header.HeaderChecksum], dx		;

; Recalculate ICMP CheckSum
	movzx	ecx, [esi + IPv4_header.TotalLength]		; Find length of IP Packet
	xchg	ch, cl						;
	sub	ecx, edi					; IP packet length - IP header length = ICMP packet length

	mov	esi, eax					; Calculate ICMP checksum
	xor	edx, edx					;
	call	checksum_1					;
	call	checksum_2					;
	mov	[eax + ICMP_header.Checksum], dx		;

; Transmit the packet (notice that packet ptr and packet size have been on stack since start of the procedure!)
	call	[ebx + NET_DEVICE.transmit]
	ret




       .check_sockets:
	; Look for an open ICMP socket
	; esi = sender ip

	mov	ebx, net_sockets
  .try_more:
;        mov     ax , [edx + ICMP_header.Identifier]
  .next_socket:
	mov	ebx, [ebx + SOCKET.NextPtr]
	or	ebx, ebx
	jz	.dump

	cmp	[ebx + SOCKET.Domain], AF_INET4
	jne	.next_socket

	cmp	[ebx + SOCKET.Protocol], IP_PROTO_ICMP
	jne	.next_socket

	cmp	[ebx + IP_SOCKET.RemoteIP], esi
	jne	.next_socket

;        cmp     [esi + ICMP_SOCKET.Identifier], ax
;        jne     .next_socket

;        call    IPv4_dest_to_dev
;        cmp     edi,-1
;        je      .dump
;        inc     [ICMP_PACKETS_RX+edi]

	DEBUGF 1,"Found valid ICMP packet for socket %x\n", ebx

	mov	eax, ebx
	add	ebx, SOCKET.lock
	call	wait_mutex

	mov	esi, edx
	jmp	SOCKET_input


  .checksum_mismatch:
	DEBUGF	1,"ICMP_Handler - checksum mismatch\n"

  .dump:
	DEBUGF	1,"ICMP_Handler - dumping\n"

	call	kernel_free
	add	esp, 4 ; pop (balance stack)

	ret


;-----------------------------------------------------------------
;
; ICMP_output
;
; IN:  eax = dest ip
;      ebx = source ip
;      ecx = data length
;      dh  = type
;      dl  = code
;      high 16 bits of edx = fragment id (for IP header)
;      esi = data offset
;      edi = identifier shl 16 + sequence number
;
;-----------------------------------------------------------------
align 4
ICMP_output:

	DEBUGF	1,"Creating ICMP Packet\n"

	push	esi edi edx

	mov	ebx, [eax + IP_SOCKET.LocalIP]
	mov	eax, [eax + IP_SOCKET.RemoteIP]
	add	ecx, sizeof.ICMP_header
	mov	di , IP_PROTO_ICMP SHL 8 + 128	; TTL
	shr	edx, 16

	call	IPv4_output
	jz	.exit

	DEBUGF	1,"full icmp packet size: %u\n", edx

	pop	eax
	mov	word [edi + ICMP_header.Type], ax	; Write both type and code bytes at once
	pop	eax
	mov	[edi + ICMP_header.SequenceNumber], ax
	shr	eax, 16
	mov	[edi + ICMP_header.Identifier], ax
	mov	[edi + ICMP_header.Checksum], 0

	push	eax ebx ecx edx
	mov	esi, edi
	xor	edx, edx
	call	checksum_1
	call	checksum_2
	mov	[edi + ICMP_header.Checksum], dx
	pop	edx ecx ebx eax esi

	sub	ecx, sizeof.ICMP_header
	add	edi, sizeof.ICMP_header
	push	cx
	shr	cx , 2
	rep	movsd
	pop	cx
	and	cx , 3
	rep	movsb

	sub	edi, edx				;;; TODO: find a better way to remember start of packet
	push	edx edi
	DEBUGF	1,"Sending ICMP Packet\n"
	call	[ebx + NET_DEVICE.transmit]
	ret
  .exit:
	DEBUGF	1,"Creating ICMP Packet failed\n"
	add	esp, 3*4
	ret




;-----------------------------------------------------------------
;
; ICMP_output
;
; IN:  eax = socket ptr
;      ecx = data length
;      esi = data offset
;
;-----------------------------------------------------------------
align 4
ICMP_output_raw:

	DEBUGF	1,"Creating ICMP Packet for socket %x, data ptr=%x\n", eax, edx

	push	edx

	mov	di, IP_PROTO_ICMP SHL 8 + 128  ; TTL
	shr	edx, 16
	mov	ebx, [eax + IP_SOCKET.LocalIP]
	mov	eax, [eax + IP_SOCKET.RemoteIP]
	call	IPv4_output
	jz	.exit

	pop	esi
	push	edx
	push	eax

	push	edi ecx
	DEBUGF	1,"copying %u bytes from %x to %x\n", ecx, esi, edi
	rep	movsb
	pop	ecx edi

	mov	[edi + ICMP_header.Checksum], 0

	mov	esi, edi
	xor	edx, edx
	call	checksum_1
	call	checksum_2
	mov	[edi + ICMP_header.Checksum], dx

	DEBUGF	1,"Sending ICMP Packet\n"
	call	[ebx + NET_DEVICE.transmit]
	ret
  .exit:
	DEBUGF	1,"Creating ICMP Packet failed\n"
	add	esp, 4
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
