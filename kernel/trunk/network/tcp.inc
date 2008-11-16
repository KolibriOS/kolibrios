;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  TCP.INC                                                     ;;
;;                                                              ;;
;;  TCP Processes for Menuet OS  TCP/IP stack                   ;;
;;                                                              ;;
;;  Copyright 2002 Mike Hibbett, mikeh@oceanfree.net            ;;
;;                                                              ;;
;;  See file COPYING for details                                ;;
;;  v0.6 : Added reset handling in the established state        ;;
;;         Added a timer per socket to allow delays when        ;;
;;         rx window gets below 1KB                             ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


; TCP TCB states
TCB_LISTEN	   equ	      1
TCB_SYN_SENT	   equ	      2
TCB_SYN_RECEIVED   equ	      3
TCB_ESTABLISHED    equ	      4
TCB_FIN_WAIT_1	   equ	      5
TCB_FIN_WAIT_2	   equ	      6
TCB_CLOSE_WAIT	   equ	      7
TCB_CLOSING	   equ	      8
TCB_LAST_ACK	   equ	      9
TCB_TIMED_WAIT	   equ	      10
TCB_CLOSED	   equ	      11

TH_FIN	= 0x01
TH_SYN	= 0x02
TH_RST	= 0x04
TH_PUSH = 0x08
TH_ACK	= 0x10
TH_URG	= 0x20

TWOMSL		    equ     10	    ; # of secs to wait before closing socket

TCP_RETRIES	    equ 	5		; Number of times to resend a packet
TCP_TIMEOUT	    equ 	10		; resend if not replied to in x hs

;*******************************************************************
;   Interface
;
;       tcp_tx_handler      Handles the TCP transmit queue
;       tcp_rx              The protocol handler for received data
;       buildTCPPacket      fills in the packet headers and data
;       tcpStateMachine     Main state machine for received TCP packets
;       tcp_tcb_handler     1s timer, to erase tcb's in TIME_WAIT state
;
;*******************************************************************


;   TCP Payload ( Data field in IP datagram )
;
;    0                   1                   2                   3
;    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;20 |          Source Port          |       Destination Port        |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;24 |                        Sequence Number                        |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;28 |                    Acknowledgment Number                      |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;32 |  Data |           |U|A|P|R|S|F|                               |
;   | Offset| Reserved  |R|C|S|S|Y|I|            Window             |
;   |       |           |G|K|H|T|N|N|                               |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;36 |           Checksum            |         Urgent Pointer        |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;40 |                    Options                    |    Padding    |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   |                             data


struc TCP_PACKET
{  .SourcePort	     dw  ?  ;+00
   .DestinationPort  dw  ?  ;+02
   .SequenceNumber   dd  ?  ;+04
   .AckNumber	     dd  ?  ;+08
   .DataOffset	     db  ?  ;+12 - DataOffset[0-3 bits] and Reserved[4-7]
   .Flags	     db  ?  ;+13 - Reserved[0-1 bits]|URG|ACK|PSH|RST|SYN|FIN
   .Window	     dw  ?  ;+14
   .Checksum	     dw  ?  ;+16
   .UrgentPointer    dw  ?  ;+18
   .Options	     rb  3  ;+20
   .Padding	     db  ?  ;+23
   .Data	     db  ?  ;+24
}

virtual at 0
  TCP_PACKET TCP_PACKET
end virtual



;***************************************************************************
;   Function
;      tcp_tcb_handler
;
;   Description
;       Handles sockets in the timewait state, closing them
;       when the TCB timer expires
;
;***************************************************************************

proc tcp_tcb_handler stdcall uses ebx
	; scan through all the sockets, decrementing active timers

	mov	ebx, net_sockets

	cmp	[ebx + SOCKET.NextPtr], 0
	je	.exit
	DEBUGF	1, "K : sockets:\n"

  .next_socket:
	mov	ebx, [ebx + SOCKET.NextPtr]
	or	ebx, ebx
	jz	.exit

	DEBUGF	1, "K :   %x-%x: %x-%x-%x-%u\n", [ebx + SOCKET.PID]:2, [ebx + SOCKET.Number]:2, [ebx + SOCKET.LocalPort]:4, [ebx + SOCKET.RemoteIP], [ebx + SOCKET.RemotePort]:4, [ebx + SOCKET.TCBState]

	cmp	[ebx + SOCKET.TCBTimer], 0
	jne	.decrement_tcb
	cmp	[ebx + SOCKET.wndsizeTimer], 0
	jne	.decrement_wnd
	jmp	.next_socket

  .decrement_tcb:
	; decrement it, delete socket if TCB timer = 0 & socket in timewait state
	dec	[ebx + SOCKET.TCBTimer]
	jnz	.next_socket

	cmp	[ebx + SOCKET.TCBState], TCB_TIMED_WAIT
	jne	.next_socket

	push	[ebx + SOCKET.PrevPtr]
	stdcall net_socket_free, ebx
	pop	ebx
	jmp	.next_socket

  .decrement_wnd:
	; TODO - prove it works!
	dec	[ebx + SOCKET.wndsizeTimer]
	jmp	.next_socket

  .exit:
	ret
