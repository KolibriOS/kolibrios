;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org,                          ;;
;;     and Clevermouse.                                            ;;
;;                                                                 ;;
;;       Based on code by mike.dld                                 ;;
;;                                                                 ;;
;;         GNU GENERAL PUBLIC LICENSE                              ;;
;;          Version 2, June 1991                                   ;;
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
	.Type			dd ? ; RAW/STREAM/DGRAP
	.Protocol		dd ? ; ICMP/IPv4/ARP/TCP/UDP
	.errorcode		dd ?

	.options		dd ?
	.state			dd ?
	.backlog		dw ? ; how many incomming connections that can be queued

	.snd_proc		dd ?
	.rcv_proc		dd ?

	.end:
end virtual

virtual at SOCKET.end

	IP_SOCKET:
	.LocalIP		rd 4
	.RemoteIP		rd 4

	.end:
end virtual

virtual at IP_SOCKET.end

	TCP_SOCKET:

	.LocalPort		dw ?
	.RemotePort		dw ?

	.t_state		dd ? ; TCB state
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


;-------
; Timers
	.timer_retransmission	dw ? ; rexmt
	.timer_persist		dw ?
	.timer_keepalive	dw ? ; keepalive/syn timeout
	.timer_timed_wait	dw ? ; also used as 2msl timer

	.end:
end virtual

virtual at IP_SOCKET.end

	UDP_SOCKET:

	.LocalPort		dw ?
	.RemotePort		dw ?
	.firstpacket		db ?

	.end:
end virtual

virtual at IP_SOCKET.end

	ICMP_SOCKET:

	.Identifier		dw ?

	.end:
end virtual

struc	RING_BUFFER {
	.start_ptr		dd ? ; Pointer to start of buffer
	.end_ptr		dd ? ; pointer to end of buffer
	.read_ptr		dd ? ; Read pointer
	.write_ptr		dd ? ; Write pointer
	.size			dd ? ; Number of bytes buffered
	.end:
}

virtual at 0

	RING_BUFFER	RING_BUFFER

end virtual

virtual at TCP_SOCKET.end

 STREAM_SOCKET:
	.rcv	rd     RING_BUFFER.end/4
	.snd	rd     RING_BUFFER.end/4
	.end:

end virtual

struct	socket_queue_entry
	.data_ptr	dd ?
	.buf_ptr	dd ?
	.data_size	dd ?
	.size:
ends


SOCKETBUFFSIZE		equ 4096     ; in bytes

SOCKET_QUEUE_SIZE	equ 10	     ; maximum number ofincoming packets queued for 1 socket
; the incoming packet queue for sockets is placed in the socket struct itself, at this location from start
SOCKET_QUEUE_LOCATION	equ (SOCKETBUFFSIZE - SOCKET_QUEUE_SIZE*socket_queue_entry.size - queue.data)

uglobal
	net_sockets	rd 4
	last_socket_num dd ?
	last_UDP_port	dw ? ; These values give the number of the last used ephemeral port
	last_TCP_port	dw ? ;
endg


;-----------------------------------------------------------------
;
; SOCKET_init
;
;-----------------------------------------------------------------
macro	SOCKET_init {

	xor	eax, eax
	mov	edi, net_sockets
	mov	ecx, 5
	rep	stosd

       @@:
	pseudo_random eax
	cmp	ax, MIN_EPHEMERAL_PORT
	jb	@r
	cmp	ax, MAX_EPHEMERAL_PORT
	ja	@r
	mov	[last_UDP_port], ax

       @@:
	pseudo_random eax
	cmp	ax, MIN_EPHEMERAL_PORT
	jb	@r
	cmp	ax, MAX_EPHEMERAL_PORT
	ja	@r
	mov	[last_TCP_port], ax

}


;-----------------------------------------------------------------
;
; Socket API (function 74)
;
;-----------------------------------------------------------------
align 16
sock_sysfn_table:
	dd	SOCKET_open	; 0
	dd	SOCKET_close	; 1
	dd	SOCKET_bind	; 2
	dd	SOCKET_listen	; 3
	dd	SOCKET_connect	; 4
	dd	SOCKET_accept	; 5
	dd	SOCKET_send	; 6
	dd	SOCKET_receive	; 7
	dd	SOCKET_set_opt	; 8
	dd	SOCKET_get_opt	; 9

