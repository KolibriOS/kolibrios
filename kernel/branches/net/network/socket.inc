;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2009. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  SOCKET.INC                                                     ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;    based on code by mike.dld                                    ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

struct	SOCKET_head
	 .PrevPtr		dd ? ; pointer to previous socket in list
	 .NextPtr		dd ? ; pointer to next socket in list
	 .Number		dd ? ; socket number (unique within single process)
	 .PID			dd ? ; application process id
	 .Domain		dd ? ; INET/UNIX/..
	 .Type			dd ? ; RAW/UDP/TCP/...
	 .Protocol		dd ? ; ICMP/IPv4/ARP/
	 .lock			dd ? ; lock mutex
	 .end:
ends

struct	IPv4_SOCKET
	 .LocalIP		dd ?
	 .RemoteIP		dd ?
	 .SequenceNumber	dd ?

	; todo: add options (for func 8 and 9)

	 .end:
ends

struct	TCP_SOCKET

	 .LocalPort		dw ? ; In INET byte order
	 .RemotePort		dw ? ; In INET byte order

	 .backlog		dw ? ; Backlog
	 .backlog_cur		dw ? ; current size of queue for un-accept-ed connections
	 .last_ack_number	dd ? ; used only to let application know that ACK has been received
					; todo: may be use SND_UNA instead
					; todo: may be use events which allow additional information instead
					; todo: may be count acknowledged bytes (at least it has obvious sense)
	 .OrigRemoteIP		dd ? ; original remote IP address (used to reset to LISTEN state)
	 .OrigRemotePort	dw ? ; original remote port (used to reset to LISTEN state)
	 .wndsizeTimer		dd ? ; window size timer

	; Transmission control block
	 .state 		dd ? ; TCB state
	 .timer 		dd ? ; TCB timer (seconds)
	 .ISS			dd ? ; initial send sequence number
	 .IRS			dd ? ; initial receive sequence number
	 .SND_UNA		dd ? ; sequence number of unack'ed sent Packets
	 .SND_NXT		dd ? ; next send sequence number to use
	 .SND_WND		dd ? ; send window
	 .RCV_NXT		dd ? ; next receive sequence number to use
	 .RCV_WND		dd ? ; receive window
	 .SEG_LEN		dd ? ; segment length
	 .SEG_WND		dd ? ; segment window

	 .flags 		db ? ; packet flags

	 .end:
ends

struct	UDP_SOCKET

	 .LocalPort		dw ? ; In INET byte order
	 .RemotePort		dw ? ; In INET byte order

	 .end:
ends

struct	ICMP_SOCKET

	.Identifier		dw ? ;

	.end:

ends

struct	IPC_SOCKET

	.ConnectedTo		dd ? ; Socket number of other socket this one is connected to

	.end:

ends

struct	socket_queue_entry
	.data_ptr	dd ?
	.data_size	dd ?
	.offset 	dd ?
	.size:
ends

MAX_backlog		equ 20	     ; backlog for stream sockets
SOCKETBUFFSIZE		equ 4096     ; in bytes
SOCKET_QUEUE_SIZE	equ 10	     ; maximum number ofincoming packets queued for 1 socket
SOCKET_QUEUE_LOCATION	equ 2048     ; the incoming packet queue for sockets is placed in the socket struct itself, at this location from start

uglobal
	net_sockets	rd 2
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

	mov	[net_sockets], 0
	mov	[net_sockets + 4], 0

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
	and	ebx, 0x000000FF ; should i remove this line ?
	cmp	bl , 8		; highest possible number
	jg	s_error
	lea	ebx, [.table + 4*ebx]
	jmp	dword [ebx]

.table:
	dd	socket_open	; 0
	dd	socket_close	; 1
	dd	socket_bind	; 2
	dd	socket_listen	; 3
	dd	socket_connect	; 4
	dd	socket_accept	; 5
	dd	socket_send	; 6
	dd	socket_recv	; 7
	dd	socket_get_opt	; 8
;        dd      socket_set_opt  ; 9


s_error:
	mov	dword [esp+32],-1

	ret


