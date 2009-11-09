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

TCP_RETRIES		equ 5		; Number of times to resend a Packet
TCP_PACKET_TTL		equ 50		; resend if not replied to in 1/100 s
TCP_SOCKET_TTL		equ 10		; # of secs to wait before closing socket

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
	TCP_OUT_QUEUE		dd  ?
				rd  (tcp_out_queue_entry.size*TCP_QUEUE_SIZE)/4
endg

align 4
iglobal

stateHandler:

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

	; tcp_out_queue is a special type of queue
	xor	eax, eax
	mov	esi, TCP_OUT_QUEUE
	mov	ecx, TCP_QUEUE_SIZE*tcp_out_queue_entry/4+1
	rep	stosd

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

;        DEBUGF  1, "K :   %x-%x: %x-%x-%x-%u\n", [ebx + SOCKET.PID]:2, [ebx + SOCKET.Number]:2, [ebx + SOCKET.LocalPort]:4, [ebx + SOCKET.RemoteIP], [ebx + SOCKET.RemotePort]:4, [ebx + SOCKET.state]

	cmp	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.timer], 0
	jne	.decrement_tcb
	cmp	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.wndsizeTimer], 0
	jne	.decrement_wnd
	jmp	.next_socket

  .decrement_tcb:
	; decrement it, delete socket if TCB timer = 0 & socket in timewait state
	dec	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.timer]
	jnz	.next_socket

	cmp	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_TIMED_WAIT
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
	add	esi, tcp_out_queue_entry.size
	dec	eax
	jz	.exit
	test	ecx, ecx
	jnz	.loop
	ret

  .send_it:
	push	eax ecx esi

	mov	ebx, [esi + tcp_out_queue_entry.owner]
	push	[esi + tcp_out_queue_entry.data_size]
	push	[esi + tcp_out_queue_entry.data_ptr]
	DEBUGF 1,"Now sending TCP packet %x, size: %u, owner: %x, sendproc %x\n", [esp], [esp+4], ebx, [esi + tcp_out_queue_entry.sendproc]
	call	[esi + tcp_out_queue_entry.sendproc]
	add	esp, 8
	pop	esi ecx eax

	dec	[esi + tcp_out_queue_entry.retries]
	jz	.remove_it

	mov	[esi + tcp_out_queue_entry.ttl], TCP_PACKET_TTL
	jmp	.find_next

  .remove_it:
	push	[esi + tcp_out_queue_entry.data_ptr]
	mov	[esi + tcp_out_queue_entry.data_ptr], 0
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

	DEBUGF 1,"Adding packet to TCP queue, buffer: %x, size: %u, driver: %x, acknum: %x\n", [esp], [esp+4], ebx, edx

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

	DEBUGF 1,"TCP queue is full!\n"

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
	mov	[eax + tcp_out_queue_entry.seq_num], edx

	inc	[TCP_OUT_QUEUE]

	sub	eax, TCP_OUT_QUEUE+4
	DEBUGF 1,"Added to queue in pos %u\n", eax

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

	; TODO: validate checksum

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
	jnz	.socket_loop

  .change_state:

       DEBUGF 1,"Found valid socket for packet\n"

	push	ebx
	lea	ebx, [ebx + SOCKET_head.lock]
	call	wait_mutex
	pop	ebx

;----------------------------------
; ebx is pointer to socket
; ecx is size of tcp packet
; edx is pointer to tcp packet

	; as a Packet has been received, update the TCB timer
	mov	[ebx +	SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.timer], TCP_SOCKET_TTL

	; If the received Packet has an ACK bit set, remove any Packets in the resend queue that this received Packet acknowledges
	test	[edx + TCP_Packet.Flags], TH_ACK
	jz	.call_handler					; No ACK, so no data yet