endp


;***************************************************************************
;   Function
;      tcp_tx_handler
;
;   Description
;       Handles queued TCP data
;       This is a kernel function, called by stack_handler
;
;***************************************************************************

proc tcp_tx_handler stdcall
    ; decrement all resend buffers timers. If they
    ; expire, queue them for sending, and restart the timer.
    ; If the retries counter reach 0, delete the entry

	mov	esi, resendQ
	mov	ecx, 0

  .next_resendq:
	cmp	ecx, NUMRESENDENTRIES
	je	.exit		    ; None left
	cmp	dword[esi + 4], 0
	jne	@f		     ; found one
	inc	ecx
	add	esi, 8
	jmp	.next_resendq

    @@: ; we have one. decrement it's timer by 1
	dec	word[esi + 2]
	jz	@f
	inc	ecx
	add	esi, 8
	jmp	.next_resendq	    ; Timer not zero, so move on

    @@:
	xor	ebx, ebx
	; restart timer, and decrement retries
	; After the first resend, back of on next, by a factor of 5
	mov	[esi + 2], word TCP_TIMEOUT * 5
	dec	byte[esi + 1]
	jnz	@f

	; retries now 0, so delete from queue
	xchg	 [esi + 4], ebx

    @@: ; resend packet
	pushad

	mov	eax, EMPTY_QUEUE
	call	dequeue
	cmp	ax, NO_BUFFER
	jne	.tth004z

	; TODO - try again in 10ms.
	test	ebx, ebx
	jnz	@f
	mov	[esi + 4], ebx

    @@: ; Mark it to expire in 10ms - 1 tick
	mov	byte[esi + 1], 1
	mov	word[esi + 2], 1
	jmp	.tth005

  .tth004z:
	; we have a buffer # in ax
	push	eax ecx
	mov	ecx, IPBUFFSIZE
	mul	ecx
	add	eax, IPbuffs

	; we have the buffer address in eax
	mov	edi, eax
	pop	ecx
	; Now get buffer location, and copy buffer across. argh! more copying,,
	mov	esi, resendBuffer
    @@: add	esi, IPBUFFSIZE
	loop	@b

	; we have resend buffer location in esi
	mov	ecx, IPBUFFSIZE

	; copy data across
	push	edi
	cld
	rep	movsb
	pop	edi

	; queue packet
	mov	eax, NET1OUT_QUEUE
	mov	edx, [stack_ip]
	cmp	edx, [edi + IP_PACKET.DestinationAddress]
	jne	.not_local
	mov	eax, IPIN_QUEUE

  .not_local:
	pop	ebx
	call	queue

  .tth005:
	popad

	inc	ecx
	add	esi, 8
	jmp	.next_resendq

  .exit:
	ret
endp


;***************************************************************************
;   Function
;      tcp_rx
;
;   Description
;       TCP protocol handler
;       This is a kernel function, called by ip_rx
;       IP buffer address given in edx
;          IP buffer number in eax
;          Free up (or re-use) IP buffer when finished
;
;***************************************************************************

proc tcp_rx stdcall uses ebx
	; The process is as follows.
	; Look for a socket with matching remote IP, remote port, local port
	; if not found, then
	; look for remote IP + local port match ( where sockets remote port = 0)
	; if not found, then
	; look for a socket where local socket port == IP packets remote port
	; where sockets remote port, remote IP = 0
	; discard if not found
	; Call sockets tcbStateMachine, with pointer to packet.
	; the state machine will not delete the packet, so do that here.

	push	eax

	; Look for a socket where
	; IP Packet TCP Destination Port = local Port
	; IP Packet SA = Remote IP
	; IP Packet TCP Source Port = remote Port

	mov	ebx, net_sockets

  .next_socket.1:
	mov	ebx, [ebx + SOCKET.NextPtr]
	or	ebx, ebx
	jz	.next_socket.1.exit

