;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2009. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  TCP.INC                                                        ;;
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


TCB_LISTEN		equ 1
TCB_SYN_SENT		equ 2
TCB_SYN_RECEIVED	equ 3
TCB_ESTABLISHED 	equ 4
TCB_FIN_WAIT_1		equ 5
TCB_FIN_WAIT_2		equ 6
TCB_CLOSE_WAIT		equ 7
TCB_CLOSING		equ 8
TCB_LAST_ACK		equ 9
TCB_TIMED_WAIT		equ 10
TCB_CLOSED		equ 11

TH_FIN			equ 1 shl 0
TH_SYN			equ 1 shl 1
TH_RST			equ 1 shl 2
TH_PUSH 		equ 1 shl 3
TH_ACK			equ 1 shl 4
TH_URG			equ 1 shl 5

TWOMSL			equ 10		; # of secs to wait before closing socket

TCP_RETRIES		equ 5		; Number of times to resend a Packet
TCP_TIMEOUT		equ 10		; resend if not replied to in 1/100 s

TCP_QUEUE_SIZE		equ 16


struct	TCP_Packet
	.SourcePort		dw ?
	.DestinationPort	dw ?
	.SequenceNumber 	dd ?
	.AckNumber		dd ?
	.DataOffset		db ?	; DataOffset[0-3 bits] and Reserved[4-7]
	.Flags			db ?	; Reserved[0-1 bits]|URG|ACK|PSH|RST|SYN|FIN
	.Window 		dw ?
	.Checksum		dw ?
	.UrgentPointer		dw ?
	.Options		rb 3
	.Padding		db ?
	.Data:
ends

align 4
uglobal
	TCP_PACKETS_TX		rd  MAX_IP
	TCP_PACKETS_RX		rd  MAX_IP

	TCP_IN_QUEUE		rd  (tcp_in_queue_entry.size*TCP_QUEUE_SIZE+queue.data)/4
	TCP_OUT_QUEUE		dd ?
				rd  (tcp_out_queue_entry.size*TCP_QUEUE_SIZE)/4
endg

align 4
iglobal

TCBStateHandler:

  dd  stateTCB_LISTEN
  dd  stateTCB_SYN_SENT
  dd  stateTCB_SYN_RECEIVED
  dd  stateTCB_ESTABLISHED
  dd  stateTCB_FIN_WAIT_1
  dd  stateTCB_FIN_WAIT_2
  dd  stateTCB_CLOSE_WAIT
  dd  stateTCB_CLOSING
  dd  stateTCB_LAST_ACK
  dd  stateTCB_TIME_WAIT
  dd  stateTCB_CLOSED

endg


macro inc_INET reg {

	inc	byte [reg + 0]
	adc	byte [reg + 1], 0
	adc	byte [reg + 2], 0
	adc	byte [reg + 3], 0

}


macro add_INET reg {

	rol	ecx, 16
	adc	byte [reg + 0], ch
	adc	byte [reg + 1], cl
	rol	ecx, 16
	adc	byte [reg + 2], ch
	adc	byte [reg + 3], cl

}




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
	mov	edi, TCP_PACKETS_TX
	mov	ecx, 2*MAX_IP
	rep	stosd

	init_queue TCP_IN_QUEUE
	init_queue TCP_OUT_QUEUE

	ret


;-----------------------------------------------------------------
;
;  TCP_decrease_socket_ttls
;
;  IN:  /
;  OUT: /
;
;-----------------------------------------------------------------

align 4
TCP_decrease_socket_ttls:
	; scan through all the sockets, decrementing active timers

	mov	ebx, net_sockets

	cmp	[ebx + SOCKET_head.NextPtr], 0
	je	.exit

  .next_socket:
	mov	ebx, [ebx + SOCKET_head.NextPtr]
	or	ebx, ebx
	jz	.exit

	cmp	[ebx + SOCKET_head.Type], IP_PROTO_TCP
	jne	.next_socket

