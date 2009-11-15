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
;        .Options                rb 3
;        .Padding                db ?
	.Data:
ends

struct	tcp_in_queue_entry
	.data_ptr	dd ?
	.data_size	dd ?
	.offset 	dd ?
	.size:
ends

struct	tcp_out_queue_entry
	.data_ptr	dd ?
	.data_size	dd ?
	.ttl		dd ?
	.retries	dd ?
	.owner		dd ?
	.sendproc	dd ?
	.seq_num	dd ?
	.socket 	dd ?
	.size:
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

; tcp_out_queue is a special type of queue:
; The first dword is a counter of total packets queued.
; The remaining bytes are socket 'slots' wich use tcp_out_queue_entry data structure.
; An empty slot is know by the fact that tcp_out_queue_entry.data_ptr (first dword of the slot) is set to 0
; There are TCP_OUT_QUEUE_SIZE number of slots

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
	inc	[TCP_PACKETS_TX]
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
; TCP_handler:
;
;  Called by IPv4_handler,
;  this procedure will inject the tcp data diagrams in the application sockets.
;
;  IN:  Pointer to buffer in [esp]
;       size of buffer in [esp+4]
;       pointer to device struct in ebx
;       TCP Packet size in ecx
;       pointer to TCP Packet in edx
;       SourceAddres (IPv4) in esi
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
	je	.found_socket
	test	ax, ax
	jnz	.socket_loop
  .found_socket:
       DEBUGF 1,"Found valid socket for packet\n"

	inc	[TCP_PACKETS_RX]

	add	ebx, SOCKET_head.lock
	call	wait_mutex
	sub	ebx, SOCKET_head.lock

;-------------------------------
; ebx is pointer to socket
; ecx is size of tcp packet
; edx is pointer to tcp packet

; calculate header length
	movzx	eax, [edx + TCP_Packet.DataOffset]
	and	eax, 11110000b
	shr	eax, 2
       DEBUGF 1,"TCP header size: %u\n", eax
	sub	ecx, eax

;-------------------------------
; ecx is size of tcp data

; as a Packet has been received, update the TCB timer
	mov	[ebx +	SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.timer], TCP_SOCKET_TTL

; If the received Packet has an ACK bit set, remove any Packets in the resend queue that this received Packet acknowledges
	test	[edx + TCP_Packet.Flags], TH_ACK
	jz	.no_ack 				 ; No ACK, so no data yet

; Calculate ACK number
	mov	edi, [edx + TCP_Packet.AckNumber]
	bswap	edi
	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.last_ack_number], edi
       DEBUGF 1,"Setting last_ack_number to %u\n", edi
	bswap	edi

; Dequeue all acknowledged packets
	cmp	[TCP_OUT_QUEUE], 0		; first, check if any packets are queued at all
	je	.no_ack

	push	ecx
       DEBUGF 1,"Removing all queued packets with smaller ACK\n"
	mov	ecx, TCP_QUEUE_SIZE
	mov	esi, TCP_OUT_QUEUE+4
  .loop:
	cmp	[esi + tcp_out_queue_entry.data_ptr], 0
	je	.maybe_next

	cmp	[esi + tcp_out_queue_entry.socket], ebx
	jne	.maybe_next

	cmp	[esi + tcp_out_queue_entry.seq_num], edi
	jg	.maybe_next

       DEBUGF 1,"Removing a queued packet\n"

	push	[esi + tcp_out_queue_entry.data_ptr]
	mov	[esi + tcp_out_queue_entry.data_ptr], 0
	dec	[TCP_OUT_QUEUE]
	call	kernel_free

  .maybe_next:
	add	esi, tcp_out_queue_entry.size
	loop	.loop
	pop	ecx


; Now call the correct handler, depending on the socket state
  .no_ack:
	mov	eax, [ebx +  SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state]

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
; TCP_send  (Assumes socket mutex set)
;
; IN: eax = socket pointer
;      bl = flags
;      ecx = number of bytes to send, may be set to 0
;      esi = pointer to data
;
;-----------------------------------------------------------------
align 4
TCP_send:

	DEBUGF 1,"Creating TCP packet, socket: %x, flags: %x\n",eax, bl

	mov	di , IP_PROTO_TCP
	add	ecx, TCP_Packet.Data

	push	bx eax esi
; Create an IPv4 Packet of the correct size
	mov	ebx, [eax + SOCKET_head.end + IPv4_SOCKET.LocalIP]
	mov	eax, [eax + SOCKET_head.end + IPv4_SOCKET.RemoteIP]

	call	IPv4_create_packet
	cmp	edi, -1
	je	.fail

; If there is any data, copy it first
	pop	esi
	push	edi
	add	edi, TCP_Packet.Data
	sub	ecx, TCP_Packet.Data

	shr	ecx, 1
	jnc	.nb
	movsb
.nb:	shr	ecx, 1
	jnc	.nw
	movsw
.nw:	test	ecx, ecx
	jz	.nd
	rep	movsd