;        DEBUGF  1, "K : tcp_rx - 1.dport: %x - %x\n", [edx + 20 + TCP_PACKET.DestinationPort]:4, [ebx + SOCKET.LocalPort]:4

	mov	ax, [edx + 20 + TCP_PACKET.DestinationPort]  ; get the dest. port from the TCP hdr
	cmp	[ebx + SOCKET.LocalPort], ax		; get the dest. port from the TCP hdr
	jne	.next_socket.1				; different - try next socket

;        DEBUGF  1, "K : tcp_rx - 1.addr: %x - %x\n", [edx + IP_PACKET.SourceAddress], [ebx + SOCKET.RemoteIP]

	mov	eax, [edx + IP_PACKET.SourceAddress]	; get the source IP Addr from the IP hdr
	cmp	[ebx + SOCKET.RemoteIP], eax		; compare with socket's remote IP
	jne	.next_socket.1				; different - try next socket

;        DEBUGF  1, "K : tcp_rx - 1.sport: %x - %x\n", [edx + 20 + TCP_PACKET.SourcePort]:4, [ebx + SOCKET.RemotePort]:4

	mov	ax, [edx + 20 + TCP_PACKET.SourcePort]	; get the source port from the TCP hdr
	cmp	[ebx + SOCKET.RemotePort], ax		; compare with socket's remote port
	jne	.next_socket.1				; different - try next socket

	; We have a complete match - use this socket
	jmp	.change_state

  .next_socket.1.exit:

	; If we got here, there was no match
	; Look for a socket where
	; IP Packet TCP Destination Port = local Port
	; IP Packet SA = Remote IP
	; socket remote Port = 0

	mov	ebx, net_sockets

  .next_socket.2:
	mov	ebx, [ebx + SOCKET.NextPtr]
	or	ebx, ebx
	jz	.next_socket.2.exit

;        DEBUGF  1, "K : tcp_rx - 2.dport: %x - %x\n", [edx + 20 + TCP_PACKET.DestinationPort]:4, [ebx + SOCKET.LocalPort]:4

	mov	ax, [edx + 20 + TCP_PACKET.DestinationPort]  ; get the dest. port from the TCP hdr
	cmp	[ebx + SOCKET.LocalPort], ax		; compare with socket's local port
	jne	.next_socket.2				; different - try next socket

;        DEBUGF  1, "K : tcp_rx - 2.addr: %x - %x\n", [edx + IP_PACKET.SourceAddress], [ebx + SOCKET.RemoteIP]

	mov	eax, [edx + IP_PACKET.SourceAddress]	; get the source IP Addr from the IP hdr
	cmp	[ebx + SOCKET.RemoteIP], eax		; compare with socket's remote IP
	jne	.next_socket.2				; different - try next socket

;        DEBUGF  1, "K : tcp_rx - 2.sport: 0000 - %x\n", [ebx + SOCKET.RemotePort]:4

	cmp	[ebx + SOCKET.RemotePort], 0		; only match a remote socket of 0
	jne	.next_socket.2				; different - try next socket

	; We have a complete match - use this socket
	jmp	.change_state

  .next_socket.2.exit:

	; If we got here, there was no match
	; Look for a socket where
	; IP Packet TCP Destination Port = local Port
	; socket Remote IP = 0
	; socket remote Port = 0

	mov	ebx, net_sockets

  .next_socket.3:
	mov	ebx, [ebx + SOCKET.NextPtr]
	or	ebx, ebx
	jz	.next_socket.3.exit

;        DEBUGF  1, "K : tcp_rx - 3.dport: %x - %x\n", [edx + 20 + TCP_PACKET.DestinationPort]:4, [ebx + SOCKET.LocalPort]:4

	mov	ax, [edx + 20 + TCP_PACKET.DestinationPort]  ; get destination port from the TCP hdr
	cmp	[ebx + SOCKET.LocalPort], ax		; compare with socket's local port
	jne	.next_socket.3				; different - try next socket

;        DEBUGF  1, "K : tcp_rx - 3.addr: 00000000 - %x\n", [ebx + SOCKET.RemoteIP]

	cmp	[ebx + SOCKET.RemoteIP], 0		; only match a socket remote IP of 0
	jne	.next_socket.3				; different - try next socket