;        DEBUGF  1, "K :   %x-%x: %x-%x-%x-%u\n", [ebx + SOCKET.PID]:2, [ebx + SOCKET.Number]:2, [ebx + SOCKET.LocalPort]:4, [ebx + SOCKET.RemoteIP], [ebx + SOCKET.RemotePort]:4, [ebx + SOCKET.TCBState]

	cmp	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.TCBTimer], 0
	jne	.decrement_tcb
	cmp	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.wndsizeTimer], 0
	jne	.decrement_wnd
	jmp	.next_socket

  .decrement_tcb:
	; decrement it, delete socket if TCB timer = 0 & socket in timewait state
	dec	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.TCBTimer]
	jnz	.next_socket

	cmp	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.TCBState], TCB_TIMED_WAIT
	jne	.next_socket

	push	[ebx + SOCKET_head.PrevPtr]
	stdcall net_socket_free, ebx
	pop	ebx
	jmp	.next_socket

  .decrement_wnd:
	; TODO - prove it works!
	dec	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.wndsizeTimer]
	jmp	.next_socket

  .exit:
	ret



;-----------------------------------------------------------------
;
; TCP_send_queued:
;
;  Decreases 'ttl' of tcp packets queued.
;  if 'ttl' reaches 0, resend the packet and decrease 'retries'
;  if 'retries' reaches zero, remove the queued packet
;
;  IN:  /
;  OUT: /
;
;-----------------------------------------------------------------

align 4
TCP_send_queued:

	cmp	[TCP_OUT_QUEUE], 0
	je	.exit

	mov	eax, TCP_QUEUE_SIZE
	mov	ecx, [TCP_OUT_QUEUE]
	mov	esi, TCP_OUT_QUEUE+4

  .loop:
	cmp	[esi + tcp_out_queue_entry.data_ptr], 0
	jnz	.found_one
	add	esi, tcp_out_queue_entry.size
	loop	.loop
  .exit:
	ret

  .found_one:
	dec	[esi + tcp_out_queue_entry.ttl]
	jz	.send_it
  .find_next:
	dec	eax
	jz	.exit
	jmp	.loop

  .send_it:
	push	eax ecx esi

	push	[esi + tcp_out_queue_entry.data_size]
	push	[esi + tcp_out_queue_entry.data_ptr]
	mov	ebx, [esi + tcp_out_queue_entry.owner]

	call	[esi + tcp_out_queue_entry.sendproc]

	pop	esi ecx eax

	dec	[esi + tcp_out_queue_entry.retries]
	jz	.remove_it
	mov	[esi + tcp_out_queue_entry.ttl], TCP_TIMEOUT
	jmp	.find_next

  .remove_it:
	push	[esi + tcp_out_queue_entry.data_ptr]
	mov	[esi + tcp_out_queue_entry.data_ptr], 0
	dec	[TCP_OUT_QUEUE]
	call	kernel_free
	jmp	.find_next



;-----------------------------------------------------------------
;
; TCP_add_to_queue:
;
;  Queue a TCP packet for sending
;
;  IN:  [esp] pointer to buffer
;       [esp + 4] size of buffer
;       ebx = driver struct
;       esi = sender proc
;       edx = acknum
;  OUT: /
;
;-----------------------------------------------------------------

align 4
TCP_add_to_queue:

	cmp	[TCP_OUT_QUEUE], TCP_QUEUE_SIZE
	jge	.full

	mov	ecx, TCP_QUEUE_SIZE
	mov	eax, TCP_OUT_QUEUE+4

  .loop:
	cmp	[eax + tcp_out_queue_entry.data_ptr], 0
	je	.found_it
	add	eax, tcp_out_queue_entry.size
	loop	.loop

  .full:			; silently discard the packet
	call	kernel_free
	add	esp, 4

	ret

  .found_it:			; eax point to empty queue entry

	pop	[eax + tcp_out_queue_entry.data_ptr]
	pop	[eax + tcp_out_queue_entry.data_size]
	mov	[eax + tcp_out_queue_entry.ttl], 1			; send immediately
	mov	[eax + tcp_out_queue_entry.retries], TCP_RETRIES
	mov	[eax + tcp_out_queue_entry.owner], ebx
	mov	[eax + tcp_out_queue_entry.sendproc], esi
	mov	[eax + tcp_out_queue_entry.ack_num], edx


	ret


