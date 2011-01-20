
;----------------------
; 160 ms timer
;----------------------
macro	TCP_timer_160ms {

local	.loop
local	.exit

	mov	eax, net_sockets
  .loop:
	mov	eax, [eax + SOCKET.NextPtr]
	or	eax, eax
	jz	.exit

	cmp	[eax + SOCKET.Protocol], IP_PROTO_TCP		;;; We should also check if family is AF_INET
	jne	.loop

	dec	[eax + TCP_SOCKET.timer_ack]
	jnz	.loop

	DEBUGF	1,"TCP ack for socket %x expired, time to piggyback!\n", eax

	push	eax
	call	TCP_respond_socket
	pop	eax

	jmp	.loop

  .exit:

}


;----------------------
; 640 ms timer
;----------------------
macro	TCP_timer_640ms {

local	.loop
local	.exit

; Update TCP sequence number

	add	[TCP_sequence_num], 64000

; scan through all the active TCP sockets, decrementing ALL timers
; timers do not have the chance to wrap because the keepalive timer will kill the socket when it expires

	mov	eax, net_sockets
  .loop:
	mov	eax, [eax + SOCKET.NextPtr]
  .check_only:
	or	eax, eax
	jz	.exit

	cmp	[eax + SOCKET.Domain], AF_INET4
	jne	.loop

	cmp	[eax + SOCKET.Protocol], IP_PROTO_TCP
	jne	.loop

	inc	[eax + TCP_SOCKET.t_idle]
	dec	[eax + TCP_SOCKET.timer_retransmission]
	jnz	.check_more2

	DEBUGF	1,"socket %x: Retransmission timer expired\n", eax

	push	eax
	call	TCP_output
	pop	eax

  .check_more2:
	dec	[eax + TCP_SOCKET.timer_keepalive]
	jnz	.check_more3

	DEBUGF	1,"socket %x: Keepalive expired\n", eax

	call	TCP_close
	jmp	.loop

  .check_more3:
	dec	[eax + TCP_SOCKET.timer_timed_wait]
	jnz	.check_more5

	DEBUGF	1,"socket %x: 2MSL timer expired\n", eax

  .check_more5:
	dec	[eax + TCP_SOCKET.timer_persist]
	jnz	.loop

	DEBUGF	1,"socket %x: persist timer expired\n", eax

	jmp	.loop
  .exit:

}