SOCKET_SYSFUNCS = ($ - sock_sysfn_table)/4


align 4
sys_socket:
	cmp	ebx, SOCKET_SYSFUNCS-1
	ja	@f
	jmp	dword [sock_sysfn_table + 4*ebx]
       @@:
	cmp	ebx, 255
	jz	SOCKET_debug

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

	DEBUGF	1,"SOCKET_open: domain: %u, type: %u protocol: %x\n", ecx, edx, esi

	call	SOCKET_alloc
	jz	s_error

	mov	[esp+32], edi			; return socketnumber

	mov	[eax + SOCKET.Domain], ecx
	mov	[eax + SOCKET.Type], edx
	mov	[eax + SOCKET.Protocol], esi

	cmp	ecx, AF_INET4
	jne	.no_inet4

	cmp	edx, SOCK_DGRAM
	je	.udp

	cmp	edx, SOCK_STREAM
	je	.tcp

	cmp	edx, SOCK_RAW
	je	.raw

  .no_inet4:
	ret

align 4
  .raw:
	test	esi, esi       ; IP_PROTO_IP
	jz	.ip

	cmp	esi, IP_PROTO_ICMP
	je	.icmp

	cmp	esi, IP_PROTO_UDP
	je	.udp

	cmp	esi, IP_PROTO_TCP
	je	.tcp

	ret

align 4
  .udp:
	mov	[eax + SOCKET.Protocol], IP_PROTO_UDP
	mov	[eax + SOCKET.snd_proc], SOCKET_send_udp
	mov	[eax + SOCKET.rcv_proc], SOCKET_receive_dgram
	ret

align 4
  .tcp:
	mov	[eax + SOCKET.Protocol], IP_PROTO_TCP
	mov	[eax + SOCKET.snd_proc], SOCKET_send_tcp
	mov	[eax + SOCKET.rcv_proc], SOCKET_receive_tcp


	mov	[eax + TCP_SOCKET.t_maxseg], 1480	;;;;; FIXME
	ret


align 4
  .ip:
	mov	[eax + SOCKET.snd_proc], SOCKET_send_ip
	mov	[eax + SOCKET.rcv_proc], SOCKET_receive_dgram
	ret


align 4
  .icmp:
	mov	[eax + SOCKET.snd_proc], SOCKET_send_icmp
	mov	[eax + SOCKET.rcv_proc], SOCKET_receive_dgram
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
	jb	s_error

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
	jb	s_error

	pushw	[edx + 2]
	pop	[eax + UDP_SOCKET.LocalPort]

	pushd	[edx + 4]
	pop	[eax + IP_SOCKET.LocalIP]

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

	DEBUGF	1,"SOCKET_connect: socknum: %u sockaddr: %x, length: %u\n", ecx, edx, esi

	call	SOCKET_num_to_ptr
	jz	s_error

	cmp	esi, 8
	jb	s_error

	cmp	word [edx], AF_INET4
	je	.af_inet4

	jmp	s_error

  .af_inet4:
	cmp	[eax + IP_SOCKET.LocalIP], 0
	jne	@f
	push	[IP_LIST]
	pop	[eax + IP_SOCKET.LocalIP]
       @@:

	cmp	[eax + SOCKET.Protocol], IP_PROTO_UDP
	je	.udp

	cmp	[eax + SOCKET.Protocol], IP_PROTO_TCP
	je	.tcp

	cmp	[eax + SOCKET.Protocol], IP_PROTO_IP
	je	.ip

	cmp	[eax + SOCKET.Protocol], IP_PROTO_ICMP
	je	.ip

	jmp	s_error