;-----------------------------------------------------------------
;
; TCP_handler:
;
;  Called by IPv4_handler,
;  this procedure will inject the tcp data diagrams in the application sockets.
;
;  IN:  Pointer to buffer in [esp]
;       size of buffer in [esp+4]
;       pointer to device struct in ebx
;       TCP Packet size in ecx
;       pointer to TCP Packet data in edx
;       SourceAddres in esi
;  OUT: /
;
;-----------------------------------------------------------------

align 4
TCP_handler :

       DEBUGF 1,"TCP_Handler\n"

	; IP Packet TCP Destination Port = local Port
	; IP Packet SA = Remote IP  OR = 0
	; IP Packet TCP Source Port = remote Port  OR = 0

	mov	ebx, net_sockets

  .socket_loop:
	mov	ebx, [ebx + SOCKET_head.NextPtr]
	or	ebx, ebx
	jz	.dump

	mov	ax, [edx + TCP_Packet.DestinationPort]
	cmp	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.LocalPort], ax
	jne	.socket_loop

	mov	eax, [ebx + SOCKET_head.end + IPv4_SOCKET.RemoteIP]
	cmp	eax, esi
	je	@f
	test	eax, eax
	jne	.socket_loop
       @@:

	mov	ax, [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RemotePort]
	cmp	[edx + TCP_Packet.SourcePort] , ax
	je	.change_state
	test	ax, ax
	jne	.socket_loop

  .change_state:

	push	ebx
	lea	ebx, [ebx + SOCKET_head.lock]
	call	wait_mutex
	pop	ebx

;----------------------------------
; ebx is pointer to socket
; ecx is size of tcp packet
; edx is pointer to tcp packet

	; as a Packet has been received, update the TCB timer
	mov	[ebx +	SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.TCBTimer], TWOMSL

	; If the received Packet has an ACK bit set, remove any Packets in the resend queue that this received Packet acknowledges
	test	[edx + TCP_Packet.Flags], TH_ACK
	jz	.call_handler					; No ACK, so no data yet

	mov	eax, [edx + TCP_Packet.SequenceNumber]		; Calculate sequencenumber in eax
	bswap	eax						;
	add	eax, ecx					;

	cmp	[TCP_OUT_QUEUE], 0
	je	.call_handler

	push	ecx
	mov	ecx, TCP_QUEUE_SIZE
	mov	esi, TCP_OUT_QUEUE+4

  .loop:
	cmp	[esi + tcp_out_queue_entry.data_ptr], 0
	jne	.maybe_next
	cmp	[esi + tcp_out_queue_entry.ack_num], eax
	jg	.maybe_next

	push	[esi + tcp_out_queue_entry.data_ptr]
	mov	[esi + tcp_out_queue_entry.data_ptr], 0
	dec	[TCP_OUT_QUEUE]
	call	kernel_free

  .maybe_next:
	add	esi, tcp_out_queue_entry.size
	loop	.loop
	pop	ecx

  .call_handler:
	; Call handler for given TCB state
	mov	eax, [ebx +  SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.TCBState]
	cmp	eax, TCB_LISTEN
	jb	.exit
	cmp	eax, TCB_CLOSED
	ja	.exit

	shl	eax, 2
	add	eax, TCBStateHandler - 4

	push	.exit
	jmp	eax

  .exit:
	mov	[ebx + SOCKET_head.lock], 0

  .dump:
	DEBUGF 1,"Dumping TCP packet\n"
	call	kernel_free
	add	esp, 4 ; pop (balance stack)

	ret



