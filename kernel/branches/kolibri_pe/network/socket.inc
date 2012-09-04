;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  SOCKET.INC                                                  ;;
;;                                                              ;;
;;  Sockets constants, structures and functions                 ;;
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

; socket data structure
struct SOCKET
  .PrevPtr	  dd ? ; pointer to previous socket in list
  .NextPtr	  dd ? ; pointer to next socket in list
  .Number	  dd ? ; socket number (unique within single process)
  .PID		  dd ? ; application process id
  .LocalIP	  dd ? ; local IP address
  .LocalPort	  dw ? ; local port
  .RemoteIP	  dd ? ; remote IP address
  .RemotePort	  dw ? ; remote port
  .OrigRemoteIP   dd ? ; original remote IP address (used to reset to LISTEN state)
  .OrigRemotePort dw ? ; original remote port (used to reset to LISTEN state)
  .rxDataCount	  dd ? ; rx data count
  .TCBState	  dd ? ; TCB state
  .TCBTimer	  dd ? ; TCB timer (seconds)
  .ISS		  dd ? ; initial send sequence
  .IRS		  dd ? ; initial receive sequence
  .SND_UNA	  dd ? ; sequence number of unack'ed sent packets
  .SND_NXT	  dd ? ; bext send sequence number to use
  .SND_WND	  dd ? ; send window
  .RCV_NXT	  dd ? ; next receive sequence number to use
  .RCV_WND	  dd ? ; receive window
  .SEG_LEN	  dd ? ; segment length
  .SEG_WND	  dd ? ; segment window
  .wndsizeTimer   dd ? ; window size timer
  .lock           dd ? ; lock mutex
  .rxData	  dd ? ; receive data buffer here
ends

; TCP opening modes
SOCKET_PASSIVE = 0
SOCKET_ACTIVE  = 1

; socket types
SOCK_STREAM = 1
SOCK_DGRAM  = 2

;; Allocate memory for socket data and put new socket into the list
; Newly created socket is initialized with calling PID and number and
; put into beginning of list (which is a fastest way).
;
; @return socket structure address in EAX
;;
proc net_socket_alloc stdcall uses ebx ecx edx edi
    mov ecx, SOCKETBUFFSIZE
    mov edx, PG_SW
    call @mem_alloc@8
	DEBUGF	1, "K : net_socket_alloc (0x%x)\n", eax
	; check if we can allocate needed amount of memory
	or	eax, eax
	jz	.exit

	; zero-initialize allocated memory
	push	eax
	mov	edi, eax
	mov	ecx, SOCKETBUFFSIZE / 4
	cld
	xor	eax, eax
	rep	stosd
	pop	eax

	; add socket to the list by changing pointers
	mov	ebx, net_sockets
	push	[ebx + SOCKET.NextPtr]
	mov	[ebx + SOCKET.NextPtr], eax
	mov	[eax + SOCKET.PrevPtr], ebx
	pop	ebx
	mov	[eax + SOCKET.NextPtr], ebx
	or	ebx, ebx
	jz	@f
	mov	[ebx + SOCKET.PrevPtr], eax

    @@: ; set socket owner PID to the one of calling process
	mov	ebx, [TASK_BASE]
	mov	ebx, [ebx + TASKDATA.pid]
	mov	[eax + SOCKET.PID], ebx

	; find first free socket number and use it
	;mov     edx, ebx
	mov	ebx, net_sockets
	xor	ecx, ecx
  .next_socket_number:
	inc	ecx
  .next_socket:
	mov	ebx, [ebx + SOCKET.NextPtr]
	or	ebx, ebx
	jz	.last_socket_number
	cmp	[ebx + SOCKET.Number], ecx
	jne	.next_socket
	;cmp     [ebx + SOCKET.PID], edx
	;jne     .next_socket
	mov	ebx, net_sockets
	jmp	.next_socket_number

  .last_socket_number:
	mov	[eax + SOCKET.Number], ecx

  .exit:
	ret