align 4
  .udp:
	lea	ebx, [eax + SOCKET.lock]
	call	wait_mutex

	pushw	[edx + 2]
	pop	[eax + UDP_SOCKET.RemotePort]

	pushd	[edx + 4]
	pop	[eax + IP_SOCKET.RemoteIP]

	cmp	[eax + UDP_SOCKET.LocalPort], 0
	jne	@f
	call	SOCKET_find_port
       @@:

	mov	[eax + UDP_SOCKET.firstpacket], 0

	push	eax
	init_queue (eax + SOCKET_QUEUE_LOCATION)	; Set up data receiving queue
	pop	eax

	mov	[eax + SOCKET.lock], 0
	mov	dword [esp+32], 0
	ret

align 4
  .tcp:
	lea	ebx, [eax + SOCKET.lock]
	call	wait_mutex

	pushw	[edx + 2]
	pop	[eax + TCP_SOCKET.RemotePort]

	pushd	[edx + 4]
	pop	[eax + IP_SOCKET.RemoteIP]

	cmp	[eax + TCP_SOCKET.LocalPort], 0
	jne	@f
	call	SOCKET_find_port
       @@:

	mov	[eax + TCP_SOCKET.timer_persist], 0
	mov	[eax + TCP_SOCKET.t_state], TCPS_SYN_SENT
	push	[TCP_sequence_num]
	add	[TCP_sequence_num], 6400
	pop	[eax + TCP_SOCKET.ISS]
	mov	[eax + TCP_SOCKET.timer_keepalive], TCP_time_keep_init

	TCP_sendseqinit eax

;        mov     [ebx + TCP_SOCKET.timer_retransmission],   ;; todo: create macro to set retransmission timer

	mov	ebx, eax

	lea	eax, [ebx + STREAM_SOCKET.snd]
	call	SOCKET_ring_create

	lea	eax, [ebx + STREAM_SOCKET.rcv]
	call	SOCKET_ring_create

	mov	[ebx + SOCKET.lock], 0

	mov	eax, ebx
	call	TCP_output

	mov	dword [esp+32], 0
	ret

align 4
  .ip:
	lea	ebx, [eax + SOCKET.lock]
	call	wait_mutex

	pushd	[edx + 4]
	pop	[eax + IP_SOCKET.RemoteIP]

	push	eax
	init_queue (eax + SOCKET_QUEUE_LOCATION)	; Set up data receiving queue
	pop	eax

	mov	[eax + SOCKET.lock], 0
	mov	dword [esp+32], 0
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

	DEBUGF	1,"SOCKET_listen: socknum: %u backlog: %u\n", ecx, edx

	call	SOCKET_num_to_ptr
	jz	s_error

	cmp	word [eax + SOCKET.Domain], AF_INET4
	jne	s_error

	cmp	[eax + SOCKET.Protocol], IP_PROTO_TCP
	jne	s_error

	cmp	[eax + TCP_SOCKET.LocalPort], 0
	je	s_error

	cmp	[eax + IP_SOCKET.LocalIP], 0
	jne	@f
	push	[IP_LIST]
	pop	[eax + IP_SOCKET.LocalIP]
       @@:

	cmp	edx, MAX_backlog
	jbe	@f
	mov	edx, MAX_backlog
       @@:

	mov	[eax + SOCKET.backlog], dx
	or	[eax + SOCKET.options], SO_ACCEPTCON
	mov	[eax + TCP_SOCKET.t_state], TCPS_LISTEN

	push	eax
	init_queue (eax + SOCKET_QUEUE_LOCATION)		; Set up sockets queue
	pop	eax

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

	DEBUGF	1,"SOCKET_accept: socknum: %u sockaddr: %x, length: %u\n", ecx, edx, esi

	call	SOCKET_num_to_ptr
	jz	s_error

	test	[eax + SOCKET.options], SO_ACCEPTCON
	jz	s_error

	cmp	word [eax + SOCKET.Domain], AF_INET4
	jne	s_error

	cmp	[eax + SOCKET.Protocol], IP_PROTO_TCP
	jne	s_error

	get_from_queue (eax + SOCKET_QUEUE_LOCATION), MAX_backlog, 4, s_error

	mov	eax, [esi]
	call	SOCKET_ptr_to_num
	jz	s_error
	mov	dword [esp+32], eax
	ret


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

	DEBUGF	1,"SOCKET_close: socknum: %u\n", ecx

	call	SOCKET_num_to_ptr
	jz	s_error

	cmp	[eax + SOCKET.Domain], AF_INET4
	jne	s_error

	cmp	[eax + SOCKET.Protocol], IP_PROTO_UDP
	je	.free

	cmp	[eax + SOCKET.Protocol], IP_PROTO_ICMP
	je	.free

	cmp	[eax + SOCKET.Protocol], IP_PROTO_IP
	je	.free

	cmp	[eax + SOCKET.Protocol], IP_PROTO_TCP
	je	.tcp

	jmp	s_error

  .tcp:
	cmp	[eax + TCP_SOCKET.t_state], TCPS_SYN_RECEIVED	 ; state must be LISTEN, SYN_SENT or CLOSED
	jb	.free

	call	TCP_output
	mov	dword [esp+32], 0

	ret

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

	DEBUGF	1,"SOCKET_receive: socknum: %u bufaddr: %x, buflength: %u, flags: %x, ", ecx, edx, esi, edi

	call	SOCKET_num_to_ptr
	jz	s_error

	jmp	[eax + SOCKET.rcv_proc]


