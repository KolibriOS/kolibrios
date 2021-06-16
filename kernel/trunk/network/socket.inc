;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2021. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  Part of the TCP/IP network stack for KolibriOS                 ;;
;;                                                                 ;;
;;   Written by hidnplayr@kolibrios.org,                           ;;
;;     and Clevermouse.                                            ;;
;;                                                                 ;;
;;       Based on code by mike.dld                                 ;;
;;                                                                 ;;
;;         GNU GENERAL PUBLIC LICENSE                              ;;
;;          Version 2, June 1991                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

struct  SOCKET

        NextPtr                 dd ?    ; pointer to next socket in list
        PrevPtr                 dd ?    ; pointer to previous socket in list
        Number                  dd ?    ; socket number

        mutex                   MUTEX

        PID                     dd ?    ; process ID
        TID                     dd ?    ; thread ID
        Domain                  dd ?    ; INET4/INET6/LOCAL/..
        Type                    dd ?    ; RAW/STREAM/DGRAM
        Protocol                dd ?    ; UDP/TCP/ARP/ICMP
        errorcode               dd ?
        device                  dd ?    ; device pointer, paired socket pointer if it's a local socket

        options                 dd ?
        state                   dd ?
        backlog                 dw ?    ; number of incoming connections that can be queued

        snd_proc                dd ?
        rcv_proc                dd ?
        connect_proc            dd ?

ends

struct  IP_SOCKET               SOCKET

        LocalIP                 rd 4    ; network byte order
        RemoteIP                rd 4    ; network byte order
        ttl                     db ?
                                rb 3    ; align

ends

struct  TCP_SOCKET              IP_SOCKET

        LocalPort               dw ?    ; network byte order
        RemotePort              dw ?    ; network byte order

        t_state                 dd ?    ; TCB state
        t_rxtshift              db ?
                                rb 3    ; align
        t_rxtcur                dd ?
        t_dupacks               dd ?
        t_maxseg                dd ?
        t_flags                 dd ?

;---------------
; RFC783 page 21

; send sequence
        SND_UNA                 dd ?    ; sequence number of unack'ed sent Packets
        SND_NXT                 dd ?    ; next send sequence number to use
        SND_UP                  dd ?    ; urgent pointer
        SND_WL1                 dd ?    ; the sequence number of the last segment used to update the send window
        SND_WL2                 dd ?    ; the acknowledgment number of the last segment used to update the send window
        ISS                     dd ?    ; initial send sequence number
        SND_WND                 dd ?    ; send window

; receive sequence
        RCV_WND                 dd ?    ; receive window
        RCV_NXT                 dd ?    ; next receive sequence number to use
        RCV_UP                  dd ?    ; urgent pointer
        IRS                     dd ?    ; initial receive sequence number

;---------------------
; Additional variables

; receive variables
        RCV_ADV                 dd ?

; retransmit variables
        SND_MAX                 dd ?

; congestion control
        SND_CWND                dd ?    ; congestion window
        SND_SSTHRESH            dd ?    ; slow start threshold

;----------------------
; Transmit timing stuff
        t_idle                  dd ?
        t_rtt                   dd ?    ; round trip time
        t_rtseq                 dd ?
        t_srtt                  dd ?    ; smoothed round trip time
        t_rttvar                dd ?
        t_rttmin                dd ?
        max_sndwnd              dd ?

;-----------------
; Out-of-band data
        t_oobflags              dd ?
        t_iobc                  dd ?
        t_softerror             dd ?


;---------
; RFC 1323                              ; the order of next 4 elements may not change

        SND_SCALE               db ?
        RCV_SCALE               db ?
        requested_s_scale       db ?
        request_r_scale         db ?

        ts_recent               dd ?    ; a copy of the most-recent valid timestamp from the other end
        ts_recent_age           dd ?
        last_ack_sent           dd ?


;-------
; Timers
        timer_flags             dd ?
        timer_retransmission    dd ?    ; rexmt
        timer_persist           dd ?
        timer_keepalive         dd ?    ; keepalive/syn timeout
        timer_timed_wait        dd ?    ; also used as 2msl timer
        timer_connect           dd ?

; extra

        ts_ecr                  dd ?    ; timestamp echo reply
        ts_val                  dd ?

        seg_next                dd ?    ; re-assembly queue

ends

struct  UDP_SOCKET              IP_SOCKET

        LocalPort               dw ?    ; in network byte order
        RemotePort              dw ?    ; in network byte order

ends

struct  RING_BUFFER

        mutex                   MUTEX
        start_ptr               dd ?    ; Pointer to start of buffer
        end_ptr                 dd ?    ; pointer to end of buffer
        read_ptr                dd ?    ; Read pointer
        write_ptr               dd ?    ; Write pointer
        size                    dd ?    ; Number of bytes buffered

ends

struct  STREAM_SOCKET           TCP_SOCKET

        rcv                     RING_BUFFER
        snd                     RING_BUFFER

ends

struct  socket_queue_entry

        data_ptr                dd ?
        data_size               dd ?
        buf_ptr                 dd ?

ends

struct  socket_options

        level                   dd ?
        optname                 dd ?
        optlen                  dd ?
        optval                  dd ?

ends

SOCKET_STRUCT_SIZE      = 4096          ; in bytes

SOCKET_QUEUE_SIZE       = 10            ; maximum number of incoming packets queued for 1 socket
; the incoming packet queue for sockets is placed in the socket struct itself, at this location from start
SOCKET_QUEUE_LOCATION   = (SOCKET_STRUCT_SIZE - SOCKET_QUEUE_SIZE*sizeof.socket_queue_entry - sizeof.queue)

uglobal
align 4

        net_sockets     rd 4
        last_socket_num dd ?
        last_UDP_port   dw ?            ; last used ephemeral port
        last_TCP_port   dw ?            ;
        socket_mutex    MUTEX

endg


;-----------------------------------------------------------------;
;                                                                 ;
; socket_init                                                     ;
;                                                                 ;
;-----------------------------------------------------------------;
macro   socket_init {

        xor     eax, eax
        mov     edi, net_sockets
        mov     ecx, 5
        rep stosd

       @@:
        pseudo_random eax
        cmp     ax, EPHEMERAL_PORT_MIN
        jb      @r
        cmp     ax, EPHEMERAL_PORT_MAX
        ja      @r
        xchg    al, ah
        mov     [last_UDP_port], ax

       @@:
        pseudo_random eax
        cmp     ax, EPHEMERAL_PORT_MIN
        jb      @r
        cmp     ax, EPHEMERAL_PORT_MAX
        ja      @r
        xchg    al, ah
        mov     [last_TCP_port], ax

        mov     ecx, socket_mutex
        call    mutex_init

}

;-----------------------------------------------------------------;
;                                                                 ;
; Sockets API (system function 75)                                ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
sys_socket:

        mov     dword[esp+20], 0        ; Set error code to 0

        cmp     ebx, 255
        jz      socket_debug

        cmp     ebx, .number
        ja      .error
        jmp     dword [.table + 4*ebx]

  .table:
        dd      socket_open             ; 0
        dd      socket_close            ; 1
        dd      socket_bind             ; 2
        dd      socket_listen           ; 3
        dd      socket_connect          ; 4
        dd      socket_accept           ; 5
        dd      socket_send             ; 6
        dd      socket_receive          ; 7
        dd      socket_set_opt          ; 8
        dd      socket_get_opt          ; 9
        dd      socket_pair             ; 10
  .number = ($ - .table) / 4 - 1

  .error:
        mov     dword[esp+32], -1
        mov     dword[esp+20], EINVAL

        ret