endp

;; Free socket data memory and pop socket off the list
;
; @param sockAddr is a socket structure address
;;
proc net_socket_free stdcall uses ebx ecx edx, sockAddr:DWORD
	mov	eax, [sockAddr]
	DEBUGF	1, "K : net_socket_free (0x%x)\n", eax
	; check if we got something similar to socket structure address
	or	eax, eax
	jz	.error

	; make sure sockAddr is one of the socket addresses in the list
	mov	ebx, net_sockets
	;mov     ecx, [TASK_BASE]
	;mov     ecx, [ecx + TASKDATA.pid]
  .next_socket:
	mov	ebx, [ebx + SOCKET.NextPtr]
	or	ebx, ebx
	jz	.error
	cmp	ebx, eax
	jne	.next_socket
	;cmp     [ebx + SOCKET.PID], ecx
	;jne     .next_socket

	; okay, we found the correct one
	; remove it from the list first, changing pointers
	mov	ebx, [eax + SOCKET.NextPtr]
	mov	eax, [eax + SOCKET.PrevPtr]
	mov	[eax + SOCKET.NextPtr], ebx
	or	ebx, ebx
	jz	@f
	mov	[ebx + SOCKET.PrevPtr], eax

@@:
    mov ecx, [sock]
    call @mem_free@4
	ret

  .error:
	DEBUGF	1, "K :   failed\n"
	ret
endp

;; Get socket structure address by its number
; Scan through sockets list to find the socket with specified number.
; This proc uses SOCKET.PID indirectly to check if socket is owned by
; calling process.
;
; @param sockNum is a socket number
; @return socket structure address or 0 (not found) in EAX
;;
proc net_socket_num_to_addr stdcall uses ebx ecx, sockNum:DWORD
	mov	eax, [sockNum]
	; check if we got something similar to socket number
	or	eax, eax
	jz	.error

	; scan through sockets list
	mov	ebx, net_sockets
	;mov     ecx, [TASK_BASE]
	;mov     ecx, [ecx + TASKDATA.pid]
  .next_socket:
	mov	ebx, [ebx + SOCKET.NextPtr]
	or	ebx, ebx
	jz	.error
	cmp	[ebx + SOCKET.Number], eax
	jne	.next_socket
	;cmp     [ebx + SOCKET.PID], ecx
	;jne     .next_socket

	; okay, we found the correct one
	mov	eax, ebx
	ret

  .error:
	xor	eax, eax
	ret
endp

;; Get socket number by its structure address
; Scan through sockets list to find the socket with specified address.
; This proc uses SOCKET.PID indirectly to check if socket is owned by
; calling process.
;
; @param sockAddr is a socket structure address
; @return socket number (SOCKET.Number) or 0 (not found) in EAX
;;
proc net_socket_addr_to_num stdcall uses ebx ecx, sockAddr:DWORD
	mov	eax, [sockAddr]
	; check if we got something similar to socket structure address
	or	eax, eax
	jz	.error

	; scan through sockets list
	mov	ebx, net_sockets
	;mov     ecx, [TASK_BASE]
	;mov     ecx, [ecx + TASKDATA.pid]
  .next_socket:
	mov	ebx, [ebx + SOCKET.NextPtr]
	or	ebx, ebx
	jz	.error
	cmp	ebx, eax
	jne	.next_socket
	;cmp     [ebx + SOCKET.PID], ecx
	;jne     .next_socket

	; okay, we found the correct one
	mov	eax, [ebx + SOCKET.Number]
	ret

  .error:
	xor	eax, eax
	ret
endp