;        DEBUGF  1, "K : tcp_rx - 3.sport: 0000 - %x\n", [ebx + SOCKET.RemotePort]:4

	cmp	[ebx + SOCKET.RemotePort], 0		; only match a remote socket of 0
	jne	.next_socket.3				; different - try next socket

	; We have a complete match - use this socket
	jmp	.change_state

  .next_socket.3.exit:

	; If we got here, we need to reject the packet

	DEBUGF	1, "K : tcp_rx - dumped\n"
	DEBUGF	1, "K :   --------: %x-%x-%x (flags: %x)\n", [edx + 20 + TCP_PACKET.DestinationPort]:4, [edx + IP_PACKET.SourceAddress], [edx + 20 + TCP_PACKET.SourcePort]:4, [edx + 20 + TCP_PACKET.Flags]:2

	inc	[dumped_rx_count]
	jmp	.exit

  .change_state:

	; We have a valid socket/TCB, so call the TCB State Machine for that skt.
	; socket is pointed to by ebx
	; IP packet is pointed to by edx
	; IP buffer number is on stack ( it will be popped at the end)

	stdcall tcpStateMachine, ebx

  .exit:
	pop	eax
	call	freeBuff
	ret
endp


;***************************************************************************
;   Function
;      buildTCPPacket
;
;   Description
;       builds an IP Packet with TCP data fully populated for transmission
;       You may destroy any and all registers
;          TCP control flags specified in bl
;          This TCB is in [sktAddr]
;          User data pointed to by esi
;       Data length in ecx
;          Transmit buffer number in eax
;
;***************************************************************************

proc build_tcp_packet stdcall, sockAddr:DWORD
	push	ecx			   ; Save data length

	; convert buffer pointer eax to the absolute address
	mov	ecx, IPBUFFSIZE
	mul	ecx
	add	eax, IPbuffs

	mov	edx, eax

	mov	[edx + 20 + TCP_PACKET.Flags], bl		; TCP flags

	mov	ebx, [sockAddr]

	; So, ebx holds the socket ptr, edx holds the IPbuffer ptr

	; Fill in the IP header ( some data is in the socket descriptor)
	mov	eax, [ebx + SOCKET.LocalIP]
	mov	[edx + IP_PACKET.SourceAddress], eax
	mov	eax, [ebx + SOCKET.RemoteIP]
	mov	[edx + IP_PACKET.DestinationAddress], eax

	mov	[edx + IP_PACKET.VersionAndIHL], 0x45
	mov	[edx + IP_PACKET.TypeOfService], 0

	pop	eax		      ; Get the TCP data length
	push	eax

	add	eax, 20 + 20	       ; add IP header and TCP header lengths
	rol	ax, 8
	mov	[edx + IP_PACKET.TotalLength], ax
	mov	[edx + IP_PACKET.Identification], 0
	mov	[edx + IP_PACKET.FlagsAndFragmentOffset], 0x0040
	mov	[edx + IP_PACKET.TimeToLive], 0x20
	mov	[edx + IP_PACKET.Protocol], PROTOCOL_TCP

	; Checksum left unfilled
	mov	[edx + IP_PACKET.HeaderChecksum], 0

	; Fill in the TCP header (some data is in the socket descriptor)
	mov	ax, [ebx + SOCKET.LocalPort]
	mov	[edx + 20 + TCP_PACKET.SourcePort], ax		; Local Port

	mov	ax, [ebx + SOCKET.RemotePort]
	mov	[edx + 20 + TCP_PACKET.DestinationPort], ax	; desitination Port

	; Checksum left unfilled
	mov	[edx + 20 + TCP_PACKET.Checksum], 0

	; sequence number
	mov	eax, [ebx + SOCKET.SND_NXT]
	mov	[edx + 20 + TCP_PACKET.SequenceNumber], eax

	; ack number
	mov	eax, [ebx + SOCKET.RCV_NXT]
	mov	[edx + 20 + TCP_PACKET.AckNumber], eax

	; window ( 0x2000 is default ).I could accept 4KB, fa0, ( skt buffer size)
	; 768 bytes seems better
	mov	[edx + 20 + TCP_PACKET.Window], 0x0003

	; Urgent pointer (0)
	mov	[edx + 20 + TCP_PACKET.UrgentPointer], 0

	; data offset ( 0x50 )
	mov	[edx + 20 + TCP_PACKET.DataOffset], 0x50

	pop	ecx		     ; count of bytes to send
	mov	ebx, ecx	    ; need the length later

	cmp	ebx, 0
	jz	@f

	mov	edi, edx
	add	edi, 40
	cld
	rep	movsb		    ; copy the data across

    @@: ; we have edx as IPbuffer ptr.
	; Fill in the TCP checksum
	; First, fill in pseudoheader
	mov	eax, [edx + IP_PACKET.SourceAddress]
	mov	[pseudoHeader], eax
	mov	eax, [edx + IP_PACKET.DestinationAddress]
	mov	[pseudoHeader + 4], eax
	mov	word[pseudoHeader + 8], PROTOCOL_TCP shl 8 + 0
	add	ebx, 20
	mov	[pseudoHeader + 10], bh
	mov	[pseudoHeader + 11], bl

	mov	eax, pseudoHeader
	mov	[checkAdd1], eax
	mov	word[checkSize1], 12
	mov	eax, edx
	add	eax, 20
	mov	[checkAdd2], eax
	mov	eax, ebx
	mov	[checkSize2], ax

	call	checksum

	; store it in the TCP checksum ( in the correct order! )
	mov	ax, [checkResult]
	rol	ax, 8
	mov	[edx + 20 + TCP_PACKET.Checksum], ax

	; Fill in the IP header checksum
	GET_IHL eax, edx	       ; get IP-Header length
	stdcall checksum_jb, edx, eax  ; buf_ptr, buf_size
	rol	ax, 8
	mov	[edx + IP_PACKET.HeaderChecksum], ax

	ret