.nd:
	pop	edi

; Fill in the TCP header
	pop	esi

; fill in tcp sequence number
	push	[esi + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.SND_NXT]
	pop	[edi + TCP_Packet.SequenceNumber]
	inc_INET (ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.SND_NXT) ;;;;;;;;

; Fill in local and remote ports
	push	dword [esi + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.LocalPort]
	pop	dword [edi + TCP_Packet.SourcePort]

; Acknumber
	push	[esi + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RCV_NXT]
	pop	[edi + TCP_Packet.AckNumber]

; Fill  in other tcp options
	pop	cx
	mov	[edi + TCP_Packet.Flags], cl
	mov	[edi + TCP_Packet.Window], 0x0005	   ; 1280 bytes
	mov	[edi + TCP_Packet.UrgentPointer], 0
	mov	[edi + TCP_Packet.DataOffset], 0x50
	mov	[edi + TCP_Packet.Checksum], 0

; Push pointer to and size of total packet (needed for send procedure)
	push	edx eax

; push socket number (for TCP_add_to_queue)
	push	esi

; Now, calculate the checksum                  ; TODO: calculate correct checksum for packets with data
	pushw	TCP_Packet.Data shl 8
	pushw	IP_PROTO_TCP shl 8
	pushd	[edi-4] ; destination address  ; TODO: fix this, IPv4 packet could have options..
	pushd	[edi-8] ; source address

	xor	edx, edx
	mov	ecx, TCP_Packet.Data
	mov	esi, edi
	call	checksum_1
	mov	ecx, 12
	mov	esi, esp
	call	checksum_1
	add	esp, 12 				   ; remove the pseudoheader from stack
; and store it in TCP header
	call	checksum_2
	mov	[edi + TCP_Packet.Checksum], dx

; At last send the packet!
	DEBUGF 1,"Sending TCP Packet to device %x\n", ebx
	mov	edx, [edi + TCP_Packet.SequenceNumber]
	bswap	edx
	mov	esi, [ebx + ETH_DEVICE.transmit]
	pop	edi
	jmp	TCP_queue

  .fail:
	add	esp, 2+4
	or	eax, -1
	ret


;-----------------------------------------------------------------
;
;  Queue a TCP packet for sending
;
;  IN:  [esp] pointer to buffer
;       [esp + 4] size of buffer
;       ebx = driver struct
;       esi = sender proc
;       edx = sequence number of this packet in normal byte order
;       edi = socket number
;  OUT: /
;
;-----------------------------------------------------------------
align 4
TCP_queue:

	bswap	edx
	DEBUGF 1,"Adding packet to TCP queue, buffer: %x, size: %u, driver: %x, acknum: %u\n", [esp], [esp+4], ebx, edx
	bswap	edx

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

  .found_it:			; eax points to empty queue entry

	pop	[eax + tcp_out_queue_entry.data_ptr]
	pop	[eax + tcp_out_queue_entry.data_size]
	mov	[eax + tcp_out_queue_entry.ttl], 1			; send immediately
	mov	[eax + tcp_out_queue_entry.retries], TCP_RETRIES
	mov	[eax + tcp_out_queue_entry.owner], ebx
	mov	[eax + tcp_out_queue_entry.sendproc], esi
	mov	[eax + tcp_out_queue_entry.seq_num], edx
	mov	[eax + tcp_out_queue_entry.socket], edi

	inc	[TCP_OUT_QUEUE]

	sub	eax, TCP_OUT_QUEUE+4
	DEBUGF 1,"Added to queue in pos %u\n", eax

	ret





;---------- TCB state handlers start here




align 4
stateTCB_LISTEN:

	DEBUGF	1,"TCBStateHandler: Listen\n"

	test	[edx + TCP_Packet.Flags], TH_SYN	; SYN packet? => send syn+ack, open new socket and set connection to established
	jz	.exit
; Exit if backlog queue is full
	mov	ax, [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.backlog_cur]
	cmp	ax, [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.backlog]
	jae	.exit
; Allocate new socket
	push	esi edi
	call	net_socket_alloc
	test	eax, eax
	jz	.fail
; Copy structure from current socket to new, including lock
	lea	esi, [ebx + SOCKET_head.PID]		; yes, PID must also be copied
	lea	edi, [eax + SOCKET_head.PID]
	mov	ecx, ((SOCKET_head.end - SOCKET_head.PID) + IPv4_SOCKET.end + TCP_SOCKET.end + 3)/4
	rep	movsd
	pop	edi esi
; Push pointer to new socket to queue
	movzx	ecx, [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.backlog_cur]
	inc	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.backlog_cur]
	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.end + ecx*4], eax

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

	mov	[ebx + SOCKET_head.lock], 0

	push	eax