;        mov     eax, [edx + TCP_Packet.SequenceNumber]          ; Calculate sequencenumber in eax
;        bswap   eax                                             ;
;        add     eax, ecx                                        ;

	mov	eax, [edx + TCP_Packet.AckNumber]
	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.last_ack_number], eax
	;---------

	cmp	[TCP_OUT_QUEUE], 0
	je	.call_handler
	push	ecx

       DEBUGF 1,"Removing all queued packets with smaller ACK\n"

	mov	ecx, TCP_QUEUE_SIZE
	mov	esi, TCP_OUT_QUEUE+4

  .loop:
	cmp	[esi + tcp_out_queue_entry.data_ptr], 0
	je	.maybe_next
	cmp	[esi + tcp_out_queue_entry.seq_num], eax
	jg	.maybe_next
	; TODO: check if the packets belong to the same tcp connection !

       DEBUGF 1,"Removing a queued packet\n"

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
	mov	eax, [ebx +  SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state]
	DEBUGF 1,"Socket state: %u\n", eax

	cmp	eax, TCB_LISTEN
	jb	.dump
	cmp	eax, TCB_CLOSED
	ja	.dump

	dec	eax
	shl	eax, 2
	add	eax, stateHandler

	call	dword[eax]

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
	add	ecx, TCP_Packet.Options
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

	mov	[edi + TCP_Packet.Window], 0x0005	   ; 1280 bytes
	mov	[edi + TCP_Packet.UrgentPointer], 0
	mov	[edi + TCP_Packet.DataOffset], 0x50
	mov	[edi + TCP_Packet.Checksum], 0

; Copy the data
	mov	esi, [esp]
	mov	ecx, [esp+4]
	add	edi, TCP_Packet.Options

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
	lea	esi, [ebx+ETH_DEVICE.transmit]
	mov	edx, [edi + TCP_Packet.AckNumber]
	jmp	TCP_add_to_queue

  .fail:
	add	esp, 12+12+4
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

	DEBUGF 1,"Creating TCP ACK, socket: %x, flags: %x\n",eax, bl

	mov	di , IP_PROTO_TCP
	mov	ecx, TCP_Packet.Options

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

	push	dword [esi + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.LocalPort] ; both ports at once
	pop	dword [edi + TCP_Packet.SourcePort]

	push	[esi + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RCV_NXT]
	pop	[edi + TCP_Packet.AckNumber]

	pop	cx
	mov	[edi + TCP_Packet.Flags], cl
	mov	[edi + TCP_Packet.Window], 0x0005	   ; 1280 bytes
	mov	[edi + TCP_Packet.UrgentPointer], 0
	mov	[edi + TCP_Packet.DataOffset], 0x50
	mov	[edi + TCP_Packet.Checksum], 0

	push	edx eax

;        lea     esi, [esi + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.SND_NXT]
;        inc_INET esi

; Now, calculate the checksum
	pushw	TCP_Packet.Options shl 8
	pushw	IP_PROTO_TCP shl 8
	pushd	[edi-4] ; destination address
	pushd	[edi-8] ; source address

	xor	edx, edx
	mov	ecx, TCP_Packet.Options
	mov	esi, edi
	call	checksum_1
	mov	ecx, 12
	mov	esi, esp
	call	checksum_1
	add	esp, 12 				   ; remove the pseudoheader from stack
; and store it in TCP header
	call	checksum_2
	mov	[edi + TCP_Packet.Checksum], dx

; And now, send the packet!
	DEBUGF 1,"Sending TCP Packet to device %x\n", ebx
	mov	esi, [ebx + ETH_DEVICE.transmit]
	mov	edx, [edi + TCP_Packet.SequenceNumber]
	jmp	TCP_add_to_queue

  .fail:
	add	esp, 2+4
	ret





;---------- TCB state handlers start here




