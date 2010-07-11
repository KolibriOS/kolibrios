;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2010. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  SOCKET.INC                                                     ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;     based on code by mike.dld                                   ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

virtual at 0

	SOCKET:
	.NextPtr		dd ? ; pointer to next socket in list
	.PrevPtr		dd ? ; pointer to previous socket in list
	.Number 		dd ? ; socket number

	.lock			dd ? ; lock mutex

	.PID			dd ? ; application process id
	.Domain 		dd ? ; INET/UNIX/..
	.Type			dd ? ; RAW/UDP/TCP/...
	.Protocol		dd ? ; ICMP/IPv4/ARP/
	.errorcode		dd ?

	.options		dd ?
	.SO_SND.SB_CC		dd ?  ;;;;; socket options: number of bytes in socket
	.SO_RCV.SB_CC		dd ?
	.state			dd ?  ;;;;;;;;;

	.end:
end virtual

virtual at SOCKET.end

	IP_SOCKET:

	.LocalIP		dd ?
				rd 3 ; for IPv6 addresses

	.RemoteIP		dd ?
				rd 3 ; for IPv6 addresses

	.end:
end virtual

virtual at SOCKET.end

	SOCKET_virtual:

	.ConnectedTo		dd ? ; Socket number of other socket this one is connected to

	.end:
end virtual

virtual at IP_SOCKET.end

	TCP_SOCKET:

	.LocalPort		dw ? ; In INET byte order
	.RemotePort		dw ? ; In INET byte order

	.backlog		dw ? ; Backlog
	.backlog_cur		dw ? ; current size of queue for un-accept-ed connections

	.OrigRemoteIP		dd ? ; original remote IP address (used to reset to LISTEN state)
	.OrigRemotePort 	dw ? ; original remote port (used to reset to LISTEN state)

	.t_state		dd ? ; TCB state
	.t_timer		dd ? ; TCB timer (seconds)
	.t_rxtshift		dd ?
	.t_rxtcur		dd ?
	.t_dupacks		dd ?
	.t_maxseg		dd ?
	.t_force		dd ?
	.t_flags		dd ?

;---------------
; RFC783 page 21

; send sequence
	.SND_UNA		dd ? ; sequence number of unack'ed sent Packets
	.SND_NXT		dd ? ; next send sequence number to use
	.SND_UP 		dd ?
	.SND_WL1		dd ? ; window minus one
	.SND_WL2		dd ? ;
	.ISS			dd ? ; initial send sequence number
	.SND_WND		dd ? ; send window

; receive sequence
	.RCV_WND		dw ? ; receive window
	.RCV_NXT		dd ? ; next receive sequence number to use
	.RCV_UP 		dd ?
	.IRS			dd ? ; initial receive sequence number

;---------------------
; Additional variables

; receive variables
	.RCV_ADV		dd ?

; retransmit variables
	.SND_MAX		dd ?

; congestion control
	.SND_CWND		dd ?
	.SND_SSTHRESH		dd ?

;----------------------
; Transmit timing stuff

	.t_idle 		dd ?
	.t_rtt			dd ?
	.t_rtseq		dd ?
	.t_srtt 		dd ?
	.t_rttvar		dd ?
	.t_rttmin		dd ?
	.max_sndwnd		dd ?

;-----------------
; Out-of-band data

	.t_oobflags		dd ?
	.t_iobc 		dd ?
	.t_softerror		dd ?


;---------
; RFC 1323

	.SND_SCALE		db ? ; Scale factor
	.RCV_SCALE		db ?
	.request_r_scale	db ?
	.requested_s_scale	dd ?

	.ts_recent		dd ?
	.ts_recent_age		dd ?
	.last_ack_sent		dd ?

	.end:
end virtual

virtual at IP_SOCKET.end

	UDP_SOCKET:

	.LocalPort		dw ? ; In INET byte order
	.RemotePort		dw ? ; In INET byte order
	.firstpacket		db ?

	.end:
end virtual

virtual at IP_SOCKET.end

	ICMP_SOCKET:

	.Identifier		dw ? ;

	.end:
end virtual


struct	socket_queue_entry
;        .owner          dd ?
	.data_ptr	dd ?
	.buf_ptr	dd ?
	.data_size	dd ?
	.size:
ends


MAX_backlog		equ 20	     ; backlog for stream sockets
SOCKETBUFFSIZE		equ 4096     ; in bytes

SOCKET_QUEUE_SIZE	equ 10	     ; maximum number ofincoming packets queued for 1 socket
; the incoming packet queue for sockets is placed in the socket struct itself, at this location from start
SOCKET_QUEUE_LOCATION	equ SOCKETBUFFSIZE - SOCKET_QUEUE_SIZE*socket_queue_entry.size - queue.data

uglobal
	net_sockets	rd 4
	last_UDP_port	dw ? ; These values give the number of the last used ephemeral port
	last_TCP_port	dw ? ;
endg


;-----------------------------------------------------------------
;
; SOCKET_init
;
;  -
;
;  IN:  /
;  OUT: /
;
;-----------------------------------------------------------------
align 4
socket_init:

	xor	eax, eax
	mov	edi, net_sockets
	mov	ecx, 4
	rep	stosd

	mov	[last_UDP_port], MIN_EPHEMERAL_PORT
	mov	[last_TCP_port], MIN_EPHEMERAL_PORT

	ret


;-----------------------------------------------------------------
;
; Socket API (function 74)
;
;-----------------------------------------------------------------
align 4
sys_socket:
	cmp	ebx, 8		; highest possible number
	jg	s_error
	lea	ebx, [.table + 4*ebx]
	jmp	dword [ebx]

.table:
	dd	SOCKET_open	; 0
	dd	SOCKET_close	; 1
	dd	SOCKET_bind	; 2
	dd	SOCKET_listen	; 3
	dd	SOCKET_connect	; 4
	dd	SOCKET_accept	; 5
	dd	SOCKET_send	; 6
	dd	SOCKET_receive	; 7
	dd	SOCKET_get_opt	; 8
;        dd      SOCKET_set_opt  ; 9


s_error:
	DEBUGF	1,"socket error\n"
	mov	dword [esp+32], -1

	ret


;-----------------------------------------------------------------
;
; SOCKET_open
;
;  IN:  domain in ecx
;       type in edx
;       protocol in esi
;  OUT: eax is socket num, -1 on error
;
;-----------------------------------------------------------------
align 4
SOCKET_open:

	DEBUGF	1,"socket_open: domain: %u, type: %u protocol: %x\n", ecx, edx, esi

	call	SOCKET_alloc
	jz	s_error

	mov	[eax + SOCKET.Domain], ecx
	mov	[eax + SOCKET.Type], edx
	mov	[eax + SOCKET.Protocol], esi

	mov	[esp+32], edi

	ret



;-----------------------------------------------------------------
;
; SOCKET_bind
;
;  IN:  socket number in ecx
;       pointer to sockaddr struct in edx
;       length of that struct in esi
;  OUT: 0 on success
;
;-----------------------------------------------------------------
align 4
SOCKET_bind:

	DEBUGF	1,"socket_bind: socknum: %u sockaddr: %x, length: %u\n", ecx, edx, esi

	call	SOCKET_num_to_ptr
	jz	s_error

	cmp	esi, 2
	jl	s_error

	cmp	word [edx], AF_INET4
	je	.af_inet4

	cmp	word [edx], AF_UNIX
	je	.af_unix

	jmp	s_error

  .af_unix:

	; TODO: write code here

	mov	dword [esp+32], 0
	ret

  .af_inet4:

	DEBUGF	1,"af_inet4\n"

	cmp	esi, 6
	jl	s_error

	mov	ecx, [eax + SOCKET.Type]

	mov	bx, word [edx + 2]
	test	bx, bx
	jz	.find_free

	call	SOCKET_check_port