;-----------------------------------------------------------------
;
; TCP_socket_send
;
; IN: eax = socket pointer
;     ecx = number of bytes to send
;     esi = pointer to data
;
;-----------------------------------------------------------------

align 4
TCP_socket_send:

	DEBUGF 1,"Creating TCP Packet\n"

	mov	di , IP_PROTO_TCP

; Create an IPv4 Packet of the correct size
	push	eax
	mov	ebx, [eax + SOCKET_head.end + IPv4_SOCKET.LocalIP]
	mov	eax, [eax + SOCKET_head.end + IPv4_SOCKET.RemoteIP]

; meanwhile, create the pseudoheader in stack,
; (now that we still have all the variables that are needed.)
	push	cx
	push	di
	push	eax
	push	ebx


	push	ecx esi eax		; save some variables for later
	add	ecx, TCP_Packet.Data
	call	IPv4_create_packet
	cmp	edi, -1
	je	.fail

	pop	esi

; Now add the TCP header to the IPv4 packet

	push	[esi + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.SND_NXT]
	pop	[edi + TCP_Packet.SequenceNumber]

	push	dword [esi + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.LocalPort]
	pop	dword [edi + TCP_Packet.SourcePort]


	push	[esi + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.SND_NXT]
	pop	[edi + TCP_Packet.AckNumber]

	mov	al, [eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.flags]
	mov	[edi + TCP_Packet.Flags], al

	mov	[edi + TCP_Packet.Window], 0x0005	   ; 1280 bytes               ;;; TODO: read RFC !
	mov	[edi + TCP_Packet.UrgentPointer], 0
	mov	[edi + TCP_Packet.DataOffset], 0x50
	mov	[edi + TCP_Packet.Checksum], 0

; Copy the data
	mov	esi, [esp]
	mov	ecx, [esp+4]
	add	edi, TCP_Packet.Data

	shr	ecx, 1
	jnc	.nb
	movsb
.nb:	shr	ecx, 1
	jnc	.nw
	movsw
.nw:	rep movsd

; Now, calculate the checksum for pseudoheader
	xor	edx, edx
	mov	ecx, 12
	mov	esi, esp
	call	checksum_1
	add	esp, 12 				   ; remove the pseudoheader from stack
; And that of the data
	pop	esi
	pop	ecx
	call	checksum_1
; Now create the final checksum and store it in TCP header
	call	checksum_2
	mov	[edi + TCP_Packet.Checksum], dx

; And now, send it!
	DEBUGF 1,"Sending TCP Packet to device %x\n", ebx
	mov	esi, ETH_sender
	mov	edx, [edi + TCP_Packet.AckNumber]
	jmp	TCP_add_to_queue

  .fail:
	add	esp, 12+4
	ret





;-----------------------------------------------------------------
;
; TCP_send_ack
;
; IN: eax = socket pointer
;      bl = flags
;
;-----------------------------------------------------------------

align 4
TCP_send_ack:

	DEBUGF 1,"Creating TCP ACK\n"

	mov	di , IP_PROTO_TCP
	mov	cx , TCP_Packet.Data

	push	bx eax

; Create an IPv4 Packet of the correct size
	mov	ebx, [eax + SOCKET_head.end + IPv4_SOCKET.LocalIP]
	mov	eax, [eax + SOCKET_head.end + IPv4_SOCKET.RemoteIP]

	call	IPv4_create_packet
	cmp	edi, -1
	je	.fail

; Fill in the TCP header
	pop	esi

	push	[esi + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.SND_NXT]
	pop	[edi + TCP_Packet.SequenceNumber]

	push	dword [esi + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.LocalPort]
	pop	dword [edi + TCP_Packet.SourcePort]

	push	[esi + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.SND_NXT]
	pop	[edi + TCP_Packet.AckNumber]

	pop	cx
	mov	[edi + TCP_Packet.Flags], cl
	mov	[edi + TCP_Packet.Window], 0x0005	   ; 1280 bytes
	mov	[edi + TCP_Packet.UrgentPointer], 0
	mov	[edi + TCP_Packet.DataOffset], 0x50

	push	eax edx

	push	word TCP_Packet.Data shl 8
	push	IP_PROTO_TCP
	push	[esi + SOCKET_head.end + SOCKET_head.end + IPv4_SOCKET.RemoteIP]
	push	[esi + SOCKET_head.end + SOCKET_head.end + IPv4_SOCKET.LocalIP]