;; [53.9] Check if local port is used by any socket in the system.
; Scan through sockets list, checking SOCKET.LocalPort.
; Useful when you want a to generate a unique local port number.
; This proc doesn't guarantee that after calling it and trying to use
; the port reported being free in calls to socket_open/socket_open_tcp it'll
; still be free or otherwise it'll still be used if reported being in use.
;
; @param BX is a port number
; @return 1 (port is free) or 0 (port is in use) in EAX
;;
proc is_localport_unused stdcall

	xchg	bl, bh

	; assume the return value is 'free'
	xor	eax, eax
	inc	al
	mov	edx, net_sockets

  .next_socket:
	mov	edx, [edx + SOCKET.NextPtr]
	or	edx, edx
	jz	.exit
	cmp	[edx + SOCKET.LocalPort], bx
	jne	.next_socket

	; return 'in use'
	dec	al

  .exit:
    ret
endp

;; [53.0] Open DGRAM socket (connectionless, unreliable)
;
; @param BX is local port number
; @param CX is remote port number
; @param EDX is remote IP address
; @return socket number or -1 (error) in EAX
;;
proc socket_open stdcall
	call	net_socket_alloc
	or	eax, eax
	jz	.error

	DEBUGF	1, "K : socket_open (0x%x)\n", eax

    push    eax

    xchg    bh, bl
    mov     [eax + SOCKET.LocalPort], bx
    xchg    ch, cl
    mov     [eax + SOCKET.RemotePort], cx

    mov     ebx, [stack_ip]
    mov     [eax + SOCKET.LocalIP], ebx
    mov     [eax + SOCKET.RemoteIP], edx

	;pop     eax      ; Get the socket number back, so we can return it
	stdcall net_socket_addr_to_num
    ret

  .error:
	DEBUGF	1, "K : socket_open (fail)\n"
	or	eax, -1
    ret
endp

;; [53.5] Open STREAM socket (connection-based, sequenced, reliable, two-way)
;
; @param BX is local port number
; @param CX is remote port number
; @param EDX is remote IP address
; @param ESI is open mode (SOCKET_ACTIVE, SOCKET_PASSIVE)
; @return socket number or -1 (error) in EAX
;;
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
	;mov     [eax + SOCKET.wndsizeTimer], 0     ; Reset the window timer.

	xchg	bh, bl
	mov	[eax + SOCKET.LocalPort], bx
	xchg	ch, cl
	mov	[eax + SOCKET.RemotePort], cx
	mov	[eax + SOCKET.OrigRemotePort], cx
    mov     ebx, [stack_ip]
    mov     [eax + SOCKET.LocalIP], ebx
    mov     [eax + SOCKET.RemoteIP], edx
	mov	[eax + SOCKET.OrigRemoteIP], edx

    mov     ebx, TCB_LISTEN
    cmp     esi, SOCKET_PASSIVE
	je	@f
    mov     ebx, TCB_SYN_SENT
    @@: mov	[eax + SOCKET.TCBState], ebx		; Indicate the state of the TCB

    cmp     ebx, TCB_LISTEN
	je	.exit

    ; Now, if we are in active mode, then we have to send a SYN to the specified remote port
    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
	je	.exit

    push    eax

	mov	bl, TH_SYN
	xor	ecx, ecx
	stdcall build_tcp_packet, [sockAddr]

    mov     eax, NET1OUT_QUEUE

    mov     edx, [stack_ip]
	mov	ecx, [sockAddr]
	cmp	edx, [ecx + SOCKET.RemoteIP]
	jne	.not_local
    mov     eax, IPIN_QUEUE

  .not_local:
       ; Send it.
    pop     ebx
    call    queue

	mov	esi, [sockAddr]

    ; increment SND.NXT in socket
	add	esi, SOCKET.SND_NXT
    call    inc_inet_esi

  .exit:
	; Get the socket number back, so we can return it
	stdcall net_socket_addr_to_num, [sockAddr]
    ret

  .error:
	DEBUGF	1, "K : socket_open_tcp (fail)\n"
	or	eax, -1
    ret
endp