;        test    bx, bx
	jz	s_error
	jmp	.got_port

    .find_free:
	call	SOCKET_find_port
;        test    bx, bx
	jz	s_error

    .got_port:
	DEBUGF	1,"using local port: %u", bx
	mov	word [eax + UDP_SOCKET.LocalPort], bx

	mov	ebx, dword [edx + 4]
	mov	dword [eax + IP_SOCKET.LocalIP], ebx

	DEBUGF	1,"local ip: %u.%u.%u.%u\n",\
	[eax + IP_SOCKET.LocalIP + 0]:1,[eax + IP_SOCKET.LocalIP + 1]:1,\
	[eax + IP_SOCKET.LocalIP + 2]:1,[eax + IP_SOCKET.LocalIP + 3]:1

	mov	dword [esp+32], 0
	ret




;-----------------------------------------------------------------
;
; SOCKET_connect
;
;  IN:  socket number in ecx
;       pointer to sockaddr struct in edx
;       length of that struct in esi
;  OUT: 0 on success
;
;-----------------------------------------------------------------
align 4
SOCKET_connect:

	DEBUGF	1,"socket_connect: socknum: %u sockaddr: %x, length: %u\n", ecx, edx, esi

	call	SOCKET_num_to_ptr
	jz	s_error

	cmp	esi, 8
	jl	s_error

	cmp	word [edx], AF_INET4
	je	.af_inet4

	jmp	s_error

  .af_inet4:

	cmp	[eax + SOCKET.Type], IP_PROTO_UDP
	je	.udp

	cmp	[eax + SOCKET.Type], IP_PROTO_TCP
	je	.tcp

	jmp	s_error

  .udp:
	mov	bx , word [edx + 2]
	mov	word [eax + UDP_SOCKET.RemotePort], bx
	mov	[eax + UDP_SOCKET.firstpacket], 0
	DEBUGF	1,"remote port: %u ",bx

	mov	ebx, dword [edx + 4]
	mov	dword [eax + IP_SOCKET.RemoteIP], ebx
	DEBUGF	1,"remote ip: %u.%u.%u.%u\n",[edx+4]:1,[edx+5]:1,[edx+6]:1,[edx+7]:1

	mov	dword [esp+32], 0
	ret


  .tcp:
	; set sequence number

	mov	ebx, [TCP_sequence_num]
	add	[TCP_sequence_num], 6400
	mov	[eax + TCP_SOCKET.ISS], ebx

	lea	ebx, [eax + SOCKET.lock]
	call	wait_mutex

	; fill in remote port and IP

;;;;;;        mov     [eax + TCP_SOCKET.wndsizeTimer], 0     ; Reset the window timer.

	mov	bx , word [edx + 2]
	mov	[eax + TCP_SOCKET.RemotePort], bx
	DEBUGF	1,"remote port: %u ",bx

	mov	ebx, dword [edx + 4]
	mov	[eax + IP_SOCKET.RemoteIP], ebx

	; check if local port and IP is ok

	cmp	[eax + IP_SOCKET.LocalIP], 0
	jne	@f
	push	[IP_LIST]	    ;;;;; device zero = default
	pop	[eax + IP_SOCKET.LocalIP]
       @@:

	cmp	[eax + TCP_SOCKET.LocalPort], 0
	jne	@f
	call	SOCKET_find_port
       @@:

;        mov     [eax + TCP_SOCKET.t_state], TCB_SYN_SENT
	call	TCP_output

	mov	[eax + SOCKET.lock], 0

	mov	dword [esp+32], 0	; success!
	ret