;-----------------------------------------------------------------;
;                                                                 ;
; socket_open: Create a new socket.                               ;
;                                                                 ;
;   IN: ecx = domain                                              ;
;       edx = type                                                ;
;       esi = protocol                                            ;
;                                                                 ;
;  OUT: eax = socket number                                       ;
;       eax = -1 on error                                         ;
;       ebx = errorcode on error                                  ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_open:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_open: domain=%u type=%u protocol=%x\n", ecx, edx, esi

        push    ecx edx esi
        call    socket_alloc
        pop     esi edx ecx
        test    eax, eax
        jz      .nobuffs

        mov     [esp+32], edi           ; return socketnumber
        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_open: socknum=%u\n", edi

        test    edx, SO_NONBLOCK
        jz      @f
        or      [eax + SOCKET.options], SO_NONBLOCK
        and     edx, not SO_NONBLOCK
  @@:

        mov     [eax + SOCKET.Domain], ecx
        mov     [eax + SOCKET.Type], edx
        mov     [eax + SOCKET.Protocol], esi
        mov     [eax + SOCKET.connect_proc], connect_notsupp

        cmp     ecx, AF_INET4
        jne     .no_inet4

        mov     [eax + IP_SOCKET.ttl], 128

        cmp     edx, SOCK_DGRAM
        je      .udp

        cmp     edx, SOCK_STREAM
        je      .tcp

        cmp     edx, SOCK_RAW
        je      .raw

  .no_inet4:
        cmp     ecx, AF_PPP
        jne     .no_ppp

        cmp     esi, PPP_PROTO_ETHERNET
        je      .pppoe

  .no_ppp:
  .unsupported:
        push    eax
        call    socket_free
        pop     eax
        mov     dword[esp+20], EOPNOTSUPP
        mov     dword[esp+32], -1
        ret

  .nobuffs:
        mov     dword[esp+20], ENOBUFS
        mov     dword[esp+32], -1
        ret

  .raw:
        test    esi, esi        ; IP_PROTO_IP
        jz      .raw_ip

        cmp     esi, IP_PROTO_ICMP
        je      .raw_icmp

        jmp     .unsupported

align 4
  .udp:
        push    eax
        init_queue (eax + SOCKET_QUEUE_LOCATION)        ; Set up data receiving queue
        pop     eax

        mov     [eax + SOCKET.Protocol], IP_PROTO_UDP
        mov     [eax + SOCKET.snd_proc], socket_send_udp
        mov     [eax + SOCKET.rcv_proc], socket_receive_dgram
        mov     [eax + SOCKET.connect_proc], udp_connect
        ret

align 4
  .tcp:
        mov     [eax + SOCKET.Protocol], IP_PROTO_TCP
        mov     [eax + SOCKET.snd_proc], socket_send_tcp
        mov     [eax + SOCKET.rcv_proc], socket_receive_tcp
        mov     [eax + SOCKET.connect_proc], tcp_connect

        tcp_init_socket eax
        ret


align 4
  .raw_ip:
        push    eax
        init_queue (eax + SOCKET_QUEUE_LOCATION)        ; Set up data receiving queue
        pop     eax

        mov     [eax + SOCKET.snd_proc], socket_send_ip
        mov     [eax + SOCKET.rcv_proc], socket_receive_dgram
        mov     [eax + SOCKET.connect_proc], ipv4_connect
        ret


align 4
  .raw_icmp:
        push    eax
        init_queue (eax + SOCKET_QUEUE_LOCATION)        ; Set up data receiving queue
        pop     eax

        mov     [eax + SOCKET.snd_proc], socket_send_icmp
        mov     [eax + SOCKET.rcv_proc], socket_receive_dgram
        mov     [eax + SOCKET.connect_proc], ipv4_connect
        ret

align 4
  .pppoe:
        push    eax
        init_queue (eax + SOCKET_QUEUE_LOCATION)        ; Set up data receiving queue
        pop     eax

        mov     [eax + SOCKET.snd_proc], socket_send_pppoe
        mov     [eax + SOCKET.rcv_proc], socket_receive_dgram
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; socket_bind: Bind to a local port.                              ;
;                                                                 ;
;   IN: ecx = socket number                                       ;
;       edx = pointer to sockaddr struct                          ;
;       esi = length of sockaddr struct                           ;
;                                                                 ;
;  OUT: eax = 0 on success                                        ;
;       eax = -1 on error                                         ;
;       ebx = errorcode on error                                  ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_bind:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_bind: socknum=%u sockaddr=%x length=%u\n", ecx, edx, esi

        call    socket_num_to_ptr
        test    eax, eax
        jz      .invalid

        cmp     esi, 2
        jb      .invalid

        cmp     [eax + UDP_SOCKET.LocalPort], 0 ; Socket can only be bound once
        jnz     .invalid

        cmp     word[edx], AF_INET4
        je      .af_inet4

        cmp     word[edx], AF_LOCAL
        je      .af_local

  .notsupp:
        mov     dword[esp+20], EOPNOTSUPP
        mov     dword[esp+32], -1
        ret

  .invalid:
        mov     dword[esp+20], EINVAL
        mov     dword[esp+32], -1
        ret

  .af_local:
        ; TODO: write code here
        mov     dword[esp+32], 0
        ret

  .af_inet4:
        cmp     esi, 6
        jb      .invalid

        cmp     [eax + SOCKET.Protocol], IP_PROTO_UDP
        je      .udp

        cmp     [eax + SOCKET.Protocol], IP_PROTO_TCP
        je      .tcp

        jmp     .notsupp

  .tcp:
  .udp:
        pushd   [edx + 4]                       ; First, fill in the IP
        popd    [eax + IP_SOCKET.LocalIP]

        mov     bx, [edx + 2]                   ; Did caller specify a local port?
        test    bx, bx
        jnz     .just_check
        call    socket_find_port                ; Nope, find an ephemeral one
        jmp     .done

  .just_check:
        call    socket_check_port               ; Yes, check if it's still available
        jz      .addrinuse                      ; ZF is set by socket_check_port on error

  .done:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_bind: local ip=%u.%u.%u.%u\n",\
        [eax + IP_SOCKET.LocalIP + 0]:1,[eax + IP_SOCKET.LocalIP + 1]:1,\
        [eax + IP_SOCKET.LocalIP + 2]:1,[eax + IP_SOCKET.LocalIP + 3]:1

        mov     dword[esp+32], 0
        ret

  .addrinuse:
        mov     dword[esp+32], -1
        mov     dword[esp+20], EADDRINUSE
        ret




;-----------------------------------------------------------------;
;                                                                 ;
; socket_connect: Connect to the remote host.                     ;
;                                                                 ;
;   IN: ecx = socket number                                       ;
;       edx = pointer to sockaddr struct                          ;
;       esi = length of sockaddr struct                           ;
;                                                                 ;
;  OUT: eax = 0 on success                                        ;
;       eax = -1 on error                                         ;
;       ebx = errorcode on error                                  ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_connect:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_connect: socknum=%u sockaddr=%x length=%u\n", ecx, edx, esi

        call    socket_num_to_ptr
        test    eax, eax
        jz      .invalid

        cmp     esi, 8
        jb      .invalid

        cmp     [eax + SOCKET.state], SS_ISCONNECTING
        je      .already

        test    [eax + SOCKET.options], SO_ACCEPTCON
        jnz     .notsupp

        call    [eax + SOCKET.connect_proc]

        mov     dword[esp+20], ebx
        mov     dword[esp+32], eax
        ret


  .notsupp:
        mov     dword[esp+20], EOPNOTSUPP
        mov     dword[esp+32], -1
        ret

  .invalid:
        mov     dword[esp+20], EINVAL
        mov     dword[esp+32], -1
        ret

  .already:
        mov     dword[esp+20], EALREADY
        mov     dword[esp+32], -1
        ret