;; [53.1] Close DGRAM socket
;
; @param EBX is socket number
; @return 0 (closed successfully) or -1 (error) in EAX
;;
proc socket_close stdcall
	DEBUGF	1, "K : socket_close (0x%x)\n", ebx
	stdcall net_socket_num_to_addr, ebx
	or	eax, eax
	jz	.error

	stdcall net_socket_free, eax

    xor     eax, eax
    ret

  .error:
	DEBUGF	1, "K : socket_close (fail)\n"
	or	eax, -1
    ret
endp

;; [53.8] Close STREAM socket
; Closing TCP sockets takes time, so when you get successful return code
; from this function doesn't always mean that socket is actually closed.
;
; @param EBX is socket number
; @return 0 (closed successfully) or -1 (error) in EAX
;;
proc socket_close_tcp stdcall
local sockAddr dd ?
	DEBUGF	1, "K : socket_close_tcp (0x%x)\n", ebx
    ; first, remove any resend entries
    pusha

    mov     esi, resendQ
    mov     ecx, 0

  .next_resendq:
    cmp     ecx, NUMRESENDENTRIES
	je	.last_resendq	    ; None left
	cmp	[esi + 4], ebx
	je	@f		    ; found one
    inc     ecx
	add	esi, 8
	jmp	.next_resendq

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

	cmp	[ebx + SOCKET.TCBState], TCB_LISTEN
	je	.destroy_tcb
	cmp	[ebx + SOCKET.TCBState], TCB_SYN_SENT
	je	.destroy_tcb

    ; Now construct the response, and queue for sending by IP
    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
	je	.error

    push    eax

	mov	bl, TH_FIN
	xor	ecx, ecx
	xor	esi, esi
	stdcall build_tcp_packet, [sockAddr]

	mov	 ebx, [sockAddr]
    ; increament SND.NXT in socket
	lea	esi, [ebx + SOCKET.SND_NXT]
    call    inc_inet_esi


    ; Get the socket state
    mov     eax, [ebx + SOCKET.TCBState]
    cmp     eax, TCB_SYN_RECEIVED
	je	.fin_wait_1
    cmp     eax, TCB_ESTABLISHED
	je	.fin_wait_1

    ; assume CLOSE WAIT
    ; Send a fin, then enter last-ack state
	mov	[ebx + SOCKET.TCBState], TCB_LAST_ACK
	jmp	.send

  .fin_wait_1:
    ; Send a fin, then enter finwait2 state
	mov	[ebx + SOCKET.TCBState], TCB_FIN_WAIT_1

  .send:
    mov     eax, NET1OUT_QUEUE

    mov     edx, [stack_ip]
	mov	ecx, [sockAddr]
	cmp	edx, [ecx + SOCKET.RemoteIP]
	jne	.not_local
    mov     eax, IPIN_QUEUE

  .not_local:
       ; Send it.
    pop     ebx
    call    queue
	jmp	.exit

  .destroy_tcb:

	; Clear the socket variables
	stdcall net_socket_free, ebx

  .exit:
    xor     eax, eax
    ret

  .error:
	DEBUGF	1, "K : socket_close_tcp (fail)\n"
	or	eax, -1
    ret
endp

;; [53.2] Poll socket
;
; @param EBX is socket number
; @return count or bytes in rx buffer or 0 (error) in EAX
;;
proc socket_poll stdcall
;        DEBUGF  1, "socket_poll(0x%x)\n", ebx
	stdcall net_socket_num_to_addr, ebx
	or	eax, eax
	jz	.error

	mov	eax, [eax + SOCKET.rxDataCount]
    ret

  .error:
	xor	eax, eax
	ret
endp

;; [53.6] Get socket TCB state
;
; @param EBX is socket number
; @return socket TCB state or 0 (error) in EAX
;;
proc socket_status stdcall
;;       DEBUGF  1, "socket_status(0x%x)\n", ebx
	stdcall net_socket_num_to_addr, ebx
	or	eax, eax
	jz	.error

	mov	eax, [eax + SOCKET.TCBState]
    ret

  .error:
	xor	eax, eax
    ret
endp