;-----------------------------------------------------------------
;
; SOCKET_listen
;
;  IN:  socket number in ecx
;       backlog in edx
;  OUT: eax is socket num, -1 on error
;
;-----------------------------------------------------------------
align 4
SOCKET_listen:

	DEBUGF	1,"Socket_listen: socknum: %u backlog: %u\n", ecx, edx

	call	SOCKET_num_to_ptr
	jz	s_error

	cmp	word [eax + SOCKET.Domain], AF_INET4
	jne	s_error

	cmp	[eax + SOCKET.Type], IP_PROTO_TCP
	jne	s_error

	; TODO: check local port number

	cmp	edx, MAX_backlog
	jle	.ok
	mov	edx, MAX_backlog
  .ok:

	mov	[eax + TCP_SOCKET.backlog], dx
	mov	[eax + TCP_SOCKET.t_state], TCB_LISTEN
	 or	[eax + SOCKET.options], SO_ACCEPTCON

	mov	dword [esp+32], 0

	ret


;-----------------------------------------------------------------
;
; SOCKET_accept
;
;  IN:  socket number in ecx
;       addr in edx
;       addrlen in esi
;  OUT: eax is socket num, -1 on error
;
;-----------------------------------------------------------------
align 4
SOCKET_accept:

	DEBUGF	1,"Socket_accept: socknum: %u sockaddr: %x, length: %u\n", ecx, edx, esi

	call	SOCKET_num_to_ptr
	jz	s_error

	cmp	word [eax + SOCKET.Domain], AF_INET4
	je	.af_inet4

	jmp	s_error

  .af_inet4:

	cmp	[eax + SOCKET.Type], IP_PROTO_TCP
	je	.tcp

	jmp	s_error

  .tcp:

	lea	ebx, [eax + SOCKET.lock]
	call	wait_mutex

	movzx	ebx, [eax + TCP_SOCKET.backlog_cur]
	test	ebx, ebx
	jz	.unlock_err

	dec	[eax + TCP_SOCKET.backlog_cur]
	mov	eax, [eax + TCP_SOCKET.end + (ebx-1)*4]
	mov	[eax + SOCKET.lock], 0
	mov	dword [esp+32], 0

	call	TCP_output	;;;;;

	ret

  .unlock_err:
	mov	[eax + SOCKET.lock], 0
	jmp	s_error


;-----------------------------------------------------------------
;
; SOCKET_close
;
;  IN:  socket number in ecx
;  OUT: eax is socket num, -1 on error
;
;-----------------------------------------------------------------
align 4
SOCKET_close:

	DEBUGF	1,"socket_close: socknum: %u\n", ecx

	call	SOCKET_num_to_ptr
	jz	s_error

	cmp	[eax + SOCKET.Domain], AF_INET4
	jne	s_error

	cmp	[eax + SOCKET.Type], IP_PROTO_UDP
	je	.free

	cmp	[eax + SOCKET.Type], IP_PROTO_ICMP
	je	.free

	cmp	[eax + SOCKET.Type], IP_PROTO_TCP
	je	.tcp

	jmp	s_error

  .tcp:
	test	[eax + TCP_SOCKET.t_state], TCB_SYN_RECEIVED  ;;;;;;
	jz	.free

	call	TCP_output

	mov	dword [esp+32], 0

	ret

;       state must be LISTEN, SYN_SENT, CLOSED or maybe even invalid
;       so, we may destroy the socket
  .free:
	call	SOCKET_free
	mov	dword [esp+32], 0

	ret


;-----------------------------------------------------------------
;
; SOCKET_receive
;
;  IN:  socket number in ecx
;       addr to buffer in edx
;       length of buffer in esi
;       flags in edi
;  OUT: eax is number of bytes copied, -1 on error
;
;-----------------------------------------------------------------
align 4
SOCKET_receive:

	DEBUGF	1,"socket_receive: socknum: %u bufaddr: %x, buflength: %u, flags: %x\n", ecx, edx, esi, edi

	call	SOCKET_num_to_ptr
	jz	s_error

	mov	ebx, esi
	get_from_queue (eax + SOCKET_QUEUE_LOCATION),\
		       SOCKET_QUEUE_SIZE,\
		       socket_queue_entry.size,\
		       s_error
		       ; destroys esi and ecx

	mov	edi, edx					; addr to buffer

	mov	ecx, [esi + socket_queue_entry.data_size]
	DEBUGF	1,"Got %u bytes of data\n", ecx

	cmp	ecx, ebx
	jle	.large_enough
	DEBUGF	1,"Buffer too small...\n"
	jmp	s_error

  .large_enough:
	push	[esi + socket_queue_entry.buf_ptr]	       ; save the buffer addr so we can clear it later
	mov	esi, [esi + socket_queue_entry.data_ptr]
	DEBUGF	1,"Source buffer: %x, real addr: %x\n", [esp], esi
	mov	dword[esp+32+4], ecx				; return number of bytes copied in ebx