endp


; Increments the 32 bit value pointed to by esi in internet order
proc inc_inet_esi stdcall
	push	eax
	mov	eax, [esi]
	bswap	eax
	inc	eax
	bswap	eax
	mov	[esi], eax
	pop	eax
	ret
endp


; Increments the 32 bit value pointed to by esi in internet order
; by the value in ecx
proc add_inet_esi stdcall
	push	eax
	mov	eax, [esi]
	bswap	eax
	add	eax, ecx
	bswap	eax
	mov	[esi], eax
	pop	eax
	ret
endp


iglobal
  TCBStateHandler dd \
    stateTCB_LISTEN, \
    stateTCB_SYN_SENT, \
    stateTCB_SYN_RECEIVED, \
    stateTCB_ESTABLISHED, \
    stateTCB_FIN_WAIT_1, \
    stateTCB_FIN_WAIT_2, \
    stateTCB_CLOSE_WAIT, \
    stateTCB_CLOSING, \
    stateTCB_LAST_ACK, \
    stateTCB_TIME_WAIT, \
    stateTCB_CLOSED
endg


;***************************************************************************
;   Function
;      tcpStateMachine
;
;   Description
;       TCP state machine
;       This is a kernel function, called by tcp_rx
;
;       IP buffer address given in edx
;          Socket/TCB address in ebx
;
;       The IP buffer will be released by the caller
;***************************************************************************

proc tcpStateMachine stdcall, sockAddr:DWORD
	; as a packet has been received, update the TCB timer
	mov	[ebx + SOCKET.TCBTimer], TWOMSL

	; If the received packet has an ACK bit set,
	; remove any packets in the resend queue that this
	; received packet acknowledges
	pushad
	test	[edx + 20 + TCP_PACKET.Flags], TH_ACK
	jz	.call_handler					; No ACK, so no data yet

	; get skt number in eax
	stdcall net_socket_addr_to_num, ebx

	; The ack number is in [edx + 28], inet format
	; skt in eax

	mov	esi, resendQ
	xor	ecx, ecx

  .next_resendq:
	cmp	ecx, NUMRESENDENTRIES
	je	.call_handler	  ; None left
	cmp	[esi + 4], eax
	je	@f		  ; found one
	inc	ecx
	add	esi, 8
	jmp	.next_resendq

    @@: 		  ; Can we delete this buffer?

			  ; If yes, goto @@. No, goto .next_resendq
	; Get packet data address

	push	ecx
	; Now get buffer location, and copy buffer across. argh! more copying,,
	imul	edi, ecx, IPBUFFSIZE
	add	edi, resendBuffer

	; we have dest buffer location in edi. incoming packet in edx.
	; Get this packets sequence number
	; preserve al, ecx, esi, edx
	mov	ecx, [edi + 20 + TCP_PACKET.SequenceNumber]
	bswap	ecx
	movzx	ebx, word[edi + 2]
	xchg	bl, bh
	sub	ebx, 40
	add	ecx, ebx	  ; ecx is now seq# of last byte +1, intel format

	; get recievd ack #, in intel format
	mov	ebx, [edx + 20 + TCP_PACKET.AckNumber]
	bswap	ebx

	cmp	ebx, ecx	; Finally. ecx = rx'ed ack. ebx = last byte in que
				; DANGER! need to handle case that we have just
				; passed the 2**32, and wrapped round!
	pop	ecx
	jae	@f		; if rx > old, delete old

	inc	ecx
	add	esi, 8
	jmp	.next_resendq

    @@: mov	dword[esi + 4], 0
	inc	ecx
	add	esi, 8
	jmp	.next_resendq

  .call_handler:
	popad

	; Call handler for given TCB state

	mov	eax, [ebx + SOCKET.TCBState]
	cmp	eax, TCB_LISTEN
	jb	.exit
	cmp	eax, TCB_CLOSED
	ja	.exit

	stdcall [TCBStateHandler + (eax - 1) * 4], [sockAddr]

  .exit:
	ret