align 4
SOCKET_receive_dgram:

	DEBUGF	1,"SOCKET_receive: DGRAM\n"

	mov	ebx, esi
	mov	edi, edx					; addr to buffer

	get_from_queue (eax + SOCKET_QUEUE_LOCATION), SOCKET_QUEUE_SIZE, socket_queue_entry.size, s_error	; destroys esi and ecx

	mov	ecx, [esi + socket_queue_entry.data_size]
	DEBUGF	1,"Got %u bytes of data\n", ecx

	cmp	ecx, ebx
	ja	.too_small

	push	[esi + socket_queue_entry.buf_ptr]		; save the buffer addr so we can clear it later
	mov	esi, [esi + socket_queue_entry.data_ptr]
	DEBUGF	1,"Source buffer: %x, real addr: %x\n", [esp], esi
	mov	dword[esp+32+4], ecx				; return number of bytes copied

; copy the data
	shr	ecx, 1
	jnc	.nb
	movsb
  .nb:
	shr	ecx, 1
	jnc	.nw
	movsw
  .nw:
	test	ecx, ecx
	jz	.nd
	rep	movsd
  .nd:

	call	kernel_free					; remove the packet
	ret

  .too_small:

	DEBUGF	1,"Buffer too small...\n"
	jmp	s_error

align 4
SOCKET_receive_tcp:

	DEBUGF	1,"SOCKET_receive: TCP\n"

	mov	ecx, esi
	mov	edi, edx
	add	eax, STREAM_SOCKET.rcv
	call	SOCKET_ring_read
	call	SOCKET_ring_free

	mov	dword[esp+32], ecx				; return number of bytes copied

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

	DEBUGF	1,"SOCKET_send: socknum: %u data ptr: %x, length: %u, flags: %x, ", ecx, edx, esi, edi

	call	SOCKET_num_to_ptr
	jz	s_error

	mov	ecx, esi
	mov	esi, edx

	jmp	[eax + SOCKET.snd_proc]


align 4
SOCKET_send_udp:

	DEBUGF	1,"SOCKET_send: UDP\n"

	call	UDP_output

	mov	dword [esp+32], 0
	ret


align 4
SOCKET_send_tcp:

	DEBUGF	1,"SOCKET_send: TCP\n"

	push	eax
	add	eax, STREAM_SOCKET.snd
	call	SOCKET_ring_write
	pop	eax

	call	TCP_output

	mov	[esp+32], eax
	ret


align 4
SOCKET_send_ip:

	DEBUGF	1,"type: IP\n"

	call	IPv4_output_raw

	mov	[esp+32], eax
	ret

align 4
SOCKET_send_icmp:

	DEBUGF	1,"SOCKET_send: ICMP\n"

	call	ICMP_output_raw

	mov	dword [esp+32], 0
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
; Also pseudo-optname -3 is valid and returns socket state, one of TCPS_*.
;
;-----------------------------------------------------------------
align 4
SOCKET_get_opt:

	DEBUGF	1,"SOCKET_get_opt\n"

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