;-----------------------------------------------------------------
;
; SOCKET_open
;
;
;  IN:  domain in ecx
;       type in edx
;       protocol in esi
;  OUT: eax is socket num, -1 on error
;
;-----------------------------------------------------------------
align 4
socket_open:

	DEBUGF	1,"socket_open: domain: %u, type: %u",ecx, edx

	call	net_socket_alloc
	or	eax, eax
	jz	s_error

	mov	[eax + SOCKET_head.Domain], ecx
	mov	[eax + SOCKET_head.Type], edx
	mov	[eax + SOCKET_head.Protocol], esi

	stdcall net_socket_addr_to_num, eax
	DEBUGF	1,", socketnumber: %u\n", eax

	; TODO: if it is a tcp socket, set state to TCB_CLOSED

	mov	[esp+32], eax

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
socket_bind:

	DEBUGF	1,"Socket_bind: socknum: %u sockaddr: %x, length: %u, ",ecx,edx,esi

	stdcall net_socket_num_to_addr, ecx
	cmp	eax, -1
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

	mov	dword [esp+32],0
	ret

  .af_inet4:

	cmp	esi, 6
	jl	s_error

	mov	ecx, [eax + SOCKET_head.Type]

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
	mov	word [eax + SOCKET_head.end + IPv4_SOCKET.end + UDP_SOCKET.LocalPort], bx

	mov	ebx, dword [edx + 4]
	mov	dword [eax + SOCKET_head.end + IPv4_SOCKET.LocalIP], ebx

	DEBUGF	1,"local ip: %u.%u.%u.%u\n",\
	[eax + SOCKET_head.end + IPv4_SOCKET.LocalIP + 0]:1,[eax + SOCKET_head.end + IPv4_SOCKET.LocalIP + 1]:1,\
	[eax + SOCKET_head.end + IPv4_SOCKET.LocalIP + 2]:1,[eax + SOCKET_head.end + IPv4_SOCKET.LocalIP + 3]:1

	mov	dword [esp+32],0
	ret