connect_notsupp:
        xor     eax, eax
        dec     eax
        mov     ebx, EOPNOTSUPP
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; socket_listen: Listen for incoming connections.                 ;
;                                                                 ;
;   IN: ecx = socket number                                       ;
;       edx = backlog in edx                                      ;
;                                                                 ;
;  OUT: eax = 0 on success                                        ;
;       eax = -1 on error                                         ;
;       ebx = errorcode on error                                  ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_listen:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_listen: socknum=%u backlog=%u\n", ecx, edx

        call    socket_num_to_ptr
        test    eax, eax
        jz      .invalid

        cmp     [eax + SOCKET.Domain], AF_INET4
        jne     .notsupp

        cmp     [eax + SOCKET.Protocol], IP_PROTO_TCP
        jne     .invalid

        cmp     [eax + TCP_SOCKET.LocalPort], 0
        je      .already

        cmp     [eax + IP_SOCKET.LocalIP], 0
        jne     @f
        push    [IPv4_address + 4]           ;;; fixme!!!!
        pop     [eax + IP_SOCKET.LocalIP]
       @@:

        cmp     edx, MAX_backlog
        jbe     @f
        mov     edx, MAX_backlog
       @@:

        mov     [eax + SOCKET.backlog], dx
        or      [eax + SOCKET.options], SO_ACCEPTCON
        mov     [eax + TCP_SOCKET.t_state], TCPS_LISTEN
        mov     [eax + TCP_SOCKET.timer_keepalive], 0           ; disable keepalive timer

        push    eax
        init_queue (eax + SOCKET_QUEUE_LOCATION)                ; Set up sockets queue
        pop     eax

        mov     dword[esp+32], 0
        ret

  .notsupp:
        mov     dword[esp+20], EOPNOTSUPP
        mov     dword[esp+32], -1
        ret

  .invalid:
        mov     dword[esp+20], EINVAL
        mov     dword[esp+32], -1
        ret

  .already:
        mov     dword[esp+20], EALREADY
        mov     dword[esp+32], -1
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; socket_accept: Accept an incoming connection.                   ;
;                                                                 ;
;   IN: ecx = socket number (of listening socket)                 ;
;       edx = ptr to sockaddr struct                              ;
;       esi = length of sockaddr struct                           ;
;                                                                 ;
;  OUT: eax = newly created socket num                            ;
;       eax = -1 on error                                         ;
;       ebx = errorcode on error                                  ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_accept:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_accept: socknum=%u sockaddr=%x length=%u\n", ecx, edx, esi

        call    socket_num_to_ptr
        test    eax, eax
        jz      .invalid

        test    [eax + SOCKET.options], SO_ACCEPTCON
        jz      .invalid

        cmp     [eax + SOCKET.Domain], AF_INET4
        jne     .notsupp

        cmp     [eax + SOCKET.Protocol], IP_PROTO_TCP
        jne     .invalid

  .loop:
        get_from_queue (eax + SOCKET_QUEUE_LOCATION), MAX_backlog, 4, .block

; Ok, we got a socket ptr
        mov     eax, [esi]

; Verify that it is (still) a valid socket
        call    socket_check
        jz      .invalid

; Change sockets thread owner ID to that of the current thread
        mov     ebx, [TASK_BASE]
        mov     ebx, [ebx + TASKDATA.pid]
        mov     [eax + SOCKET.TID], ebx

; Return socket number to caller
        mov     eax, [eax + SOCKET.Number]
        mov     [esp+32], eax
        ret

  .block:
        test    [eax + SOCKET.options], SO_NONBLOCK
        jnz     .wouldblock

        call    socket_block
        jmp     .loop

  .wouldblock:
        mov     dword[esp+20], EWOULDBLOCK
        mov     dword[esp+32], -1
        ret

  .invalid:
        mov     dword[esp+20], EINVAL
        mov     dword[esp+32], -1
        ret

  .notsupp:
        mov     dword[esp+20], EOPNOTSUPP
        mov     dword[esp+32], -1
        ret

;-----------------------------------------------------------------;
;                                                                 ;
; socket_close: Close the socket (and connection).                ;
;                                                                 ;
;   IN: ecx = socket number                                       ;
;                                                                 ;
;  OUT: eax = 0 on success                                        ;
;       eax = -1 on error                                         ;
;       ebx = errorcode on error                                  ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_close:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_close: socknum=%u\n", ecx

        call    socket_num_to_ptr
        test    eax, eax
        jz      .invalid

        mov     dword[esp+32], 0                                ; The socket exists, so we will succeed in closing it.

        or      [eax + SOCKET.options], SO_NONBLOCK             ; Mark the socket as non blocking, we dont want it to block any longer!

        test    [eax + SOCKET.state], SS_BLOCKED                ; Is the socket still in blocked state?
        jz      @f
        call    socket_notify                                   ; Unblock it.
  @@:

        cmp     [eax + SOCKET.Domain], AF_INET4
        jne     .free

        cmp     [eax + SOCKET.Protocol], IP_PROTO_TCP
        je      .tcp

  .free:
        call    socket_free
        ret

  .tcp:
        test    [eax + SOCKET.state], SS_ISCONNECTED
        jz      @f
        test    [eax + SOCKET.state], SS_ISDISCONNECTING
        jnz     @f
        call    tcp_disconnect
  @@:
; TODO:
; ...
;       call    socket_free
        ret


  .invalid:
        mov     dword[esp+20], EINVAL
        mov     dword[esp+32], -1
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; socket_receive: Receive some data from the remote end.          ;
;                                                                 ;
;   IN: ecx = socket number                                       ;
;       edx = addr to application buffer                          ;
;       edx = length of application buffer                        ;
;       edi = flags                                               ;
;                                                                 ;
;  OUT: eax = number of bytes copied                              ;
;       eax = -1 on error                                         ;
;       eax = 0 when socket has been closed by the remote end     ;
;       ebx = errorcode on error                                  ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_receive:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_receive: socknum=%u bufaddr=%x buflength=%u flags=%x\n", ecx, edx, esi, edi

        call    socket_num_to_ptr
        test    eax, eax
        jz      .invalid

  .loop:
        push    edi
        call    [eax + SOCKET.rcv_proc]
        pop     edi

        test    [eax + SOCKET.state], SS_CANTRCVMORE
        jnz     .last_data

        cmp     ebx, EWOULDBLOCK
        jne     .return

        test    edi, MSG_DONTWAIT
        jnz     .return_err

        test    [eax + SOCKET.options], SO_NONBLOCK
        jnz     .return_err

        call    socket_block
        jmp     .loop


  .invalid:
        push    EINVAL
        pop     ebx
  .return_err:
        mov     ecx, -1
  .return:
        mov     [esp+20], ebx
        mov     [esp+32], ecx
        ret

  .last_data:
        test    ecx, ecx
        jz      .return
        call    socket_notify                                   ; Call me again!
        jmp     .return




align 4
socket_receive_dgram:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_receive: DGRAM\n"

        test    edi, MSG_PEEK
        jnz     .peek

        mov     ebx, esi                                        ; buffer length

        get_from_queue (eax + SOCKET_QUEUE_LOCATION), SOCKET_QUEUE_SIZE, sizeof.socket_queue_entry, .wouldblock ; sets esi only on success.
        mov     ecx, [esi + socket_queue_entry.data_size]
        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_receive: %u bytes data\n", ecx

        cmp     ecx, ebx                                        ; If data segment does not fit in applications buffer, abort
        ja      .too_small

        push    eax ecx
        push    [esi + socket_queue_entry.buf_ptr]              ; save the buffer addr so we can clear it later
        mov     esi, [esi + socket_queue_entry.data_ptr]
        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_receive: Source buffer=%x real addr=%x\n", [esp], esi

; copy the data from kernel buffer to application buffer
        mov     edi, edx                                        ; bufferaddr
        shr     ecx, 1
        jnc     .nb
        movsb
  .nb:
        shr     ecx, 1
        jnc     .nw
        movsw
  .nw:
        test    ecx, ecx
        jz      .nd
        rep movsd
  .nd:

        call    net_buff_free
        pop     ecx eax                                         ; return number of bytes copied to application
        cmp     [eax + SOCKET_QUEUE_LOCATION + queue.size], 0
        je      @f
        call    socket_notify                                   ; Queue another network event
  @@:
        xor     ebx, ebx                                        ; errorcode = 0 (no error)
        ret

  .too_small:
        mov     ecx, -1
        push    EMSGSIZE
        pop     ebx
        ret

  .wouldblock:
        push    EWOULDBLOCK
        pop     ebx
        ret

  .peek:
        xor     ebx, ebx
        xor     ecx, ecx
        cmp     [eax + SOCKET_QUEUE_LOCATION + queue.size], 0
        je      @f
        mov     esi, [eax + SOCKET_QUEUE_LOCATION + queue.r_ptr]
        mov     ecx, [esi + socket_queue_entry.data_size]
  @@:
        ret