align 4
SOCKET_set_opt:

	ret



;-----------------------------------------------------------------
;
; SOCKET_debug
;
;  Copies socket variables to application buffer
;
;  IN:  ecx = socket number
;       edx = pointer to buffer
;
;  OUT: -1 on error
;-----------------------------------------------------------------
align 4
SOCKET_debug:

	DEBUGF	1,"socket_debug\n"

	call	SOCKET_num_to_ptr
	jz	s_error

	mov	esi, eax
	mov	edi, edx
	mov	ecx, SOCKETBUFFSIZE/4
	rep	movsd

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

	DEBUGF	1,"SOCKET_find_port\n"

	push	ebx esi ecx

	cmp	[eax + SOCKET.Protocol], IP_PROTO_UDP
	je	.udp

	cmp	[eax + SOCKET.Protocol], IP_PROTO_TCP
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
; SOCKET_check_port (to be used with AF_INET only!)
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

	DEBUGF	1,"SOCKET_check_port\n"

	mov	ecx, [eax + SOCKET.Protocol]
	mov	esi, net_sockets

  .next_socket:
	mov	esi, [esi + SOCKET.NextPtr]
	or	esi, esi
	jz	.port_ok

	cmp	[esi + SOCKET.Protocol], ecx
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
; Updates a (stateless) socket with received data
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

	DEBUGF	1,"SOCKET_input: socket=%x, data=%x size=%u\n", eax, esi, ecx

	mov	dword[esp+4], ecx
	push	esi
	mov	esi, esp

	add_to_queue (eax + SOCKET_QUEUE_LOCATION), SOCKET_QUEUE_SIZE, socket_queue_entry.size, SOCKET_input.full

	DEBUGF	1,"SOCKET_input: queued packet successfully\n"
	add	esp, socket_queue_entry.size
	mov	[eax + SOCKET.lock], 0
	jmp	SOCKET_notify_owner

  .full:
	DEBUGF	2,"SOCKET_input: socket %x is full!\n", eax
	mov	[eax + SOCKET.lock], 0
	call	kernel_free
	add	esp, 8

	ret


;--------------------------
;
; eax = ptr to ring struct (just a buffer of the right size)
;
align 4
SOCKET_ring_create:

	push	esi
	mov	esi, eax

	push	edx
	stdcall create_ring_buffer, SOCKET_MAXDATA, PG_SW
	pop	edx

	DEBUGF	1,"SOCKET_ring_created: %x\n", eax
	mov	[esi + RING_BUFFER.start_ptr], eax
	mov	[esi + RING_BUFFER.write_ptr], eax
	mov	[esi + RING_BUFFER.read_ptr], eax
	mov	[esi + RING_BUFFER.size], 0
	add	eax,  SOCKET_MAXDATA
	mov	[esi + RING_BUFFER.end_ptr], eax
	mov	eax, esi
	pop	esi

	ret

;-----------------------------------------------------------------
;
; SOCKET_ring_write
;
; Adds data to a stream socket, and updates write pointer and size
;
;  IN:  eax = ptr to ring struct
;       ecx = data size
;       esi = ptr to data
;
;  OUT: ecx = number of bytes stored
;
;-----------------------------------------------------------------
align 4
SOCKET_ring_write:

	DEBUGF	1,"SOCKET_ring_write: ringbuff=%x ptr=%x size=%u\n", eax, esi, ecx

	add	[eax + RING_BUFFER.size], ecx
	cmp	[eax + RING_BUFFER.size], SOCKET_MAXDATA
	ja	.too_large

  .copy:
	mov	edi, [eax + RING_BUFFER.write_ptr]
	DEBUGF	2,"SOCKET_ring_write: %u bytes from %x to %x\n", ecx, esi, edi

	push	ecx
	shr	ecx, 1
	jnc	.nb
	movsb
  .nb:
	shr	ecx, 1
	jnc	.nw
	movsw
  .nw:
	test	ecx, ecx
	jz	.nd
	rep	movsd
  .nd:
	pop	ecx

	cmp	edi, [eax + RING_BUFFER.end_ptr]
	jae	.wrap
	mov	[eax + RING_BUFFER.write_ptr], edi

	ret

  .wrap:
	sub	edi, SOCKET_MAXDATA
	mov	[eax + RING_BUFFER.write_ptr], edi

	ret

  .too_large:
	mov	ecx, SOCKET_MAXDATA				; calculate number of bytes available in buffer
	sub	ecx, [eax + RING_BUFFER.size]
	jae	.full

	mov	[eax + RING_BUFFER.size], SOCKET_MAXDATA	; update size, we will fill buffer completely
	jmp	.copy

  .full:
	DEBUGF	2,"SOCKET_ring_write: ring buffer is full!\n"
	xor	ecx, ecx
	ret