; copy the data
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
; remove the packet     ;;; TODO: only if it is empty!!

;;;;        call    TCP_output  ; only if it is tcp

	call	kernel_free

	ret


;-----------------------------------------------------------------
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
;-----------------------------------------------------------------
align 4
SOCKET_send:

	DEBUGF	1,"socket_send: socknum: %u sockaddr: %x, length: %u, flags: %x\n", ecx, edx, esi, edi

	call	SOCKET_num_to_ptr
	jz	s_error

	cmp	word [eax + SOCKET.Domain], AF_INET4
	je	.af_inet4

	jmp	s_error

  .af_inet4:
	DEBUGF	1,"af_inet4\n"

	cmp	[eax + SOCKET.Type], IP_PROTO_TCP
	je	.tcp

	cmp	[eax + SOCKET.Type], IP_PROTO_UDP
	je	.udp

	jmp	s_error

  .udp:
	DEBUGF	1,"type: UDP\n"

; check if local port is valid
	cmp	[eax + UDP_SOCKET.LocalPort], 0
	jne	@f

	call	SOCKET_find_port
	jz	s_error

; Now, send the packet
     @@:
	mov	ecx, esi
	mov	esi, edx

	call	UDP_output

	mov	dword [esp+32], 0
	ret

  .tcp:
	DEBUGF	1,"type: TCP\n"

; check if local port is valid
	cmp	[eax + TCP_SOCKET.LocalPort], 0
	jne	@f

	call	SOCKET_find_port
	jz	s_error

     @@:
;;;; TODO: queue the data

	call	TCP_output

	mov	[esp+32], eax
	ret




;-----------------------------------------------------------------
;
; SOCKET_get_options
;
;  IN:  ecx = socket number
;       edx = pointer to the options:
;               dd      level, optname, optval, optlen
;  OUT: -1 on error
;
; At moment, uses only pseudo-optname -2 for get last_ack_number for TCP.
; TODO: find best way to notify that send()'ed data were acknowledged
; Also pseudo-optname -3 is valid and returns socket state, one of TCB_*.
;
;-----------------------------------------------------------------
align 4
SOCKET_get_opt:

	DEBUGF	1,"socket_get_opt\n"

	call	SOCKET_num_to_ptr
	jz	s_error

	cmp	dword [edx], IP_PROTO_TCP
	jnz	s_error
	cmp	dword [edx+4], -2
	jz	@f
	cmp	dword [edx+4], -3
	jnz	s_error
@@:
;        mov     eax, [edx+12]
;        test    eax, eax
;        jz      .fail
;        cmp     dword [eax], 4
;        mov     dword [eax], 4
;        jb      .fail
;        stdcall net_socket_num_to_addr, ecx
;        test    eax, eax
;        jz      .fail
;        ; todo: check that eax is really TCP socket
;        mov     ecx, [eax + TCP_SOCKET.last_ack_number]
;        cmp     dword [edx+4], -2
;        jz      @f
;        mov     ecx, [eax + TCP_SOCKET.state]
@@:
	mov	eax, [edx+8]
	test	eax, eax
	jz	@f
	mov	[eax], ecx
@@:
	mov	dword [esp+32], 0
	ret