; Now, calculate the checksum for pseudoheader
	xor	edx, edx
	mov	ecx, 12
	mov	esi, esp
	call	checksum_1
	add	esp, 12 				   ; remove the pseudoheader from stack
; Now create the final checksum and store it in TCP header
	call	checksum_2
	mov	[edi + TCP_Packet.Checksum], dx

; And now, send it!
	DEBUGF 1,"Sending TCP Packet to device %x\n", ebx
	mov	esi, ETH_sender
	mov	edx, [edi + TCP_Packet.AckNumber]
	jmp	TCP_add_to_queue

  .fail:
	add	esp, 12+4
	ret





align 4
stateTCB_LISTEN:
	; In this case, we are expecting a SYN Packet
	; For now, if the Packet is a SYN, process it, and send a response
	; If not, ignore it

	; Look at control flags
	test	[edx + TCP_Packet.Flags], TH_SYN
	jz	.exit

	; We have a SYN. update the socket with this IP Packets details,
	; And send a response

	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.RemoteIP], esi ; IP source address
	mov	ax, [edx + TCP_Packet.SourcePort]
	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RemotePort], ax
	mov	eax, [edx + TCP_Packet.SequenceNumber]
	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.IRS], eax
	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RCV_NXT], eax
	lea	esi, [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RCV_NXT]
	inc_INET esi ; RCV.NXT
	mov	eax, [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.ISS]
	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.SND_NXT], eax

	; Now construct the response
	mov	bl, TH_SYN + TH_ACK
	call	TCP_send_ack

	mov	[ebx +	SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.TCBState], TCB_SYN_RECEIVED

	; increment SND.NXT in socket
	lea	esi, [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.SND_NXT]
	inc_INET esi

  .exit:
	ret


align 4
stateTCB_SYN_SENT:
	; We are awaiting an ACK to our SYN, with a SYM
	; Look at control flags - expecting an ACK

	mov	al, [edx + TCP_Packet.Flags]
	and	al, TH_SYN + TH_ACK
	cmp	al, TH_SYN + TH_ACK
	je	.syn_ack

	test	al, TH_SYN
	jz	.exit

	mov	[ebx +	SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.TCBState], TCB_SYN_RECEIVED
	push	TH_SYN + TH_ACK
	jmp	.send

  .syn_ack:
	mov	[ebx +	SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.TCBState], TCB_ESTABLISHED
	push	TH_ACK

  .send:
	; Store the recv.nxt field
	mov	eax, [edx + TCP_Packet.SequenceNumber]

	; Update our recv.nxt field
	mov	[ebx +	SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RCV_NXT], eax
	lea	esi, [ebx +  SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RCV_NXT]
	inc_INET esi

	; Send an ACK
	pop	ebx
	call	TCP_send_ack

  .exit:
	ret



align 4
stateTCB_SYN_RECEIVED:
	; In this case, we are expecting an ACK Packet
	; For now, if the Packet is an ACK, process it,
	; If not, ignore it

	test	[edx + TCP_Packet.Flags], TH_RST
	jz	.check_ack

	push	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.OrigRemotePort]
	pop	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RemotePort]
	push	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.OrigRemoteIP]
	pop	[ebx + SOCKET_head.end + IPv4_SOCKET.RemoteIP]

	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.TCBState], TCB_LISTEN
	jmp	.exit

  .check_ack:
	; Look at control flags - expecting an ACK
	test	[edx + TCP_Packet.Flags], TH_ACK
	jz	.exit

	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.TCBState], TCB_ESTABLISHED

  .exit:
	ret