;-----------------------------------------------------------------
;
; SOCKET_ring_read
;
; reads the data, BUT DOES NOT CLEAR IT FROM MEMORY YET
;
;  IN:  eax = ptr to ring struct
;       ecx = buffer size
;       edi = ptr to buffer
;
;  OUT: ecx = number of bytes read
;
;-----------------------------------------------------------------
align 4
SOCKET_ring_read:

	DEBUGF	1,"SOCKET_ring_read: ringbuff=%x ptr=%x size=%u\n", eax, edi, ecx

	cmp	ecx, [eax + RING_BUFFER.size]
	ja	.less_data

  .copy:
	mov	esi, [eax + RING_BUFFER.read_ptr]

	DEBUGF	2,"SOCKET_ring_read: %u bytes from %x to %x\n", ecx, esi, edi
	push	ecx
	shr	ecx, 1
	jnc	.nb
	movsb
  .nb:
	shr	ecx, 1
	jnc	.nw
	movsw
  .nw:
	test	ecx, ecx
	jz	.nd
	rep	movsd
  .nd:
	pop	ecx

  .no_data_at_all:
	ret

  .less_data:
	mov	ecx, [eax + RING_BUFFER.size]
;        test    ecx, ecx
;        jz      .no_data_at_all
	jmp	.copy


;-----------------------------------------------------------------
;
; SOCKET_ring_free
;
; Free's some bytes from the ringbuffer
;
;  IN:  eax = ptr to ring struct
;       ecx = data size
;
;  OUT: ecx = number of bytes free-ed
;
;-----------------------------------------------------------------
align 4
SOCKET_ring_free:

	DEBUGF	1,"SOCKET_ring_free: %u bytes from ring %x\n", ecx, eax

	sub	[eax + RING_BUFFER.size], ecx
	jb	.sumthinwong
	add	[eax + RING_BUFFER.read_ptr], ecx

	mov	edx, [eax + RING_BUFFER.end_ptr]
	cmp	[eax + RING_BUFFER.read_ptr], edx
	jb	@f
	sub	[eax + RING_BUFFER.read_ptr], SOCKET_MAXDATA
       @@:
	ret

  .sumthinwong: 	       ; we could free all available bytes, but that would be stupid, i guess..
	add	[eax + RING_BUFFER.size], ecx
	xor	ecx, ecx
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

	DEBUGF	1,"SOCKET_notify_owner: %x\n", eax

	call	SOCKET_check
	jz	.error

	push	eax ecx esi

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

	DEBUGF	1,"SOCKET_notify_owner: succes!\n"

  .error2:
	pop	esi ecx eax

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
	DEBUGF	1, "SOCKET_alloc: ptr=%x\n", eax
	or	eax, eax
	jz	.exit

; zero-initialize allocated memory
	push	eax edi
	mov	edi, eax
	mov	ecx, SOCKETBUFFSIZE / 4
	xor	eax, eax
	rep	stosd
	pop	edi eax

; set send-and receive procedures to return -1
	mov	[eax + SOCKET.snd_proc], s_error
	mov	[eax + SOCKET.rcv_proc], s_error