align 4
stateTCB_LISTEN:

	DEBUGF	1,"TCBStateHandler: Listen\n"

	; In this case, we are expecting a SYN Packet
	; For now, if the Packet is a SYN, process it, and send a response
	; If not, ignore it

	; Look at control flags
	test	[edx + TCP_Packet.Flags], TH_SYN
	jz	.exit
	; Exit if backlog queue is full
	mov	ax, [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.backlog_cur]
	cmp	ax, [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.backlog]
	jae	.exit
	; Allocate new socket
	push	esi
	call	net_socket_alloc
	pop	esi
	test	eax, eax
	jz	.exit
	; Copy structure from current socket to new, including lock
	push	esi edi
	lea	esi, [ebx + SOCKET_head.PID]	; yes, PID must also be copied
	lea	edi, [eax + SOCKET_head.PID]
	mov	ecx, ((SOCKET_head.end - SOCKET_head.PID) + IPv4_SOCKET.end + TCP_SOCKET.end + 3)/4
	rep	movsd
	pop	edi esi
	; Push pointer to new socket to queue
	movzx	ecx, [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.backlog_cur]
	inc	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.backlog_cur]
	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.end + ecx*4], eax

	; We have a SYN. update the socket with this IP Packets details,
	; And send a response

	mov	[eax + SOCKET_head.end + IPv4_SOCKET.RemoteIP], esi ; IP source address
	mov	cx, [edx + TCP_Packet.SourcePort]
	mov	[eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RemotePort], cx
	mov	ecx, [edx + TCP_Packet.SequenceNumber]
	mov	[eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.IRS], ecx
	mov	[eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RCV_NXT], ecx
	lea	esi, [eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RCV_NXT]
	inc_INET esi ; RCV.NXT
	mov	ecx, [eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.ISS]
	mov	[eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.SND_NXT], ecx

	mov	[eax + SOCKET_head.lock], 0
	mov	[ebx + SOCKET_head.lock], 0

	push	eax
	; Now construct the response
	mov	bl, TH_SYN + TH_ACK
	call	TCP_send_ack
	pop	eax

	mov	[eax +	SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_SYN_RECEIVED
	call	notify_network_event

	; increment SND.NXT in socket
	lea	esi, [eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.SND_NXT]
	inc_INET esi
	ret

  .exit:
	mov	[ebx + SOCKET_head.lock], 0
	ret


align 4
stateTCB_SYN_SENT:

	DEBUGF	1,"TCBStateHandler: Syn_Sent\n"

	; We are awaiting an ACK to our SYN, with a SYM
	; Look at control flags - expecting an ACK

	mov	al, [edx + TCP_Packet.Flags]
	and	al, TH_SYN + TH_ACK
	cmp	al, TH_SYN + TH_ACK
	je	.syn_ack

	test	al, TH_SYN
	jz	.exit

	mov	[ebx +	SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_SYN_RECEIVED
	pushd	TH_SYN + TH_ACK
	jmp	.send

  .syn_ack:
	mov	[ebx +	SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_ESTABLISHED
	pushd	TH_ACK

  .send:
	; Store the recv.nxt field
	mov	eax, [edx + TCP_Packet.SequenceNumber]
	mov	[ebx +	SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.IRS], eax
	bswap	eax
	inc	eax
	bswap	eax
	mov	[ebx +	SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RCV_NXT], eax	      ; Update our recv.nxt field
	mov	[ebx + SOCKET_head.lock], 0

	lea	esi, [ebx +  SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.SND_NXT]
	inc_INET esi

	; Send an ACK
	mov	eax, ebx
	pop	ebx
	call	TCP_send_ack

  .exit:
	mov	[ebx + SOCKET_head.lock], 0
	ret



align 4
stateTCB_SYN_RECEIVED:

	DEBUGF	1,"TCBStateHandler: Syn_received\n"

	; In this case, we are expecting an ACK Packet
	; For now, if the Packet is an ACK, process it,
	; If not, ignore it

	test	[edx + TCP_Packet.Flags], TH_RST
	jz	.check_ack

  ;      push    [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.OrigRemotePort]
 ;       pop     [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RemotePort]
  ;      push    [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.OrigRemoteIP]
 ;       pop     [ebx + SOCKET_head.end + IPv4_SOCKET.RemoteIP]

	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_LISTEN
	jmp	.exit

  .check_ack:
	; Look at control flags - expecting an ACK
	test	[edx + TCP_Packet.Flags], TH_ACK
	jz	.exit

	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_ESTABLISHED
	mov	eax, ebx
	call	notify_network_event

  .exit:
	mov	[ebx + SOCKET_head.lock], 0
	ret



align 4
stateTCB_ESTABLISHED:


	DEBUGF	1,"TCBStateHandler: Established\n"

	mov	eax, [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RCV_NXT]
	cmp	eax, [edx + TCP_Packet.SequenceNumber]
	jne	.exit

	; Here we are expecting data, or a request to close
	; OR both...

	; Did we receive a FIN or RST?
	test	[edx + TCP_Packet.Flags], TH_FIN
	jz	.check_ack

	; It was a fin or reset.

;;; TODO: write following code:
	; Remove resend entries from the queue  - I dont want to send any more data
	; Send an ACK to that fin, and enter closewait state

  .check_ack:
	; Check that we received an ACK
	test	[edx + TCP_Packet.Flags], TH_ACK
	jz	.exit

	DEBUGF	1,"Received ACK\n"

	; First, look at the incoming window. If this is less than or equal to 1024,
	; Set the socket window timer to 1. This will stop an additional Packets being queued.
	; ** I may need to tweak this value, since I do not know how many Packets are already queued
	push	ecx
	mov	cx, [edx + TCP_Packet.Window]
	xchg	cl, ch
	cmp	cx, 1024
	ja	@f
	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.wndsizeTimer], 1
      @@:
	pop	ecx

	test	ecx, ecx
	jnz	.data			   ; Read data, if any

	lea	esi, [ebx +  SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.SND_NXT]
	inc_INET esi

	; If we had received a fin, we need to ACK it.
	cmp	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_CLOSE_WAIT
	je	.ack
	jmp	.exit

  .data:
	     ;;;
	lea	esi, [ebx +  SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.SND_NXT]
	add_INET esi

	DEBUGF	1,"Got data!\n"
	mov	esi, [esp + 4]
	sub	edx, esi
	mov	edi, edx
	mov	eax, ebx
	call	socket_internal_receiver

  .ack:
	mov	[ebx + SOCKET_head.lock], 0
	; Send an ACK
	mov	eax, ebx
	mov	bl, TH_ACK
	call	TCP_send_ack
  .exit:

	mov	[ebx + SOCKET_head.lock], 0
	ret



align 4
stateTCB_FIN_WAIT_1:

	DEBUGF	1,"TCBStateHandler: Fin_wait_1\n"

	; We can either receive an ACK of a fin, or a fin
	mov	al, [edx + TCP_Packet.Flags]
	and	al, TH_FIN + TH_ACK

	cmp	al, TH_ACK
	jne	@f

	; It was an ACK
	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_FIN_WAIT_2
	jmp	.exit

    @@: mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_CLOSING
	cmp	al, TH_FIN
	je	@f
	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_TIMED_WAIT

    @@: lea	esi, [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RCV_NXT]
	inc_INET esi

	mov	[ebx + SOCKET_head.lock], 0
	; Send an ACK
	mov	eax, ebx
	mov	bl, TH_ACK
	call	TCP_send_ack

  .exit:
	mov	[ebx + SOCKET_head.lock], 0
	ret



align 4
stateTCB_FIN_WAIT_2:

	DEBUGF	1,"TCBStateHandler: Fin_wait_2\n"

	test	[edx + TCP_Packet.Flags], TH_FIN
	jz	.exit

	; Change state, as we have a fin
	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_TIMED_WAIT

	lea	esi, [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RCV_NXT]
	inc_INET esi

	mov	[ebx + SOCKET_head.lock], 0

	; Send an ACK
	mov	eax, ebx
	mov	bl, TH_ACK
	call	TCP_send_ack

  .exit:
	mov	[ebx + SOCKET_head.lock], 0
	ret



align 4
stateTCB_CLOSE_WAIT:

	DEBUGF	1,"TCBStateHandler: close_wait\n"
	; Intentionally left empty
	; socket_close_tcp handles this

	mov	[ebx + SOCKET_head.lock], 0
	ret



align 4
stateTCB_CLOSING:

	DEBUGF	1,"TCBStateHandler: closingn\n"

	; We can either receive an ACK of a fin, or a fin
	test	[edx + TCP_Packet.Flags], TH_ACK
	jz	.exit

	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_TIMED_WAIT

  .exit:

	mov	[ebx + SOCKET_head.lock], 0
	ret


align 4
stateTCB_LAST_ACK:

	DEBUGF	1,"TCBStateHandler: last_ackn\n"

	; Look at control flags - expecting an ACK
	test	[edx + TCP_Packet.Flags], TH_ACK
	jz	.exit

	mov	[ebx + SOCKET_head.lock], 0

	; delete the socket
	stdcall net_socket_free, ebx

  .exit:
	ret


align 4
stateTCB_TIME_WAIT:

	DEBUGF	1,"TCBStateHandler: time_wait\n"

	mov	[ebx + SOCKET_head.lock], 0

	ret


align 4
stateTCB_CLOSED:

	DEBUGF	1,"TCBStateHandler: closed\n"

	mov	[ebx + SOCKET_head.lock], 0

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
	add	eax, TCP_PACKETS_TX
	mov	eax, [eax]
	ret

.packets_rx:
	add	eax, TCP_PACKETS_RX
	mov	eax, [eax]
	ret