endp


proc stateTCB_LISTEN stdcall, sockAddr:DWORD
	; In this case, we are expecting a SYN packet
	; For now, if the packet is a SYN, process it, and send a response
	; If not, ignore it

	; Look at control flags
	test	[edx + 20 + TCP_PACKET.Flags], TH_SYN
	jz	.exit

	; We have a SYN. update the socket with this IP packets details,
	; And send a response

	mov	eax, [edx + IP_PACKET.SourceAddress]
	mov	[ebx + SOCKET.RemoteIP], eax
	mov	ax, [edx + 20 + TCP_PACKET.SourcePort]
	mov	[ebx + SOCKET.RemotePort], ax
	mov	eax, [edx + 20 + TCP_PACKET.SequenceNumber]
	mov	[ebx + SOCKET.IRS], eax
	mov	[ebx + SOCKET.RCV_NXT], eax
	lea	esi, [ebx + SOCKET.RCV_NXT]
	call	inc_inet_esi ; RCV.NXT
	mov	eax, [ebx + SOCKET.ISS]
	mov	[ebx + SOCKET.SND_NXT], eax

	; Now construct the response, and queue for sending by IP
	mov	eax, EMPTY_QUEUE
	call	dequeue
	cmp	ax, NO_BUFFER
	je	.exit

	push	eax
	mov	bl, TH_SYN + TH_ACK
	xor	ecx, ecx
	xor	esi, esi
	stdcall build_tcp_packet, [sockAddr]

	mov	eax, NET1OUT_QUEUE
	mov	edx, [stack_ip]
	mov	ecx, [sockAddr]
	cmp	edx, [ecx + SOCKET.RemoteIP]
	jne	.not_local
	mov	eax, IPIN_QUEUE

  .not_local:
	; Send it.
	pop	ebx
	call	queue

	mov	esi, [sockAddr]
	mov	[esi + SOCKET.TCBState], TCB_SYN_RECEIVED

	; increment SND.NXT in socket
	add	esi, SOCKET.SND_NXT
	call	inc_inet_esi

  .exit:
	ret
endp


proc stateTCB_SYN_SENT stdcall, sockAddr:DWORD
	; We are awaiting an ACK to our SYN, with a SYM
	; Look at control flags - expecting an ACK

	mov	al, [edx + 20 + TCP_PACKET.Flags]
	and	al, TH_SYN + TH_ACK
	cmp	al, TH_SYN + TH_ACK
	je	.syn_ack

	test	al, TH_SYN
	jz	.exit

	mov	[ebx + SOCKET.TCBState], TCB_SYN_RECEIVED
	push	TH_SYN + TH_ACK
	jmp	.send

  .syn_ack:
	mov	[ebx + SOCKET.TCBState], TCB_ESTABLISHED
	push	TH_ACK

  .send:
	; Store the recv.nxt field
	mov	eax, [edx + 20 + TCP_PACKET.SequenceNumber]

	; Update our recv.nxt field
	mov	[ebx + SOCKET.RCV_NXT], eax
	lea	esi, [ebx + SOCKET.RCV_NXT]
	call	inc_inet_esi

	; Send an ACK
	; Now construct the response, and queue for sending by IP
	mov	eax, EMPTY_QUEUE
	call	dequeue
	cmp	ax, NO_BUFFER
	pop	ebx
	je	.exit

	push	eax

	xor	ecx, ecx
	xor	esi, esi
	stdcall build_tcp_packet, [sockAddr]

	mov	eax, NET1OUT_QUEUE
	mov	edx, [stack_ip]
	mov	ecx, [sockAddr]
	cmp	edx, [ecx + SOCKET.RemoteIP]
	jne	.not_local
	mov	eax, IPIN_QUEUE

  .not_local:
	; Send it.
	pop	ebx
	call	queue

  .exit:
	ret
endp