; find first free socket number and use it
	mov	ecx, [last_socket_num]
  .next_socket_number:
	inc	ecx
	jz	.next_socket_number	; avoid socket nr 0
	cmp	ecx, -1
	je	.next_socket_number	; avoid socket nr -1
	mov	ebx, net_sockets
  .next_socket:
	mov	ebx, [ebx + SOCKET.NextPtr]
	test	ebx, ebx
	jz	.last_socket

	cmp	[ebx + SOCKET.Number], ecx
	jne	.next_socket
	jmp	.next_socket_number

  .last_socket:
	mov	[last_socket_num], ecx
	mov	[eax + SOCKET.Number], ecx
	DEBUGF	1, "SOCKET_alloc: number=%u\n", ecx
	mov	edi, ecx

; Fill in PID
	mov	ebx, [TASK_BASE]
	mov	ebx, [ebx + TASKDATA.pid]
	mov	[eax + SOCKET.PID], ebx

; add socket to the list by re-arranging some pointers
	mov	ebx, [net_sockets + SOCKET.NextPtr]

	mov	[eax + SOCKET.PrevPtr], net_sockets
	mov	[eax + SOCKET.NextPtr], ebx

	test	ebx, ebx
	jz	@f
	add	ebx, SOCKET.lock	; lock the next socket
	call	wait_mutex
	sub	ebx, SOCKET.lock
	mov	[ebx + SOCKET.PrevPtr], eax
	mov	[ebx + SOCKET.lock], 0	; and unlock it again
       @@:

	mov	[net_sockets + SOCKET.NextPtr], eax
	or	eax, eax		; used to clear zero flag
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

	DEBUGF	1, "SOCKET_free: %x\n", eax

	call	SOCKET_check
	jz	.error

	push	ebx
	lea	ebx, [eax + SOCKET.lock]
	call	wait_mutex

	DEBUGF	1, "SOCKET_free: freeing socket..\n"

	cmp	[eax + SOCKET.Domain], AF_INET4
	jnz	.no_tcp

	cmp	[eax + SOCKET.Protocol], IP_PROTO_TCP
	jnz	.no_tcp

	mov	ebx, eax
	stdcall kernel_free, [ebx + STREAM_SOCKET.rcv + RING_BUFFER.start_ptr]
	stdcall kernel_free, [ebx + STREAM_SOCKET.snd + RING_BUFFER.start_ptr]
	mov	eax, ebx
  .no_tcp:

	push	eax				; this will be passed to kernel_free
	mov	ebx, [eax + SOCKET.NextPtr]
	mov	eax, [eax + SOCKET.PrevPtr]

	DEBUGF	1, "SOCKET_free: linking socket %x to socket %x\n", eax, ebx

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

	DEBUGF	1, "SOCKET_free: success!\n"

  .error:
	ret

;------------------------------------
;
; SOCKET_fork
;
; Create a child socket
;
; IN:  socket nr in ebx
; OUT: child socket nr in eax
;
;-----------------------------------
align 4
SOCKET_fork:

	DEBUGF	1,"SOCKET_fork: %x\n", ebx

; Exit if backlog queue is full
	mov	eax, [ebx + SOCKET_QUEUE_LOCATION + queue.size]
	cmp	ax, [ebx + SOCKET.backlog]
	jae	.fail

; Allocate new socket
	call	SOCKET_alloc
	jz	.fail

	push	esi ecx edi
	push	eax
	mov	esi, esp
	add_to_queue (ebx + SOCKET_QUEUE_LOCATION), MAX_backlog, 4, .fail2
	pop	eax

; Copy structure from current socket to new
; We start at PID to preserve the socket num, and the 2 pointers at beginning of socket
	lea	esi, [ebx + SOCKET.PID]
	lea	edi, [eax + SOCKET.PID]
	mov	ecx, (SOCKET_QUEUE_LOCATION - SOCKET.PID + 3)/4
	rep	movsd

	and	[eax + SOCKET.options], not SO_ACCEPTCON
	pop	edi ecx esi

	ret

  .fail2:
	add	esp, 4+4+4
  .fail:
	DEBUGF	1,"SOCKET_fork: failed\n"
	xor	eax, eax
	ret