;; [53.3] Get one byte from rx buffer
; This function can return 0 in two cases: if there's one byte read and
; non left, and if an error occured. Behavior should be changed and function
; shouldn't be used for now. Consider using [53.11] instead.
;
; @param EBX is socket number
; @return number of bytes left in rx buffer or 0 (error) in EAX
; @return byte read in BL
;;
proc socket_read stdcall
;        DEBUGF  1, "socket_read(0x%x)\n", ebx
	stdcall net_socket_num_to_addr, ebx
	or	eax, eax
	jz	.error

	lea	ebx, [eax + SOCKET.lock]
	call	wait_mutex

	mov	ebx, eax
    mov     eax, [ebx + SOCKET.rxDataCount]	    ; get count of bytes
    test    eax, eax
	jz	.error_release

    dec     eax
    mov     esi, ebx		; esi is address of socket
    mov     [ebx + SOCKET.rxDataCount], eax	    ; store new count
	movzx	eax, byte[ebx + SOCKET.rxData]		; get the byte

	mov	ecx, SOCKETBUFFSIZE - SOCKET.rxData - 1
	lea	edi, [esi + SOCKET.rxData]
	lea	esi, [edi + 1]
    cld
	push	ecx
	shr	ecx, 2
    rep     movsd
	pop	ecx
	and	ecx, 3
	rep	movsb

	mov	[ebx + SOCKET.lock], 0
	mov	ebx, eax

	ret

  .error_release:
	mov	[ebx + SOCKET.lock], 0
  .error:
	xor	ebx, ebx
    ret
endp