proc stateTCB_SYN_RECEIVED stdcall, sockAddr:DWORD
	; In this case, we are expecting an ACK packet
	; For now, if the packet is an ACK, process it,
	; If not, ignore it

	test	[edx + 20 + TCP_PACKET.Flags], TH_RST
	jz	.check_ack

	push	[ebx + SOCKET.OrigRemotePort] [ebx + SOCKET.OrigRemoteIP]
	pop	[ebx + SOCKET.RemoteIP] [ebx + SOCKET.RemotePort]

	mov	[ebx + SOCKET.TCBState], TCB_LISTEN
	jmp	.exit

  .check_ack:
	; Look at control flags - expecting an ACK
	test	[edx + 20 + TCP_PACKET.Flags], TH_ACK
	jz	.exit

	mov	[ebx + SOCKET.TCBState], TCB_ESTABLISHED

  .exit:
	ret
endp


proc stateTCB_ESTABLISHED stdcall, sockAddr:DWORD
	; Here we are expecting data, or a request to close
	; OR both...

	; Did we receive a FIN or RST?
	test	[edx + 20 + TCP_PACKET.Flags], TH_FIN
	jz	.check_ack

	; It was a fin or reset.

	; Remove resend entries from the queue  - I dont want to send any more data
	pushad

	; get skt #
	stdcall net_socket_addr_to_num, ebx

	mov	esi, resendQ
	mov	ecx, 0

  .next_resendq:
	cmp	ecx, NUMRESENDENTRIES
	je	.last_resendq	    ; None left
	cmp	[esi + 4], eax
	je	@f		    ; found one
	inc	ecx
	add	esi, 8
	jmp	.next_resendq

    @@: mov	dword[esi + 4], 0
	inc	ecx
	add	esi, 8
	jmp	.next_resendq

  .last_resendq:
	popad

    @@: ; Send an ACK to that fin, and enter closewait state

	mov	[ebx + SOCKET.TCBState], TCB_CLOSE_WAIT
	lea	esi, [ebx + SOCKET.RCV_NXT]
	mov	eax, [esi]		; save original
	call	inc_inet_esi
	;; jmp    ste_ack - NO, there may be data

  .check_ack:
	; Check that we received an ACK
	test	[edx + 20 + TCP_PACKET.Flags], TH_ACK
	jz	.exit

	; TODO - done, I think!
	; First, look at the incoming window. If this is less than or equal to 1024,
	; Set the socket window timer to 1. This will stop an additional packets being queued.
	; ** I may need to tweak this value, since I do not know how many packets are already queued
	mov	cx, [edx + 20 + TCP_PACKET.Window]
	xchg	cl, ch
	cmp	cx, 1024
	ja	@f

	mov	[ebx + SOCKET.wndsizeTimer], 1

    @@: ; OK, here is the deal
	; My recv.nct field holds the seq of the expected next rec byte
	; if the recevied sequence number is not equal to this, do not
	; increment the recv.nxt field, do not copy data - just send a
	; repeat ack.

	; recv.nxt is in dword [edx+24], in inet format
	; recv seq is in [sktAddr]+56, in inet format
	; just do a comparision
	mov	ecx, [ebx + SOCKET.RCV_NXT]
	cmp	[ebx + SOCKET.TCBState], TCB_CLOSE_WAIT
	jne	@f
	mov	ecx, eax

    @@: cmp	ecx, [edx + 20 + TCP_PACKET.SequenceNumber]
	jne	.ack


	; Read the data bytes, store in socket buffer
	movzx	ecx, [edx + IP_PACKET.TotalLength]
	xchg	cl, ch
	sub	ecx, 40 		   ; Discard 40 bytes of header
	jnz	.data			   ; Read data, if any

	; If we had received a fin, we need to ACK it.
	cmp	[ebx + SOCKET.TCBState], TCB_CLOSE_WAIT
	je	.ack
	jmp	.exit

  .data:
	push	ecx

	add	[ebx + SOCKET.rxDataCount], ecx      ; increment the count of bytes in buffer

	mov	eax, [ebx + SOCKET.PID]       ; get socket owner PID
	push	eax

	mov	eax, [ebx + SOCKET.rxDataCount]      ; get # of bytes already in buffer

	; point to the location to store the data
	lea	edi, [ebx + eax + SOCKETHEADERSIZE]
	sub	edi, ecx

	add	edx, 40        ; edx now points to the data
	mov	esi, edx

	cld
	rep	movsb	       ; copy the data across

	; flag an event to the application
	pop	eax
	mov	ecx, 1
	mov	esi, TASK_DATA + TASKDATA.pid

  .next_pid:
	cmp	[esi], eax
	je	.found_pid
	inc	ecx
	add	esi, 0x20
	cmp	ecx, [TASK_COUNT]
	jbe	.next_pid

  .found_pid:
	shl	ecx, 8
	or	[ecx + SLOT_BASE + APPDATA.event_mask], EVENT_NETWORK ; stack event

	pop	ecx

	; Update our recv.nxt field
	lea	esi, [ebx + SOCKET.RCV_NXT]
	call	add_inet_esi

  .ack:
	; Send an ACK
	; Now construct the response, and queue for sending by IP
	mov	eax, EMPTY_QUEUE
	call	dequeue
	cmp	ax, NO_BUFFER
	je	.exit

	push	eax

	mov	bl, TH_ACK
	xor	ecx, ecx
	xor	esi, esi
	stdcall build_tcp_packet, [sockAddr]

	mov	eax, NET1OUT_QUEUE

	mov	edx, [stack_ip]
	mov	ecx, [sockAddr]
	cmp	edx, [ecx + SOCKET.RemoteIP]
	jne	.not_local
	mov	eax, IPIN_QUEUE

  .not_local:
	; Send it.
	pop	ebx
	call	queue

  .exit:
	ret