;-----------------------------------------------------------------
;
; SOCKET_find_port
;
; Fills in the local port number for TCP and UDP sockets
; This procedure always works because the number of sockets is
; limited to a smaller number then the number of possible ports
;
;  IN:  eax = socket pointer
;  OUT: /
;
;-----------------------------------------------------------------
align 4
SOCKET_find_port:

	DEBUGF	1,"socket_find_free_port\n"

	push	ebx esi ecx

	cmp	[eax + SOCKET.Type], IP_PROTO_UDP
	je	.udp

	cmp	[eax + SOCKET.Type], IP_PROTO_TCP
	je	.tcp

	jmp	.error

  .done:
	mov	[eax + UDP_SOCKET.LocalPort], bx
  .error:
	pop	ecx esi ebx
	ret

  .udp:
	mov	bx, [last_UDP_port]
	call	.findit
	mov	[last_UDP_port], bx
	jmp	.done

  .tcp:
	mov	bx, [last_TCP_port]
	call	.findit
	mov	[last_TCP_port], bx
	jmp	.done


  .restart:
	mov	bx, MIN_EPHEMERAL_PORT
  .findit:
	inc	bx

	cmp	bx, MAX_EPHEMERAL_PORT
	jz	.restart

	call	SOCKET_check_port
	jz	.findit

	ret



;-----------------------------------------------------------------
;
; SOCKET_check_port
;
; Checks if a local port number is unused
; If the proposed port number is unused, it is filled in in the socket structure
;
;  IN:  eax = socket ptr (to find out if its a TCP/UDP socket)
;        bx = proposed socket number
;
;  OUT:  ZF = cleared on error
;
;-----------------------------------------------------------------
align 4
SOCKET_check_port:

	DEBUGF	1,"socket_check_port\n"

	mov	ecx, [eax + SOCKET.Type]
	mov	esi, net_sockets

  .next_socket:
	mov	esi, [esi + SOCKET.NextPtr]
	or	esi, esi
	jz	.port_ok

	cmp	[esi + SOCKET.Type], ecx
	jne	.next_socket

	cmp	[esi + UDP_SOCKET.LocalPort], bx
	jne	.next_socket

	DEBUGF	1,"local port %u already in use\n", bx
	ret

  .port_ok:
	mov	[eax + UDP_SOCKET.LocalPort], bx
	or	bx, bx					; set the zero-flag

	ret



;-----------------------------------------------------------------
;
; SOCKET_input
;
; Updates a socket with received data
;
; Note: the mutex should already be set !
;
;  IN:  eax = socket ptr
;       ebx = pointer to device struct
;       ecx = data size
;       esi = ptr to data
;       [esp] = ptr to buf
;       [esp + 4] = buf size
;
;  OUT: /
;
;-----------------------------------------------------------------
align 4
SOCKET_input:

	DEBUGF	1,"socket_input: socket=%x, data=%x size=%u\n", eax, esi, ecx

	mov	dword[esp+4], ecx
	push	esi
	mov	esi, esp

	add_to_queue (eax + SOCKET_QUEUE_LOCATION),\
		     SOCKET_QUEUE_SIZE,\
		     socket_queue_entry.size,\
		     SOCKET_input.full

	DEBUGF	1,"Queued packet successfully\n"
	add	esp, socket_queue_entry.size
	mov	[eax + SOCKET.lock], 0
	jmp	SOCKET_notify_owner

  .full:
	DEBUGF	2,"Socket %x is full!\n", eax
	mov	[eax + SOCKET.lock], 0
	call	kernel_free
	add	esp, 8

	ret

;-----------------------------------------------------------------
;
; SOCKET_notify_owner
;
; notify's the owner of a socket that something happened
;
;  IN:  eax = socket ptr
;  OUT: /
;
;-----------------------------------------------------------------
align 4
SOCKET_notify_owner:

	DEBUGF	1,"socket_notify_owner\n"

	call	SOCKET_check
	jz	.error

	push	ecx eax esi