;; [53.11] Get specified number of bytes from rx buffer
; Number of bytes in rx buffer can be less than requested size. In this case,
; only available number of bytes is read.
; This function can return 0 in two cases: if there's no data to read, and if
; an error occured. Behavior should be changed.
;
; @param EBX is socket number
; @param ECX is pointer to application buffer
; @param EDX is application buffer size (number of bytes to read)
; @return number of bytes read or 0 (error) in EAX
;;
proc socket_read_packet stdcall
;        DEBUGF  1, "socket_read_packet(0x%x)\n", ebx
	stdcall net_socket_num_to_addr, ebx		   ; get real socket address
	or	eax, eax
	jz	.error

	lea	ebx, [eax + SOCKET.lock]
	call	wait_mutex

	mov	ebx, eax
    mov     eax, [ebx + SOCKET.rxDataCount]		   ; get count of bytes
    test    eax, eax					   ; if count of bytes is zero..
    jz	    .exit					   ; exit function (eax will be zero)

    test    edx, edx					   ; if buffer size is zero, copy all data
	jz	.copy_all_bytes
    cmp     edx, eax					   ; if buffer size is larger then the bytes of data, copy all data
	jge	.copy_all_bytes

    sub     eax, edx					   ; store new count (data bytes in buffer - bytes we're about to copy)
    mov     [ebx + SOCKET.rxDataCount], eax		   ;
    push    eax
    mov     eax, edx					   ; number of bytes we want to copy must be in eax
	call	.start_copy				   ; copy to the application

    mov     esi, ebx					   ; now we're going to copy the remaining bytes to the beginning
	add	esi, SOCKET.rxData			   ; we dont need to copy the header
    mov     edi, esi					   ; edi is where we're going to copy to
    add     esi, edx					   ; esi is from where we copy
    pop     ecx 					   ; count of bytes we have left
    push    ecx 					   ; push it again so we can re-use it later
    shr     ecx, 2					   ; divide eax by 4
    cld
    rep     movsd					   ; copy all full dwords
    pop     ecx
    and     ecx, 3
    rep     movsb					   ; copy remaining bytes

  .exit:
	mov	[ebx + SOCKET.lock], 0
    ret 						   ; at last, exit

  .error:
	xor	eax, eax
	ret

  .copy_all_bytes:
    xor     esi, esi
    mov     [ebx + SOCKET.rxDataCount], esi		   ; store new count (zero)
	call	.start_copy
	mov	[ebx + SOCKET.lock], 0
	ret

  .start_copy:
	mov	edi, ecx
	mov	esi, ebx
	add	esi, SOCKET.rxData			   ; we dont need to copy the header
    mov     ecx, eax					   ; eax is count of bytes
    push    ecx
    shr     ecx, 2					   ; divide eax by 4
    cld 						   ; copy all full dwords
	rep	movsd
    pop     ecx
    and     ecx, 3
    rep     movsb					   ; copy the rest bytes
	retn						   ; exit, or go back to shift remaining bytes if any
endp

;; [53.4] Send data through DGRAM socket
;
; @param EBX is socket number
; @param ECX is application data size (number of bytes to send)
; @param EDX is pointer to application data buffer
; @return 0 (sent successfully) or -1 (error) in EAX
;;
proc socket_write stdcall
;        DEBUGF  1, "socket_write(0x%x)\n", ebx
	stdcall net_socket_num_to_addr, ebx		   ; get real socket address
	or	eax, eax
	jz	.error

	mov	ebx, eax

    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
	je	.error

    ; Save the queue entry number
    push    eax

    ; save the pointers to the data buffer & size
    push    edx
    push    ecx

    ; convert buffer pointer eax to the absolute address
    mov     ecx, IPBUFFSIZE
    mul     ecx
    add     eax, IPbuffs

    mov     edx, eax

    ; So, ebx holds the socket ptr, edx holds the IPbuffer ptr

	; Fill in the IP header (some data is in the socket descriptor)
	mov	eax, [ebx + SOCKET.LocalIP]
	mov	[edx + IP_PACKET.SourceAddress], eax
	mov	eax, [ebx + SOCKET.RemoteIP]
	mov	[edx + IP_PACKET.DestinationAddress], eax

	mov	[edx + IP_PACKET.VersionAndIHL], 0x45
	mov	[edx + IP_PACKET.TypeOfService], 0

    pop     eax 		  ; Get the UDP data length
    push    eax

    add     eax, 20 + 8 	  ; add IP header and UDP header lengths
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

    pop     eax
    push    eax

    add     eax, 8
	xchg	al, ah
	mov	[edx + 20 + UDP_PACKET.Length], ax

    ; Checksum left unfilled
	xor	eax, eax
	mov	[edx + 20 + UDP_PACKET.Checksum], ax

    pop     ecx 		 ; count of bytes to send
    mov     ebx, ecx		; need the length later
    pop     eax 		 ; get callers ptr to data to send

    ; Get the address of the callers data
    mov     edi, [TASK_BASE]
    add     edi, TASKDATA.mem_start
    add     eax, [edi]
    mov     esi, eax

    mov     edi, edx
    add     edi, 28
    cld
    rep     movsb		; copy the data across

    ; we have edx as IPbuffer ptr.
    ; Fill in the UDP checksum
    ; First, fill in pseudoheader
	mov	eax, [edx + IP_PACKET.SourceAddress]
    mov     [pseudoHeader], eax
	mov	eax, [edx + IP_PACKET.DestinationAddress]
	mov	[pseudoHeader + 4], eax
	mov	word[pseudoHeader + 8], PROTOCOL_UDP shl 8 + 0	    ; 0 + protocol
    add     ebx, 8
    mov     eax, ebx
	xchg	al, ah
	mov	[pseudoHeader + 10], ax

    mov     eax, pseudoHeader
    mov     [checkAdd1], eax
    mov     [checkSize1], word 12
    mov     eax, edx
    add     eax, 20
    mov     [checkAdd2], eax
    mov     eax, ebx
    mov     [checkSize2], ax	  ; was eax!! mjh 8/7/02

    call    checksum

    ; store it in the UDP checksum ( in the correct order! )
    mov     ax, [checkResult]

    ; If the UDP checksum computes to 0, we must make it 0xffff
    ; (0 is reserved for 'not used')
	test	ax, ax
	jnz	@f
    mov     ax, 0xffff

    @@: xchg	al, ah
	mov	[edx + 20 + UDP_PACKET.Checksum], ax

    ; Fill in the IP header checksum
    GET_IHL ecx,edx		 ; get IP-Header length
    stdcall checksum_jb,edx,ecx  ; buf_ptr, buf_size
	xchg	al, ah
	mov	[edx + IP_PACKET.HeaderChecksum], ax

    ; Check destination IP address.
    ; If it is the local host IP, route it back to IP_RX

    pop     ebx
    mov     eax, NET1OUT_QUEUE
	mov	ecx, [edx + SOCKET.RemoteIP]
    mov     edx, [stack_ip]
    cmp     edx, ecx
	jne	.not_local
    mov     eax, IPIN_QUEUE

  .not_local:
    ; Send it.
    call    queue

    xor     eax, eax
    ret

  .error:
	or	eax, -1
    ret
endp

;; [53.7] Send data through STREAM socket
;
; @param EBX is socket number
; @param ECX is application data size (number of bytes to send)
; @param EDX is pointer to application data buffer
; @return 0 (sent successfully) or -1 (error) in EAX
;;
proc socket_write_tcp stdcall
local sockAddr dd ?

;        DEBUGF  1, "socket_write_tcp(0x%x)\n", ebx
	stdcall net_socket_num_to_addr, ebx
	or	eax, eax
	jz	.error

	mov	ebx, eax
	mov	[sockAddr], ebx

    ; If the sockets window timer is nonzero, do not queue packet
	cmp	[ebx + SOCKET.wndsizeTimer], 0
	jne	.error

    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
	je	.error

    push    eax

    ; Get the address of the callers data
    mov     edi, [TASK_BASE]
    add     edi, TASKDATA.mem_start
    add     edx, [edi]
    mov     esi, edx

    pop     eax
    push    eax

    push    ecx
	mov	bl, TH_ACK
	stdcall build_tcp_packet, [sockAddr]
    pop     ecx

    ; Check destination IP address.
    ; If it is the local host IP, route it back to IP_RX

    pop     ebx
    push    ecx

	mov	eax, NET1OUT_QUEUE
    mov     edx, [stack_ip]
	mov	ecx, [sockAddr]
	cmp	edx, [ecx + SOCKET.RemoteIP]
	jne	.not_local
    mov     eax, IPIN_QUEUE

  .not_local:
    pop     ecx

    push    ebx 		; save ipbuffer number

    call    queue

	mov	esi, [sockAddr]

    ; increament SND.NXT in socket
    ; Amount to increment by is in ecx
	add	esi, SOCKET.SND_NXT
    call    add_inet_esi

    pop     ebx

    ; Copy the IP buffer to a resend queue
    ; If there isn't one, dont worry about it for now
    mov     esi, resendQ
    mov     ecx, 0

  .next_resendq:
    cmp     ecx, NUMRESENDENTRIES
	je	.exit		   ; None found
	cmp	dword[esi + 4], 0
	je	@f		   ; found one
    inc     ecx
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
	mov	[esi + 4], eax
	mov	byte[esi + 1], TCP_RETRIES
	mov	word[esi + 2], TCP_TIMEOUT

    inc     ecx
    ; Now get buffer location, and copy buffer across. argh! more copying,,
    mov     edi, resendBuffer - IPBUFFSIZE

    @@: add	edi, IPBUFFSIZE
	loop	@b

    ; we have dest buffer location in edi
    pop     eax
    ; convert source buffer pointer eax to the absolute address
    mov     ecx, IPBUFFSIZE
    mul     ecx
    add     eax, IPbuffs
    mov     esi, eax

    ; do copy
    mov     ecx, IPBUFFSIZE
    cld
    rep     movsb

  .exit:
    xor     eax, eax
    ret

  .error:
	or	eax, -1
    ret
endp