align 4
stateTCB_ESTABLISHED:
	; Here we are expecting data, or a request to close
	; OR both...

	; Did we receive a FIN or RST?
	test	[edx + TCP_Packet.Flags], TH_FIN
	jz	.check_ack

	; It was a fin or reset.

	; Remove resend entries from the queue  - I dont want to send any more data
	; Send an ACK to that fin, and enter closewait state

  .check_ack:
	; Check that we received an ACK
	test	[edx + TCP_Packet.Flags], TH_ACK
	jz	.exit

	; First, look at the incoming window. If this is less than or equal to 1024,
	; Set the socket window timer to 1. This will stop an additional Packets being queued.
	; ** I may need to tweak this value, since I do not know how many Packets are already queued
	mov	cx, [edx + TCP_Packet.Window]
	xchg	cl, ch
	cmp	cx, 1024
	ja	@f

	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.wndsizeTimer], 1

    @@: ; OK, here is the deal
	; My recv.nct field holds the seq of the expected next rec byte
	; if the recevied sequence number is not equal to this, do not
	; increment the recv.nxt field, do not copy data - just send a
	; repeat ack.

	; recv.nxt is in dword [edx+24], in inet format
	; recv seq is in [sktAddr]+56, in inet format
	; just do a comparision
	mov	ecx, [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RCV_NXT]
	cmp	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.TCBState], TCB_CLOSE_WAIT
	jne	@f
	mov	ecx, eax

    @@: cmp	ecx, [edx + TCP_Packet.SequenceNumber]
	jne	.ack

	test	ecx, ecx
	jnz	.data

	; If we had received a fin, we need to ACK it.
	cmp	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.TCBState], TCB_CLOSE_WAIT
	je	.ack
	jmp	.exit

  .data:
	mov	esi, [esp + 4]
	sub	edx, esi
	mov	edi, edx
	call	socket_internal_receiver

  .ack:
	; Send an ACK
	mov	bl, TH_ACK
	call	TCP_send_ack
  .exit:
	ret



align 4
stateTCB_FIN_WAIT_1:
	; We can either receive an ACK of a fin, or a fin
	mov	al, [edx + TCP_Packet.Flags]
	and	al, TH_FIN + TH_ACK

	cmp	al, TH_ACK
	jne	@f

	; It was an ACK
	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.TCBState], TCB_FIN_WAIT_2
	jmp	.exit

    @@: mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.TCBState], TCB_CLOSING
	cmp	al, TH_FIN
	je	@f
	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.TCBState], TCB_TIMED_WAIT

    @@: lea	esi, [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RCV_NXT]
	inc_INET esi
	; Send an ACK
	mov	bl, TH_ACK
	call	TCP_send_ack

  .exit:
	ret



align 4
stateTCB_FIN_WAIT_2:
	test	[edx + TCP_Packet.Flags], TH_FIN
	jz	.exit

	; Change state, as we have a fin
	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.TCBState], TCB_TIMED_WAIT

	lea	esi, [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RCV_NXT]
	inc_INET esi

	; Send an ACK
	mov	bl, TH_ACK
	call	TCP_send_ack

  .exit:
	ret



align 4
stateTCB_CLOSE_WAIT:
	; Intentionally left empty
	; socket_close_tcp handles this
	ret



align 4
stateTCB_CLOSING:
	; We can either receive an ACK of a fin, or a fin
	test	[edx + TCP_Packet.Flags], TH_ACK
	jz	.exit

	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.TCBState], TCB_TIMED_WAIT

  .exit:
	ret


align 4
stateTCB_LAST_ACK:
	; Look at control flags - expecting an ACK
	test	[edx + TCP_Packet.Flags], TH_ACK
	jz	.exit

	; delete the socket
	stdcall net_socket_free, ebx

  .exit:
	ret


align 4
stateTCB_TIME_WAIT:
	ret


align 4
stateTCB_CLOSED:
	ret