; socket exists, now try to flag an event to the application

	mov	eax, [eax + SOCKET.PID]
	mov	ecx, 1
	mov	esi, TASK_DATA + TASKDATA.pid

       .next_pid:
	cmp	[esi], eax
	je	.found_pid
	inc	ecx
	add	esi, 0x20
	cmp	ecx, [TASK_COUNT]
	jbe	.next_pid

; PID not found, TODO: close socket!

	jmp	.error2

       .found_pid:
	shl	ecx, 8
	or	[ecx + SLOT_BASE + APPDATA.event_mask], EVENT_NETWORK
	mov	[check_idle_semaphore], 200

	DEBUGF	1,"owner notified\n"

  .error2:
	pop	esi eax ecx
  .error:

	ret


;--------------------------------------------------------------------
;
; SOCKET_alloc
;
; Allocate memory for socket data and put new socket into the list
; Newly created socket is initialized with calling PID and number and
; put into beginning of list (which is a fastest way).
;
; IN:  /
; OUT: eax = 0 on error, socket ptr otherwise
;      edi = socket number
;       ZF = cleared on error
;
;--------------------------------------------------------------------
align 4
SOCKET_alloc:

	push	ecx ebx

	stdcall kernel_alloc, SOCKETBUFFSIZE
	DEBUGF	1, "socket_alloc: %x ", eax
	or	eax, eax
	jz	.exit

; zero-initialize allocated memory
	push	eax edi
	mov	edi, eax
	mov	ecx, SOCKETBUFFSIZE / 4
	xor	eax, eax
	rep	stosd
	pop	edi eax

	init_queue (eax + SOCKET_QUEUE_LOCATION)

; find first free socket number and use it

	mov	ebx, net_sockets
	xor	ecx, ecx
  .next_socket_number:
	inc	ecx
  .next_socket:
	mov	ebx, [ebx + SOCKET.NextPtr]
	or	ebx, ebx
	jz	.last_socket
	cmp	[ebx + SOCKET.Number], ecx
	jne	.next_socket
	mov	ebx, net_sockets
	jmp	.next_socket_number

  .last_socket:
	mov	[eax + SOCKET.Number], ecx

	DEBUGF	1, "(number: %u)\n", ecx

; Fill in PID
	mov	ebx, [TASK_BASE]
	mov	ebx, [ebx + TASKDATA.pid]

	mov	[eax + SOCKET.PID], ebx

; add socket to the list by changing pointers

	mov	ebx, [net_sockets + SOCKET.NextPtr]

	mov	[eax + SOCKET.PrevPtr], net_sockets
	mov	[eax + SOCKET.NextPtr], ebx

	or	ebx, ebx
	jz	@f
	add	ebx, SOCKET.lock	; lock the next socket
	call	wait_mutex
	sub	ebx, SOCKET.lock
	mov	[ebx + SOCKET.PrevPtr], eax
	mov	[ebx + SOCKET.lock], 0
       @@:

	mov	[net_sockets + SOCKET.NextPtr], eax

	mov	edi, ecx
	or	eax, eax	; used to clear zero flag
  .exit:
	pop	ebx ecx

	ret


;----------------------------------------------------
;
; SOCKET_free
;
; Free socket data memory and remove socket from the list
;
; IN:  eax = socket ptr
; OUT: /
;
;----------------------------------------------------
align 4
SOCKET_free:

	DEBUGF	1, "socket_free: %x\n", eax

	call	SOCKET_check
	jz	.error

	push	ebx
	lea	ebx, [eax + SOCKET.lock]
	call	wait_mutex

	DEBUGF	1, "freeing socket..\n"

	push	eax				; this will be passed to kernel_free
	mov	ebx, [eax + SOCKET.NextPtr]
	mov	eax, [eax + SOCKET.PrevPtr]

	DEBUGF	1, "linking socket %x to socket %x\n", eax, ebx

	test	eax, eax
	jz	@f
	mov	[eax + SOCKET.NextPtr], ebx
       @@:

	test	ebx, ebx
	jz	@f
	mov	[ebx + SOCKET.PrevPtr], eax
       @@:

	call	kernel_free
	pop	ebx

	DEBUGF	1, "socket is gone!\n"

  .error:
	ret


