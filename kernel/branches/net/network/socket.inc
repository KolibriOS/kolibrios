;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2009. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  SOCKET.INC                                                     ;;
;;                                                                 ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;    based on code by mike.dld                                    ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

align 4
struct	 SOCKET
	 .PrevPtr		dd ? ; pointer to previous socket in list
	 .NextPtr		dd ? ; pointer to next socket in list
	 .Number		dd ? ; socket number (unique within single process)
	 .PID			dd ? ; application process id
	 .Domain		dd ? ; INET/UNIX/..
	 .Type			dd ? ; RAW/UDP/TCP/...
	 .Protocol		dd ? ; ICMP/IPv4/ARP/
	 .LocalIP		dd ? ; local IP address
	 .RemoteIP		dd ? ; remote IP address
	 .LocalPort		dw ? ; local port       (In INET byte order)
	 .RemotePort		dw ? ; remote port      (IN INET byte order
	 .OrigRemoteIP		dd ? ; original remote IP address (used to reset to LISTEN state)
	 .OrigRemotePort	dw ? ; original remote port (used to reset to LISTEN state)
	 .rxDataCount		dd ? ; rx data count
	 .TCBState		dd ? ; TCB state
	 .TCBTimer		dd ? ; TCB timer (seconds)
	 .ISS			dd ? ; initial send sequence
	 .IRS			dd ? ; initial receive sequence
	 .SND_UNA		dd ? ; sequence number of unack'ed sent Packets
	 .SND_NXT		dd ? ; bext send sequence number to use
	 .SND_WND		dd ? ; send window
	 .RCV_NXT		dd ? ; next receive sequence number to use
	 .RCV_WND		dd ? ; receive window
	 .SEG_LEN		dd ? ; segment length
	 .SEG_WND		dd ? ; segment window
	 .wndsizeTimer		dd ? ; window size timer
	 .lock			dd ? ; lock mutex
	 .backlog		dw ? ; Backlog
	 .rxData:		     ; receive data buffer here
ends

MAX_backlog	equ 20

; socket buffers
SOCKETBUFFSIZE		equ 4096  ; state + config + buffer.
SOCKETHEADERSIZE	equ SOCKET.rxData ; thus 4096 - SOCKETHEADERSIZE bytes data

uglobal
	net_sockets	rd 2
	last_UDP_port	dw ? ; These values give the number of the last used ephemeral port
	last_TCP_port	dw ? ;
endg


;-----------------------------------------------
;
; SOCKET_init
;
;  -
;
;  IN:  /
;  OUT: /
;
;-----------------------------------------------

align 4
socket_init:

	mov	[net_sockets], 0
	mov	[net_sockets + 4], 0

	mov	[last_UDP_port], MIN_EPHEMERAL_PORT
	mov	[last_TCP_port], MIN_EPHEMERAL_PORT

	ret


;-----------------------------------------------------------------------------
;
; Socket API (function 74)
;
;-----------------------------------------------------------------------------

align 4
sys_socket:

	test	bl, bl
	jz	socket_open	; 0
	dec	bl
	jz	socket_close	; 1
	dec	bl
	jz	socket_bind	; 2
	dec	bl
	jz	socket_listen	; 3
	dec	bl
	jz	socket_connect	; 4
	dec	bl
	jz	socket_accept	; 5
	dec	bl
	jz	socket_send	; 6
	dec	bl
	jz	socket_recv	; 7

s_error:
	mov	dword [esp+32],-1

	ret




;-----------------------------------------------
;
; SOCKET_open
;
;
;  IN:  domain in ecx
;       type in edx
;       protocol in esi
;  OUT: eax is socket num, -1 on error
;
;-----------------------------------------------
align 4
socket_open:

	DEBUGF	1,"socket_open: domain: %u, type: %u",ecx, edx

	call	net_socket_alloc
	or	eax, eax
	jz	s_error

	mov	[eax + SOCKET.Domain], ecx
	mov	[eax + SOCKET.Type], edx
	mov	[eax + SOCKET.Protocol], esi

	stdcall net_socket_addr_to_num, eax
	DEBUGF	1,", socketnumber: %u\n", eax

	mov	[esp+32], eax

	ret




;-----------------------------------------------
;
; SOCKET_bind
;
;  IN:  socket number in ecx
;       pointer to sockaddr struct in edx
;       length of that struct in esi
;  OUT: 0 on success
;
;-----------------------------------------------
align 4
socket_bind:

	DEBUGF	1,"Socket_bind: socknum: %u sockaddr: %x, length: %u, ",ecx,edx,esi

	stdcall net_socket_num_to_addr, ecx
	cmp	eax, -1
	jz	s_error

	cmp	esi, 2
	jl	s_error

	cmp	word [edx], AF_INET4
	jne	s_error

  .af_inet4:

	cmp	esi, 6
	jl	s_error

	mov	ecx, [eax + SOCKET.Type]
	mov	bx, word [edx + 2]
	DEBUGF	1,"local port: %x ",bx
	test	bx, bx
	jz	.find_free

	call	socket_check_port
	test	bx, bx
	je	s_error
	jmp	.got_port

  .find_free:

	call	socket_find_port
	test	bx, bx
	je	s_error

  .got_port:
	DEBUGF	1,"using port: %x ",bx
	mov	word [eax + SOCKET.LocalPort], bx

	mov	ebx, dword [edx + 4]
	mov	dword [eax + SOCKET.LocalIP], ebx

	DEBUGF	1,"local ip: %u.%u.%u.%u\n",\
	[eax + SOCKET.LocalIP]:1,[eax + SOCKET.LocalIP + 1]:1,[eax + SOCKET.LocalIP + 2]:1,[eax + SOCKET.LocalIP + 3]:1

	mov	dword [esp+32],0
	ret




;-----------------------------------------------
;
; SOCKET_connect
;
;
;  IN:  socket number in ecx
;       pointer to sockaddr struct in edx
;       length of that struct in esi
;  OUT: 0 on success
;
;-----------------------------------------------
align 4
socket_connect:

	DEBUGF	1,"Socket_connect: socknum: %u sockaddr: %x, length: %u,",ecx,edx,esi

	stdcall net_socket_num_to_addr, ecx
	cmp	eax, -1
	jz	s_error

	cmp	esi, 2
	jl	s_error

	cmp	word [edx], AF_INET4
	je	.af_inet4

	jmp	s_error

  .af_inet4:

	cmp	esi, 8
	jl	s_error

	cmp	[eax + SOCKET.Type], IP_PROTO_UDP
	je	.udp

	cmp	[eax + SOCKET.Type], IP_PROTO_ICMP
	je	.icmp

	cmp	[eax + SOCKET.Type], IP_PROTO_TCP
	je	.tcp

	jmp	s_error

  .udp:

	mov	bx , word [edx + 2]
	mov	word [eax + SOCKET.RemotePort], bx
	DEBUGF	1,"remote port: %x ",bx

	mov	ebx, dword [edx + 4]
	mov	dword [eax + SOCKET.RemoteIP], ebx
	DEBUGF	1,"remote ip: %u.%u.%u.%u\n",[edx+4]:1,[edx+5]:1,[edx+6]:1,[edx+7]:1

	mov	dword [esp+32],0
	ret

  .icmp:


	ret



  .tcp:

;local sockAddr dd ?

;        cmp     esi, SOCKET_PASSIVE
;        jne     .skip_port_check
;
;        push    ebx
;        mov     eax, ebx
;        xchg    al, ah
;        mov     ebx, net_sockets
;
;  .next_socket:
;        mov     ebx, [ebx + SOCKET.NextPtr]
;        or      ebx, ebx
;        jz      .last_socket
;        cmp     [ebx + SOCKET.TCBState], TCB_LISTEN
;        jne     .next_socket
;        cmp     [ebx + SOCKET.LocalPort], ax
;        jne     .next_socket
;
;        xchg    al, ah
;        DEBUGF  1, "K : error: port %u is listened by 0x%x\n", ax, ebx
;        pop     ebx
;        jmp     .error
;
;  .last_socket:
;        pop     ebx
;
;  .skip_port_check:

;        mov     [eax + SOCKET.wndsizeTimer], 0     ; Reset the window timer.
;
;        xchg    bh, bl
;        mov     [eax + SOCKET.LocalPort], bx
;        xchg    ch, cl
;        mov     [eax + SOCKET.RemotePort], cx
;        mov     [eax + SOCKET.OrigRemotePort], cx
;        mov     ebx, [IP_LIST]
;        mov     [eax + SOCKET.LocalIP], ebx
;        mov     [eax + SOCKET.RemoteIP], edx
;        mov     [eax + SOCKET.OrigRemoteIP], edx

;        mov     ebx, TCB_LISTEN
;        cmp     esi, SOCKET_PASSIVE
;        je      @f
;        mov     ebx, TCB_SYN_SENT
;    @@: mov     [eax + SOCKET.TCBState], ebx            ; Indicate the state of the TCB

;        cmp     ebx, TCB_LISTEN
;        je      .exit

	; Now, if we are in active mode, then we have to send a SYN to the specified remote port
;        mov     eax, EMPTY_QUEUE
;        call    dequeue
;        cmp     ax, NO_BUFFER
;        je      .exit

;        push    eax

;        mov     bl, TH_SYN
;        xor     ecx, ecx
;        stdcall build_tcp_Packet, [sockAddr]

;        mov     eax, NET1OUT_QUEUE
;        mov     edx, [IP_LIST]
;        mov     ecx, [sockAddr]
;        cmp     edx, [ecx + SOCKET.RemoteIP]
;        jne     .not_local
;        mov     eax, IPIN_QUEUE

;  .not_local:
	; Send it.
;        pop     ebx
;        call    queue

  .exit:
	xor	eax, eax
	ret




;-----------------------------------------------
;
; SOCKET_listen
;
;
;  IN:  socket number in ecx
;       backlog in edx
;  OUT: eax is socket num, -1 on error
;
;-----------------------------------------------
align 4
socket_listen:

	DEBUGF	1,"Socket_listen: socknum: %u backlog: %u\n",ecx,edx

	stdcall net_socket_num_to_addr, ecx
	cmp	eax, -1
	jz	s_error

	cmp	edx, MAX_backlog
	jl	.ok
	mov	dx , 20
  .ok:

	mov	[eax + SOCKET.backlog], dx

	; TODO: insert code for active connections like TCP

	mov	dword [esp+32], 0
	ret






;-----------------------------------------------
;
; SOCKET_accept
;
;
;  IN:  socket number in ecx
;       addr in edx
;       addrlen in esi
;  OUT: eax is socket num, -1 on error
;
;-----------------------------------------------
align 4
socket_accept:

	DEBUGF	1,"Socket_accept: socknum: %u sockaddr: %x, length: %u\n",ecx,edx,esi

	stdcall net_socket_num_to_addr, ecx
	or	eax, eax
	jz	s_error
	mov	esi, eax

	cmp	[esi + SOCKET.backlog], 0
	jz	s_error

	call	net_socket_alloc
	or	eax, eax
	jz	s_error
	mov	edi, eax

	dec	[esi + SOCKET.backlog]

	mov	ecx, (SOCKET.rxData+3)/4
	rep	movsd

	mov	[edi + SOCKET.backlog], 0

	; TODO: fill in structure in ecx

	mov	[esi + SOCKET.RemoteIP], 0
	mov	[esi + SOCKET.RemotePort], 0

	stdcall net_socket_addr_to_num, eax
	mov	[esp+32], eax

	ret



;-----------------------------------------------
;
; SOCKET_close
;
;
;  IN:  socket number in ecx
;  OUT: eax is socket num, -1 on error
;
;-----------------------------------------------
align 4
socket_close:

	DEBUGF	1,"Socket_close: socknum: %u\n",ecx

	stdcall net_socket_num_to_addr, ecx
	or	eax, eax
	jz	s_error


	cmp	[eax + SOCKET.Type], IP_PROTO_UDP
	je	.udp

	cmp	[eax + SOCKET.Type], IP_PROTO_ICMP
	je	.icmp

	cmp	[eax + SOCKET.Type], IP_PROTO_TCP
	je	.tcp

	jmp	s_error

  .udp:

	lea	ebx, [eax + SOCKET.lock]
	call	wait_mutex
	; TODO: mark the socket for deletion, using the mutex

	stdcall net_socket_free, eax

	mov	dword [esp+32],0
	ret


  .icmp:



	ret

  .tcp:

if 1 = 0
;local sockAddr dd ?

;        DEBUGF  1, "K : socket_close_tcp (0x%x)\n", ebx
	; first, remove any resend entries
	pusha

	mov	esi, resendQ
	mov	ecx, 0

  .next_resendq:
	cmp	ecx, NUMRESENDENTRIES
	je	.last_resendq	    ; None left
	cmp	[esi + 4], ebx
	je	@f		    ; found one
	inc	ecx
	add	esi, 8
	jmp	.next_resendq

    @@: mov	dword[esi + 4], 0
	inc	ecx
	add	esi, 8
	jmp	.next_resendq

  .last_resendq:
	popa

	mov	ebx, eax
;        mov     [sockAddr], eax

	cmp	[eax + SOCKET.TCBState], TCB_LISTEN
	je	.destroy_tcb
	cmp	[eax + SOCKET.TCBState], TCB_SYN_SENT
	je	.destroy_tcb

; Now construct the response, and queue for sending by IP
	mov	eax, EMPTY_QUEUE
	call	dequeue
	cmp	ax, NO_BUFFER
	je	.error

	push	eax

	mov	bl, TH_FIN
	xor	ecx, ecx
	xor	esi, esi
	stdcall build_tcp_Packet, [sockAddr]

	mov	 ebx, [sockAddr]
	; increament SND.NXT in socket
	lea	esi, [ebx + SOCKET.SND_NXT]
	call	inc_inet_esi

	; Get the socket state
	mov	eax, [ebx + SOCKET.TCBState]
	cmp	eax, TCB_SYN_RECEIVED
	je	.fin_wait_1
	cmp	eax, TCB_ESTABLISHED
	je	.fin_wait_1

	; assume CLOSE WAIT
	; Send a fin, then enter last-ack state
	mov	[ebx + SOCKET.TCBState], TCB_LAST_ACK
	jmp	.send

  .fin_wait_1:
	; Send a fin, then enter finwait2 state
	mov	[ebx + SOCKET.TCBState], TCB_FIN_WAIT_1

  .send:
	mov	eax, NET1OUT_QUEUE
	mov	edx, [IP_LIST]
;        mov     ecx, [sockAddr]
	cmp	edx, [ecx + SOCKET.RemoteIP]
	jne	.not_local
	mov	eax, IPIN_QUEUE

  .not_local:
	; Send it.
	pop	ebx
	call	queue
	jmp	.exit


  .destroy_tcb:

	stdcall net_socket_free, eax

end if

  .exit:
	mov	dword [esp+32],0
	ret




;-----------------------------------------------
;
; SOCKET_receive
;
;
;  IN:  socket number in ecx
;       addr in edx
;       addrlen in esi
;       flags in edi
;  OUT: eax is number of bytes copied, -1 on error
;
;-----------------------------------------------
align 4
socket_recv:

	DEBUGF	1,"Socket_receive: socknum: %u sockaddr: %x, length: %u, flags: %x\n",ecx,edx,esi,edi

	stdcall net_socket_num_to_addr, ecx		   ; get real socket address
	or	eax, eax
	jz	s_error

	DEBUGF 1,"real socket address:%x\n", eax

	mov	dword[esp+32], -1

	mov	edi, edx

	lea	ebx, [eax + SOCKET.lock]
	call	wait_mutex

	mov	ecx, [eax + SOCKET.rxDataCount] 	   ; get count of bytes
	DEBUGF 1,"bytes in socket:%u\n", ecx
	test	ecx, ecx				   ; if count of bytes is zero..
	jz	.exit					   ; exit function (eax will be zero)

	cmp	ecx, esi				   ; if buffer size is larger then the bytes of data, copy all data
	jle	.copy_all_bytes

	sub	ecx, esi				   ; store new count (data bytes in buffer - bytes we're about to copy)
	mov	[eax + SOCKET.rxDataCount], ecx 	   ;
	push	ecx
	mov	edx, esi

	call	.start_copy				   ; copy to the application

	mov	dword[esp+32], edx

	lea	edi, [eax + SOCKET.rxData]		   ; Now shift the remaining bytes to start of buffer
	lea	esi, [edi + edx]
	mov	ecx, [esp]
	shr	ecx, 2					   ; divide eax by 4
	rep	movsd					   ; copy all full dwords
	pop	ecx
	and	ecx, 3
	rep	movsb					   ; copy remaining bytes

  .exit:
	mov	[eax + SOCKET.lock], 0
	ret

  .copy_all_bytes:
	mov	dword[esp+32], ecx
	mov	[eax + SOCKET.rxDataCount], 0		  ; store new count (zero)
	push	dword .exit				   ; this code results in same as commented out code

  .start_copy:
	DEBUGF	1,"copying %u bytes\n",ecx

	lea	esi, [eax + SOCKET.rxData]
	push	ecx
	shr	ecx, 2					   ; divide eax by 4
	rep	movsd
	pop	ecx
	and	ecx, 3
	rep	movsb					   ; copy the rest bytes

	ret						   ; exit, or go back to shift remaining bytes if any



;-----------------------------------------------
;
; SOCKET_send
;
;
;  IN:  socket number in ecx
;       pointer to data in edx
;       datalength in esi
;       flags in edi
;  OUT: -1 on error
;
;-----------------------------------------------
align 4
socket_send:

	DEBUGF	1,"Socket_send: socknum: %u sockaddr: %x, length: %u, flags: %x, ",ecx,edx,esi,edi

	stdcall net_socket_num_to_addr, ecx		   ; get real socket address
	or	eax, eax
	jz	s_error

	cmp	word [eax + SOCKET.Domain], AF_INET4
	je	.af_inet4

	jmp	s_error

  .af_inet4:

	DEBUGF	1,"Socket type:%u\n", [eax + SOCKET.Type]:4

	cmp	[eax + SOCKET.Type], IP_PROTO_UDP
	je	.udp

	cmp	[eax + SOCKET.Type], IP_PROTO_ICMP
	je	.icmp

	cmp	[eax + SOCKET.Type], IP_PROTO_TCP
	je	.tcp

	jmp	s_error

  .udp:

	DEBUGF	1,"type: UDP, "

	cmp	[eax + SOCKET.LocalPort],0
	jne	.port_ok

	push	esi
	mov	ecx, [eax + SOCKET.Type]
	call	socket_find_port
	test	bx, bx
	pop	esi
	je	s_error
	mov	[eax + SOCKET.LocalPort], bx

  .port_ok:

	mov	ecx, esi
	mov	esi, edx
	mov	edx, dword [eax + SOCKET.LocalPort] ; load local port and remote port at once
	DEBUGF	1,"local port: %x, remote port: %x\n",[eax + SOCKET.LocalPort]:4, [eax + SOCKET.RemotePort]:4
	mov	ebx, [eax + SOCKET.LocalIP]
	mov	eax, [eax + SOCKET.RemoteIP]

	call	UDP_create_packet

	mov	[esp+32], eax
	ret

  .icmp:
	; note: for ICMP sockets the SOCKET.LocalPort is used as the 'Identifier' value for ICMP packets
	; the application must add the header to the data, the kernel will fill in 'identifier' and 'checksum'

	sub	ecx, ICMP_Packet.Data
	mov	esi, edx
	push	ax
	call	IPv4_get_frgmnt_num
	mov	dx, ax
	pop	ax
	shl	edx, 16
	mov	dh , [esi + ICMP_Packet.Type]
	mov	dl , [esi + ICMP_Packet.Code]
	mov	di , [esi + ICMP_Packet.Identifier]
;        mov     [eax + SOCKET.LocalPort], di            ; Set localport to the identifier number, so we can receive reply's
	shl	edi, 16
	mov	di , [esi + ICMP_Packet.SequenceNumber]
	add	esi, ICMP_Packet.Data
	mov	ebx, [eax + SOCKET.LocalIP]
	mov	eax, [eax + SOCKET.RemoteIP]
	call	ICMP_create_packet

	mov	[esp+32], eax
	ret

  .tcp:

	ret




;-----------------------------------------------
;
; SOCKET_find_free_port (local port)
;
; works with INET byte order
;
;  IN:  type in ecx (TCP/UDP)
;  OUT: bx = 0 on error, portnumber otherwise
;
;-----------------------------------------------
align 4
socket_find_port:

	DEBUGF	1,"Socket_find_free_port\n"

	cmp	ecx, IP_PROTO_UDP
	je	.udp

	cmp	ecx, IP_PROTO_TCP
	je	.tcp

  .udp:
	mov	bx, [last_UDP_port]
	je	.continue

  .tcp:
	mov	bx, [last_TCP_port]


  .continue:
	inc	bx

  .check_only:
	mov	esi, net_sockets

  .next_socket:
	mov	esi, [esi + SOCKET.NextPtr]
	or	esi, esi
	jz	.port_ok

	cmp	[esi + SOCKET.Type], ecx
	jne	.next_socket

	rol	bx, 8
	cmp	[esi + SOCKET.LocalPort], bx
	rol	bx, 8				; this doesnt change the zero flag, does it ?
	jne	.next_socket

	cmp	bx, MAX_EPHEMERAL_PORT
	jle	.continue

	; todo: WRAP!
;        mov     [last_UDP_port], MIN_EPHEMERAL_PORT
  .exit:
	xor	ebx, ebx

  .port_ok:
	rol	bx, 8
	ret

;-----------------------------------------------
;
; SOCKET_check_port (local port)
;
; works with INET byte order
;
;  IN:  type in ecx (TCP/UDP)
;       port to check in bx
;  OUT: bx = 0 on error, unchanged otherwise
;
;-----------------------------------------------
align 4
socket_check_port:
	mov	esi, net_sockets

  .next_socket:
	mov	esi, [esi + SOCKET.NextPtr]
	or	esi, esi
	jz	.port_ok

	cmp	[esi + SOCKET.Type], ecx
	jne	.next_socket

	cmp	[esi + SOCKET.LocalPort], bx
	jne	.next_socket

	xor	ebx, ebx

  .port_ok:
	ret



;-----------------------------------------------
;
; SOCKET_internal_receiver
;
; Checks if any socket wants the received data
; If so, update the socket
;
;  IN:  eax = socket number
;       ecx = number of bytes
;       esi = pointer to beginning of data
;       dx = Remote port (in INET byte order)
;       edi = IP address of sender
;
;  OUT: xxx
;
;-----------------------------------------------
align 4
socket_internal_receiver:

	DEBUGF 1,"internal socket receiver\n"

	lea	ebx, [eax + SOCKET.lock]
	call	wait_mutex

	mov	[eax + SOCKET.RemotePort], dx		; update remote port number
	mov	[eax + SOCKET.RemoteIP], edi

	mov	edx, [eax + SOCKET.rxDataCount] 	; get # of bytes already in buffer
	DEBUGF 1,"bytes already in socket: %u ", edx

	lea	edi, [ecx + edx]			; check for buffer overflow
	cmp	edi, SOCKETBUFFSIZE - SOCKETHEADERSIZE	;
	jg	.dump					;

	lea	edi, [eax + SOCKET.rxData + edx]
	add	[eax + SOCKET.rxDataCount], ecx 	; increment the count of bytes in buffer
	DEBUGF 1,"adding %u bytes\n", ecx

	; copy the data across
	push	cx
	shr	ecx, 2
	rep	movsd
	pop	cx
	and	cx, 3
	rep	movsb

	DEBUGF 1,"socket updated\n"

	mov	[eax + SOCKET.lock], 0

	; flag an event to the application
	mov	edx, [eax + SOCKET.PID] 		; get socket owner PID
	mov	ecx, 1
	mov	esi, TASK_DATA + TASKDATA.pid

       .next_pid:
	cmp	[esi], edx
	je	.found_pid
	inc	ecx
	add	esi, 0x20
	cmp	ecx, [TASK_COUNT]
	jbe	.next_pid
	ret

       .found_pid:
	shl	ecx, 8
	or	[ecx + SLOT_BASE + APPDATA.event_mask], EVENT_NETWORK ; stack event
	mov	[check_idle_semaphore], 200
	ret

       .dump:
	mov	[eax + SOCKET.lock], 0
	ret





; Allocate memory for socket data and put new socket into the list
; Newly created socket is initialized with calling PID and number and
; put into beginning of list (which is a fastest way).
;
; @return socket structure address in EAX
;
proc net_socket_alloc stdcall uses ebx ecx edx edi
	stdcall kernel_alloc, SOCKETBUFFSIZE
	DEBUGF	1, "K : net_socket_alloc (0x%x)\n", eax
	; check if we can allocate needed amount of memory
	or	eax, eax
	jz	.exit

	; zero-initialize allocated memory
	push	eax
	mov	edi, eax
	mov	ecx, SOCKETBUFFSIZE / 4
;        cld
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

; Free socket data memory and pop socket off the list
;
; @param sockAddr is a socket structure address
;
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

    @@: ; and finally free the memory structure used
	stdcall kernel_free, [sockAddr]
	ret

  .error:
	DEBUGF	1, "K :   failed\n"
	ret
endp

; Get socket structure address by its number
; Scan through sockets list to find the socket with specified number.
; This proc uses SOCKET.PID indirectly to check if socket is owned by
; calling process.
;
; @param sockNum is a socket number
; @return socket structure address or 0 (not found) in EAX
;
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

; Get socket number by its structure address
; Scan through sockets list to find the socket with specified address.
; This proc uses SOCKET.PID indirectly to check if socket is owned by
; calling process.
;
; @param sockAddr is a socket structure address
; @return socket number (SOCKET.Number) or 0 (not found) in EAX
;
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