endp


proc stateTCB_FIN_WAIT_1 stdcall, sockAddr:DWORD
	; We can either receive an ACK of a fin, or a fin
	mov	al, [edx + 20 + TCP_PACKET.Flags]
	and	al, TH_FIN + TH_ACK

	cmp	al, TH_ACK
	jne	@f

	; It was an ACK
	mov	[ebx + SOCKET.TCBState], TCB_FIN_WAIT_2
	jmp	.exit

    @@: mov	[ebx + SOCKET.TCBState], TCB_CLOSING
	cmp	al, TH_FIN
	je	@f
	mov	[ebx + SOCKET.TCBState], TCB_TIMED_WAIT

    @@: lea	esi, [ebx + SOCKET.RCV_NXT]
	call	inc_inet_esi

	; Send an ACK
	mov	eax, EMPTY_QUEUE
	call	dequeue
	cmp	ax, NO_BUFFER
	je	.exit

	push	eax

	mov	bl, TH_ACK
	xor	ecx, ecx
	xor	esi, esi
	stdcall build_tcp_packet, [sockAddr]

	mov	eax, NET1OUT_QUEUE
	mov	edx, [stack_ip]
	mov	ecx, [sockAddr]
	cmp	edx, [ecx + SOCKET.RemoteIP]
	jne	.not_local
	mov	eax, IPIN_QUEUE

  .not_local:
	; Send it.
	pop	ebx
	call	queue

  .exit:
	ret
endp


proc stateTCB_FIN_WAIT_2 stdcall, sockAddr:DWORD
	test	[edx + 20 + TCP_PACKET.Flags], TH_FIN
	jz	.exit

	; Change state, as we have a fin
	mov	[ebx + SOCKET.TCBState], TCB_TIMED_WAIT

	lea	esi, [ebx + SOCKET.RCV_NXT]
	call	inc_inet_esi

	; Send an ACK
	mov	eax, EMPTY_QUEUE
	call	dequeue
	cmp	ax, NO_BUFFER
	je	.exit

	push	eax

	mov	bl, TH_ACK
	xor	ecx, ecx
	xor	esi, esi
	stdcall build_tcp_packet, [sockAddr]

	mov	eax, NET1OUT_QUEUE
	mov	edx, [stack_ip]
	mov	ecx, [sockAddr]
	cmp	edx, [ecx + SOCKET.RemoteIP]
	jne	.not_local
	mov	eax, IPIN_QUEUE

  .not_local:
	; Send it.
	pop	ebx
	call	queue

  .exit:
	ret
endp


proc stateTCB_CLOSE_WAIT stdcall, sockAddr:DWORD
	; Intentionally left empty
	; socket_close_tcp handles this
	ret
endp


proc stateTCB_CLOSING stdcall, sockAddr:DWORD
	; We can either receive an ACK of a fin, or a fin
	test	[edx + 20 + TCP_PACKET.Flags], TH_ACK
	jz	.exit

	mov	[ebx + SOCKET.TCBState], TCB_TIMED_WAIT

  .exit:
	ret
endp


proc stateTCB_LAST_ACK stdcall, sockAddr:DWORD
	; Look at control flags - expecting an ACK
	test	[edx + 20 + TCP_PACKET.Flags], TH_ACK
	jz	.exit

	; delete the socket
	stdcall net_socket_free, ebx

  .exit:
	ret
endp


proc stateTCB_TIME_WAIT stdcall, sockAddr:DWORD
	ret
endp


proc stateTCB_CLOSED stdcall, sockAddr:DWORD
	ret
endp