;---------------------------------------------------
;
; SOCKET_num_to_ptr
;
; Get socket structure address by its number
;
; IN:  ecx = socket number
; OUT: ecx = 0 on error, socket ptr otherwise
;       ZF = set on error
;
;---------------------------------------------------
align 4
SOCKET_num_to_ptr:

	DEBUGF	1,"socket_num_to_ptr: %u ", ecx

	mov	eax, net_sockets

  .next_socket:
	mov	eax, [eax + SOCKET.NextPtr]
	or	eax, eax
	jz	.error
	cmp	[eax + SOCKET.Number], ecx
	jne	.next_socket

	test	eax, eax

	DEBUGF	1,"(%x)\n", eax
  .error:
	ret


;---------------------------------------------------
;
; SOCKET_ptr_to_num
;
; Get socket number by its address
;
; IN:  eax = socket ptr
; OUT: eax = 0 on error, socket num otherwise
;       ZF = set on error
;
;---------------------------------------------------
align 4
SOCKET_ptr_to_num:

	DEBUGF	1,"socket_ptr_to_num: %x ", eax

	call	SOCKET_check
	jz	.error

	mov	eax, [eax + SOCKET.Number]

	DEBUGF	1,"(%u)\n", eax

  .error:
	ret


;---------------------------------------------------
;
; SOCKET_check
;
; checks if the given value is really a socket ptr
;
; IN:  eax = socket ptr
; OUT: eax = 0 on error, unchanged otherwise
;       ZF = set on error
;
;---------------------------------------------------
align 4
SOCKET_check:

	DEBUGF	1,"socket_check\n"

	push	ebx
	mov	ebx, net_sockets

  .next_socket:
	mov	ebx, [ebx + SOCKET.NextPtr]
	or	ebx, ebx
	jz	.done
	cmp	ebx, eax
	jnz	.next_socket

  .done:
	mov	eax, ebx
	test	eax, eax
	pop	ebx

	ret



;---------------------------------------------------
;
; SOCKET_check_owner
;
; checks if the caller application owns the socket
;
; IN:  eax = socket ptr
; OUT:  ZF = true/false
;
;---------------------------------------------------
align 4
SOCKET_check_owner:

	DEBUGF	1,"socket_check_owner\n"

	push	ebx
	mov	ebx, [TASK_BASE]
	mov	ebx, [ecx + TASKDATA.pid]
	cmp	[eax + SOCKET.PID], ebx
	pop	 ebx

	ret




;---------------------------------------------------
;
; SOCKET_process_end
;
; Kernel calls this function when a certain process ends
; This function will check if the process had any open sockets
; And update them accordingly
;
; IN:  eax = pid
; OUT: /
;
;------------------------------------------------------
align 4
SOCKET_process_end:

	DEBUGF	1,"socket_process_end: %x\n", eax

	push	ebx
	mov	ebx, net_sockets

  .next_socket:

	mov	ebx, [ebx + SOCKET.NextPtr]
  .test_socket:
	test	ebx, ebx
	jz	.done

	cmp	[ebx + SOCKET.PID], eax
	jne	.next_socket

	DEBUGF	1,"closing socket %x", eax, ebx

	mov	[ebx + SOCKET.PID], 0

	cmp	[ebx + SOCKET.Type], IP_PROTO_UDP
	je	.udp

	cmp	[ebx + SOCKET.Type], IP_PROTO_TCP
	je	.tcp

	jmp	.next_socket	; kill all sockets for given PID

  .udp:
	mov	eax, ebx
	mov	ebx, [ebx + SOCKET.NextPtr]
	call	SOCKET_free
	jmp	.test_socket

  .tcp:

	jmp	.next_socket

  .done:
	pop	ebx

	ret