;---------------------------------------------------
;
; SOCKET_num_to_ptr
;
; Get socket structure address by its number
;
; IN:  ecx = socket number
; OUT: eax = 0 on error, socket ptr otherwise
;       ZF = set on error
;
;---------------------------------------------------
align 4
SOCKET_num_to_ptr:

	DEBUGF	1,"SOCKET_num_to_ptr: %u ", ecx

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

	DEBUGF	1,"SOCKET_ptr_to_num: %x ", eax

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

	DEBUGF	1,"SOCKET_check: %x\n", eax

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

	DEBUGF	1,"SOCKET_check_owner: %x\n", eax

	push	ebx
	mov	ebx, [TASK_BASE]
	mov	ebx, [ecx + TASKDATA.pid]
	cmp	[eax + SOCKET.PID], ebx
	pop	 ebx

	ret




;------------------------------------------------------
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

	DEBUGF	1,"SOCKET_process_end: %x\n", eax

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

	cmp	[ebx + SOCKET.Protocol], IP_PROTO_UDP
	je	.udp

	cmp	[ebx + SOCKET.Protocol], IP_PROTO_TCP
	je	.tcp

	jmp	.next_socket	; kill all sockets for given PID

  .udp:
	mov	eax, ebx
	mov	ebx, [ebx + SOCKET.NextPtr]
	call	SOCKET_free
	jmp	.test_socket

  .tcp:

	;;; TODO

	jmp	.next_socket

  .done:
	pop	ebx

	ret




;-----------------------------------------------------------------
;
; SOCKET_is_connecting
;
;  IN:  eax = socket ptr
;  OUT: /
;
;-----------------------------------------------------------------

align 4
SOCKET_is_connecting:


	and	[eax + SOCKET.options], not (SS_ISCONNECTED + SS_ISDISCONNECTING + SS_ISCONFIRMING)
	or	[eax + SOCKET.options], SS_ISCONNECTING

	jmp	SOCKET_notify_owner



;-----------------------------------------------------------------
;
; SOCKET_is_connected
;
;  IN:  eax = socket ptr
;  OUT: /
;
;-----------------------------------------------------------------

align 4
SOCKET_is_connected:


	and	[eax + SOCKET.options], not (SS_ISCONNECTING + SS_ISDISCONNECTING + SS_ISCONFIRMING)
	or	[eax + SOCKET.options], SS_ISCONNECTED

	jmp	SOCKET_notify_owner




;-----------------------------------------------------------------
;
; SOCKET_is_disconnecting
;
;  IN:  eax = socket ptr
;  OUT: /
;
;-----------------------------------------------------------------

align 4
SOCKET_is_disconnecting:

	and	[eax + SOCKET.options], not (SS_ISCONNECTING)
	or	[eax + SOCKET.options], SS_ISDISCONNECTING + SS_CANTRCVMORE + SS_CANTSENDMORE

	jmp	SOCKET_notify_owner



;-----------------------------------------------------------------
;
; SOCKET_is_disconnected
;
;  IN:  eax = socket ptr
;  OUT: /
;
;-----------------------------------------------------------------

align 4
SOCKET_is_disconnected:

	and	[eax + SOCKET.options], not (SS_ISCONNECTING + SS_ISCONNECTED + SS_ISDISCONNECTING)
	or	[eax + SOCKET.options], SS_CANTRCVMORE + SS_CANTSENDMORE

	jmp	SOCKET_notify_owner


;-----------------------------------------------------------------
;
; SOCKET_cant_recv_more
;
;  IN:  eax = socket ptr
;  OUT: /
;
;-----------------------------------------------------------------

align 4
SOCKET_cant_recv_more:

	or	[eax + SOCKET.options], SS_CANTRCVMORE

	ret



;-----------------------------------------------------------------
;
; SOCKET_cant_send_more
;
;  IN:  eax = socket ptr
;  OUT: /
;
;-----------------------------------------------------------------

align 4
SOCKET_cant_send_more:

	or	[eax + SOCKET.options], SS_CANTSENDMORE

	ret