;-----------------------------------------------------------------
;
; SOCKET_connect
;
;
;  IN:  socket number in ecx
;       pointer to sockaddr struct in edx
;       length of that struct in esi
;  OUT: 0 on success
;
;-----------------------------------------------------------------
align 4
socket_connect:

	DEBUGF	1,"Socket_connect: socknum: %u sockaddr: %x, length: %u,",ecx,edx,esi

	stdcall net_socket_num_to_addr, ecx
	cmp	eax, -1
	jz	s_error

	cmp	esi, 8
	jl	s_error

	cmp	word [edx], AF_INET4
	je	.af_inet4

	jmp	s_error

  .af_inet4:

	cmp	[eax + SOCKET_head.Type], IP_PROTO_UDP
	je	.udp

	cmp	[eax + SOCKET_head.Type], IP_PROTO_TCP
	je	.tcp

	jmp	s_error

  .udp:

	mov	bx , word [edx + 2]
	mov	word [eax + SOCKET_head.end + IPv4_SOCKET.end + UDP_SOCKET.RemotePort], bx
	DEBUGF	1,"remote port: %x ",bx

	mov	ebx, dword [edx + 4]
	mov	dword [eax + SOCKET_head.end + IPv4_SOCKET.RemoteIP], ebx
	DEBUGF	1,"remote ip: %u.%u.%u.%u\n",[edx+4]:1,[edx+5]:1,[edx+6]:1,[edx+7]:1

	mov	dword [esp+32],0
	ret


  .tcp:
	; TODO: set sequence number to random value


	; fill in remote port and IP

	mov	[eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.wndsizeTimer], 0	   ; Reset the window timer.
											   ; TODO: figure out WTF this is
	mov	bx , word [edx + 2]
	mov	[eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.RemotePort], bx
	DEBUGF	1,"remote port: %x ",bx

	mov	ebx, dword [edx + 4]
	mov	[eax + SOCKET_head.end + IPv4_SOCKET.RemoteIP], ebx

	; check if local port and IP is ok

	cmp	[eax + SOCKET_head.end + IPv4_SOCKET.LocalIP], 0
	jne	@f
	push	[IP_LIST]	    ; device zero = default
	pop	[eax + SOCKET_head.end + IPv4_SOCKET.LocalIP]
       @@:

	cmp	[eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.LocalPort], 0
	jne	@f

	mov	ecx, [eax + SOCKET_head.Type]
	call	socket_find_port
	test	bx, bx
	jz	s_error

	mov	[eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.LocalPort], bx
       @@:


	mov	[eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_SYN_SENT
	; now say hello to the remote tcp socket

	mov	bl, TH_SYN
	xor	ecx, ecx
	call	TCP_send

	mov	dword [esp+32],0
	ret


;-----------------------------------------------------------------
;
; SOCKET_listen
;
;
;  IN:  socket number in ecx
;       backlog in edx
;  OUT: eax is socket num, -1 on error
;
;-----------------------------------------------------------------
align 4
socket_listen:

	DEBUGF	1,"Socket_listen: socknum: %u backlog: %u\n",ecx,edx

	stdcall net_socket_num_to_addr, ecx
	cmp	eax, -1
	jz	s_error

	cmp	word [eax + SOCKET_head.Domain], AF_INET4
	jne	s_error

	cmp	[eax + SOCKET_head.Type], IP_PROTO_TCP
	jne	s_error

	cmp	edx, MAX_backlog
	jb	.ok
	mov	dx , MAX_backlog
  .ok:

	mov	[eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.backlog], dx
	mov	[eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_LISTEN

	mov	dword [esp+32], 0
	ret


;-----------------------------------------------------------------
;
; SOCKET_accept
;
;
;  IN:  socket number in ecx
;       addr in edx
;       addrlen in esi
;  OUT: eax is socket num, -1 on error
;
;-----------------------------------------------------------------
align 4
socket_accept:

	DEBUGF	1,"Socket_accept: socknum: %u sockaddr: %x, length: %u\n",ecx,edx,esi

	stdcall net_socket_num_to_addr, ecx
	or	eax, eax
	jz	s_error
	mov	esi, eax

	cmp	word [esi + SOCKET_head.Domain], AF_INET4
	je	.af_inet4

	jmp	s_error

  .af_inet4:

	cmp	[esi + SOCKET_head.Type], IP_PROTO_TCP
	je	.tcp

	jmp	s_error

  .tcp:

	lea	ebx, [esi + SOCKET_head.lock]
	call	wait_mutex
	movzx	eax, [esi + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.backlog_cur]
	test	eax, eax
	jz	.unlock_err
	dec	[esi + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.backlog_cur]
	mov	eax, [eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.end + (eax-1)*4]
	mov	[esi + SOCKET_head.lock], 0
	stdcall net_socket_addr_to_num, eax
	mov	[esp+32], eax
	ret
  .unlock_err:
	mov	[esi + SOCKET_head.lock], 0
	jmp	s_error


;-----------------------------------------------------------------
;
; SOCKET_close
;
;
;  IN:  socket number in ecx
;  OUT: eax is socket num, -1 on error
;
;-----------------------------------------------------------------
align 4
socket_close:

	DEBUGF	1,"Socket_close: socknum: %u\n",ecx

	stdcall net_socket_num_to_addr, ecx
	or	eax, eax
	jz	s_error

	cmp	[eax + SOCKET_head.Domain], AF_INET4
	jne	s_error

	cmp	[eax + SOCKET_head.Type], IP_PROTO_UDP
	je	.udp

	cmp	[eax + SOCKET_head.Type], IP_PROTO_ICMP
	je	.icmp

	cmp	[eax + SOCKET_head.Type], IP_PROTO_TCP
	je	.tcp

	jmp	s_error

  .udp:

	stdcall net_socket_free, eax
	mov	dword [esp+32],0
	ret


  .icmp:



	ret

  .tcp:
	; first, remove all resend entries for this socket

	cmp	[eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_LISTEN
	je	.destroy_tcb
	cmp	[eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_SYN_SENT
	je	.destroy_tcb

	; Now construct the response, and queue for sending by IP

	mov	bl, TH_FIN
	xor	ecx, ecx
	call	TCP_send

	; increament SND.NXT in socket
	lea	esi, [eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.SND_NXT]
	inc_INET esi

	; Get the socket state
	cmp	[eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_SYN_RECEIVED
	je	.fin_wait_1
	cmp	[eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_ESTABLISHED
	je	.fin_wait_1

	; assume CLOSE WAIT
	; Send a fin, then enter last-ack state
	mov	[eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_LAST_ACK
	jmp	.send

  .fin_wait_1:
	; Send a fin, then enter finwait2 state
	mov	[eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.state], TCB_FIN_WAIT_1

  .send:

	;;;;;

  .destroy_tcb:

	stdcall net_socket_free, eax
	mov	dword [esp+32],0
	ret




;-----------------------------------------------------------------
;
; SOCKET_receive
;
;
;  IN:  socket number in ecx
;       addr to buffer in edx
;       length of buffer in esi
;       flags in edi
;  OUT: eax is number of bytes copied, -1 on error
;
;-----------------------------------------------------------------
align 4
socket_recv:

	DEBUGF	1,"Socket_receive: socknum: %u bufferaddress: %x, length: %u, flags: %x\n",ecx,edx,esi,edi
	stdcall net_socket_num_to_addr, ecx		   ; get real socket address
	or	eax, eax
	jz	s_error

	DEBUGF	1,"Socket pointer: %x\n", eax

	get_from_queue (eax + SOCKET_QUEUE_LOCATION), SOCKET_QUEUE_SIZE, socket_queue_entry.size, s_error

	mov	edi, edx
	mov	ecx, [esi + socket_queue_entry.data_size]

	DEBUGF	1,"Got %u bytes of data\n", ecx

	cmp	ecx, edx
	jle	.large_enough
	DEBUGF	1,"Buffer too small...\n"
	jmp	s_error
  .large_enough:

	push	[esi + socket_queue_entry.data_ptr]
	mov	esi, [esi + socket_queue_entry.offset]
	add	esi, [esp]
	DEBUGF	1,"Source buffer: %x, real addr: %x\n", [esp], esi

	mov	dword[esp+32+4], ecx			     ; return number of bytes copied

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
socket_send:

	DEBUGF	1,"Socket_send: socknum: %u sockaddr: %x, length: %u, flags: %x, ",ecx,edx,esi,edi

	stdcall net_socket_num_to_addr, ecx		   ; get real socket address
	or	eax, eax
	jz	s_error

	cmp	word [eax + SOCKET_head.Domain], AF_INET4
	je	.af_inet4

	jmp	s_error

  .af_inet4:
	DEBUGF	1,"Socket type:%u\n", [eax + SOCKET_head.Type]:4

	cmp	[eax + SOCKET_head.Type], IP_PROTO_TCP
	je	.tcp

	cmp	[eax + SOCKET_head.Type], IP_PROTO_UDP
	je	.udp

	cmp	[eax + SOCKET_head.Type], SOCK_RAW
	je	.raw

	jmp	s_error

  .udp:

	DEBUGF	1,"type: UDP, "

	cmp	[eax + SOCKET_head.end + IPv4_SOCKET.end + UDP_SOCKET.LocalPort],0
	jne	@f

	push	esi
	mov	ecx, [eax + SOCKET_head.Type]
	call	socket_find_port
	test	bx, bx
	pop	esi
	je	s_error
	mov	[eax + SOCKET_head.end + IPv4_SOCKET.end + UDP_SOCKET.LocalPort], bx

     @@:

	mov	ecx, esi
	mov	esi, edx

	call	UDP_socket_send

	and	dword [esp+32], 0
	ret

  .tcp:

	cmp	[eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.LocalPort],0
	jne	@f

	push	esi
	mov	ecx, [eax + SOCKET_head.Type]
	call	socket_find_port
	test	bx, bx
	pop	esi
	je	s_error
	mov	[eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.LocalPort], bx

     @@:

	mov	ecx, esi
	mov	esi, edx
	xor	bl , bl

	call	TCP_send

	mov	[esp+32], eax
	ret

  .raw:
	cmp	[eax + SOCKET_head.Protocol], IP_PROTO_IP
	je	.raw_ip

	cmp	[eax + SOCKET_head.Protocol], IP_PROTO_ICMP
	je	.raw_icmp

	jmp	s_error


  .raw_ip:

	;;;;;;

	mov	[esp+32], eax
	ret


  .raw_icmp:

;        sub     ecx, ICMP_Packet.Data
;        mov     esi, edx
;        push    ax
;        call    IPv4_get_frgmnt_num
;        mov     dx, ax
;        pop     ax
;        shl     edx, 16
;        mov     dh , [esi + ICMP_Packet.Type]
;        mov     dl , [esi + ICMP_Packet.Code]
;        mov     di , [esi + ICMP_Packet.Identifier]
;        mov     [eax + SOCKET.LocalPort], di            ; Set localport to the identifier number, so we can receive reply's
;        shl     edi, 16
;        mov     di , [esi + ICMP_Packet.SequenceNumber]
;        add     esi, ICMP_Packet.Data
;        mov     ebx, [eax + SOCKET.LocalIP]
;        mov     eax, [eax + SOCKET.RemoteIP]
;        call    ICMP_create_packet

	mov	[esp+32], eax
	ret

;-----------------------------------------------------------------
;
; SOCKET_get_options
;
;
;  IN:  socket number in ecx
;       edx points to the options:
;               dd      level, optname, optval, optlen
;  OUT: -1 on error
;
; At moment, uses only pseudo-optname -2 for get last_ack_number for TCP.
; TODO: find best way to notify that send()'ed data were acknowledged
;
;-----------------------------------------------------------------
align 4
socket_get_opt:

	cmp	dword [edx], IP_PROTO_TCP
	jnz	.unknown
	cmp	dword [edx+4], -2
	jnz	.unknown
	mov	eax, [edx+12]
	test	eax, eax
	jz	.fail
	cmp	dword [eax], 4
	mov	dword [eax], 4
	jb	.fail
	stdcall net_socket_num_to_addr, ecx
	test	eax, eax
	jz	.fail
	; todo: check that eax is really TCP socket
	mov	ecx, [eax + SOCKET_head.end + IPv4_SOCKET.end + TCP_SOCKET.last_ack_number]
	mov	eax, [edx+8]
	test	eax, eax
	jz	@f
	mov	[eax], ecx
@@:
	mov	dword [esp+32], 0
	ret
.fail:
.unknown:
	mov	dword [esp+32], -1
	ret


;-----------------------------------------------------------------
;
; SOCKET_find_free_port (local port)
;
; works with INET byte order
;
;  IN:  type in ecx (TCP/UDP)
;  OUT: bx = 0 on error, portnumber otherwise
;
;-----------------------------------------------------------------
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
	mov	esi, [esi + SOCKET_head.NextPtr]
	or	esi, esi
	jz	.port_ok

	cmp	[esi + SOCKET_head.Type], ecx
	jne	.next_socket

	rol	bx, 8
	cmp	[esi + SOCKET_head.end + IPv4_SOCKET.end + UDP_SOCKET.LocalPort], bx
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



;-----------------------------------------------------------------
;
; SOCKET_check_port (local port)
;
; works with INET byte order
;
;  IN:  type in ecx (TCP/UDP)
;       port to check in bx
;  OUT: bx = 0 on error, unchanged otherwise
;
;-----------------------------------------------------------------
align 4
socket_check_port:
	mov	esi, net_sockets

  .next_socket:
	mov	esi, [esi + SOCKET_head.NextPtr]
	or	esi, esi
	jz	.port_ok

	cmp	[esi + SOCKET_head.Type], ecx
	jne	.next_socket

	cmp	[esi + SOCKET_head.end + IPv4_SOCKET.end + UDP_SOCKET.LocalPort], bx
	jne	.next_socket

	xor	ebx, ebx

  .port_ok:
	ret



;-----------------------------------------------------------------
;
; SOCKET_internal_receiver
;
; Updates a socket with received data
;
; Note: the mutex must already be set !
;
;  IN:  eax = socket ptr
;       ecx = size
;       esi = pointer to buffer
;       edi = offset
;
;  OUT: xxx
;
;-----------------------------------------------------------------
align 4
socket_internal_receiver:

	DEBUGF 1,"Internal socket receiver: buffer %x, offset: %x size=%u socket: %x\n", esi, edi, ecx, eax

	push	edi	; offset
	push	ecx	; size
	push	esi	; data_ptr
	mov	esi, esp
	add_to_queue (eax + SOCKET_QUEUE_LOCATION), SOCKET_QUEUE_SIZE, socket_queue_entry.size, notify_network_event.full
	DEBUGF	1,"Queued packet successfully\n"
	add	esp, socket_queue_entry.size

	mov	[eax + SOCKET_head.lock], 0

notify_network_event:
	; flag an event to the application
	mov	edx, [eax + SOCKET_head.PID]				; get socket owner PID
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
	or	[ecx + SLOT_BASE + APPDATA.event_mask], EVENT_NETWORK	; stack event
	mov	[check_idle_semaphore], 200
	ret

  .full:
	DEBUGF 1,"Socket %x is full!\n",eax
	mov	[eax + SOCKET_head.lock], 0
	call	kernel_free
	add	esp, 8
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

	init_queue (eax + SOCKET_QUEUE_LOCATION)

	; add socket to the list by changing pointers
	mov	ebx, net_sockets
	push	[ebx + SOCKET_head.NextPtr]
	mov	[ebx + SOCKET_head.NextPtr], eax
	mov	[eax + SOCKET_head.PrevPtr], ebx
	pop	ebx
	mov	[eax + SOCKET_head.NextPtr], ebx
	or	ebx, ebx
	jz	@f
	mov	[ebx + SOCKET_head.PrevPtr], eax

    @@: ; set socket owner PID to the one of calling process
	mov	ebx, [TASK_BASE]
	mov	ebx, [ebx + TASKDATA.pid]
	mov	[eax + SOCKET_head.PID], ebx

	; find first free socket number and use it
	;mov     edx, ebx
	mov	ebx, net_sockets
	xor	ecx, ecx
  .next_socket_number:
	inc	ecx
  .next_socket:
	mov	ebx, [ebx + SOCKET_head.NextPtr]
	or	ebx, ebx
	jz	.last_socket_number
	cmp	[ebx + SOCKET_head.Number], ecx
	jne	.next_socket
	;cmp     [ebx + SOCKET.PID], edx
	;jne     .next_socket
	mov	ebx, net_sockets
	jmp	.next_socket_number

  .last_socket_number:
	mov	[eax + SOCKET_head.Number], ecx

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
	mov	ebx, [ebx + SOCKET_head.NextPtr]
	or	ebx, ebx
	jz	.error
	cmp	ebx, eax
	jne	.next_socket
	;cmp     [ebx + SOCKET.PID], ecx
	;jne     .next_socket

	; okay, we found the correct one
	; remove it from the list first, changing pointers
	mov	ebx, [eax + SOCKET_head.NextPtr]
	mov	eax, [eax + SOCKET_head.PrevPtr]
	mov	[eax + SOCKET_head.NextPtr], ebx
	or	ebx, ebx
	jz	@f
	mov	[ebx + SOCKET_head.PrevPtr], eax

	lea	ebx, [eax + SOCKET_head.lock]
	call	wait_mutex

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
	mov	ebx, [ebx + SOCKET_head.NextPtr]
	or	ebx, ebx
	jz	.error
	cmp	[ebx + SOCKET_head.Number], eax
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
	mov	ebx, [ebx + SOCKET_head.NextPtr]
	or	ebx, ebx
	jz	.error
	cmp	ebx, eax
	jne	.next_socket
	;cmp     [ebx + SOCKET.PID], ecx
	;jne     .next_socket

	; okay, we found the correct one
	mov	eax, [ebx + SOCKET_head.Number]
	ret

  .error:
	xor	eax, eax
	ret
endp
