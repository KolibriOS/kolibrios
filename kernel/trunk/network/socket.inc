;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  SOCKET.INC                                                  ;;
;;                                                              ;;
;;  Sockets constants, structures and functions                 ;;
;;                                                              ;;
;;  Last revision: 11.11.2006                                   ;;
;;                                                              ;;
;;  This file contains the following:                           ;;
;;    is_localport_unused                                       ;;
;;    get_free_socket                                           ;;
;;    socket_open                                               ;;
;;    socket_open_tcp                                           ;;
;;    socket_close                                              ;;
;;    socket_close_tcp                                          ;;
;;    socket_poll                                               ;;
;;    socket_status                                             ;;
;;    socket_read                                               ;;
;;    socket_write                                              ;;
;;    socket_write_tcp                                          ;;
;;                                                              ;;
;;                                                              ;;
;;  Changes history:                                            ;;
;;   22.09.2003 - [Mike Hibbett] : mikeh@oceanfree.net          ;;
;;   11.11.2006 - [Johnny_B] and [smb]                          ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


;
;  Socket Descriptor + Buffer
;
;    0                   1                   2                   3
;    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
;
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;  0|                    Status ( of this buffer )                  |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;  4|  Application Process ID                                       |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;  8|                  Local IP Address                             |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 12| Local IP Port                 | Unused ( set to 0 )           |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 16|                  Remote IP Address                            |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 20| Remote IP Port                | Unused ( set to 0 )           |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 24|   Rx Data Count                                   INTEL format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 28|                 TCB STATE                         INTEL format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 32|   TCB Timer (seconds)                             INTEL format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 36| ISS (Inital Sequence # used by this connection )   INET format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 40| IRS ( Inital Receive Sequence # )                  INET format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 44| SND.UNA  Seq # of unack'ed sent packets            INET format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 48| SND.NXT  Next send seq # to use                    INET format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 52| SND.WND  Send window                               INET format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 56| RCV.NXT  Next expected receive sequence #          INET format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 60| RCV.WND  Receive window                            INET format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 64| SEG.LEN  Segment length                           INTEL format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 68| SEG.WND  Segment window                           INTEL format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 72| Retransmit queue # NOW WINDOW SIZE TIMER          INTEL format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 76|       RX Data Buffer                                          |
;   +-+-+-..........                                               -+


; so, define struct
struc SOCKET
{
   .PrevPtr		    dd	 ?
   .NextPtr		    dd	 ?
   .Status		    dd	 ?  ;+00 - Status ( of this buffer )
   .PID 		    dd	 ?  ;+04 - Application Process ID
   .LocalIP		    dd	 ?  ;+08 - Local IP Address
   .LocalPort		    dw	 ?  ;+12 - Local Port
   .RemoteIP		    dd	 ?  ;+16 - Remote IP Address
   .RemotePort		    dw	 ?  ;+20 - Remote Port
   .OrigRemoteIP	    dd	 ?
   .OrigRemotePort	    dw	 ?
   .rxDataCount 	    dd	 ?  ;+24 - Rx Data Count
   .TCBState		    dd	 ?  ;+28 - TCB STATE
   .TCBTimer		    dd	 ?  ;+32 - TCB Timer (seconds)
   .ISS 		    dd	 ?  ;+36 - Initial Send Sequence
   .IRS 		    dd	 ?  ;+40 - Initial Receive Sequence
   .SND_UNA		    dd	 ?  ;+44 - Sequence number of unack'ed sent packets
   .SND_NXT		    dd	 ?  ;+48 - Next send sequence number to use
   .SND_WND		    dd	 ?  ;+52 - Send window
   .RCV_NXT		    dd	 ?  ;+56 - Next receive sequence number to use
   .RCV_WND		    dd	 ?  ;+60 - Receive window
   .SEG_LEN		    dd	 ?  ;+64 - Segment length
   .SEG_WND		    dd	 ?  ;+68 - Segment window
   .wndsizeTimer	    dd	 ?  ;+72 - Retransmit queue # NOW WINDOW SIZE TIMER
   .rxData		    dd	 ?  ;+76 - receive data buffer here
}

virtual at 0
  SOCKET SOCKET
end virtual

; simple macro calcing real memory address of SOCKET struct by socket's
;macro   Index2RealAddr reg
;{
;        shl     reg, 12
;    add     reg, sockets
;}

;Constants
; current socket statuses
SOCK_EMPTY	   =	 0	  ; socket not in use
SOCK_OPEN	   =	 1	  ; open issued, but no data sent

; TCP opening modes
SOCKET_PASSIVE	   equ	   0
SOCKET_ACTIVE	   equ	   1

proc net_socket_alloc stdcall uses ebx ecx edx edi
	stdcall kernel_alloc, SOCKETBUFFSIZE
	DEBUGF	1, "K : net_socket_alloc (0x%x)\n", eax
	or	eax, eax
	jz	.exit

	push	eax
	mov	edi, eax
	mov	ecx, SOCKETBUFFSIZE / 4
	cld
	xor	eax, eax
	rep	stosd
	pop	eax

	mov	ebx, net_sockets
	push	[ebx + SOCKET.NextPtr]
	mov	[ebx + SOCKET.NextPtr], eax
	mov	[eax + SOCKET.PrevPtr], ebx
	pop	ebx
	mov	[eax + SOCKET.NextPtr], ebx
	or	ebx, ebx
	jz	@f
	mov	[ebx + SOCKET.PrevPtr], eax

    @@: mov	ebx, [TASK_BASE]
	mov	ebx, [ebx + TASKDATA.pid]
	mov	[eax + SOCKET.PID], ebx

  .exit:
	ret
endp

proc net_socket_free stdcall uses ebx ecx edx, sock:DWORD
	mov	eax, [sock]
	DEBUGF	1, "K : net_socket_free (0x%x)\n", eax
	or	eax, eax
	jz	.error

	mov	ebx, net_sockets
	mov	ecx, [TASK_BASE]
	mov	ecx, [ecx + TASKDATA.pid]
  .next_socket:
	mov	ebx, [ebx + SOCKET.NextPtr]
	or	ebx, ebx
	jz	.error
	cmp	ebx, eax
	jne	.next_socket
	;cmp     [ebx + SOCKET.PID], ecx
	;jne     .next_socket

	mov	ebx, [eax + SOCKET.NextPtr]
	mov	eax, [eax + SOCKET.PrevPtr]
	mov	[eax + SOCKET.NextPtr], ebx
	or	ebx, ebx
	jz	@f
	mov	[ebx + SOCKET.PrevPtr], eax

    @@: stdcall kernel_free, [sock]
	ret

  .error:
	DEBUGF	1, "K :   failed\n"
	ret
endp

proc net_socket_num_to_addr stdcall uses ebx ecx, x:DWORD
; FIXME: do real transform
	mov	eax, [x]
	mov	ebx, net_sockets
	mov	ecx, [TASK_BASE]
	mov	ecx, [ecx + TASKDATA.pid]
  .next_socket:
	mov	ebx, [ebx + SOCKET.NextPtr]
	or	ebx, ebx
	jz	.error
	cmp	ebx, eax
	jne	.next_socket
	;cmp     [ebx + SOCKET.PID], ecx
	;jne     .next_socket
	ret

  .error:
	xor	eax, eax
	ret
endp

proc net_socket_addr_to_num stdcall uses ebx ecx, x:DWORD
; FIXME: do real transform
	mov	eax, [x]
	mov	ebx, net_sockets
	mov	ecx, [TASK_BASE]
	mov	ecx, [ecx + TASKDATA.pid]
  .next_socket:
	mov	ebx, [ebx + SOCKET.NextPtr]
	or	ebx, ebx
	jz	.error
	cmp	ebx, eax
	jne	.next_socket
	;cmp     [ebx + SOCKET.PID], ecx
	;jne     .next_socket
	ret

  .error:
	xor	eax, eax
	ret
endp

;***************************************************************************
;   Function
;      is_localport_unused
;
;   Description
;         scans through all the active sockets , looking to see if the
;      port number specified in bx is in use as a localport number.
;      This is useful when you want a to generate a unique local port
;      number.
;          On return, eax = 1 for free, 0 for in use
;
;***************************************************************************
proc is_localport_unused stdcall

	xchg	bl, bh

	xor	eax, eax			; Assume the return value is 'free'
	inc	al
	mov	edx, net_sockets

  .next_socket:
	mov	edx, [edx + SOCKET.NextPtr]
	or	edx, edx
	jz	.exit
	cmp	[edx + SOCKET.LocalPort], bx
	jne	.next_socket			; Return back if the port is not occupied

	dec	al				; return 'in use'

  .exit:
	ret
endp


;***************************************************************************
;   Function
;      socket_open
;
;   Description
;       find a free socket
;       local port in ebx
;       remote port in ecx
;       remote ip in edx
;       return socket # in eax, -1 if none available
;
;***************************************************************************
proc socket_open stdcall
	call	net_socket_alloc
	or	eax, eax
	jz	.error

	DEBUGF	1, "K : socket_open (0x%x)\n", eax

	push	eax

	mov	[eax + SOCKET.Status], SOCK_OPEN
	xchg	bh, bl
	mov	[eax + SOCKET.LocalPort], bx
	xchg	ch, cl
	mov	[eax + SOCKET.RemotePort], cx
	mov	ebx, [stack_ip]
	mov	[eax + SOCKET.LocalIP], ebx
	mov	[eax + SOCKET.RemoteIP], edx

	;pop     eax      ; Get the socket number back, so we can return it
	stdcall net_socket_addr_to_num
	ret

  .error:
	DEBUGF	1, "K : socket_open (fail)\n"
	or	eax, -1
	ret
endp


;***************************************************************************
;   Function
;      socket_open_tcp
;
;   Description
;       Opens a TCP socket in PASSIVE or ACTIVE mode
;       find a free socket
;       local port in ebx ( intel format )
;       remote port in ecx ( intel format )
;       remote ip in edx ( in Internet byte order )
;       Socket open mode in esi  ( SOCKET_PASSIVE or SOCKET_ACTIVE )
;       return socket # in eax, -1 if none available
;
;***************************************************************************
proc socket_open_tcp stdcall
local sockAddr dd ?

	cmp	esi, SOCKET_PASSIVE
	jne	.skip_port_check

	push	ebx
	mov	eax, ebx
	xchg	al, ah
	mov	ebx, net_sockets

  .next_socket:
	mov	ebx, [ebx + SOCKET.NextPtr]
	or	ebx, ebx
	jz	.last_socket
	cmp	[ebx + SOCKET.TCBState], TCB_LISTEN
	jne	.next_socket
	cmp	[ebx + SOCKET.LocalPort], ax
	jne	.next_socket

	xchg	al, ah
	DEBUGF	1, "K : error: port %u is listened by 0x%x\n", ax, ebx
	pop	ebx
	jmp	.error

  .last_socket:
	pop	ebx

  .skip_port_check:
	call	net_socket_alloc
	or	eax, eax
	jz	.error

	DEBUGF	1, "K : socket_open_tcp (0x%x)\n", eax

	mov	[sockAddr], eax

	; TODO - check this works!
	;xxx: already 0 (intialized by net_socket_alloc)
	;mov     [eax + SOCKET.wndsizeTimer], 0     ; Reset the window timer.

	xchg	bh, bl
	mov	[eax + SOCKET.LocalPort], bx
	xchg	ch, cl
	mov	[eax + SOCKET.RemotePort], cx
	mov	[eax + SOCKET.OrigRemotePort], cx
	mov	ebx, [stack_ip]
	mov	[eax + SOCKET.LocalIP], ebx
	mov	[eax + SOCKET.RemoteIP], edx
	mov	[eax + SOCKET.OrigRemoteIP], edx

	mov	ebx, TCB_LISTEN
	cmp	esi, SOCKET_PASSIVE
	je	@f
	mov	ebx, TCB_SYN_SENT
    @@: mov	[eax + SOCKET.TCBState], ebx		; Indicate the state of the TCB

	cmp	ebx, TCB_LISTEN
	je	.exit

	; Now, if we are in active mode, then we have to send a SYN to the specified remote port
	mov	eax, EMPTY_QUEUE
	call	dequeue
	cmp	ax, NO_BUFFER
	je	.exit

	push	eax

	mov	bl, TH_SYN
	xor	ecx, ecx
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

	; increment SND.NXT in socket
	add	esi, SOCKET.SND_NXT
	call	inc_inet_esi

  .exit:
	mov	ebx, [sockAddr]
	mov	[ebx + SOCKET.Status], SOCK_OPEN
	;pop     eax      ; Get the socket number back, so we can return it
	stdcall net_socket_addr_to_num, ebx
	ret

  .error:
	DEBUGF	1, "K : socket_open_tcp (fail)\n"
	or	eax, -1
	ret
endp


;***************************************************************************
;   Function
;      socket_close
;
;   Description
;       socket # in ebx
;       returns 0 for ok, -1 for socket not open (fail)
;
;***************************************************************************
proc socket_close stdcall
	DEBUGF	1, "K : socket_close (0x%x)\n", ebx
	stdcall net_socket_num_to_addr, ebx
	or	eax, eax
	jz	.error

	cmp	[eax + SOCKET.Status], dword SOCK_EMPTY
	jz	.error

	; Clear the socket varaibles
	stdcall net_socket_free, eax
;        mov     edi, eax
;        xor     eax, eax
;        mov     ecx, SOCKETHEADERSIZE
;        cld
;        rep     stosb

	xor	eax, eax
	ret

  .error:
	DEBUGF	1, "K : socket_close (fail)\n"
	or	eax, -1
	ret
endp


;***************************************************************************
;   Function
;      socket_close_tcp
;
;   Description
;       socket # in ebx
;       returns 0 for ok, -1 for socket not open (fail)
;
;***************************************************************************
proc socket_close_tcp stdcall
local sockAddr dd ?
	DEBUGF	1, "K : socket_close_tcp (0x%x)\n", ebx
	; first, remove any resend entries
	pusha

	mov	esi, resendQ
	mov	ecx, 0

  .next_resendq:
	cmp	ecx, NUMRESENDENTRIES
	je	.last_resendq	    ; None left
	;cmp     [esi], bl                               ; XTODO: bl -> ebx
	cmp	[esi + 4], ebx
	je	@f		    ; found one
	inc	ecx
	add	esi, 8
	jmp	.next_resendq

    ;@@: mov     byte[esi], 0xff                         ; XTODO: 0xff -> 0
    @@: mov	dword[esi + 4], 0
	inc	ecx
	add	esi, 8
	jmp	.next_resendq

  .last_resendq:
	popa

	stdcall net_socket_num_to_addr, ebx
	or	eax, eax
	jz	.error

	mov	ebx, eax
	mov	[sockAddr], eax
	cmp	[ebx + SOCKET.Status], SOCK_EMPTY
	je	.error

	cmp	[ebx + SOCKET.TCBState], TCB_LISTEN		;xxx
	je	.destroy_tcb					;xxx
	cmp	[ebx + SOCKET.TCBState], TCB_SYN_SENT		;xxx
	je	.destroy_tcb					;xxx

	; Now construct the response, and queue for sending by IP
	mov	eax, EMPTY_QUEUE
	call	dequeue
	cmp	ax, NO_BUFFER
	je	.error

	push	eax

;xxx    mov     bl, TH_FIN + TH_ACK
	mov	bl, TH_FIN					;xxx
	xor	ecx, ecx
	xor	esi, esi
	stdcall build_tcp_packet, [sockAddr]

	mov	 ebx, [sockAddr]
	; increament SND.NXT in socket
	lea	esi, [ebx + SOCKET.SND_NXT]
	call	inc_inet_esi

	; Get the socket state
	mov	eax, [ebx + SOCKET.TCBState]
;xxx    cmp     eax, TCB_LISTEN
;xxx    je      .destroy_tcb
;xxx    cmp     eax, TCB_SYN_SENT
;xxx    je      .destroy_tcb
	cmp	eax, TCB_SYN_RECEIVED
	je	.fin_wait_1
	cmp	eax, TCB_ESTABLISHED
	je	.fin_wait_1

	; assume CLOSE WAIT
	; Send a fin, then enter last-ack state
	; TODO: check if it's really a TCB_CLOSE_WAIT
	mov	[ebx + SOCKET.TCBState], TCB_LAST_ACK
	jmp	.send

  .fin_wait_1:
	; Send a fin, then enter finwait2 state
	mov	[ebx + SOCKET.TCBState], TCB_FIN_WAIT_1

  .send:
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
	jmp	.exit

  .destroy_tcb:
;xxx    pop     eax

	; Clear the socket variables
;xxx    stdcall net_socket_free, [sockAddr]
	stdcall net_socket_free, ebx

  .exit:
	xor	eax, eax
	ret

  .error:
	DEBUGF	1, "K : socket_close_tcp (fail)\n"
	or	eax, -1
	ret
endp


;***************************************************************************
;   Function
;      socket_poll
;
;   Description
;       socket # in ebx
;       returns count in eax.
;
;***************************************************************************
proc socket_poll stdcall
;        DEBUGF  1, "socket_poll(0x%x)\n", ebx
	stdcall net_socket_num_to_addr, ebx
	or	eax, eax
	jz	.error

	mov	eax, [eax + SOCKET.rxDataCount]
	ret

  .error:
	;or      eax, -1
	xor	eax, eax
	ret
endp


;***************************************************************************
;   Function
;      socket_status
;
;   Description
;       socket # in ebx
;       returns TCB state in eax.
;
;***************************************************************************
proc socket_status stdcall
;;       DEBUGF  1, "socket_status(0x%x)\n", ebx
	stdcall net_socket_num_to_addr, ebx
	or	eax, eax
	jz	.error

	mov	eax, [eax + SOCKET.TCBState]
	ret

  .error:
	;or      eax, -1
	xor	eax, eax
	ret
endp

;    Index2RealAddr ebx
;    mov     eax, [ebx + SOCKET.TCBState]
;
;    ret


;***************************************************************************
;   Function
;      socket_read
;
;   Description
;       socket # in ebx
;       returns # of bytes remaining in eax, data in bl
;
;***************************************************************************
proc socket_read stdcall
;        DEBUGF  1, "socket_read(0x%x)\n", ebx
	stdcall net_socket_num_to_addr, ebx
	or	eax, eax
	jz	.error

	mov	ebx, eax
	mov	eax, [ebx + SOCKET.rxDataCount] 	; get count of bytes
	test	eax, eax
	jz	.error

	dec	eax
	mov	esi, ebx	    ; esi is address of socket
	mov	[ebx + SOCKET.rxDataCount], eax 	; store new count
	;movzx   ebx, byte[ebx + SOCKET.rxData]  ; get the byte
	movzx	ebx, byte[ebx + SOCKETHEADERSIZE]  ; get the byte
	add	esi, SOCKETHEADERSIZE
	mov	edi, esi
	inc	esi

	mov	ecx, (SOCKETBUFFSIZE - SOCKETHEADERSIZE) / 4
	lea	edi, [ebx + SOCKETHEADERSIZE]
	lea	esi, [edi + 1]
	cld
	rep	movsd

	ret

  .error:
	;or      eax, -1
	xor	eax, eax
	xor	ebx, ebx
	ret
endp


;***************************************************************************
;   Function
;      socket_read_packet
;
;   Description
;       socket # in ebx
;       datapointer # in ecx
;       buffer size in edx
;       returns # of bytes copied in eax
;
;***************************************************************************
proc socket_read_packet stdcall
;        DEBUGF  1, "socket_read_packet(0x%x)\n", ebx
	stdcall net_socket_num_to_addr, ebx		   ; get real socket address
	or	eax, eax
	jz	.error

	mov	ebx, eax
	mov	eax, [ebx + SOCKET.rxDataCount] 	   ; get count of bytes
	test	eax, eax				   ; if count of bytes is zero..
	jz	.exit					   ; exit function (eax will be zero)

	test	edx, edx				   ; if buffer size is zero, copy all data
	jz	.copy_all_bytes
	cmp	edx, eax				   ; if buffer size is larger then the bytes of data, copy all data
	jge	.copy_all_bytes

	sub	eax, edx				   ; store new count (data bytes in buffer - bytes we're about to copy)
	mov	[ebx + SOCKET.rxDataCount], eax 	   ;
	push	eax
	mov	eax, edx				   ; number of bytes we want to copy must be in eax
	call	.start_copy				   ; copy to the application

	mov	esi, ebx				   ; now we're going to copy the remaining bytes to the beginning
	add	esi, SOCKETHEADERSIZE			   ; we dont need to copy the header
	mov	edi, esi				   ; edi is where we're going to copy to
	add	esi, edx				   ; esi is from where we copy
	pop	ecx					   ; count of bytes we have left
	push	ecx					   ; push it again so we can re-use it later
	shr	ecx, 2					   ; divide eax by 4
	cld
	rep	movsd					   ; copy all full dwords
	pop	ecx
	and	ecx, 3
	rep	movsb					   ; copy remaining bytes

  .exit:
	ret						   ; at last, exit

  .error:
	;or      eax, -1
	xor	eax, eax
	ret

  .copy_all_bytes:
	xor	esi, esi
	mov	[ebx + SOCKET.rxDataCount], esi 	   ; store new count (zero)
	call	.start_copy
	ret

  .start_copy:
	mov	edi, ecx
	mov	esi, ebx
	add	esi, SOCKETHEADERSIZE			   ; we dont need to copy the header
	mov	ecx, eax				   ; eax is count of bytes
	push	ecx
	shr	ecx, 2					   ; divide eax by 4
	cld						   ; copy all full dwords
	rep	movsd
	pop	ecx
	and	ecx, 3
	rep	movsb					   ; copy the rest bytes
	retn						   ; exit, or go back to shift remaining bytes if any
endp


;***************************************************************************
;   Function
;      socket_write
;
;   Description
;       socket in ebx
;       # of bytes to write in ecx
;       pointer to data in edx
;       returns 0 in eax ok, -1 == failed ( invalid socket, or
;       could not queue IP packet )
;
;***************************************************************************
proc socket_write stdcall
;        DEBUGF  1, "socket_write(0x%x)\n", ebx
	stdcall net_socket_num_to_addr, ebx		   ; get real socket address
	or	eax, eax
	jz	.error

	mov	ebx, eax

	; If the socket is invalid, return with an error code
	cmp	[ebx + SOCKET.Status], SOCK_EMPTY
	je	.error

	mov	eax, EMPTY_QUEUE
	call	dequeue
	cmp	ax, NO_BUFFER
	je	.error

	; Save the queue entry number
	push	eax

	; save the pointers to the data buffer & size
	push	edx
	push	ecx

	; convert buffer pointer eax to the absolute address
	mov	ecx, IPBUFFSIZE
	mul	ecx
	add	eax, IPbuffs

	mov	edx, eax

	; So, ebx holds the socket ptr, edx holds the IPbuffer ptr

	; Fill in the IP header (some data is in the socket descriptor)
	mov	eax, [ebx + SOCKET.LocalIP]
	mov	[edx + IP_PACKET.SourceAddress], eax
	mov	eax, [ebx + SOCKET.RemoteIP]
	mov	[edx + IP_PACKET.DestinationAddress], eax

	mov	[edx + IP_PACKET.VersionAndIHL], 0x45
	mov	[edx + IP_PACKET.TypeOfService], 0

	pop	eax		      ; Get the UDP data length
	push	eax

	add	eax, 20 + 8	      ; add IP header and UDP header lengths
	xchg	al, ah
	mov	[edx + IP_PACKET.TotalLength], ax
	xor	eax, eax
	mov	[edx + IP_PACKET.Identification], ax
	mov	[edx + IP_PACKET.FlagsAndFragmentOffset], 0x0040
	mov	[edx + IP_PACKET.TimeToLive], 0x20
	mov	[edx + IP_PACKET.Protocol], PROTOCOL_UDP

	; Checksum left unfilled
	mov	[edx + IP_PACKET.HeaderChecksum], ax

	; Fill in the UDP header (some data is in the socket descriptor)
	mov	ax, [ebx + SOCKET.LocalPort]
	mov	[edx + 20 + UDP_PACKET.SourcePort], ax

	mov	ax, [ebx + SOCKET.RemotePort]
	mov	[edx + 20 + UDP_PACKET.DestinationPort], ax

	pop	eax
	push	eax

	add	eax, 8
	xchg	al, ah
	mov	[edx + 20 + UDP_PACKET.Length], ax

	; Checksum left unfilled
	xor	eax, eax
	mov	[edx + 20 + UDP_PACKET.Checksum], ax

	pop	ecx		     ; count of bytes to send
	mov	ebx, ecx	     ; need the length later
	pop	eax		     ; get callers ptr to data to send

	; Get the address of the callers data
	mov	edi, [TASK_BASE]
	add	edi, TASKDATA.mem_start
	add	eax, [edi]
	mov	esi, eax

	mov	edi, edx
	add	edi, 28
	cld
	rep	movsb		    ; copy the data across

	; we have edx as IPbuffer ptr.
	; Fill in the UDP checksum
	; First, fill in pseudoheader
	mov	eax, [edx + IP_PACKET.SourceAddress]
	mov	[pseudoHeader], eax
	mov	eax, [edx + IP_PACKET.DestinationAddress]
	mov	[pseudoHeader + 4], eax
	mov	word[pseudoHeader + 8], PROTOCOL_UDP shl 8 + 0	    ; 0 + protocol
	add	ebx, 8
	mov	eax, ebx
	xchg	al, ah
	mov	[pseudoHeader + 10], ax

	mov	eax, pseudoHeader
	mov	[checkAdd1], eax
	mov	[checkSize1], word 12
	mov	eax, edx
	add	eax, 20
	mov	[checkAdd2], eax
	mov	eax, ebx
	mov	[checkSize2], ax      ; was eax!! mjh 8/7/02

	call	checksum

	; store it in the UDP checksum ( in the correct order! )
	mov	ax, [checkResult]

	; If the UDP checksum computes to 0, we must make it 0xffff
	; (0 is reserved for 'not used')
	test	ax, ax
	jnz	@f
	mov	ax, 0xffff

    @@: xchg	al, ah
	mov	[edx + 20 + UDP_PACKET.Checksum], ax

	; Fill in the IP header checksum
	GET_IHL ecx,edx 	     ; get IP-Header length
	stdcall checksum_jb,edx,ecx  ; buf_ptr, buf_size
	xchg	al, ah
	mov	[edx + IP_PACKET.HeaderChecksum], ax

	; Check destination IP address.
	; If it is the local host IP, route it back to IP_RX

	pop	ebx

	mov	eax, NET1OUT_QUEUE
	mov	ecx, [edx + SOCKET.RemoteIP]
	mov	edx, [stack_ip]
	cmp	edx, ecx
	jne	.not_local
	mov	eax, IPIN_QUEUE

  .not_local:
	; Send it.
	call	queue

	xor	eax, eax
	ret

  .error:
	or	eax, -1
	ret
endp


;***************************************************************************
;   Function
;      socket_write_tcp
;
;   Description
;       socket in ebx
;       # of bytes to write in ecx
;       pointer to data in edx
;       returns 0 in eax ok, -1 == failed ( invalid socket, or
;       could not queue IP packet )
;
;***************************************************************************
proc socket_write_tcp stdcall
local sockAddr dd ?

;        DEBUGF  1, "socket_write_tcp(0x%x)\n", ebx
	stdcall net_socket_num_to_addr, ebx
	or	eax, eax
	jz	.error

	mov	ebx, eax
	mov	[sockAddr], ebx

	; If the socket is invalid, return with an error code
	cmp	[ebx + SOCKET.Status], SOCK_EMPTY
	je	.error

	; If the sockets window timer is nonzero, do not queue packet
	; TODO - done
	cmp	[ebx + SOCKET.wndsizeTimer], 0
	jne	.error

	mov	eax, EMPTY_QUEUE
	call	dequeue
	cmp	ax, NO_BUFFER
	je	.error

	push	eax

	; Get the address of the callers data
	mov	edi, [TASK_BASE]
	add	edi, TASKDATA.mem_start
	add	edx, [edi]
	mov	esi, edx

	pop	eax
	push	eax

	push	ecx
	mov	bl, TH_ACK
	stdcall build_tcp_packet, [sockAddr]
	pop	ecx

	; Check destination IP address.
	; If it is the local host IP, route it back to IP_RX

	pop	ebx
	push	ecx

	mov	eax, NET1OUT_QUEUE
	mov	edx, [stack_ip]
	mov	ecx, [sockAddr]
	cmp	edx, [ecx + SOCKET.RemoteIP]
	jne	.not_local
	mov	eax, IPIN_QUEUE

  .not_local:
	pop	ecx
	push	ebx		    ; save ipbuffer number

	call	queue

	mov	esi, [sockAddr]

	; increament SND.NXT in socket
	; Amount to increment by is in ecx
	add	esi, SOCKET.SND_NXT
	call	add_inet_esi

	pop	ebx

	; Copy the IP buffer to a resend queue
	; If there isn't one, dont worry about it for now
	mov	esi, resendQ
	mov	ecx, 0

  .next_resendq:
	cmp	ecx, NUMRESENDENTRIES
	je	.exit		   ; None found
	;cmp     byte[esi], 0xff                                 ; XTODO: 0xff -> 0
	cmp	dword[esi + 4], 0
	je	@f		   ; found one
	inc	ecx
	add	esi, 8
	jmp	.next_resendq

    @@: push	ebx

	; OK, we have a buffer descriptor ptr in esi.
	; resend entry # in ecx
	;  Populate it
	;  socket #
	;  retries count
	;  retry time
	;  fill IP buffer associated with this descriptor

	stdcall net_socket_addr_to_num, [sockAddr]
	;mov     [esi], al                               ; XTODO: al -> eax
	mov	[esi + 4], eax
	mov	byte[esi + 1], TCP_RETRIES
	mov	word[esi + 2], TCP_TIMEOUT

	inc	ecx
	; Now get buffer location, and copy buffer across. argh! more copying,,
	mov	edi, resendBuffer - IPBUFFSIZE

    @@: add	edi, IPBUFFSIZE
	loop	@b

	; we have dest buffer location in edi
	pop	eax
	; convert source buffer pointer eax to the absolute address
	mov	ecx, IPBUFFSIZE
	mul	ecx
	add	eax, IPbuffs
	mov	esi, eax

	; do copy
	mov	ecx, IPBUFFSIZE
	cld
	rep	movsb

  .exit:
	xor	eax, eax
	ret

  .error:
	or	eax, -1
	ret
endp