; Now construct the response
	mov	bl, TH_SYN + TH_ACK
	xor	ecx, ecx
	call	TCP_send
	pop	eax

	mov	[eax + SOCKET_head.lock], 0
	mov	[eax +	SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_SYN_RECEIVED
	call	notify_network_event
	ret

  .exit:
	mov	[ebx + SOCKET_head.lock], 0
	ret

  .fail:
	add	esp, 8
	mov	[ebx + SOCKET_head.lock], 0
	ret


align 4
stateTCB_SYN_SENT:

	DEBUGF	1,"TCBStateHandler: Syn_Sent\n"

	; We are awaiting an ACK to our SYN, with a SYM
	; Look at control flags - expecting an ACK

	mov	al, [edx + TCP_Packet.Flags]

	test	al, TH_RST
	jnz	.reset			; jump if RST bit set

	push	[edx + TCP_Packet.SequenceNumber]				     ;;
	pop	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RCV_NXT]	     ;;
	inc_INET (ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RCV_NXT)      ;;


	push	[edx + TCP_Packet.AckNumber]					    ;;;;;;
	pop	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.SND_NXT]	    ;;;;;;

	and	al, TH_SYN + TH_ACK
	jz	.exit			; jump if none of the following is set: RST, SYN, ACK

	test	al, TH_ACK
	jz     .onlysyn 		; jump if only SYN bit is set

	; If we arrived here, SYN and ACK are set

	mov	[ebx +	SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_ESTABLISHED
	pushw	TH_ACK

  .send:	; Send an ACK
	mov	eax, ebx
	pop	bx
	push	eax
	xor	ecx, ecx
	call	TCP_send
	pop	ebx

  .exit:
	mov	[ebx + SOCKET_head.lock], 0
	ret

  .reset:
	; TODO: ....

	; remove all queued TCP packets for this connection !

	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_CLOSED
	mov	[ebx + SOCKET_head.lock], 0
	ret

  .onlysyn:
	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_SYN_RECEIVED
	pushw	TH_SYN + TH_ACK
	jmp	.send



align 4
stateTCB_SYN_RECEIVED:

	DEBUGF	1,"TCBStateHandler: Syn_received\n"

	test	[edx + TCP_Packet.Flags], TH_RST	; reset connection? => LISTEN
	jz	.check_ack

	push	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.OrigRemotePort]
	pop	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RemotePort]
	push	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.OrigRemoteIP]
	pop	[ebx + SOCKET_head.end + IPv4_SOCKET.RemoteIP]

	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_LISTEN
	jmp	.exit

  .check_ack:
	test	[edx + TCP_Packet.Flags], TH_ACK	; ACK? => connection established!
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
	bswap	eax
	DEBUGF	1,"RCV_NXT is set to:%u\n", eax
	bswap	eax
	cmp	eax, [edx + TCP_Packet.SequenceNumber]
	jne	.exit

; Calculate next sequencenumber
	test	ecx, ecx
	jnz	@f
	inc	ecx
       @@:
	add_INET (ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RCV_NXT)

	test	[edx + TCP_Packet.Flags], TH_FIN
	jnz	.fin

  .check_ack:
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

; Now, see if we received any data
	test	ecx, ecx
	jz	.ack

	DEBUGF	1,"Got %u bytes data!\n", ecx
; calculate header length
	movzx	eax, [edx + TCP_Packet.DataOffset]
	and	eax, 11110000b
	shr	eax, 2
       DEBUGF 1,"TCP header size: %u\n", eax
	add	edx, eax
	add	esp, 4
	pop	esi
	add	esp, 4
	sub	edx, esi
	mov	edi, edx
	mov	eax, ebx
	jmp	socket_internal_receiver	; Place the data from packet into socket

  .ack:
	mov	eax, ebx
	mov	bl, TH_ACK
	push	eax
	xor	ecx, ecx
	call	TCP_send		    ; send the ack
	pop	ebx
  .exit:
	mov	[ebx + SOCKET_head.lock], 0
	ret

  .fin:
; Remove all resend entries from the queue
	mov	ecx, TCP_QUEUE_SIZE
	mov	esi, TCP_OUT_QUEUE+4

  .removeloop:
	cmp	[esi + tcp_out_queue_entry.data_ptr], 0
	je	.maybe_next

	; TODO: check if the packets belong to the same tcp connection !

       DEBUGF 1,"Removing a queued packet\n"

	push	[esi + tcp_out_queue_entry.data_ptr]
	mov	[esi + tcp_out_queue_entry.data_ptr], 0
	dec	[TCP_OUT_QUEUE]
	call	kernel_free

  .maybe_next:
	add	esi, tcp_out_queue_entry.size
	loop	.removeloop

; Send an ACK to that fin, and enter closewait state
	mov	[ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_CLOSE_WAIT
	jmp	.check_ack



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

    @@:

;        lea     esi, [ebx + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RCV_NXT]
;        inc_INET esi

	; Send an ACK
	mov	eax, ebx
	mov	bl, TH_ACK
	push	eax
	xor	ecx, ecx
	call	TCP_send
	pop	ebx

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
	push	eax
	xor	ecx, ecx
	call	TCP_send
	pop	ebx

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