align 4
socket_receive_tcp:

        call    socket_receive_stream

        test    ecx, ecx
        jz      @f
        push    eax ebx ecx
        call    tcp_output
        pop     ecx ebx eax
  @@:

        ret


align 4
socket_receive_local:

        ; does this socket have a PID yet?
        cmp     [eax + SOCKET.PID], 0
        jne     @f

        ; Change PID to that of current process
        mov     ebx, [TASK_BASE]
        mov     ebx, [ebx + TASKDATA.pid]
        mov     [eax + SOCKET.PID], ebx
        mov     [eax + SOCKET.TID], ebx                         ; currently TID = PID in kolibrios :(
      @@:

        mov     [eax + SOCKET.rcv_proc], socket_receive_stream

; ... continue to SOCKET_receive_stream

align 4
socket_receive_stream:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_receive: STREAM\n"

        cmp     [eax + STREAM_SOCKET.rcv.size], 0
        je      .wouldblock

        test    edi, MSG_PEEK
        jnz     .peek

        mov     ecx, esi
        mov     edi, edx
        xor     edx, edx

        push    eax
        add     eax, STREAM_SOCKET.rcv
        call    socket_ring_read                                ; copy data from kernel buffer to application buffer
        call    socket_ring_free                                ; free read memory
        pop     eax

        cmp     [eax + STREAM_SOCKET.rcv.size], 0
        jne     .more_data
        xor     ebx, ebx                                        ; errorcode = 0 (no error)
        ret

  .more_data:
        call    socket_notify                                   ; Queue another network event
        xor     ebx, ebx                                        ; errorcode = 0 (no error)
        ret

  .wouldblock:
        push    EWOULDBLOCK
        pop     ebx
        xor     ecx, ecx
        ret

  .peek:
        mov     ecx, [eax + STREAM_SOCKET.rcv.size]
        xor     ebx, ebx
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; socket_send: Send some data to the remote end.                  ;
;                                                                 ;
;   IN: ecx = socket number                                       ;
;       edx = pointer to data                                     ;
;       esi = data length                                         ;
;       edi = flags                                               ;
;                                                                 ;
;  OUT: eax = number of bytes sent                                ;
;       eax = -1 on error                                         ;
;       ebx = errorcode on error                                  ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_send:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_send: socknum=%u data ptr=%x length=%u flags=%x\n", ecx, edx, esi, edi

        call    socket_num_to_ptr
        test    eax, eax
        jz      .invalid

        mov     ecx, esi
        mov     esi, edx

        jmp     [eax + SOCKET.snd_proc]

  .invalid:
        mov     dword[esp+20], EINVAL
        mov     dword[esp+32], -1
        ret


align 4
socket_send_udp:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_send: UDP\n"

        mov     [esp+32], ecx
        call    udp_output
        cmp     eax, -1
        je      .error
        ret

  .error:
        mov     dword[esp+32], -1
        mov     dword[esp+20], EMSGSIZE ; FIXME: UDP_output should return error codes!
        ret


align 4
socket_send_tcp:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_send: TCP\n"

        push    eax
        add     eax, STREAM_SOCKET.snd
        call    socket_ring_write
        pop     eax

        mov     [esp+32], ecx
        mov     [eax + SOCKET.errorcode], 0
        push    eax
        call    tcp_output              ; FIXME: this doesnt look pretty, does it?
        pop     eax
        mov     eax, [eax + SOCKET.errorcode]
        mov     [esp+20], eax
        ret


align 4
socket_send_ip:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_send: IPv4\n"

        mov     [esp+32], ecx
        call    ipv4_output_raw
        cmp     eax, -1
        je      .error
        ret

  .error:
        mov     dword[esp+32], eax
        mov     dword[esp+20], ebx
        ret


align 4
socket_send_icmp:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_send: ICMP\n"

        mov     [esp+32], ecx
        call    icmp_output_raw
        cmp     eax, -1
        je      .error
        ret

  .error:
        mov     dword[esp+32], eax
        mov     dword[esp+20], ebx
        ret


align 4
socket_send_pppoe:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_send: PPPoE\n"

        mov     [esp+32], ecx
        mov     ebx, [eax + SOCKET.device]

        call    pppoe_discovery_output  ; FIXME: errorcodes
        cmp     eax, -1
        je      .error
        ret

  .error:
        mov     dword[esp+32], -1
        mov     dword[esp+20], EMSGSIZE
        ret



align 4
socket_send_local:

        ; does this socket have a PID yet?
        cmp     [eax + SOCKET.PID], 0
        jne     @f

        ; Change PID to that of current process
        mov     ebx, [TASK_BASE]
        mov     ebx, [ebx + TASKDATA.pid]
        mov     [eax + SOCKET.PID], ebx
        mov     [eax + SOCKET.TID], ebx         ; currently TID = PID in kolibrios :(
      @@:
        mov     [eax + SOCKET.snd_proc], socket_send_local_initialized

align 4
socket_send_local_initialized:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_send: LOCAL\n"

        ; get the other side's socket and check if it still exists
        mov     eax, [eax + SOCKET.device]
        call    socket_check
        jz      .invalid

        ; allright, shove in the data!
        push    eax
        add     eax, STREAM_SOCKET.rcv
        call    socket_ring_write
        pop     eax

        ; return the number of written bytes (or errorcode) to application
        mov     [esp+32], ecx

        ; and notify the other end
        call    socket_notify

        ret

  .invalid:
        mov     dword[esp+32], -1
        mov     dword[esp+20], EINVAL
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; socket_get_opt: Read a socket option                            ;
;                                                                 ;
;   IN: ecx = socket number                                       ;
;       edx = pointer to socket options struct                    ;
;                                                                 ;
;  OUT: eax = 0 on success                                        ;
;       eax = -1 on error                                         ;
;       ebx = errorcode on error                                  ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_get_opt:

; FIXME:
; At moment, uses only pseudo-optname -2 for get last_ack_number for TCP.
; TODO: find best way to notify that send()'ed data were acknowledged
; Also pseudo-optname -3 is valid and returns socket state, one of TCPS_*.

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_get_opt\n"

        call    socket_num_to_ptr
        test    eax, eax
        jz      .invalid

        cmp     dword [edx], IP_PROTO_TCP
        jne     .invalid
        cmp     dword [edx+4], -2
        je      @f
        cmp     dword [edx+4], -3
        jne     .invalid
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
        mov     eax, [edx+8]
        test    eax, eax
        jz      @f
        mov     [eax], ecx
@@:
        mov     dword [esp+32], 0
        ret

  .invalid:
        mov     dword[esp+32], -1
        mov     dword[esp+20], EINVAL
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; socket_set_options: Set a socket option.                        ;
;                                                                 ;
;   IN: ecx = socket number                                       ;
;       edx = pointer to socket options struct                    ;
;                                                                 ;
;  OUT: eax = 0 on success                                        ;
;       eax = -1 on error                                         ;
;       ebx = errorcode on error                                  ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_set_opt:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_set_opt\n"

        call    socket_num_to_ptr
        test    eax, eax
        jz      .invalid

        cmp     [edx + socket_options.level], IP_PROTO_IP
        je      .ip
        cmp     [edx + socket_options.level], SOL_SOCKET
        jne     .invalid

  .socket:
        cmp     [edx + socket_options.optname], SO_BINDTODEVICE
        jne     .invalid

  .bind:
        cmp     [edx + socket_options.optlen], 0
        je      .unbind

        movzx   edx, byte[edx + socket_options.optval]
        cmp     edx, NET_DEVICES_MAX
        ja      .invalid

        mov     edx, [net_device_list + 4*edx]
        test    edx, edx
        jz      .already
        mov     [eax + SOCKET.device], edx

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_set_opt: Bound socket %x to device %x\n", eax, edx

        mov     dword[esp+32], 0        ; success!
        ret

  .unbind:
        mov     [eax + SOCKET.device], 0

        mov     dword[esp+32], 0        ; success!
        ret

  .ip:
        cmp     [edx + socket_options.optname], IP_TTL
        jne     .invalid

  .ttl:
        mov     bl, byte[edx + socket_options.optval]
        mov     [eax + IP_SOCKET.ttl], bl

        mov     dword[esp+32], 0        ; success!
        ret

  .already:
        mov     dword[esp+20], EALREADY
        mov     dword[esp+32], -1
        ret

  .invalid:
        mov     dword[esp+20], EINVAL
        mov     dword[esp+32], -1
        ret




;-----------------------------------------------------------------;
;                                                                 ;
; socket_pair: Allocate a pair of linked local sockets.           ;
;                                                                 ;
;  IN: /                                                          ;
;                                                                 ;
; OUT: eax = socket1 num on success                               ;
;      eax = -1 on error                                          ;
;      ebx = socket2 num on success                               ;
;      ebx = errorcode on error                                   ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_pair:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_pair\n"

        call    socket_alloc
        test    eax, eax
        jz      .nomem1
        mov     [esp+32], edi   ; application's eax

        mov     [eax + SOCKET.Domain], AF_LOCAL
        mov     [eax + SOCKET.Type], SOCK_STREAM
        mov     [eax + SOCKET.Protocol], 0              ;;; CHECKME
        mov     [eax + SOCKET.snd_proc], socket_send_local
        mov     [eax + SOCKET.rcv_proc], socket_receive_local
        mov     [eax + SOCKET.PID], 0
        mov     ebx, eax

        call    socket_alloc
        test    eax, eax
        jz      .nomem2
        mov     [esp+20], edi   ; application's ebx

        mov     [eax + SOCKET.Domain], AF_LOCAL
        mov     [eax + SOCKET.Type], SOCK_STREAM
        mov     [eax + SOCKET.Protocol], 0              ;;; CHECKME
        mov     [eax + SOCKET.snd_proc], socket_send_local
        mov     [eax + SOCKET.rcv_proc], socket_receive_local
        mov     [eax + SOCKET.PID], 0

        ; Link the two sockets to eachother
        mov     [eax + SOCKET.device], ebx
        mov     [ebx + SOCKET.device], eax

        lea     eax, [eax + STREAM_SOCKET.rcv]
        call    socket_ring_create
        test    eax, eax
        jz      .nomem2

        lea     eax, [ebx + STREAM_SOCKET.rcv]
        call    socket_ring_create
        test    eax, eax
        jz      .nomem2

        ret

  .nomem2:
        mov     eax, [esp+20]
        call    socket_free

  .nomem1:
        mov     eax, [esp+32]
        call    socket_free

        mov     dword[esp+32], -1
        mov     dword[esp+20], ENOMEM
        ret



;-----------------------------------------------------------------;
;                                                                 ;
; socket_debug: Copy socket variables to application buffer.      ;
;                                                                 ;
;   IN: ecx = socket number                                       ;
;       edx = pointer to application buffer                       ;
;                                                                 ;
;  OUT: eax = 0 on success                                        ;
;       eax = -1 on error                                         ;
;       ebx = errorcode on error                                  ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_debug:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_debug\n"

        mov     edi, edx

        test    ecx, ecx
        jz      .returnall

        call    socket_num_to_ptr
        test    eax, eax
        jz      .invalid

        mov     esi, eax
        mov     ecx, SOCKET_STRUCT_SIZE/4
        rep movsd

        mov     dword[esp+32], 0
        ret

  .returnall:
        mov     ebx, net_sockets
  .next_socket:
        mov     ebx, [ebx + SOCKET.NextPtr]
        test    ebx, ebx
        jz      .done
        mov     eax, [ebx + SOCKET.Number]
        stosd
        jmp     .next_socket
  .done:
        xor     eax, eax
        stosd
        mov     dword[esp+32], eax
        ret

  .invalid:
        mov     dword[esp+32], -1
        mov     dword[esp+20], EINVAL
        ret


;-----------------------------------------------------------------;
;   ____                                                 ____     ;
;   \  /              End of sockets API                 \  /     ;
;    \/                                                   \/      ;
;    ()        Internally used functions follow           ()      ;
;                                                                 ;
;-----------------------------------------------------------------;


;-----------------------------------------------------------------;
;                                                                 ;
; socket_find_port:                                               ;
; Fill in the local port number for TCP and UDP sockets           ;
; This procedure always works because the number of sockets is    ;
; limited to a smaller number then the number of possible ports   ;
;                                                                 ;
;  IN:  eax = socket pointer                                      ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_find_port:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_find_port\n"

        push    ebx esi ecx

        cmp     [eax + SOCKET.Protocol], IP_PROTO_UDP
        je      .udp

        cmp     [eax + SOCKET.Protocol], IP_PROTO_TCP
        je      .tcp

        pop     ecx esi ebx
        ret

  .udp:
        mov     bx, [last_UDP_port]
        call    .findit
        mov     [last_UDP_port], bx

        pop     ecx esi ebx
        ret

  .tcp:
        mov     bx, [last_TCP_port]
        call    .findit
        mov     [last_TCP_port], bx

        pop     ecx esi ebx
        ret


  .restart:
        mov     bx, MIN_EPHEMERAL_PORT_N
  .findit:
        cmp     bx, MAX_EPHEMERAL_PORT_N
        je      .restart

        add     bh, 1
        adc     bl, 0

        call    socket_check_port
        jz      .findit
        ret



;-----------------------------------------------------------------;
;                                                                 ;
; socket_check_port: (to be used with AF_INET only!)              ;
; Checks if a local port number is unused                         ;
; If the proposed port number is unused, it is filled in in the   ;
; socket structure.                                               ;
;                                                                 ;
;   IN: eax = socket ptr                                          ;
;       bx = proposed socket number (network byte order)          ;
;                                                                 ;
;  OUT: ZF = set on error                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_check_port:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_check_port: "

        pusha
        mov     ecx, socket_mutex
        call    mutex_lock
        popa

        mov     ecx, [eax + SOCKET.Protocol]
        mov     edx, [eax + IP_SOCKET.LocalIP]
        mov     esi, net_sockets

  .next_socket:
        mov     esi, [esi + SOCKET.NextPtr]
        or      esi, esi
        jz      .port_ok

        cmp     [esi + SOCKET.Protocol], ecx
        jne     .next_socket

        cmp     [esi + IP_SOCKET.LocalIP], edx
        jne     .next_socket

        cmp     [esi + UDP_SOCKET.LocalPort], bx
        jne     .next_socket

        pusha
        mov     ecx, socket_mutex
        call    mutex_unlock
        popa

        DEBUGF  DEBUG_NETWORK_VERBOSE, "local port %x already in use\n", bx  ; FIXME: find a way to print big endian values with debugf
        ret

  .port_ok:
        pusha
        mov     ecx, socket_mutex
        call    mutex_unlock
        popa

        DEBUGF  DEBUG_NETWORK_VERBOSE, "local port %x is free\n", bx         ; FIXME: find a way to print big endian values with debugf
        mov     [eax + UDP_SOCKET.LocalPort], bx
        or      bx, bx                                  ; clear the zero-flag
        ret



;-----------------------------------------------------------------;
;                                                                 ;
; socket_input: Update a (stateless) socket with received data.   ;
;                                                                 ;
; Note: The socket's mutex should already be set !                ;
;                                                                 ;
;   IN: eax = socket ptr                                          ;
;       ecx = data size                                           ;
;       esi = ptr to data                                         ;
;       [esp] = ptr to buf                                        ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_input:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_input: socket=%x, data=%x size=%u\n", eax, esi, ecx

        push    ecx
        push    esi
        mov     esi, esp

        add_to_queue (eax + SOCKET_QUEUE_LOCATION), SOCKET_QUEUE_SIZE, sizeof.socket_queue_entry, .full

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_input: success\n"
        add     esp, sizeof.socket_queue_entry

        pusha
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_unlock
        popa

        jmp     socket_notify

  .full:
        DEBUGF  DEBUG_NETWORK_ERROR, "SOCKET_input: socket %x is full!\n", eax

        pusha
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_unlock
        popa

        add     esp, 8
        call    net_buff_free
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; socket_ring_create: Create a ringbuffer for sockets.            ;
;                                                                 ;
;   IN: eax = ptr to ring struct                                  ;
;                                                                 ;
;  OUT: eax = 0 on error                                          ;
;       eax = start ptr                                           ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_ring_create:

        push    esi
        mov     esi, eax

        push    edx
        stdcall create_ring_buffer, SOCKET_BUFFER_SIZE, PG_SWR
        pop     edx
        test    eax, eax
        jz      .fail

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_ring_create: %x\n", eax

        pusha
        lea     ecx, [esi + RING_BUFFER.mutex]
        call    mutex_init
        popa

        mov     [esi + RING_BUFFER.start_ptr], eax
        mov     [esi + RING_BUFFER.write_ptr], eax
        mov     [esi + RING_BUFFER.read_ptr], eax
        mov     [esi + RING_BUFFER.size], 0
        add     eax, SOCKET_BUFFER_SIZE
        mov     [esi + RING_BUFFER.end_ptr], eax
        mov     eax, esi

        pop     esi
        ret

  .fail:
        DEBUGF  DEBUG_NETWORK_ERROR, "SOCKET_ring_create: Out of memory!\n"
        pop     esi
        ret

;-----------------------------------------------------------------;
;                                                                 ;
; socket_ring_write: Write data to ring buffer.                   ;
;                                                                 ;
;   IN: eax = ptr to ring struct                                  ;
;       ecx = data size                                           ;
;       esi = ptr to data                                         ;
;                                                                 ;
;  OUT: ecx = number of bytes stored                              ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_ring_write:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_ring_write: ringbuff=%x ptr=%x size=%u\n", eax, esi, ecx

; lock mutex
        pusha
        lea     ecx, [eax + RING_BUFFER.mutex]
        call    mutex_lock                                      ; TODO: check what registers this function actually destroys
        popa

; calculate available size
        mov     edi, SOCKET_BUFFER_SIZE
        sub     edi, [eax + RING_BUFFER.size]                   ; available buffer size in edi
        cmp     ecx, edi
        jbe     .copy
        mov     ecx, edi
  .copy:
        mov     edi, [eax + RING_BUFFER.write_ptr]
        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_ring_write: %u bytes from %x to %x\n", ecx, esi, edi

; update write ptr
        push    edi
        add     edi, ecx
        cmp     edi, [eax + RING_BUFFER.end_ptr]
        jb      @f
        sub     edi, SOCKET_BUFFER_SIZE                         ; WRAP
  @@:
        mov     [eax + RING_BUFFER.write_ptr], edi
        pop     edi

; update size
        add     [eax + RING_BUFFER.size], ecx

; copy the data
        push    ecx
        shr     ecx, 1
        jnc     .nb
        movsb
  .nb:
        shr     ecx, 1
        jnc     .nw
        movsw
  .nw:
        test    ecx, ecx
        jz      .nd
        rep movsd
  .nd:
        pop     ecx

; unlock mutex
        pusha
        lea     ecx, [eax + RING_BUFFER.mutex]
        call    mutex_unlock                                    ; TODO: check what registers this function actually destroys
        popa

        ret

;-----------------------------------------------------------------;
;                                                                 ;
; socket_ring_read: Read from ring buffer                         ;
;                                                                 ;
;   IN: eax = ring struct ptr                                     ;
;       ecx = bytes to read                                       ;
;       edx = offset                                              ;
;       edi = ptr to buffer start                                 ;
;                                                                 ;
;  OUT: eax = unchanged                                           ;
;       ecx = number of bytes read (0 on error)                   ;
;       edx = destroyed                                           ;
;       esi = destroyed                                           ;
;       edi = ptr to buffer end                                   ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_ring_read:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_ring_read: ringbuff=%x ptr=%x size=%u offset=%x\n", eax, edi, ecx, edx

        pusha
        lea     ecx, [eax + RING_BUFFER.mutex]
        call    mutex_lock                                      ; TODO: check what registers this function actually destroys
        popa

        mov     esi, [eax + RING_BUFFER.read_ptr]
        add     esi, edx                                        ; esi = start_ptr + offset

        neg     edx
        add     edx, [eax + RING_BUFFER.size]                   ; edx = snd.size - offset
        jle     .no_data_at_all

        pusha
        lea     ecx, [eax + RING_BUFFER.mutex]
        call    mutex_unlock                                    ; TODO: check what registers this function actually destroys
        popa

        cmp     ecx, edx
        ja      .less_data

  .copy:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_ring_read: %u bytes from %x to %x\n", ecx, esi, edi
        push    ecx
        shr     ecx, 1
        jnc     .nb
        movsb
  .nb:
        shr     ecx, 1
        jnc     .nw
        movsw
  .nw:
        test    ecx, ecx
        jz      .nd
        rep movsd
  .nd:
        pop     ecx
        ret

  .no_data_at_all:
        pusha
        lea     ecx, [eax + RING_BUFFER.mutex]
        call    mutex_unlock                                    ; TODO: check what registers this function actually destroys
        popa

        DEBUGF  DEBUG_NETWORK_ERROR, "SOCKET_ring_read: no data at all!\n"
        xor     ecx, ecx
        ret

  .less_data:
        mov     ecx, edx
        jmp     .copy


;-----------------------------------------------------------------;
;                                                                 ;
; socket_ring_free: Free data from a ringbuffer.                  ;
;                                                                 ;
;   IN: eax = ptr to ring struct                                  ;
;       ecx = data size                                           ;
;                                                                 ;
;  OUT: ecx = number of freed bytes                               ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_ring_free:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_ring_free: %u bytes from ring %x\n", ecx, eax

        push    eax ecx
        lea     ecx, [eax + RING_BUFFER.mutex]
        call    mutex_lock                                      ; TODO: check what registers this function actually destroys
        pop     ecx eax

        sub     [eax + RING_BUFFER.size], ecx
        jb      .error
        add     [eax + RING_BUFFER.read_ptr], ecx

        mov     edx, [eax + RING_BUFFER.end_ptr]
        cmp     [eax + RING_BUFFER.read_ptr], edx
        jb      @f
        sub     [eax + RING_BUFFER.read_ptr], SOCKET_BUFFER_SIZE
       @@:

        push    eax ecx
        lea     ecx, [eax + RING_BUFFER.mutex]                  ; TODO: check what registers this function actually destroys
        call    mutex_unlock
        pop     ecx eax

        ret

  .error:       ; we could free all available bytes, but that would be stupid, i guess..
        DEBUGF  DEBUG_NETWORK_ERROR, "SOCKET_ring_free: buffer=%x error!\n", eax
        add     [eax + RING_BUFFER.size], ecx

        push    eax
        lea     ecx, [eax + RING_BUFFER.mutex]
        call    mutex_unlock                                    ; TODO: check what registers this function actually destroys
        pop     eax

        xor     ecx, ecx
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; socket_block: Suspend the thread attached to a socket.          ;
;                                                                 ;
;   IN: eax = socket ptr                                          ;
;                                                                 ;
;  OUT: eax = unchanged                                           ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_block:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_block: %x\n", eax

        push    eax

        pushf
        cli
        ; Set the 'socket is blocked' flag
        or      [eax + SOCKET.state], SS_BLOCKED

        ; Suspend the thread
        push    edx
        mov     edx, [TASK_BASE]
        mov     [edx + TASKDATA.state], TSTATE_RUN_SUSPENDED

        ; Remember the thread ID so we can wake it up again
        mov     edx, [edx + TASKDATA.pid]
        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_block: suspending thread: %u\n", edx
        mov     [eax + SOCKET.TID], edx
        pop     edx
        popf

        call    change_task
        pop     eax

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_block: continuing\n"

        ret


;-----------------------------------------------------------------;
;                                                                 ;
; socket_notify: Wake up socket owner thread.                     ;
;                                                                 ;
;   IN: eax = socket ptr                                          ;
;                                                                 ;
;  OUT: eax = unchanged                                           ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_notify:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_notify: %x\n", eax

        call    socket_check
        jz      .error

; Find the associated thread's TASK_DATA
        push    ebx ecx esi
        mov     ebx, [eax + SOCKET.TID]
        test    ebx, ebx
        jz      .error2
        xor     ecx, ecx
        inc     ecx
        mov     esi, TASK_DATA
  .next:
        cmp     [esi + TASKDATA.pid], ebx
        je      .found
        inc     ecx
        add     esi, sizeof.TASKDATA
        cmp     ecx, [thread_count]
        jbe     .next

  .error2:
; PID not found, TODO: close socket!
        DEBUGF  DEBUG_NETWORK_ERROR, "SOCKET_notify: error finding thread 0x%x !\n", ebx
        pop     esi ecx ebx
        ret

  .error:
        DEBUGF  DEBUG_NETWORK_ERROR, "SOCKET_notify: invalid socket ptr: 0x%x !\n", eax
        ret

  .found:
        test    [eax + SOCKET.state], SS_BLOCKED
        jnz     .un_block

; Socket and thread exists and socket is of non blocking type.
; We'll try to flag an event to the thread.
        shl     ecx, 8
        or      [SLOT_BASE + ecx + APPDATA.event_mask], EVENT_NETWORK

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_notify: poking thread %u!\n", ebx
        pop     esi ecx ebx
        ret


  .un_block:
; Socket and thread exists and socket is of blocking type
; We'll try to unblock it.
        and     [eax + SOCKET.state], not SS_BLOCKED    ; Clear the 'socket is blocked' flag
        mov     [esi + TASKDATA.state], TSTATE_RUNNING  ; Run the thread

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_notify: Unblocked socket!\n"
        pop     esi ecx ebx
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; socket_alloc: Allocate memory for socket and put new socket     ;
; into the list. Newly created socket is initialized with calling ;
; PID and given a socket number.                                  ;
;                                                                 ;
;  IN:  /                                                         ;
;                                                                 ;
; OUT:  eax = socket ptr on success                               ;
;       eax = 0 on error                                          ;
;       edi = socket number on success                            ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_alloc:

        push    ebx

        stdcall kernel_alloc, SOCKET_STRUCT_SIZE
        or      eax, eax
        jz      .nomem
        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_alloc: ptr=%x\n", eax

; zero-initialize allocated memory
        push    eax
        mov     edi, eax
        mov     ecx, SOCKET_STRUCT_SIZE / 4
        xor     eax, eax
        rep stosd
        pop     eax

; set send-and receive procedures to return -1
        mov     [eax + SOCKET.snd_proc], .not_yet
        mov     [eax + SOCKET.rcv_proc], .not_yet

        pusha
        mov     ecx, socket_mutex
        call    mutex_lock
        popa

; find first free socket number and use it
        mov     edi, [last_socket_num]
  .next_socket_number:
        inc     edi
        jz      .next_socket_number     ; avoid socket nr 0
        cmp     edi, -1
        je      .next_socket_number     ; avoid socket nr -1
        mov     ebx, net_sockets
  .next_socket:
        mov     ebx, [ebx + SOCKET.NextPtr]
        test    ebx, ebx
        jz      .last_socket

        cmp     [ebx + SOCKET.Number], edi
        jne     .next_socket
        jmp     .next_socket_number

  .last_socket:
        mov     [last_socket_num], edi
        mov     [eax + SOCKET.Number], edi
        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_alloc: number=%u\n", edi

; Fill in PID
        mov     ebx, [TASK_BASE]
        mov     ebx, [ebx + TASKDATA.pid]
        mov     [eax + SOCKET.PID], ebx
        mov     [eax + SOCKET.TID], ebx         ; currently TID = PID in kolibrios :(

; init mutex
        pusha
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_init
        popa

; add socket to the list by re-arranging some pointers
        mov     ebx, [net_sockets + SOCKET.NextPtr]

        mov     [eax + SOCKET.PrevPtr], net_sockets
        mov     [eax + SOCKET.NextPtr], ebx

        test    ebx, ebx
        jz      @f

        pusha
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_lock
        popa

        mov     [ebx + SOCKET.PrevPtr], eax

        pusha
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_unlock
        popa
       @@:

        mov     [net_sockets + SOCKET.NextPtr], eax

        pusha
        mov     ecx, socket_mutex
        call    mutex_unlock
        popa
        pop     ebx

        ret

  .nomem:
        DEBUGF  DEBUG_NETWORK_ERROR, "SOCKET_alloc: Out of memory!\n"
        pop     ebx
        ret

  .not_yet:
        mov     dword[esp+20], ENOTCONN
        mov     dword[esp+32], -1
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; socket_free: Free socket data memory and remove socket from     ;
; the list. Caller should lock and unlock socket_mutex.           ;
;                                                                 ;
;  IN:  eax = socket ptr                                          ;
;                                                                 ;
; OUT:  /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_free:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_free: %x\n", eax

        call    socket_check
        jz      .error

        push    ebx

        pusha
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_lock
        popa

        cmp     [eax + SOCKET.Type], SOCK_STREAM
        jne     .no_stream

        mov     ebx, eax
        cmp     [eax + STREAM_SOCKET.rcv.start_ptr], 0
        je      @f
        stdcall free_kernel_space, [eax + STREAM_SOCKET.rcv.start_ptr]
  @@:
        cmp     [ebx + STREAM_SOCKET.snd.start_ptr], 0
        je      @f
        stdcall free_kernel_space, [ebx + STREAM_SOCKET.snd.start_ptr]
  @@:
        mov     eax, ebx
  .no_stream:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_free: freeing socket %x\n", eax
        push    eax                             ; this will be passed to kernel_free
        mov     ebx, [eax + SOCKET.NextPtr]
        mov     eax, [eax + SOCKET.PrevPtr]

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_free: linking socket %x to socket %x\n", eax, ebx

        test    eax, eax
        jz      @f
        mov     [eax + SOCKET.NextPtr], ebx
       @@:

        test    ebx, ebx
        jz      @f
        mov     [ebx + SOCKET.PrevPtr], eax
       @@:

        call    kernel_free
        pop     ebx

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_free: success!\n"

  .error:
        ret

  .error1:
        pop     ebx
        DEBUGF  DEBUG_NETWORK_ERROR, "SOCKET_free: error!\n"
        DEBUGF  DEBUG_NETWORK_ERROR, "socket ptr=0x%x caller=0x%x\n", eax, [esp]
        ret

;-----------------------------------------------------------------;
;                                                                 ;
; socket_fork: Create a child socket.                             ;
;                                                                 ;
;  IN:  ebx = socket number                                       ;
;                                                                 ;
; OUT:  eax = child socket number on success                      ;
;       eax = 0 on error                                          ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_fork:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_fork: %x\n", ebx

; Exit if backlog queue is full
        mov     eax, [ebx + SOCKET_QUEUE_LOCATION + queue.size]
        cmp     ax, [ebx + SOCKET.backlog]
        jae     .fail

; Allocate new socket
        push    ebx
        call    socket_alloc
        pop     ebx
        test    eax, eax
        jz      .fail

        push    eax
        mov     esi, esp
        add_to_queue (ebx + SOCKET_QUEUE_LOCATION), MAX_backlog, 4, .fail2
        pop     eax

; Copy structure from current socket to new
; We start at PID to preserve the socket num, 2 pointers and mutex
; TID will be filled in later
        lea     esi, [ebx + SOCKET.PID]
        lea     edi, [eax + SOCKET.PID]
        mov     ecx, (SOCKET_QUEUE_LOCATION - SOCKET.PID + 3)/4
        rep movsd

        and     [eax + SOCKET.options], not SO_ACCEPTCON

; Notify owner of parent socket
        push    eax
        mov     eax, ebx
        call    socket_notify
        pop     eax

        ret

  .fail2:
        add     esp, 4+4+4
  .fail:
        DEBUGF  DEBUG_NETWORK_ERROR, "SOCKET_fork: failed\n"
        xor     eax, eax
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; socket_num_to_ptr: Get socket structure address by its number.  ;
;                                                                 ;
;  IN:  ecx = socket number                                       ;
;                                                                 ;
; OUT:  eax = socket ptr                                          ;
;       eax = 0 on error                                          ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_num_to_ptr:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_num_to_ptr: num=%u ", ecx

        pusha
        mov     ecx, socket_mutex
        call    mutex_lock
        popa

        mov     eax, net_sockets
  .next_socket:
        mov     eax, [eax + SOCKET.NextPtr]
        test    eax, eax
        jz      .error
        cmp     [eax + SOCKET.Number], ecx
        jne     .next_socket

        pusha
        mov     ecx, socket_mutex
        call    mutex_unlock
        popa

        DEBUGF  DEBUG_NETWORK_VERBOSE, "ptr=%x\n", eax
        ret

  .error:
        pusha
        mov     ecx, socket_mutex
        call    mutex_unlock
        popa

        DEBUGF  DEBUG_NETWORK_ERROR, "SOCKET_num_to_ptr: socket %u not found!\n", eax
        DEBUGF  DEBUG_NETWORK_ERROR, "SOCKET_num_to_ptr: caller = 0x%x\n", [esp]
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; socket_ptr_to_num: Get socket number by its address.            ;
;                                                                 ;
;  IN:  eax = socket ptr                                          ;
;                                                                 ;
; OUT:  eax = socket number                                       ;
;       eax = 0 on error                                          ;
;       ZF = set on error                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_ptr_to_num:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_ptr_to_num: ptr=%x ", eax

        call    socket_check
        jz      .error

        mov     eax, [eax + SOCKET.Number]

        DEBUGF  DEBUG_NETWORK_VERBOSE, "num=%u\n", eax
        ret

  .error:
        DEBUGF  DEBUG_NETWORK_ERROR, "SOCKET_ptr_to_num: not found\n", eax
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; socket_check: Checks if the given ptr is really a socket ptr.   ;
;                                                                 ;
;  IN:  eax = socket ptr                                          ;
;                                                                 ;
; OUT:  eax = 0 on error                                          ;
;       ZF = set on error                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_check:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_check: %x\n", eax

        test    eax, eax
        jz      .error
        push    ebx
        mov     ebx, net_sockets

  .next_socket:
        mov     ebx, [ebx + SOCKET.NextPtr]
        or      ebx, ebx
        jz      .done
        cmp     ebx, eax
        jnz     .next_socket

  .done:
        mov     eax, ebx
        test    eax, eax
        pop     ebx
        ret

  .error:
        DEBUGF  DEBUG_NETWORK_ERROR, "SOCKET_check: called with argument 0\n"
        DEBUGF  DEBUG_NETWORK_ERROR, "stack: 0x%x, 0x%x, 0x%x\n", [esp], [esp+4], [esp+8]
        ret



;-----------------------------------------------------------------;
;                                                                 ;
; socket_check_owner: Check if the caller app owns the socket.    ;
;                                                                 ;
;  IN:  eax = socket ptr                                          ;
;                                                                 ;
; OUT:  ZF = true/false                                           ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_check_owner:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_check_owner: %x\n", eax

        push    ebx
        mov     ebx, [TASK_BASE]
        mov     ebx, [ebx + TASKDATA.pid]
        cmp     [eax + SOCKET.PID], ebx
        pop     ebx

        ret




;-----------------------------------------------------------------;
;                                                                 ;
; socket_process_end: Kernel calls this function when a certain   ;
; process ends. This function will check if the process had any   ;
; open sockets and update them accordingly (clean up).            ;
;                                                                 ;
;  IN:  edx = pid                                                 ;
;                                                                 ;
; OUT:  /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_process_end:

        ret     ; FIXME

        cmp     [net_sockets + SOCKET.NextPtr], 0       ; Are there any active sockets at all?
        je      .quickret                               ; nope, exit immediately

; TODO: run the following code in another thread, to avoid deadlock

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_process_end: %x\n", edx

        pusha
        mov     ecx, socket_mutex
        call    mutex_lock
        popa

        push    ebx
        mov     ebx, net_sockets

  .next_socket:
        mov     ebx, [ebx + SOCKET.NextPtr]
  .next_socket_test:
        test    ebx, ebx
        jz      .done

        cmp     [ebx + SOCKET.PID], edx
        jne     .next_socket

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_process_end: killing socket %x\n", ebx

        mov     [ebx + SOCKET.PID], 0
        mov     eax, ebx
        mov     ebx, [ebx + SOCKET.NextPtr]

        pusha
        cmp     [eax + SOCKET.Domain], AF_INET4
        jne     .free

        cmp     [eax + SOCKET.Protocol], IP_PROTO_TCP
        jne     .free

        call    tcp_disconnect
        jmp     .closed

  .free:
        call    socket_free

  .closed:
        popa
        jmp     .next_socket_test

  .done:
        pop     ebx

        pusha
        mov     ecx, socket_mutex
        call    mutex_unlock
        popa

  .quickret:
        ret




;-----------------------------------------------------------------;
;                                                                 ;
; socket_is_connecting: Update socket state.                      ;
;                                                                 ;
;  IN:  eax = socket ptr                                          ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_is_connecting:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_is_connecting: %x\n", eax

        and     [eax + SOCKET.state], not (SS_ISCONNECTED + SS_ISDISCONNECTING + SS_ISCONFIRMING)
        or      [eax + SOCKET.state], SS_ISCONNECTING
        ret



;-----------------------------------------------------------------;
;                                                                 ;
; socket_is_connected: Update socket state.                       ;
;                                                                 ;
;  IN:  eax = socket ptr                                          ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_is_connected:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_is_connected: %x\n", eax

        and     [eax + SOCKET.state], not (SS_ISCONNECTING + SS_ISDISCONNECTING + SS_ISCONFIRMING)
        or      [eax + SOCKET.state], SS_ISCONNECTED
        jmp     socket_notify




;-----------------------------------------------------------------;
;                                                                 ;
; socket_is_disconnecting: Update socket state.                   ;
;                                                                 ;
;  IN:  eax = socket ptr                                          ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_is_disconnecting:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_is_disconnecting: %x\n", eax

        and     [eax + SOCKET.state], not (SS_ISCONNECTING)
        or      [eax + SOCKET.state], SS_ISDISCONNECTING + SS_CANTRCVMORE + SS_CANTSENDMORE
        jmp     socket_notify



;-----------------------------------------------------------------;
;                                                                 ;
; socket_is_disconnected: Update socket state.                    ;
;                                                                 ;
;  IN:  eax = socket ptr                                          ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_is_disconnected:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_is_disconnected: %x\n", eax

        and     [eax + SOCKET.state], not (SS_ISCONNECTING + SS_ISCONNECTED + SS_ISDISCONNECTING)
        or      [eax + SOCKET.state], SS_CANTRCVMORE + SS_CANTSENDMORE
        jmp     socket_notify



;-----------------------------------------------------------------;
;                                                                 ;
; socket_cant_recv_more: Update socket state.                     ;
;                                                                 ;
;  IN:  eax = socket ptr                                          ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_cant_recv_more:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_cant_recv_more: %x\n", eax

        or      [eax + SOCKET.state], SS_CANTRCVMORE
        jmp     socket_notify



;-----------------------------------------------------------------;
;                                                                 ;
; socket_cant_send_more: Update socket state.                     ;
;                                                                 ;
;  IN:  eax = socket ptr                                          ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
socket_cant_send_more:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "SOCKET_cant_send_more: %x\n", eax

        or      [eax + SOCKET.state], SS_CANTSENDMORE
        mov     [eax + SOCKET.snd_proc], .notconn
        jmp     socket_notify

  .notconn:
        mov     dword[esp+20], ENOTCONN
        mov     dword[esp+32], -1
        ret
